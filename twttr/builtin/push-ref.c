/*
 * Push all objects representing a specific ref to the upc/pants local LMDB
 * store, as well as a remote store, if configured.
 */

#include "cache.h"
#include "commit-graph.h"
#include "config.h"
#include "hash.h"
#include "hashmap.h"
#include "merge-recursive.h"
#include "object.h"
#include "object-store.h"
#include "packfile.h"
#include "parse-options.h"
#include "pathspec.h"
#include "refs.h"
#include "repository.h"
#include "revision.h"
#include "run-command.h"
#include "sigchain.h"
#include "strbuf.h"
#include "string-list.h"
#include "tree.h"

#include "upc.h"

#include "remexec-backend.h"

#define HACKY_LOG(msg) fprintf(stderr, "%s\n", msg)
#define HACKY_LOG_ARG(fmt, arg) fprintf(stderr, fmt, arg)

static const char *const builtin_push_ref_usage[] = {
        N_("git push-ref [<revisions>...]"), NULL
};

static struct option builtin_push_ref_options[] = {
        OPT_END(),
};

struct push_params {
        const char **ref_argv;
        size_t ref_argc;
        struct remexec_ref_store *ref_destination;
};

static int parse_push_ref_options(int argc, const char **argv,
                                  const char *prefix,
                                  struct push_params *params)
{
        prefix = setup_git_directory();
        argc = parse_options(argc, argv, prefix, builtin_push_ref_options,
                             builtin_push_ref_usage, 0);
        if (argc == 0) {
                error("No refs were provided.");
                return 1;
        }

        struct remexec_ref_store *ref_destination = remexec_ref_store_create();
        struct push_params ret = {
                argv,
                argc,
                ref_destination,
        };
        *params = ret;

        return 0;
}

struct tree_traversal_options {
        TreeTraversalFFIContext ctx;
};

static char *strbuf_dup(const struct strbuf *s)
{
        char *ret = malloc(s->len + 1);
        ret[s->len] = '\0';
        memcpy(ret, s->buf, s->len);
        return ret;
}

static int tree_reader(const struct object_id *oid, struct strbuf *base,
                       const char *path, unsigned int mode, int stage,
                       void *context)
{
        int baselen = base->len;
        struct tree_traversal_options *opts =
                (struct tree_traversal_options *)context;

        char *parent_directory = strbuf_dup(base);

        /* From merge-recursive.c:save_files_dirs() -- append the parent
           directory (relative to the repo root) to the current filename. */
        strbuf_addstr(base, path);

        int ret = 0;
        Digest digest;
        switch (object_type(mode)) {
        case OBJ_TREE:
                HACKY_LOG("TREE!");
                switch (check_contains_directory(*oid, &digest)) {
                case OidMappingExists:
                        HACKY_LOG("MAPPING EXISTS!");
                        // If the LMDB backend already has this parent digest,
                        // we do not need to check all the recursive chlidren.
                        tree_traversal_add_known_directory(&opts->ctx,
                                                           parent_directory,
                                                           path, &digest, oid);
                        ret = 0;
                        break;
                case OidMappingDoesNotExist:
                        HACKY_LOG_ARG("MAPPING DOES NOT EXIST, %s",
                                      oid_to_hex(oid));
                        // If the digest was *not* already known, then we
                        // recurse!
                        tree_traversal_add_directory(
                                &opts->ctx, parent_directory, path, oid);
                        ret = READ_TREE_RECURSIVE;
                        break;
                default:
                        error("weird unknown error when trying to load oid: %s",
                              oid_to_hex(oid));
                        ret = -1;
                }
                break;
        case OBJ_BLOB:
                HACKY_LOG("BLOB!");
                // Allocate the file in shared memory. If it was already
                // allocated, it will no-op quickly.
                digest = allocate_shm_key(*oid);
                tree_traversal_add_file(&opts->ctx, parent_directory, path,
                                        &digest);
                ret = 0;
                break;
        default:
                HACKY_LOG_ARG(
                        "unrecognized mode when walking commit tree: %d\n",
                        mode);
                ret = -1;
        }

free_strings:
        free(parent_directory);
        strbuf_setlen(base, baselen);
        return ret;
}

static int sync_refs(struct push_params params)
{
        HACKY_LOG_ARG("argc: %d\n", params.ref_argc);
        for (size_t i = 0; i < params.ref_argc; i++) {
                const char *refname = params.ref_argv[i];
                HACKY_LOG_ARG("refname: %s\n", refname);
                /* (1) Resolve the ref to an `oid`, then look up the tree for
                 * that `oid`. */
                struct object_id oid;
                if (repo_get_oid_committish(the_repository, refname, &oid)) {
                        error("could not locate ref '%s' to push", refname);
                        continue;
                }
                const char *oid_hex = oid_to_hex(&oid);
                HACKY_LOG_ARG("oid_hex: %s\n", oid_hex);
                struct commit *commit =
                        lookup_commit_reference(the_repository, &oid);
                if (!commit) {
                        error("could not locate commit for oid %s", oid_hex);
                        continue;
                }
                HACKY_LOG("COMMIT!");
                struct tree *tree =
                        get_commit_tree_in_graph(the_repository, commit);
                if (!tree) {
                        error("could not locate tree for commit with oid %s",
                              oid_hex);
                        continue;
                }
                HACKY_LOG("TREE!");

                Digest digest;
                switch (check_contains_directory(oid, &digest)) {
                case OidMappingExists:
                        HACKY_LOG("top-level mapping exists!!");
                        goto print_formatted_digest;
                case OidMappingDoesNotExist:
                        HACKY_LOG("top-level mapping does not exist!");
                        break;
                default:
                        HACKY_LOG(
                                "unrecognized result from check_contains_directory() for top-level oid!!");
                        return -1;
                }

                /* (2) [Traverse all the linked objects!!] and: [write them into
                   the remexec CAS
                   store!!] in a way that will be unambiguously reproduced by a
                   remote client. */
                struct pathspec match_all;
                memset(&match_all, 0, sizeof(match_all));

                struct tree_traversal_options opts;
                tree_traversal_init_context(&opts.ctx);

                tree_traversal_set_root_oid(&opts.ctx, &oid);
                if (read_tree_recursive(the_repository, tree, "", 0, 0,
                                        &match_all, tree_reader, &opts)) {
                        error("failed to recurse down tree for ref '%s' to push",
                              refname);
                }

                digest = tree_traversal_destroy_context(&opts.ctx);
                directory_oid_add_mapping(translate_oid_new_oid(oid), digest);

        print_formatted_digest:;
                char *digest_formatted = format_digest_output(digest);
                printf("(%s, %s, %s)\n", refname, oid_hex, digest_formatted);

        free_digest_formatted:
                free_rust_string(digest_formatted);
                digest_formatted = NULL;

        free_tree:
                free_tree_buffer(tree);
                continue;
        }
        return 0;
}

int cmd_push_ref(int argc, const char **argv, const char *prefix)
{
        struct push_params params;
        if (parse_push_ref_options(argc, argv, prefix, &params)) {
                return 1;
        }
        HACKY_LOG_ARG("num args: %d\n", params.ref_argc);
        return sync_refs(params);
}

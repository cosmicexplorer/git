/*
 * Push all objects representing a specific ref to the upc/pants local LMDB
 * store, as well as a remote store, if configured.
 */

#include "cache.h"
#include "commit-graph.h"
#include "config.h"
#include "hash.h"
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

static char *null_term_strbuf(struct strbuf sb)
{
        char *s = malloc(sb.len + 1);
        memcpy(s, sb.buf, sb.len);
        s[sb.len] = '\0';
        return s;
}

static int tree_reader(const struct object_id *oid, struct strbuf *base,
                       const char *path, unsigned int mode, int stage,
                       void *context)
{
        int baselen = base->len;

        /* From merge-recursive.c:save_files_dirs() -- append the parent
           directory (relative to the repo root) to the current filename. */
        strbuf_addstr(base, path);

        /* char *cur_path = null_term_strbuf(*base); */
        /* HACKY_LOG_ARG("cur_path: %s\n", cur_path); */
        /* FREE_AND_NULL(cur_path); */
        int ret = 0;
        switch (object_type(mode)) {
        case OBJ_TREE:
                HACKY_LOG("TREE!");
                DirectoryDigest digest = as_digest(*oid);
                switch (check_directory_digest_existence(digest)) {
                case DigestExists:
                        // If the LMDB backend already has this parent digest,
                        // we do not need to check all the recursive chlidren.
                        ret = 0;
                        break;
                case DigestDoesNotExist:
                        // If the digest was *not* already known, then we
                        // recurse!
                        // FIXME: coordinate the recursion via the context
                        // object!
                        ret = READ_TREE_RECURSIVE;
                        break;
                default:
                        error("weird unknown error when trying to load oid: %s",
                              oid_to_hex(oid));
                }
                break;
        case OBJ_BLOB:
                HACKY_LOG("BLOB!");
                // Allocate the file in shared memory. If it was already
                // allocated, it will no-op quickly.
                ShmKey _key = allocate_shm_key(*oid);
                break;
        default:
                HACKY_LOG_ARG(
                        "unrecognized mode when walking commit tree: %d\n",
                        mode);
                ret = -1;
        }

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
                        continue;
                }
                HACKY_LOG("COMMIT!");
                struct tree *tree =
                        get_commit_tree_in_graph(the_repository, commit);
                if (!tree) {
                        continue;
                }
                HACKY_LOG("TREE!");
                /* (2) [Traverse all the linked objects!!] and: [write them into
                   the remexec CAS
                   store!!] in a way that will be unambiguously reproduced by a
                   remote client. */
                struct pathspec match_all;
                memset(&match_all, 0, sizeof(match_all));

                if (read_tree_recursive(the_repository, tree, "", 0, 0,
                                        &match_all, tree_reader, NULL)) {
                        error("failed to read tree for ref '%s' to push",
                              refname);
                        goto free_tree;
                }
        free_tree:
                free_tree_buffer(tree);
                continue;
        }
        /* const struct commit *commit; */
        /* if (prepare_revision_walk(&params.revs)) { */
        /*         die(_("revision walk setup failed")); */
        /* } */
        /* while ((commit = get_revision(&params.revs))) { */
        /*         struct object_id oid = commit->object.oid; */
        /*         const char *printed = print_oid_leak_string(oid); */
        /*         fprintf(stderr, "oid: %s\n", printed); */
        /*         free(printed); */
        /* } */
        /* reset_revision_walk(); */

        /* if (!commits) { */
        /*         error(_("no revisions provided!")); */
        /*         return 1; */
        /* } */
        /* printf("wow: %s\n", params.ref); */
}

/* static const char *print_oid_leak_string(struct object_id oid) */
/* { */
/*         char *s = malloc(GIT_MAX_RAWSZ + 1); */
/*         memcpy(s, &oid, GIT_MAX_RAWSZ); */
/*         s[GIT_MAX_RAWSZ] = '\0'; */
/*         return s; */
/* } */

int cmd_push_ref(int argc, const char **argv, const char *prefix)
{
        struct push_params params;
        if (parse_push_ref_options(argc, argv, prefix, &params)) {
                return 1;
        }
        HACKY_LOG_ARG("num args: %d\n", params.ref_argc);
        return sync_refs(params);
}

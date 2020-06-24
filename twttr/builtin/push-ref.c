/*
 * Push all objects representing a specific ref to the upc/pants local LMDB
 * store, as well as a remote store, if configured.
 */

#include "cache.h"
#include "config.h"
#include "hash.h"
#include "object-store.h"
#include "packfile.h"
#include "parse-options.h"
#include "refs.h"
#include "repository.h"
#include "revision.h"
#include "run-command.h"
#include "sigchain.h"
#include "string-list.h"

#include "hashing.h"
#include "directory.h"
#include "memory.h"

#if FINGERPRINT_SIZE != GIT_MAX_RAWSZ
#error "expected FINGERPRINT_SIZE and GIT_MAX_RAWSZ to be the same!"
#endif

#include "remexec-backend.h"

static const char *const builtin_push_ref_usage[] = {
        N_("git push-ref [<revisions>...]"), NULL
};

static struct option builtin_push_ref_options[] = {
        OPT_END(),
};

struct push_params {
        const char **ref_args;
        size_t num_args;
        struct ref_store *ref_source;
        struct ref_store *ref_destination;
};

static struct push_params parse_push_ref_options(int argc, const char **argv,
                                                 const char *prefix)
{
        git_config(git_default_config, NULL);

        argc = parse_options(argc, argv, prefix, builtin_push_ref_options,
                             builtin_push_ref_usage, 0);

        struct ref_store *ref_source = get_main_ref_store(the_repository);
        struct ref_store *ref_destination = remexec_ref_store_create();
        struct push_params ret = {
                argv,
                argc,
                ref_source,
                ref_destination,
        };
        return ret;
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
        struct push_params params = parse_push_ref_options(argc, argv, prefix);
        printf("num args: %d\n", params.num_args);

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
        return 0;
}

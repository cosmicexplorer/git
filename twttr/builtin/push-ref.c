/*
 * Push all objects representing a specific ref to the upc/pants local LMDB
 * store, as well as a remote store, if configured.
 */

#include "cache.h"
#include "config.h"
#include "hashmap.h"
#include "lockfile.h"
#include "object-store.h"
#include "packfile.h"
#include "parse-options.h"
#include "repository.h"
#include "run-command.h"
#include "sigchain.h"
#include "string-list.h"

static const char *const builtin_push_ref_usage[] = { N_("git push-ref ref"),
                                                      NULL };

int cmd_push_ref(int argc, const char **argv, const char *prefix)
{
        const uint64_t start = getnanotime();

        git_config(git_default_config, NULL);

        const char *ref;
        struct option builtin_push_ref_options[] = {
                OPT_STRING(0, "ref", &ref, N_("ref"), N_("the ref to upload")),
                OPT_END(),
        };
        if (argv == 2 && !strcmp(argv[1], "-h")) {
                usage_with_options(builtin_push_ref_usage,
                                   builtin_push_ref_options);
        }
        argc = parse_options(argc, argv, prefix, builtin_push_ref_options,
                             builtin_push_ref_usage, 0);
        if (argc != 1) {
                usage_with_options(builtin_push_ref_usage,
                                   builtin_push_ref_options);
        }
        return 0;
}

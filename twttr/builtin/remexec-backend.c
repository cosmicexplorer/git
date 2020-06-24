#include "refs/refs-internal.h"

#include "remexec-backend.h"

/* This "inherits" from `struct ref_store`. */
struct remexec_ref_store {
        struct ref_store base;
};

struct remexec_ref_store *remexec_ref_store_create()
{
        struct remexec_ref_store *refs = xcalloc(1, sizeof(*refs));
        struct ref_store *ref_store = (struct ref_store *)refs;

        base_ref_store_init(ref_store, &refs_be_remexec);

        return refs;
}

static struct ref_store *remexec_ref_store_do_create(const char *_gitdir,
                                                     unsigned int _flags)
{
        return (struct ref_store *)remexec_ref_store_create();
}

static int remexec_init_db(struct ref_store *ref_store, struct strbuf *err)
{
        return 0;
}

static int remexec_transaction_prepare(struct ref_store *refs,
                                       struct ref_transaction *transaction,
                                       struct strbuf *err)
{
        return 0;
}

static int remexec_transaction_finish(struct ref_store *refs,
                                      struct ref_transaction *transaction,
                                      struct strbuf *err)
{
        return 0;
}

static int remexec_transaction_abort(struct ref_store *refs,
                                     struct ref_transaction *transaction,
                                     struct strbuf *err)
{
        return 0;
}

static int
remexec_initial_transaction_commit(struct ref_store *refs,
                                   struct ref_transaction *transaction,
                                   struct strbuf *err)
{
        return 0;
}

static int remexec_pack_refs(struct ref_store *refs, unsigned int flags)
{
        return 0;
}

static int remexec_create_symref(struct ref_store *ref_store,
                                 const char *ref_target,
                                 const char *refs_heads_master,
                                 const char *logmsg)
{
        return 0;
}

static int remexec_delete_refs(struct ref_store *ref_store, const char *msg,
                               struct string_list *refnames, unsigned int flags)
{
        return 0;
}

static int remexec_rename_ref(struct ref_store *ref_store, const char *oldref,
                              const char *newref, const char *logmsg)
{
        return 0;
}

static int remexec_copy_ref(struct ref_store *ref_store, const char *oldref,
                            const char *newref, const char *logmsg)
{
        return 0;
}

/* "Inherits" from `struct ref_iterator`. */
struct remexec_ref_iterator {
        struct ref_iterator base;
};

static struct ref_iterator *remexec_ref_iterator_begin(struct ref_store *refs,
                                                       const char *prefix,
                                                       unsigned int flags)
{
        /* struct remexec_ref_iterator *it = xcalloc(1, sizeof(*it)); */
        /* struct ref_iterator *ref_it = (struct ref_iterator *)it; */
        return empty_ref_iterator_begin();
}

static int remexec_read_raw_ref(struct ref_store *ref_store,
                                const char *refname, struct object_id *oid,
                                struct strbuf *referent, unsigned int *type)
{
        return 0;
}

static struct ref_iterator *
remexec_reflog_iterator_begin(struct ref_store *refs)
{
        return empty_ref_iterator_begin();
}

static int remexec_for_each_reflog_ent(struct ref_store *refs,
                                       const char *refname,
                                       each_reflog_ent_fn fn, void *cb_data)
{
        return 0;
}

static int remexec_for_each_reflog_ent_reverse(struct ref_store *refs,
                                               const char *refname,
                                               each_reflog_ent_fn fn,
                                               void *cb_data)
{
        return 0;
}

static int remexec_reflog_exists(struct ref_store *refs, const char *refname)
{
        return 0;
}

static int remexec_create_reflog(struct ref_store *refs, const char *refname,
                                 int force_create, struct strbuf *err)
{
        return 0;
}

static int remexec_delete_reflog(struct ref_store *refs, const char *refname)
{
        return 0;
}

static int
remexec_reflog_expire(struct ref_store *ref_store, const char *refname,
                      const struct object_id *oid, unsigned int flags,
                      reflog_expiry_prepare_fn prepare_fn,
                      reflog_expiry_should_prune_fn should_prune_fn,
                      reflog_expiry_cleanup_fn cleanup_fn, void *policy_cb_data)
{
        return 0;
}

struct ref_storage_be refs_be_remexec = {
        NULL,
        "remexec",
        remexec_ref_store_do_create,
        remexec_init_db,
        remexec_transaction_prepare,
        remexec_transaction_finish,
        remexec_transaction_abort,
        remexec_initial_transaction_commit,

        remexec_pack_refs,
        remexec_create_symref,
        remexec_delete_refs,
        remexec_rename_ref,
        remexec_copy_ref,

        remexec_ref_iterator_begin,
        remexec_read_raw_ref,

        remexec_reflog_iterator_begin,
        remexec_for_each_reflog_ent,
        remexec_for_each_reflog_ent_reverse,
        remexec_reflog_exists,
        remexec_create_reflog,
        remexec_delete_reflog,
        remexec_reflog_expire,
};

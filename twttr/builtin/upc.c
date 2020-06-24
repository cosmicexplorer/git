#include "upc.h"

#include "repository.h"

ShmKey allocate_shm_key(struct object_id oid)
{
        unsigned long blob_size;
        enum object_type type;
        void *buffer =
                repo_read_object_file(the_repository, &oid, &type, &blob_size);
        if (!buffer) {
                die_errno(_("object id '%s' did not exist; expected blob"),
                          oid_to_hex(&oid));
        }
        if (type != OBJ_BLOB) {
                die_errno(_("object id '%s' was not a blob"), oid_to_hex(&oid));
        }

        /* We can just take the oid hash directly here, because the fingerprints
           are exactly the same length in bytes. */
        Fingerprint fp = { oid.hash };
        ShmKey key = { blob_size, fp };
        ShmAllocateRequest request = { key, buffer };

        ShmAllocateResult result;
        shm_allocate(&request, &result);
        if (result.status != AllocationSucceeded) {
                die_errno(_("failed to allocate in shared memory: %s"),
                          result.error_message);
        }

        return key;
}

DirectoryDigest as_digest(struct object_id oid)
{
        unsigned long blob_size;
        enum object_type type;
        /* FIXME: is not freeing this a leak? */
        void *buffer =
                repo_read_object_file(the_repository, &oid, &type, &blob_size);
        if (!buffer) {
                die_errno(_("object id '%s' did not exist ; expected tree"),
                          oid_to_hex(&oid));
        }
        if (type != OBJ_TREE) {
                die_errno(_("object id '%s' was not a tree"), oid_to_hex(&oid));
        }

        /* We can just take the oid hash directly here, because the fingerprints
           are exactly the same length in bytes. */
        Fingerprint fp = { oid.hash };
        DirectoryDigest digest = { blob_size, fp };
        return digest;
}

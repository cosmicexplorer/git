#include "upc.h"

#include "repository.h"

Fingerprint translate_oid_to_fingerprint(struct object_id oid)
{
        Fingerprint fp;
        memcpy(fp._0, oid.hash, FINGERPRINT_SIZE);
        return fp;
}

Digest allocate_shm_key(struct object_id oid)
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
        Fingerprint fp = translate_oid_to_fingerprint(oid);
        ShmKey key = { blob_size, fp };
        ShmAllocateRequest request = { key, buffer };

        ShmAllocateResult result;
        shm_allocate(&request, &result);
        if (result.status != AllocationSucceeded) {
                die_errno(_("failed to allocate in shared memory: %s"),
                          result.error_message);
        }

        Digest digest = { fp, blob_size };
        return digest;
}

DirectoryOidCheckMappingResult check_contains_directory(struct object_id oid,
                                                        Digest *digest)
{
        Fingerprint fp = translate_oid_to_fingerprint(oid);
        Oid new_oid = { fp };
        return directory_oid_check_mapping(new_oid, digest);
}

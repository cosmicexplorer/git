#pragma once

#include "hashing.h"
#include "memory.h"
#include "directory.h"

#include "hash.h"
#include "object.h"
#include "object-store.h"

#if FINGERPRINT_SIZE != GIT_MAX_RAWSZ
#error "expected FINGERPRINT_SIZE and GIT_MAX_RAWSZ to be the same!"
#endif

Fingerprint translate_oid_to_fingerprint(struct object_id oid);

Oid translate_oid_new_oid(struct object_id oid);

Digest allocate_shm_key(struct object_id oid);

DirectoryOidCheckMappingResult check_contains_directory(struct object_id oid,
                                                        Digest *digest);

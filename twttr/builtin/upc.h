#pragma once

#include "directory.h"
#include "hashing.h"
#include "memory.h"

#include "hash.h"
#include "object.h"
#include "object-store.h"

#if FINGERPRINT_SIZE != GIT_MAX_RAWSZ
#error "expected FINGERPRINT_SIZE and GIT_MAX_RAWSZ to be the same!"
#endif

ShmKey allocate_shm_key(struct object_id oid);

DirectoryDigest as_digest(struct object_id oid);

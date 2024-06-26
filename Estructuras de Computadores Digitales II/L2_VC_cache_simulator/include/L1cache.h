/*
 *  Cache simulation project
 *  Class UCR IE-521
 *  Semester: I-2019
*/

#ifndef L1CACHE_H
#define L1CACHE_H

#include <netinet/in.h> 
/* 
 * ENUMERATIONS 
 */

/* Return Values */
enum returns_types {
 OK,
 PARAM,
 ERROR
};

/* Represent the cache replacement policy */
enum replacement_policy{
 LRU,
 NRU,
 RRIP,
 RANDOM 
};

enum miss_hit_status {
 MISS_LOAD,
 MISS_STORE,
 HIT_LOAD,
 HIT_STORE
};

enum opt{
    L2,
    VICTIMCACHE
};

/*
 * STRUCTS
 */

/* Cache tag array fields */
struct entry {
 bool valid ;
 bool dirty;
 int tag ;
 uint8_t rp_value ;
};

/* Cache replacement policy results */
struct operation_result {
 enum miss_hit_status miss_hit;
 bool dirty_eviction;
 int evicted_way;
 int  evicted_address;
};

/* 
 *  Functions
 * /

/*
 * Get tag, index and offset length
 * 
 * [in] cache_size: total size of the cache in Kbytes
 * [in] associativity: number of ways of the cache
 * [in] blocksize_bytes: size of each cache block in bytes
 *
 * [out] tag_size: size in bits of the tag field
 * [out] idx_size: size in bits of the index field
 * [out] offset_size: size in bits of the offset size
 */
int field_size_get(int cachesize_kb,
                   int associativity,
                   int blocksize_bytes,
                   int *tag_size,
                   int *idx_size,
                   int *offset_size);

/* 
 * Get tag and index from address
 * 
 * [in] address: memory address
 * [in] tag_size: number of bits of the tag field
 * [in] idx_size: number of bits of the index field
 *
 * [out] idx: cache line idx
 * [out] tag: cache line tag
 */

void address_tag_idx_get(long address,
                         int tag_size,
                         int idx_size,
                         int offset_size,
                         int *idx,
                         int *tag);


/* 
 * Search for an address in a cache set and
 * replaces blocks using SRRIP(hp) policy
 * 
 * [in] idx: index field of the block
 * [in] tag: tag field of the block
 * [in] associativity: number of ways of the entry
 * [in] loadstore: type of operation false if load true if store
 * [in] debug: if set to one debug information is printed
 *
 * [in/out] cache_block: return the cache operation return (miss_hit_status)
 * [out] result: result of the operation (returns_types)
 */
int srrip_replacement_policy (int idx,
                              int tag,
                              int associativity,
                              bool loadstore,
                              entry* cache_blocks,
                              operation_result* operation_result,
                              bool debug=false);

/* 
 * Search for an address in a cache set and
 * replaces blocks using LRU policy
 * 
 * [in] idx: index field of the block
 * [in] tag: tag field of the block
 * [in] associativity: number of ways of the entry
 * [in] loadstore: type of operation false if true if store
 * [in] debug: if set to one debug information is printed
 *
 * [in/out] cache_block: return the cache operation return (miss_hit_status)
 * [out] result: result of the operation (returns_types)
 */
int lru_replacement_policy (int idx,
                           int tag,
                           int associativity,
                           bool loadstore,
                           entry* cache_blocks,
                           operation_result* operation_result,
                           bool debug=false);
/* 
 * Invalidates a line in the cache 
 * 
 * [in] tag: tag field of the block
 * [in] associativity: number of ways of the entry
 * [in] debug: if set to one debug information is printed
 *
 * [in/out] cache_block: cache entry to edit
 *
 * return error if tag is not found in cache blocks
 */
int l1_line_invalid_set(int tag,
                        int associativity,
                        entry* cache_blocks,
                        bool debug=false);
#endif

/*
 *  Cache simulation project
 *  Class UCR IE-521
 */
#ifndef L2CACHE_H
#define L2CACHE_H

/*
 * STRUCTS
 */

/* L1 and l2 info */
struct l1_l2_entry_info {
	int l1_tag;
	int l1_associativity;
	int l2_tag;
	int l2_associativity;
};

/* 
 *  Functions
 */

/* 
 * Search for an address in a cache set and
 * replaces blocks using LRU policy and a updates L2 cache 
 * 
 * [in] l1_l2_info: struct with l2 and l1 entry info (this is a ro parameter) 
 * [in] loadstore: type of operation true if load false if store
 * [in] debug: if set to one debug information is printed
 *
 * [in/out] l1_cache_blocks: l1 cache line to work on 
 * [in/out] l2_cache_blocks: l2 cache line to work on
 * [in/out] l1_result: return the cache operation result in l1(miss_hit_status)
 * [in/out] l2_result: return the cache operation result in l2 (miss_hit_status)
 * [out] result: result of the operation (returns_types)
 */
int lru_replacement_policy_l1_l2(const l1_l2_entry_info *l1_l2_info,
																 bool loadstore,
																 entry* l1_cache_blocks,
																 entry* l2_cache_blocks,
																 operation_result* l1_result,
																 operation_result* l2_result,
																 bool debug=false);


	
#endif

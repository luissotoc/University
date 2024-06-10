/*
 *  Cache simulation project
 *  Class UCR IE-521
 *  Semester: II-2019
 */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <math.h>
#include <debug_utilities.h>
#include <L1cache.h>
#include <Victimcache.h>

#define KB 1024
#define ADDRSIZE 32
using namespace std;

int fifo_replacement_policy_l1_vc(	const l1_vc_entry_info *l1_vc_info,
      	                      	 	bool loadstore,
        	                    	entry* l1_cache_blocks,
          	                  	 	entry* vc_cache_blocks,
            	                	operation_result* l1_result,
              	              	 	operation_result* vc_result,
                	            	bool debug)
{

	int l1_hit_way = 0;
	int vc_hit_way = 0;
	int l1_evicted_way = 0;
	int vc_evicted_way = 0;

	bool vc_hit = false;
	/*valores temporales*/
	int temp_tag;
	bool temp_dirty;
	bool temp_valid;


	if(l1_result->miss_hit == HIT_STORE || l1_result->miss_hit == HIT_LOAD){  /*Hit en L1*/
	/* L1 y victim cache son exclusivos, entonces no pasa nada*/
	};
	
	if(l1_result->miss_hit == MISS_STORE || l1_result->miss_hit == MISS_LOAD){  /*miss en L1*/
	/*revisar si dato esta en VC*/
		for(int i = 0; i<l1_vc_info->vc_associativity; i++){
			if(vc_cache_blocks[i].tag == l1_vc_info->l1_tag && vc_cache_blocks[i].valid){
				vc_hit_way = i;
         		vc_result->evicted_address = vc_hit_way;
         		vc_hit = true;
			}
		}
		
		if(vc_hit == true){  //  VC HIT
			/*guardar bloque de vc en temporal*/
			temp_tag = vc_cache_blocks[vc_hit_way].tag;
			temp_dirty = vc_cache_blocks[vc_hit_way].dirty;
			temp_valid = vc_cache_blocks[vc_hit_way].valid;

			/*guardar en vc el evicted de l1*/
			vc_cache_blocks[vc_hit_way].tag = l1_cache_blocks[l1_result->evicted_way].tag;
			vc_cache_blocks[vc_hit_way].dirty = l1_cache_blocks[l1_result->evicted_way].dirty;
			
			/*guardar en l1 temporal*/
			l1_cache_blocks[l1_result->evicted_way].tag = temp_tag;
			l1_cache_blocks[l1_result->evicted_way].dirty = temp_dirty;
			l1_cache_blocks[l1_result->evicted_way].valid = temp_valid;
			

			/*fifo logic*/
			for(int i = 0; i<l1_vc_info->vc_associativity; i++){
				if(vc_cache_blocks[i].rp_value < vc_cache_blocks[vc_hit_way].rp_value){
					vc_cache_blocks[i].rp_value++;
				}
			}
			vc_cache_blocks[vc_hit_way].rp_value = 0;
			l1_cache_blocks[l1_result->evicted_way].rp_value = 0; //reduntante?
			if (loadstore){
				vc_result->miss_hit = HIT_STORE;
				l1_cache_blocks[l1_result->evicted_way].dirty = true;
      		} else {
				vc_result->miss_hit = HIT_LOAD;
      		}
		}
		 
		else {	//   VC MISS
			for (int i = 0; i<l1_vc_info->vc_associativity; i++){
				if (vc_cache_blocks[i].rp_value == l1_vc_info->vc_associativity-1){
					vc_evicted_way = i;
				}
			}
			
			if(vc_cache_blocks[vc_evicted_way].dirty){
				vc_result->dirty_eviction = true;
			}
			/*l1 a vc*/
			vc_cache_blocks[vc_evicted_way].dirty = l1_cache_blocks[l1_result->evicted_way].dirty;
			vc_cache_blocks[vc_evicted_way].valid = l1_cache_blocks[l1_result->evicted_way].valid;
			vc_cache_blocks[vc_evicted_way].tag = l1_cache_blocks[l1_result->evicted_way].tag;
			
			/*memoria a l1*/
			l1_cache_blocks[l1_result->evicted_way].tag = l1_vc_info->l1_tag;
			l1_cache_blocks[l1_result->evicted_way].valid = 1;
			
			if (loadstore){
				vc_result->miss_hit = MISS_STORE;
				l1_cache_blocks[l1_result->evicted_way].dirty = true;
      		} else {
				vc_result->miss_hit = MISS_LOAD;
      		}

		}

	};
   return OK;
}

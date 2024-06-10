/*
 *  Cache simulation project
 *  Class UCR IE-521
 *  Semester: II-2019
 */

#include <gtest/gtest.h>
#include <time.h>
#include <stdlib.h>
#include <debug_utilities.h>
#include <L1cache.h>
#include <Victimcache.h>

using namespace std;

class VCcache : public ::testing::Test{
	protected:
		int debug_on;
		virtual void SetUp()
		{
  		/* Parse for debug env variable */
  		get_env_var("TEST_DEBUG", &debug_on);
		};
};

/*
 * TEST L1 Miss VC Hit: 
 * 1. Choose a random index, associativity and tag
 * 2. Initialize an empty set of the l1 cache
 * 3. Initialize an empty victim cache
 * 4. Fill a l1 cache with associativity+1 different tags, so the first gets evicted.
 * 5. Make a memory access with the first entry, forcing a miss on l1 and a hit on vc.
 */
TEST_F(VCcache,l1_miss_vc_hit){
	int status = OK;
	bool debug = true;
	DEBUG(debug_on, l1_miss_vc_hit_test);

	/*STEP 1: Choose a random index, associativity and tag*/

	//Generate idx, tag and associativity
	int idx = rand()%256;
 	int originalTag = rand()%4096;
	int currentTag = 0;
  	int associativity = 4;//1 << (rand()%4);
	int loadstore = 0;	


	//Declare structs
	struct l1_vc_entry_info l1_vc_info;
	struct entry l1_cache_blocks[256][associativity];
	struct entry vc_cache_blocks[1][16];
  	struct operation_result l1_result = {};
	struct operation_result vc_result = {};
	
	/*STEP 2: Initialize an empty L1 cache*/

	//L1 Cache
	l1_result.miss_hit = MISS_STORE;
  	l1_result.dirty_eviction = false;
  	l1_result.evicted_address = 0;
	for (int i = 0; i < associativity; i++) {
      	l1_cache_blocks[idx][i].valid = false;
      	l1_cache_blocks[idx][i].tag = 0;
      	l1_cache_blocks[idx][i].dirty = 0;
		l1_cache_blocks[idx][i].rp_value = 16-i-1;
	}

	/*STEP 3: Initialize an empty victim cache*/

	/*Victim cache*/
  	vc_result.evicted_address = 0;
  	vc_result.miss_hit = MISS_STORE;
  	for(int i = 0;i<16;i++){
    	vc_cache_blocks[0][i].valid = false;
    	vc_cache_blocks[0][i].dirty = false;
    	vc_cache_blocks[0][i].rp_value = 16-i;
    	vc_cache_blocks[0][i].tag = 0;
		
  	}

	/*Step 4: Overloading one set of the L1 cache, so the first tag gets evicted*/

	for(int i = 0; i<associativity + 1; i++){

		currentTag = originalTag + i;

		/*entry info VC*/
  		l1_vc_info.l1_associativity = associativity;
  		l1_vc_info.l1_idx = idx;
  		l1_vc_info.l1_tag = currentTag;
  		l1_vc_info.vc_associativity = 16;

		status = lru_replacement_policy (idx,
    	                         		currentTag,
        	                     		associativity,
            	                 		loadstore,
                	             		l1_cache_blocks[idx],
                    	         		&l1_result,
                        	     		debug);
		EXPECT_EQ(status, OK);
		
		

		status = fifo_replacement_policy_l1_vc(&l1_vc_info,
      	                        	      	loadstore,
        	                      	      	l1_cache_blocks[idx],
          	                    	      	vc_cache_blocks[0],
            	                  	     	&l1_result,
              	                	      	&vc_result);
		EXPECT_EQ(status, OK);
	}


	/*STEP 5: Making a memory access with the first tag, getting a miss on L1 and a hit on VC*/

	status = lru_replacement_policy (idx,
    	                         		originalTag,
        	                     		associativity,
            	                 		loadstore,
                	             		l1_cache_blocks[idx],
                    	         		&l1_result,
                        	     		debug);
	EXPECT_EQ(status, OK);
	
  	l1_vc_info.l1_tag = originalTag;
		
	status = fifo_replacement_policy_l1_vc(&l1_vc_info,
      	                        	      	loadstore,
        	                      	      	l1_cache_blocks[idx],
          	                    	      	vc_cache_blocks[0],
            	                  	     	&l1_result,
              	                	      	&vc_result);
	EXPECT_EQ(status, OK);
	EXPECT_EQ(l1_result.miss_hit, MISS_LOAD); 	//Miss in l1
	EXPECT_EQ(vc_result.miss_hit, HIT_LOAD);	//Hit in vc
			

}

/*
 * TEST L1 Miss VC Hit: 
 * 1. Choose a random index, associativity and tag
 * 2. Initialize an empty set of the l1 cache
 * 3. Initialize an empty victim cache
 * 4. Fill a l1 cache with l1_associativity+vc_associativity+1 different tags, so the first gets evicted.
 * 5. Make a memory access with the first entry, forcing a miss on l1 and vc.
 */
TEST_F(VCcache,l1_miss_vc_miss){
	int status = OK;
	bool debug = true;
	DEBUG(debug_on, l1_miss_vc_hit_test);

	/*STEP 1: Choose a random index, associativity and tag*/

	//Generate idx, tag and associativity
	int idx = rand()%256;
 	int originalTag = rand()%4096;
	int currentTag = 0;
  	int associativity = 1 << (rand()%4);
	int loadstore = 0;	


	//Declare structs
	struct l1_vc_entry_info l1_vc_info;
	struct entry l1_cache_blocks[256][associativity];
	struct entry vc_cache_blocks[1][16];
  	struct operation_result l1_result = {};
	struct operation_result vc_result = {};
	
	/*STEP 2: Initialize an empty L1 cache*/

	//L1 Cache
	l1_result.miss_hit = MISS_STORE;
  	l1_result.dirty_eviction = false;
  	l1_result.evicted_address = 0;
	for (int i = 0; i < associativity; i++) {
      	l1_cache_blocks[idx][i].valid = false;
      	l1_cache_blocks[idx][i].tag = 0;
      	l1_cache_blocks[idx][i].dirty = 0;
    	l1_cache_blocks[idx][i].rp_value = associativity-1-i;       
	}

	/*STEP 3: Initialize an empty victim cache*/

	/*Victim cache*/
  	vc_result.evicted_address = 0;
  	vc_result.miss_hit = MISS_STORE;
  	for(int i = 0;i<16;i++){
    	vc_cache_blocks[0][i].valid = false;
    	vc_cache_blocks[0][i].dirty = false;
    	vc_cache_blocks[0][i].rp_value = 16-1-i;
    	vc_cache_blocks[0][i].tag = 0;
  	}

	/*Step 4: Overloading one set of the L1 cache and VC, so the first tag gets evicted*/

	for(int i = 0; i<associativity + 16 + 1; i++){

		currentTag = originalTag + i;

		/*entry info VC*/
  		l1_vc_info.l1_associativity = associativity;
  		l1_vc_info.l1_idx = idx;
  		l1_vc_info.l1_tag = currentTag;
  		l1_vc_info.vc_associativity = 16;

		status = lru_replacement_policy (idx,
    	                         		currentTag,
        	                     		associativity,
            	                 		loadstore,
                	             		l1_cache_blocks[idx],
                    	         		&l1_result,
                        	     		debug);
		EXPECT_EQ(status, OK);
		
		

		status = fifo_replacement_policy_l1_vc(&l1_vc_info,
      	                        	      	loadstore,
        	                      	      	l1_cache_blocks[idx],
          	                    	      	vc_cache_blocks[0],
            	                  	     	&l1_result,
              	                	      	&vc_result);
		EXPECT_EQ(status, OK);
	}


	/*STEP 5: Making a memory access with the first tag, getting a miss on L1 and VC*/

	status = lru_replacement_policy (idx,
    	                         		originalTag,
        	                     		associativity,
            	                 		loadstore,
                	             		l1_cache_blocks[idx],
                    	         		&l1_result,
                        	     		debug);
	EXPECT_EQ(status, OK);

	
  	l1_vc_info.l1_tag = originalTag;
		
	status = fifo_replacement_policy_l1_vc(&l1_vc_info,
      	                        	      	loadstore,
        	                      	      	l1_cache_blocks[idx],
          	                    	      	vc_cache_blocks[0],
            	                  	     	&l1_result,
              	                	      	&vc_result);
	EXPECT_EQ(status, OK);
	EXPECT_EQ(l1_result.miss_hit, MISS_LOAD); 	//Miss in l1
	EXPECT_EQ(vc_result.miss_hit, MISS_LOAD);	//MISS in vc

}


/*
						THIS TEST WAS MADE TO PROVE THE BUG THAT COMES WITH THE CONSIDERATION
											THAT VC_TAG == L1_TAG
	
 * TEST L1 Miss VC Miss 2: 
 * 1. Choose a random index, associativity and tag
 * 2. Initialize two empty sets of the l1 cache
 * 3. Initialize an empty victim cache
 * 4. Fill a l1 cache with l1_associativity+1 different tags, so the first gets evicted to vc.
 * 5. Make a memory access with the tag but with a different idx (the second empty set), forcing a miss on l1 and vc.
 * 	  (if we get a hit on vc, this is incorrect, because we are making an access with a different address)
 */
TEST_F(VCcache,l1_miss_vc_miss2){
	int status = OK;
	bool debug = true;
	DEBUG(debug_on, l1_miss_vc_hit_test);

	/*STEP 1: Choose a random index, associativity and tag*/

	//Generate idx, tag and associativity
	int idx1 = rand()%256;
	int idx2 = rand()%256;
 	int originalTag = rand()%4096;
	int currentTag = 0;
  	int associativity = 1 << (rand()%4);
	int loadstore = 0;


	//Declare structs
	struct l1_vc_entry_info l1_vc_info;
	struct entry l1_cache_blocks[256][associativity];
	struct entry vc_cache_blocks[1][16];
  	struct operation_result l1_result = {};
	struct operation_result vc_result = {};
	
	/*STEP 2: Initialize two empty L1 cache entries*/

	//L1 Cache
	l1_result.miss_hit = MISS_STORE;
  	l1_result.dirty_eviction = false;
  	l1_result.evicted_address = 0;
	for (int i = 0; i < associativity; i++) {
      	l1_cache_blocks[idx1][i].valid = false;
      	l1_cache_blocks[idx1][i].tag = 0;
      	l1_cache_blocks[idx1][i].dirty = 0;
    	l1_cache_blocks[idx1][i].rp_value = associativity-1-i;

		l1_cache_blocks[idx2][i].valid = false;
      	l1_cache_blocks[idx2][i].tag = 0;
      	l1_cache_blocks[idx2][i].dirty = 0;
    	l1_cache_blocks[idx2][i].rp_value = associativity-1-i;       
	}

	/*STEP 3: Initialize an empty victim cache*/

	/*Victim cache*/
  	vc_result.evicted_address = 0;
  	vc_result.miss_hit = MISS_STORE;
  	for(int i = 0;i<16;i++){
    	vc_cache_blocks[0][i].valid = false;
    	vc_cache_blocks[0][i].dirty = false;
    	vc_cache_blocks[0][i].rp_value = 16-1-i;
    	vc_cache_blocks[0][i].tag = 0;
  	}

	/*Step 4: Overloading one set of the L1 cache and VC, so the first tag gets evicted*/

	for(int i = 0; i<associativity + 1; i++){

		currentTag = originalTag + i;

		/*entry info VC*/
  		l1_vc_info.l1_associativity = associativity;
  		l1_vc_info.l1_idx = idx1;
  		l1_vc_info.l1_tag = currentTag;
  		l1_vc_info.vc_associativity = 16;

		status = lru_replacement_policy (idx1,
    	                         		currentTag,
        	                     		associativity,
            	                 		loadstore,
                	             		l1_cache_blocks[idx1],
                    	         		&l1_result,
                        	     		debug);
		EXPECT_EQ(status, OK);
		
		

		status = fifo_replacement_policy_l1_vc(&l1_vc_info,
      	                        	      	loadstore,
        	                      	      	l1_cache_blocks[idx1],
          	                    	      	vc_cache_blocks[0],
            	                  	     	&l1_result,
              	                	      	&vc_result);
		EXPECT_EQ(status, OK);
	}


	/*STEP 5: Making a memory access with the first tag, getting a miss on L1 and VC*/

	status = lru_replacement_policy (idx2,
    	                         		originalTag,
        	                     		associativity,
            	                 		loadstore,
                	             		l1_cache_blocks[idx2],
                    	         		&l1_result,
                        	     		debug);
	EXPECT_EQ(status, OK);

	
  	l1_vc_info.l1_tag = originalTag;
		
	status = fifo_replacement_policy_l1_vc(&l1_vc_info,
      	                        	      	loadstore,
        	                      	      	l1_cache_blocks[idx2],
          	                    	      	vc_cache_blocks[0],
            	                  	     	&l1_result,
              	                	      	&vc_result);
	EXPECT_EQ(status, OK);

	EXPECT_EQ(l1_result.miss_hit, MISS_LOAD); 	//Miss in l1
	EXPECT_EQ(vc_result.miss_hit, MISS_LOAD);	//Miss in vc



}
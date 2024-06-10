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
#include <L2cache.h>

using namespace std;

class L2cache : public ::testing::Test{
	protected:
		int debug_on;
		virtual void SetUp()
		{
  		/* Parse for debug env variable */
  		get_env_var("TEST_DEBUG", &debug_on);
		};
};


/*
 * TEST L1 Hit L2 Hit: 
 * 1. Choose a random index, associativity and tag
 * 2. Initialize an empty set of the l1 cache
 * 3. Initialize an empty set of the l2 cache
 * 4. Fill a l1 cache with l1_associativity different tags, so the l2_cache also has them.
 * 5. Make a memory access with the first entry, forcing a hit on l1 and l2.
 */
TEST_F(L2cache,l1_hit_l2_hit){
	int status = OK;
	bool debug = true;
	DEBUG(debug_on, l1_hit_l2_hit_test);

	/*STEP 1: Choose a random index, associativity and tag*/

	//Generate idx for l1 and l2, tag and associativity
	int l1_idx = rand()%256;
	int l2_idx = l1_idx + (256*(1-rand()%2));  //This way if l1_idx = 0x32, l2_idx would be 0x132 or 0x032, adding one bit to the right 
	 
	int l1_originalTag = rand()%4096;
	int l1_currentTag = 0;
	int l2_originalTag = l1_originalTag/2;
	int l2_currentTag = 0;
  	int l1_associativity = 1 << (rand()%3);
	int l2_associativity = l1_associativity*2;  
	int loadstore = 0;	


	//Declare structs
	struct l1_l2_entry_info l1_l2_info;
	struct entry l1_cache_blocks[256][l1_associativity];
	struct entry l2_cache_blocks[256*2][l2_associativity];
  	struct operation_result l1_result = {};
	struct operation_result l2_result = {};
	
	/*STEP 2: Initialize an empty L1 cache set*/

	//L1 Cache
	l1_result.miss_hit = MISS_STORE;
  	l1_result.dirty_eviction = false;
  	l1_result.evicted_address = 0;
	for (int i = 0; i < l1_associativity; i++) {
      	l1_cache_blocks[l1_idx][i].valid = false;
      	l1_cache_blocks[l1_idx][i].tag = 0;
      	l1_cache_blocks[l1_idx][i].dirty = 0;
    	l1_cache_blocks[l1_idx][i].rp_value = l1_associativity-1-i;       
	}

	/*STEP 3: Initialize an empty L2 cache set*/

	//L2 Cache
	l2_result.miss_hit = MISS_STORE;
  	l2_result.dirty_eviction = false;
  	l2_result.evicted_address = 0;
	for (int i = 0; i < l2_associativity; i++) {
      	l2_cache_blocks[l2_idx][i].valid = false;
      	l2_cache_blocks[l2_idx][i].tag = 0;
      	l2_cache_blocks[l2_idx][i].dirty = 0;
    	l2_cache_blocks[l2_idx][i].rp_value = l2_associativity-1-i;       
	}
	

	/*Step 4: Loading one set of the L1 cache and L2 cache*/

	for(int i = 0; i<l1_associativity; i++){

		if((l1_originalTag + 10*i) < 4096) {
			l1_currentTag = l1_originalTag + 10*i;
		} else { 
			l1_currentTag = l1_originalTag - 500*i;
		}
		l2_currentTag = l1_currentTag >> 1;

  		/*entry info L2*/
  		l1_l2_info.l1_associativity = l1_associativity;
  		l1_l2_info.l1_tag = l1_currentTag;
  		l1_l2_info.l2_associativity = l2_associativity;
  		l1_l2_info.l2_tag = l2_currentTag;


		status = lru_replacement_policy (l1_idx,
    	                         		l1_currentTag,
        	                     		l1_associativity,
            	                 		loadstore,
                	             		l1_cache_blocks[l1_idx],
                    	         		&l1_result,
                        	     		debug);
		EXPECT_EQ(status, OK);
		
		

		status = lru_replacement_policy_l1_l2(&l1_l2_info,
      	                        	      	loadstore,
        	                      	      	l1_cache_blocks[l1_idx],
          	                    	      	l2_cache_blocks[l2_idx],
            	                  	     	&l1_result,
											&l2_result,
              	                	      	debug);
		EXPECT_EQ(status, OK);
	}


	/*STEP 5: Making a memory access with the first tag, getting a hit on L1 and VC*/

	status = lru_replacement_policy (l1_idx,
    	                         	l1_originalTag,
        	                     	l1_associativity,
            	                 	loadstore,
                	             	l1_cache_blocks[l1_idx],
                    	         	&l1_result,
                        	     	debug);
	EXPECT_EQ(status, OK);

	
  	l1_l2_info.l2_tag = l2_originalTag;
	l1_l2_info.l1_tag = l1_originalTag;
		
	status = lru_replacement_policy_l1_l2(&l1_l2_info,
      	                        	      	loadstore,
        	                      	      	l1_cache_blocks[l1_idx],
          	                    	      	l2_cache_blocks[l2_idx],
            	                  	     	&l1_result,
              	                	      	&l2_result,
											debug);
	EXPECT_EQ(status, OK);
	EXPECT_EQ(l1_result.miss_hit, HIT_LOAD); 	//Hit in l1
	EXPECT_EQ(l2_result.miss_hit, HIT_LOAD);	//Hit in l2

}


/*
 * TEST L1 Miss L2 Hit: 
 * 1. Choose a random index, associativity and tag
 * 2. Initialize an empty set of the l1 cache
 * 3. Initialize an empty set of the l2 cache
 * 4. Fill a l1 cache with l1_associativity+1 different tags, so the l2_cache also has them, 
 * 	  but the first one gets evicted from the l1_cache.
 * 5. Make a memory access with the first entry, forcing a miss on l1 but a hit on l2.
 */
TEST_F(L2cache,l1_miss_l2_hit){
	int status = OK;
	bool debug = true;
	DEBUG(debug_on, l1_miss_l2_hit_test);

	/*STEP 1: Choose a random index, associativity and tag*/

	//Generate idx for l1 and l2, tag and associativity
	int l1_idx = rand()%256;
	int l2_idx = l1_idx + (256*(1-rand()%2));  //This way if l1_idx = 0x32, l2_idx would be 0x132 or 0x032, adding one bit to the right 
	
	 
	int l1_originalTag = rand()%4096;
	int l1_currentTag = 0;
	int l2_originalTag = l1_originalTag/2;
	int l2_currentTag = 0;
  	int l1_associativity = 1 << (rand()%3);
	int l2_associativity = l1_associativity*2;  
	int loadstore = 0;	


	//Declare structs
	struct l1_l2_entry_info l1_l2_info;
	struct entry l1_cache_blocks[256][l1_associativity];
	struct entry l2_cache_blocks[256*2][l2_associativity];
  	struct operation_result l1_result = {};
	struct operation_result l2_result = {};
	
	/*STEP 2: Initialize an empty L1 cache set*/

	//L1 Cache
	l1_result.miss_hit = MISS_STORE;
  	l1_result.dirty_eviction = false;
  	l1_result.evicted_address = 0;
	for (int i = 0; i < l1_associativity; i++) {
      	l1_cache_blocks[l1_idx][i].valid = false;
      	l1_cache_blocks[l1_idx][i].tag = 0;
      	l1_cache_blocks[l1_idx][i].dirty = 0;
    	l1_cache_blocks[l1_idx][i].rp_value = l1_associativity-1-i;       
	}

	/*STEP 3: Initialize an empty L2 cache set*/

	//L2 Cache
	l2_result.miss_hit = MISS_STORE;
  	l2_result.dirty_eviction = false;
  	l2_result.evicted_address = 0;
	for (int i = 0; i < l2_associativity; i++) {
      	l2_cache_blocks[l2_idx][i].valid = false;
      	l2_cache_blocks[l2_idx][i].tag = 0;
      	l2_cache_blocks[l2_idx][i].dirty = 0;
    	l2_cache_blocks[l2_idx][i].rp_value = l2_associativity-1-i;       
	}
	

	/*Step 4: Overloading one set of the L1 cache and loading L2 cache*/

	for(int i = 0; i<l1_associativity + 1; i++){

		if((l1_originalTag + 10*i) < 4096) {
			l1_currentTag = l1_originalTag + 10*i;
		} else { 
			l1_currentTag = l1_originalTag - 500*i;
		}
		l2_currentTag = l1_currentTag >> 1;

  		/*entry info L2*/
  		l1_l2_info.l1_associativity = l1_associativity;
  		l1_l2_info.l1_tag = l1_currentTag;
  		l1_l2_info.l2_associativity = l2_associativity;
  		l1_l2_info.l2_tag = l2_currentTag;


		status = lru_replacement_policy (l1_idx,
    	                         		l1_currentTag,
        	                     		l1_associativity,
            	                 		loadstore,
                	             		l1_cache_blocks[l1_idx],
                    	         		&l1_result,
                        	     		debug);
		EXPECT_EQ(status, OK);
		
		

		status = lru_replacement_policy_l1_l2(&l1_l2_info,
      	                        	      	loadstore,
        	                      	      	l1_cache_blocks[l1_idx],
          	                    	      	l2_cache_blocks[l2_idx],
            	                  	     	&l1_result,
											&l2_result,
              	                	      	debug);
		EXPECT_EQ(status, OK);
	}


	/*STEP 5: Making a memory access with the first tag, getting a hit on L1 and VC*/

	status = lru_replacement_policy (l1_idx,
    	                         	l1_originalTag,
        	                     	l1_associativity,
            	                 	loadstore,
                	             	l1_cache_blocks[l1_idx],
                    	         	&l1_result,
                        	     	debug);
	EXPECT_EQ(status, OK);

	
  	l1_l2_info.l2_tag = l2_originalTag;
	l1_l2_info.l1_tag = l1_originalTag;
		
	status = lru_replacement_policy_l1_l2(&l1_l2_info,
      	                        	      	loadstore,
        	                      	      	l1_cache_blocks[l1_idx],
          	                    	      	l2_cache_blocks[l2_idx],
            	                  	     	&l1_result,
              	                	      	&l2_result,
											debug);
	EXPECT_EQ(status, OK);
	EXPECT_EQ(l1_result.miss_hit, MISS_LOAD); 	//Miss in l1
	EXPECT_EQ(l2_result.miss_hit, HIT_LOAD);	//Hit in l2

}

/*
 * TEST L1 Miss L2 Miss: 
 * 1. Choose a random index, associativity and tag
 * 2. Initialize an empty set of the l1 cache
 * 3. Initialize an empty set of the l2 cache
 * 4. Fill a l1 cache with l1_associativity*2+1 different tags, so the first tag will get evicted from both the
 	l1 and l2 cache.
 * 5. Make a memory access with the first entry, forcing a miss on l1 and l2.
 */
TEST_F(L2cache,l1_miss_l2_miss){
	int status = OK;
	bool debug = true;
	DEBUG(debug_on, l1_miss_l2_miss_test);

	/*STEP 1: Choose a random index, associativity and tag*/

	//Generate idx for l1 and l2, tag and associativity
	int l1_idx = rand()%256;
	int l2_idx = l1_idx + (256*(1-rand()%2));  //This way if l1_idx = 0x32, l2_idx would be 0x132 or 0x032, adding one bit to the right 
	 
	int l1_originalTag = rand()%4096;
	int l1_currentTag = 0;
	int l2_originalTag = l1_originalTag/2;
	int l2_currentTag = 0;
  	int l1_associativity = 1 << (rand()%3);
	int l2_associativity = l1_associativity*2;  
	int loadstore = 0;	


	//Declare structs
	struct l1_l2_entry_info l1_l2_info;
	struct entry l1_cache_blocks[256][l1_associativity];
	struct entry l2_cache_blocks[256*2][l2_associativity];
  	struct operation_result l1_result = {};
	struct operation_result l2_result = {};
	
	/*STEP 2: Initialize an empty L1 cache set*/

	//L1 Cache
	l1_result.miss_hit = MISS_STORE;
  	l1_result.dirty_eviction = false;
  	l1_result.evicted_address = 0;
	for (int i = 0; i < l1_associativity; i++) {
      	l1_cache_blocks[l1_idx][i].valid = false;
      	l1_cache_blocks[l1_idx][i].tag = 0;
      	l1_cache_blocks[l1_idx][i].dirty = 0;
    	l1_cache_blocks[l1_idx][i].rp_value = l1_associativity-1-i;       
	}

	/*STEP 3: Initialize an empty L2 cache set*/

	//L2 Cache
	l2_result.miss_hit = MISS_STORE;
  	l2_result.dirty_eviction = false;
  	l2_result.evicted_address = 0;
	for (int i = 0; i < l2_associativity; i++) {
      	l2_cache_blocks[l2_idx][i].valid = false;
      	l2_cache_blocks[l2_idx][i].tag = 0;
      	l2_cache_blocks[l2_idx][i].dirty = 0;
    	l2_cache_blocks[l2_idx][i].rp_value = l2_associativity-1-i;       
	}
	

	/*Step 4: Overloading one set of the L1 cache and L2 cache, so one tag gets evicted*/

	for(int i = 0; i<l1_associativity*2 + 1; i++){

		if((l1_originalTag + 10*i) < 4096) {
			l1_currentTag = l1_originalTag + 10*i;
		} else { 
			l1_currentTag = l1_originalTag - 500*i;
		}
		l2_currentTag = l1_currentTag >> 1;

  		/*entry info L2*/
  		l1_l2_info.l1_associativity = l1_associativity;
  		l1_l2_info.l1_tag = l1_currentTag;
  		l1_l2_info.l2_associativity = l2_associativity;
  		l1_l2_info.l2_tag = l2_currentTag;


		status = lru_replacement_policy (l1_idx,
    	                         		l1_currentTag,
        	                     		l1_associativity,
            	                 		loadstore,
                	             		l1_cache_blocks[l1_idx],
                    	         		&l1_result,
                        	     		debug);
		EXPECT_EQ(status, OK);
		
		

		status = lru_replacement_policy_l1_l2(&l1_l2_info,
      	                        	      	loadstore,
        	                      	      	l1_cache_blocks[l1_idx],
          	                    	      	l2_cache_blocks[l2_idx],
            	                  	     	&l1_result,
											&l2_result,
              	                	      	debug);
		EXPECT_EQ(status, OK);
	}


	/*STEP 5: Making a memory access with the first tag, getting a miss on both L1 and VC*/

	status = lru_replacement_policy (l1_idx,
    	                         	l1_originalTag,
        	                     	l1_associativity,
            	                 	loadstore,
                	             	l1_cache_blocks[l1_idx],
                    	         	&l1_result,
                        	     	debug);
	EXPECT_EQ(status, OK);

	
  	l1_l2_info.l2_tag = l2_originalTag;
	l1_l2_info.l1_tag = l1_originalTag;
		
	status = lru_replacement_policy_l1_l2(&l1_l2_info,
      	                        	      	loadstore,
        	                      	      	l1_cache_blocks[l1_idx],
          	                    	      	l2_cache_blocks[l2_idx],
            	                  	     	&l1_result,
              	                	      	&l2_result,
											debug);
	EXPECT_EQ(status, OK);
	EXPECT_EQ(l1_result.miss_hit, MISS_LOAD); 	//Miss in l1
	EXPECT_EQ(l2_result.miss_hit, MISS_LOAD);	//Miss in l2

}

/*
 * TEST L1 Entry Invalid: 
 * 1. Choose a random index, associativity and tag
 * 2. Initialize an empty set of the l1 cache
 * 3. Initialize an empty set of the l2 cache
 * 4. Load the Cache l1.
 * 4. Invalid the first of those entries.
 * 5. Verify the invalid bit of the entry.
 */
TEST_F(L2cache,l1_entry_invalid){
	int status = OK;
	bool debug = true;
	DEBUG(debug_on, l1_entry_invalid_test);

	/*STEP 1: Choose a random index, associativity and tag*/

	//Generate idx for l1 and l2, tag and associativity
	int l1_idx = rand()%256;
	int l2_idx = l1_idx + (256*(1-rand()%2));  //This way if l1_idx = 0x32, l2_idx would be 0x132 or 0x032, adding one bit to the right 
	
	 
	int l1_originalTag = rand()%4096;
	int l1_currentTag = 0;
	int l2_originalTag = l1_originalTag/2;
	int l2_currentTag = 0;
  	int l1_associativity = 1 << (rand()%3);
	int l2_associativity = l1_associativity*2;  
	int loadstore = 0;	


	//Declare structs
	struct l1_l2_entry_info l1_l2_info;
	struct entry l1_cache_blocks[256][l1_associativity];
	struct entry l2_cache_blocks[256*2][l2_associativity];
  	struct operation_result l1_result = {};
	struct operation_result l2_result = {};
	
	/*STEP 2: Initialize an empty L1 cache set*/

	//L1 Cache
	l1_result.miss_hit = MISS_STORE;
  	l1_result.dirty_eviction = false;
  	l1_result.evicted_address = 0;
	for (int i = 0; i < l1_associativity; i++) {
      	l1_cache_blocks[l1_idx][i].valid = false;
      	l1_cache_blocks[l1_idx][i].tag = 0;
      	l1_cache_blocks[l1_idx][i].dirty = 0;
    	l1_cache_blocks[l1_idx][i].rp_value = l1_associativity-1-i;       
	}

	/*STEP 3: Initialize an empty L2 cache set*/

	//L2 Cache
	l2_result.miss_hit = MISS_STORE;
  	l2_result.dirty_eviction = false;
  	l2_result.evicted_address = 0;
	for (int i = 0; i < l2_associativity; i++) {
      	l2_cache_blocks[l2_idx][i].valid = false;
      	l2_cache_blocks[l2_idx][i].tag = 0;
      	l2_cache_blocks[l2_idx][i].dirty = 0;
    	l2_cache_blocks[l2_idx][i].rp_value = l2_associativity-1-i;       
	}
	

	/*Step 4: Loading one set of the L1 cache and L2 cache*/

	for(int i = 0; i<l1_associativity; i++){

		if((l1_originalTag + 10*i) < 4096) {
			l1_currentTag = l1_originalTag + 10*i;
		} else { 
			l1_currentTag = l1_originalTag - 500*i;
		}
		l2_currentTag = l1_currentTag >> 1;

  		/*entry info L2*/
  		l1_l2_info.l1_associativity = l1_associativity;
  		l1_l2_info.l1_tag = l1_currentTag;
  		l1_l2_info.l2_associativity = l2_associativity;
  		l1_l2_info.l2_tag = l2_currentTag;


		status = lru_replacement_policy (l1_idx,
    	                         		l1_currentTag,
        	                     		l1_associativity,
            	                 		loadstore,
                	             		l1_cache_blocks[l1_idx],
                    	         		&l1_result,
                        	     		debug);
		EXPECT_EQ(status, OK);
		
		
		status = lru_replacement_policy_l1_l2(&l1_l2_info,
      	                        	      	loadstore,
        	                      	      	l1_cache_blocks[l1_idx],
          	                    	      	l2_cache_blocks[l2_idx],
            	                  	     	&l1_result,
											&l2_result,
              	                	      	debug);
		EXPECT_EQ(status, OK);
	}


	/*STEP 5: Invalid the first entry*/

	status = l1_line_invalid_set(l1_originalTag,
                        		l1_associativity,
                        		l1_cache_blocks[l1_idx],
                        		debug);
	EXPECT_EQ(status, OK);

	/*STEP 6: Checking for it to be invalid*/

	EXPECT_EQ(l1_cache_blocks[l1_idx][l1_associativity-1].valid, false);

}

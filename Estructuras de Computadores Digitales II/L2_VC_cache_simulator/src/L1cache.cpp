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

#define KB 1024
#define ADDRSIZE 32
using namespace std;
int field_size_get( int cachesize_kb,
                    int associativity,
                    int blocksize_bytes,
                    int *tag_size,
                    int *idx_size,
                    int *offset_size){
   *offset_size = log2 (blocksize_bytes);
   *idx_size = log2((cachesize_kb*KB)/(associativity*blocksize_bytes));
   *tag_size = ADDRSIZE - *idx_size - *offset_size;
   return OK;
}

void address_tag_idx_get(long address,
                        int tag_size,
                        int idx_size,
                        int offset_size,
                        int *idx,
                        int *tag)
{
unsigned int indexMask = address;
unsigned int tagMask = address;

indexMask = indexMask<<tag_size;
*idx = indexMask>>(tag_size+offset_size);
tagMask = tagMask>>(idx_size+offset_size);
*tag = tagMask;

}

int srrip_replacement_policy (int idx,
                             int tag,
                             int associativity,
                             bool loadstore,
                             entry* cache_blocks,
                             operation_result* result,
                             bool debug)
{
   return ERROR;
}


int lru_replacement_policy (int idx,
                             int tag,
                             int associativity,
                             bool loadstore,
                             entry* cache_blocks,
                             operation_result* result,
                             bool debug)
{
   if((associativity%2 == 1 && associativity != 1) || associativity == 0) {
      return ERROR;
   }  
   if(tag < 0 || idx < 0) {
      return ERROR;
   } 


   bool gotHit = false;
   int hit_way;
   int evicted_way = 0;
   int rp_value_reg[associativity];


   //revisa los ways
   for (int i = 0; i < associativity; i++)
   {
      rp_value_reg[i] = cache_blocks[i].rp_value; //guardo en un array los valores de los rp_values (me sirve para el miss)
      if(cache_blocks[i].tag == tag && cache_blocks[i].valid){
         gotHit = true;
         hit_way = i; //guardo en hit_way el way en que hubo hit
         result->evicted_address = hit_way; //cambio
      }
   }  
   if(gotHit){
////////////////////////////////// HIT
      //Resultados (tipo de hit, dirty eviction y evicted address)
      if (loadstore){
         result->miss_hit = HIT_STORE;
         cache_blocks[hit_way].dirty = true;
      } else {
         result->miss_hit = HIT_LOAD;
      }
      // Cambio de los valores de RP
      cache_blocks[hit_way].rp_value = 0;
      for(int j = 0; j<associativity; j++){
         if((int)cache_blocks[j].rp_value < (int)cache_blocks[hit_way].rp_value){
            cache_blocks[j].rp_value++;
         }
      }
   }
   else {
      

//////////////////////////// MISS
   //resultados
      if (loadstore){
         result->miss_hit = MISS_STORE;  
         // result->dirty_eviction = true; // ???
      } else {
         result->miss_hit = MISS_LOAD;
      }

      for (int i = 0; i < associativity; i++)
      {   
         if(cache_blocks[i].rp_value == associativity-1 || cache_blocks[i].valid == false){
            evicted_way = i;
            result->evicted_way = evicted_way;
         }
         else{
            cache_blocks[i].rp_value++;
         }
      }

   
      /*sustitucion del bloque*/
      result->evicted_address = cache_blocks[evicted_way].tag;
      cache_blocks[evicted_way].valid = true;
      cache_blocks[evicted_way].rp_value = 0; 
      // cache_blocks[evicted_way].tag = tag;

      /*resultado dirty*/
      if(loadstore){
         cache_blocks[evicted_way].dirty = true;
      } else{
         cache_blocks[evicted_way].dirty = false;
      }

   }
   return OK;
}

int l1_line_invalid_set(int tag,
                        int associativity,
                        entry* cache_blocks,
                        bool debug){
   for (int i = 0; i<associativity; i++){
      if (cache_blocks[i].tag == tag){
         cache_blocks[i].valid = false;
      }
   }
   return OK;
}
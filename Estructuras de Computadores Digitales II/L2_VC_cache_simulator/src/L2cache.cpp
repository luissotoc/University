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
#include <L2cache.h>

#define KB 1024
#define ADDRSIZE 32
using namespace std;


int lru_replacement_policy_l1_l2(const l1_l2_entry_info *l1_l2_info,
																 bool loadstore,
																 entry* l1_cache_blocks,
																 entry* l2_cache_blocks,
																 operation_result* l1_result,
																 operation_result* l2_result,
																 bool debug) 
{
int l1_hit_way = l1_result->evicted_address;
int l2_hit_way = 0;
int l1_evicted_way = l1_result->evicted_way;
int l2_evicted_way = 0;
bool l2_hit = false;

l2_result->dirty_eviction = false;

if(l1_result->miss_hit == HIT_STORE || l1_result->miss_hit == HIT_LOAD){  /*Hit en L1*/
   /* se sabe que si hay hit en L1, el dato tambien esta en L2*/
   /* si hay hit store, se debe actualizar L2*/
   /* lru replacement policy para el bloque que tuvo hit en L1 */
   
 
   for (int i = 0; i < (l1_l2_info->l2_associativity); i++)
   {
      
      if(l2_cache_blocks[i].tag == l1_l2_info->l2_tag && l2_cache_blocks[i].valid){
         l2_hit_way = i; //guardo en hit_way el way en que hubo hit
         l2_result->evicted_address = l2_hit_way;
      }
   }
   if(!loadstore){
      l2_result->miss_hit = HIT_LOAD;
   } 
   else
   {
      l2_result->miss_hit = HIT_STORE;
      l2_cache_blocks[l2_hit_way].dirty = true;
      // actualizacion L2 (write through)
      l2_cache_blocks[l2_hit_way].tag = l1_l2_info->l2_tag;
      l2_cache_blocks[l2_hit_way].valid = true;
   }

   // Cambio de los valores de RP
   l2_cache_blocks[l2_hit_way].rp_value = 0;
   for(int j = 0; j<l1_l2_info->l2_associativity; j++){
      if((int)l2_cache_blocks[j].rp_value < (int)l2_cache_blocks[l2_hit_way].rp_value){
         l2_cache_blocks[j].rp_value++;
      }
   }
}
   
if(l1_result->miss_hit == MISS_STORE || l1_result->miss_hit == MISS_LOAD){  /*miss en L1*/
   /* revisar si dato requerido esta en L2 */
   l2_hit = false;
   for (int i = 0; i<l1_l2_info->l2_associativity; i++){
      if(l2_cache_blocks[i].tag == l1_l2_info->l2_tag && l2_cache_blocks[i].valid){
         l2_hit_way = i;
         l2_result->evicted_address = l2_hit_way;
         l2_hit = true;
      }
   }

   /* HIT en L2 */
   if(l2_hit){
      /*actualizar lru de l2*/
      if (loadstore){
         l2_result->miss_hit = HIT_STORE;
         l2_cache_blocks[l2_hit_way].dirty = true;
      } else {
         l2_result->miss_hit = HIT_LOAD;
      }
      l2_cache_blocks[l2_hit_way].rp_value = 0;
      for(int j = 0; j<l1_l2_info->l2_associativity; j++){
         if((int)l2_cache_blocks[j].rp_value < (int)l2_cache_blocks[l2_hit_way].rp_value){
            l2_cache_blocks[j].rp_value++;
         }
      }
      /*actualizar dato de l1 con el de l2*/
      l1_cache_blocks[l1_evicted_way].dirty = false;
      l1_cache_blocks[l1_evicted_way].rp_value = 0;  //nota: ya el lru se habia actualizdo en L1, esta inst puede ser reduntante
      l1_cache_blocks[l1_evicted_way].tag = l1_l2_info->l1_tag; // ??? l1? 
      l1_cache_blocks[l1_evicted_way].valid = true;

   }
   /*MISS en L2*/
   else{
      if (loadstore){
         l2_result->miss_hit = MISS_STORE;  
      } else {
         l2_result->miss_hit = MISS_LOAD;
      }

      for (int i = 0; i < l1_l2_info->l2_associativity; i++)
      {   
         if(l2_cache_blocks[i].rp_value == l1_l2_info->l2_associativity-1 || l2_cache_blocks[i].valid == false){
            l2_evicted_way = i;
         }
         else{
            l2_cache_blocks[i].rp_value++;
         }
      }

      /*logica dirty*/
      if(l2_cache_blocks[l2_evicted_way].dirty){
         l2_result->dirty_eviction = true;
      } else{
         l2_result->dirty_eviction = false;
      }
      

      /*sustitucion del bloque L2*/
      l2_result->evicted_address = l2_cache_blocks[l2_evicted_way].tag;
      l2_cache_blocks[l2_evicted_way].tag = l1_l2_info->l2_tag;
      l2_cache_blocks[l2_evicted_way].valid = true;
      l2_cache_blocks[l2_evicted_way].rp_value = 0; 

      // /*invalidar el bloque en L1*/
      // l1_line_invalid_set(l1_l2_info->l1_tag, l1_l2_info->l1_associativity, l1_cache_blocks, false); // solo sirve en hardware

      /*sustitucion del bloque L1*/
      l1_result->evicted_address = l1_cache_blocks[l1_evicted_way].tag;
      l1_cache_blocks[l1_evicted_way].tag = l1_l2_info->l1_tag;
      l1_cache_blocks[l1_evicted_way].valid = true;
      l1_cache_blocks[l1_evicted_way].rp_value = 0; 

      /*resultado dirty*/
      if(loadstore){
         l2_cache_blocks[l2_evicted_way].dirty = true;
      } else{
         l2_cache_blocks[l2_evicted_way].dirty = false;
      }
   }
}

   return OK;
}

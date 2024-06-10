#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <math.h>
#include <L1cache.h>
#include <L2cache.h>
#include <Victimcache.h>
#include <debug_utilities.h>

using namespace std;
#define KB 1024


void print_usage ()
{
  printf ("Print print_usage\n");
  exit (0);
}

int main(int argc, char * argv []) {

  /* Lectura de argumentos */
  int l1_cacheSize;
  int blockSize;
  int l1_associativity; 
  int opt_int;
  string optimizacion;
  string salida;
  string options[5] = {"-t","-l","-a","-opt"};
  //Recorre los argumentos de entrada
  for (int i = 0; i < 9; i++)
  {
      //Si el argumento es -t guarda el siguiente valor en CacheSize
      if (argv[i] == options[0])
      {
        l1_cacheSize = atoi(argv[i+1]);
      }
      //Si el argumento es -l guarda el siguiente valor en BlockSize
      if (argv[i] == options[1])
      {
        blockSize = atoi(argv[i+1]);
      }
      //Si el argumento es -a guarda el siguiente valor en Associativity
      if (argv[i] == options[2])
      {
        l1_associativity = atoi(argv[i+1]);
      }
      //Si el argumento es -opt guarda el siguiente valor en Optimizacion
      if (argv[i] == options[3])
      {
        optimizacion = argv[i+1];
      }
  }


  if(optimizacion == "l2") opt_int = L2;
  if(optimizacion == "vc") opt_int = VICTIMCACHE;
  int l2_associativity = 2*l1_associativity;
  int l2_cacheSize = 4*l1_cacheSize;
  /* otros valores importantes*/ 
  int l1_byteOffsetSize;
  int l1_indexSize;
  int l1_tagSize;
  int l1_state;
  int l1_index;
  int l1_tag;

  int l2_byteOffsetSize;
  int l2_indexSize;
  int l2_tagSize;
  int l2_state;
  int l2_index;
  int l2_tag;

  int address;

  /* contadoers para cache multinivel */
  double overall_miss_rate = 0;
  double l1_miss_rate = 0;
  double l2_miss_rate = 0;
  double global_miss_rate = 0;
  int l1_misses = 0;
  int l1_hits = 0;
  int l2_misses = 0;
  int l2_hits = 0;
  int l2_dirty_evictions = 0;
  


  /*contadores para victimcache*/
  double miss_rate_l1_vc = 0.0;
  double vc_miss_rate = 0.0;
  int misses_l1_vc = 0;
  int hits_l1_vc = 0;
  int vc_hits = 0;
  int vc_misses = 0;
  int vc_dirty_evictions = 0;

  bool loadStore = false;
  bool gotHit = false;
  string datos; 
  
  /* calculo de tag, index, offset para l1 y l2 */
  l1_state = field_size_get(l1_cacheSize,
                l1_associativity,
                blockSize,
                &l1_tagSize,
                &l1_indexSize,
                &l1_byteOffsetSize);
  l2_state = field_size_get(l2_cacheSize,
                l2_associativity,
                blockSize,
                &l2_tagSize,
                &l2_indexSize,
                &l2_byteOffsetSize);


  /*CACHE L1*/
  entry l1_cache[(l1_cacheSize*KB)/(l1_associativity*blockSize)][l1_associativity];
  operation_result l1_op;
  l1_op.miss_hit = MISS_STORE;
  l1_op.dirty_eviction = false;
  l1_op.evicted_address = 0;
  for (int i = 0; i < (l1_cacheSize*KB)/(l1_associativity*blockSize); i++){
    for (int j = 0; j < l1_associativity; j++)
    {
      l1_cache[i][j].valid = false;
      l1_cache[i][j].tag = 0;
      int l1_initial_rp = l1_associativity - 1;
      l1_cache[i][j].rp_value = l1_initial_rp;
        if (l1_initial_rp != -1){
        l1_initial_rp--;
        if(l1_initial_rp == -1){
          l1_initial_rp = l1_associativity-1;
        }
      }
    }
  }

  /*CACHE l2*/
  entry l2_cache[(l2_cacheSize*KB)/(l2_associativity*blockSize)][l2_associativity];
  operation_result l2_op;
  l1_l2_entry_info l1_l2_entry_info;
  l1_vc_entry_info vc_entry_info;
  l2_op.miss_hit = MISS_STORE;
  l2_op.dirty_eviction = false;
  l2_op.evicted_address = 0;
  for (int i = 0; i < (l2_cacheSize*KB)/(l2_associativity*blockSize); i++){
    for (int j = 0; j < l2_associativity; j++)
    {
      l2_cache[i][j].valid = false;
      l2_cache[i][j].tag = 0;
      l2_cache[i][j].dirty = false;
      int l2_initial_rp = l2_associativity - 1;
      l2_cache[i][j].rp_value = l2_initial_rp;
        if (l2_initial_rp != -1){
        l2_initial_rp--;
        if(l2_initial_rp == -1){
          l2_initial_rp = l2_associativity-1;
        }
      }
    }
  }

  /*Victim cache*/
  entry vc_cache[1][16];
  operation_result vc_op;
  vc_op.evicted_address = 0;
  vc_op.miss_hit = MISS_STORE;
  for(int i = 0;i<16;i++){
    vc_cache[0][i].valid = false;
    vc_cache[0][i].dirty = false;
    vc_cache[0][i].rp_value = 0;
    vc_cache[0][i].tag = 0;
  }

  /* Get trace's lines and start your simulation */
  int count = 0;
  while(cin){
    // for(count = 0; count < 1000000; count++){
    /* #*/
    cin >> datos;
    
    /*if(datos == "1"){
      break;
    }*/
    /*load o store*/
    cin >> datos;
    if(datos == "0"){
      loadStore = false;
    }
    if(datos == "1"){
      loadStore = true;
    }
    /*address*/
    cin >> datos; 
    address = stol(datos, nullptr, 16);
    
    /*IC*/
    cin >> datos;

    /* calculo de tag, index para l1 y l2*/ 
    address_tag_idx_get(address,
                        l1_tagSize,
                        l1_indexSize,
                        l1_byteOffsetSize,
                        &l1_index,
                        &l1_tag);
    address_tag_idx_get(address,
                        l2_tagSize,
                        l2_indexSize,
                        l2_byteOffsetSize,
                        &l2_index,
                        &l2_tag);


  /*entry info L2*/
  l1_l2_entry_info.l1_associativity = l1_associativity;
  l1_l2_entry_info.l1_tag = l1_tag;
  l1_l2_entry_info.l2_associativity = l2_associativity;
  l1_l2_entry_info.l2_tag = l2_tag;

  /*entry info VC*/
  vc_entry_info.l1_associativity = l1_associativity;
  vc_entry_info.l1_idx = l1_index;
  vc_entry_info.l1_tag = l1_tag;
  vc_entry_info.vc_associativity = 16;


  switch (opt_int)
  {


  case L2:
  
    /*check L1 cache */
    lru_replacement_policy(l1_index, l1_tag, l1_associativity, loadStore, l1_cache[l1_index], &l1_op, false);

    lru_replacement_policy_l1_l2(&l1_l2_entry_info, loadStore, l1_cache[l1_index], l2_cache[l2_index],&l1_op,&l2_op, false);
    
    
    if(l1_op.miss_hit == MISS_STORE || l1_op.miss_hit == MISS_LOAD) {
      l1_misses++;}
    if(l1_op.miss_hit == HIT_STORE || l1_op.miss_hit == HIT_LOAD) {
      l1_hits++;}

    if(l2_op.miss_hit == MISS_STORE || l2_op.miss_hit == MISS_LOAD) {l2_misses++;}
    if(l2_op.miss_hit == HIT_STORE || l2_op.miss_hit == HIT_LOAD) {l2_hits++;}
  
    if(l2_op.dirty_eviction == true) {l2_dirty_evictions++;}

    break;


  case VICTIMCACHE:

    /*check L1 cache*/

    lru_replacement_policy(l1_index, l1_tag, l1_associativity, loadStore, l1_cache[l1_index], &l1_op, false);
  
    fifo_replacement_policy_l1_vc(&vc_entry_info, loadStore, l1_cache[l1_index], vc_cache[0], &l1_op, &vc_op, false);
    
    if(l1_op.miss_hit == MISS_LOAD  || l1_op.miss_hit == MISS_STORE){
      l1_misses++;
      misses_l1_vc++;
    };
    if(l1_op.miss_hit == HIT_LOAD  || l1_op.miss_hit == HIT_STORE){
      l1_hits++;
      hits_l1_vc++;
    };
    
    if(vc_op.miss_hit == MISS_STORE || vc_op.miss_hit == MISS_LOAD){
      misses_l1_vc++;
      vc_misses++;
    };

    if(vc_op.miss_hit == HIT_STORE || vc_op.miss_hit == HIT_LOAD){
      vc_hits++; 
      hits_l1_vc++;
    };


    if(vc_op.dirty_eviction == true){ vc_dirty_evictions++; }
    
    break;



  default:  ; 

  }
  }

    miss_rate_l1_vc = (double)misses_l1_vc/((double)misses_l1_vc+(double)hits_l1_vc);
    l2_miss_rate = (double)l2_misses/((double)l2_misses+(double)l2_hits)*100;
    l1_miss_rate = (double)l1_misses/((double)l1_misses+(double)l1_hits)*100;
    global_miss_rate = (double)l2_misses/((double)l1_misses+(double)l1_hits)*100;
    vc_miss_rate = (double)vc_misses/((double)vc_misses+(double)vc_hits)*100;


  /* parametros cache */
    cout << "---------------------------" << endl;
    cout << "|  PARAMETROS DE LA CACHE |" << endl;
    cout << "---------------------------" << endl;
  if(opt_int == L2){
    cout << "Tipo de optimizacion: Cache multinivel" << endl;
    cout << "Tamaño de la caché L1: " << l1_cacheSize << "KB \n";
    cout << "Tamaño de la caché L2: " << l2_cacheSize << "KB \n";
    cout << "Asociatividad L1: " << l1_associativity << " ways \n";
    cout << "Asociatividad L2: " << l2_associativity << " ways \n";
    cout << "Tamaño del bloque: " << blockSize << "bytes \n";
    cout << "---------------------------" << endl;
  }
  else if(opt_int == VICTIMCACHE){
    cout << "Tipo de optimizacion: Victim Cache" << endl;
    cout << "Tamaño de la caché L1: " << l1_cacheSize << "KB \n";
    cout << "Asociatividad L1: " << l1_associativity << " ways \n";
    cout << "Tamaño del bloque: " << blockSize << "bytes \n";
    cout << "---------------------------" << endl;
  } else return ERROR;

  /* resultados */
    cout << "---------------------------" << endl;
    cout << "| RESULTADOS DE SIMULACION |" << endl;
    cout << "---------------------------" << endl;
  if(opt_int == L2){
    cout << "L1 miss rate: " << l1_miss_rate << endl;
    cout << "L2 miss rate: " << l2_miss_rate << endl;
    cout << "Global miss rate: " << global_miss_rate << endl;
    cout << "L1 misses: " << l1_misses << endl;
    cout << "L1 hits: " << l1_hits << endl;
    cout << "L2 misses: " << l2_misses << endl;
    cout << "L2 hits: " << l2_hits << endl;
    cout << "L2 dirty evictions: " << l2_dirty_evictions << endl;
    cout << "---------------------------" << endl;
  }
  else if(opt_int == VICTIMCACHE){
    cout << "Miss rate (L1 + VC): " << fixed << miss_rate_l1_vc << endl;
    cout << "Misses (L1 + VC): " <<  misses_l1_vc << endl;
    cout << "Hits (L1 + VC): " << hits_l1_vc << endl;
    cout << "Victim cache hits: " << vc_hits << endl;
    cout << "VC dirty evictions: " << vc_dirty_evictions << endl;
    cout << "Miss rate L1: " <<  l1_miss_rate << endl;
    cout << "Misses L1: " <<  l1_misses << endl;
    // cout << "Miss rate VC: " <<  vc_miss_rate << endl;
    // cout << "Misses VC: " <<  vc_misses << endl;
  } else return ERROR;


return 0;
}

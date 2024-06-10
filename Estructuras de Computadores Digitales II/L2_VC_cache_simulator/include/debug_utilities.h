/*
 * 	Cache simulation project
 * 	Class UCR IE-521
 * 	Semester: I-2019
*/
#ifndef DEBUG_UTILITIES
#define DEBUG_UTILITIES
#include "../include/L1cache.h"

#define CYN "\x1B[36m"
#define RESET "\x1B[0m"
#define YEL   "\x1B[33m"

/* MACROS */
#define DEBUG(y,x) if (y) printf(CYN "[INFO]:" RESET " %s\n",#x) 

/* FUNCTIONS */

/* Get enviroment var */
void get_env_var( const char* var_name,
                 int *var_value );

/* Print way info */
void print_way_info(int idx,
                    int associativity,
                    entry* cache_blocks); 
#endif

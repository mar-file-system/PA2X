/*
 * 
 *   example-3.c
 *
 *   XML config file parser example 2
 *
 *   Ron Croonenberg rocr@lanl.gov
 *   High Performance Computing (HPC-3)
 *   Los Alamos National Laboratory
 *
 *
 *   6-8-2015: initial start
 *  
 *
 *
 */

//
//    This example code is based on the ./config/config-2 example configuration
//    This example code is based on the ./config/config-2 example configuration
//    This example code is based on the ./config/config-2 example configuration
//    This example code is based on the ./config/config-2 example configuration
//    This example code is based on the ./config/config-2 example configuration
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "parse-inc/config-structs.h"

#include "confpars-structs.h"
#include "confpars.h"
#include "parse-types.h"



main(int argc, char *argv[])
{
struct config *config;
struct line h_page, pseudo_h, fld_nm_lst;
struct varNameTypeList vNTL;
int idx, idx2;

config = (struct config *)malloc(sizeof(struct config));

memset(config,      0x00, sizeof(struct config));
memset(&h_page,     0x00, sizeof(struct line));							// don't want any pointers on the loose
memset(&pseudo_h,   0x00, sizeof(struct line));							// ditto
memset(&fld_nm_lst, 0x00, sizeof(struct line));							// ditto


if (argc == 2) {
   parseConfigFile(argv[1], CREATE_STRUCT_PATHS, &h_page, &fld_nm_lst, config, &vNTL, VERBOSE);	// create the structure paths for parsing/populating verbose
   freeHeaderFile(h_page.next);									// free the header page

   // play with the config structire a bit
   printf("\nconfig name:  |%s|\n", config->config_name);
   idx = 0;
   while (config->repo[idx] != (struct repo *)NULL) {
      printf("repo[%d] name: %s\n", idx, config->repo[idx]->name);
      idx++;
      }
   idx = 0; idx2 = 0;
   while (config->namespace[idx] != (struct namespace *)NULL) {
      printf("namespace[%d] name: %s\n", idx, config->namespace[idx]->name);
      idx2=0;
      while (config->namespace[idx]->brepo_list.range[idx2] != (struct range *)NULL) {
         printf("   range[%d] name: %s\n", idx2, config->namespace[idx]->brepo_list.range[idx2]->brepo);
         idx2++;
         }
      idx++;
      }

   //freeConfigStructContent(config, &vNTL, QUIET);								// free the config structure
   freeConfigStructContent(config, &vNTL, VERBOSE);
   }
else
   printf("Usage: ./example-3 <config file>\n");
}

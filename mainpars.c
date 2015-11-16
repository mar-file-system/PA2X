/*
 * 
 *   mainpars.c
 *
 *   main for config file parser 
 *
 *   Ron Croonenberg rocr@lanl.gov
 *   High Performance Computing (HPC-3)
 *   Los Alamos National Laboratory
 *
 *
 *   6-8-2015:
 *    - initial start 
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#ifdef DATAPARSE
#include "parse-inc/config-structs.h"
#endif

#include "confpars-structs.h"
#include "confpars.h"
#include "parse-types.h"

main(int argc, char *argv[])
{
struct line h_page, pseudo_h, fld_nm_lst;
struct varNameTypeList vNTL;
int task;

#ifdef DATAPARSE
struct config *config;
config = (struct config *)malloc(sizeof(struct config));
#endif

#ifdef MD5_CONFIG_CHECK
char *xml_conf;
#endif


memset(&h_page,     0x00, sizeof(struct line));							// don't want any pointers on the loose
memset(&pseudo_h,   0x00, sizeof(struct line));							// ditto
memset(&fld_nm_lst, 0x00, sizeof(struct line));							// ditto

//task = DISPLAY;
task = CREATE_STRUCT;

if (argc == 2) {
#ifdef DATAPARSE
   //parseConfigFile(argv[1], CREATE_STRUCT,      &h_page, &fld_nm_lst, config, VERBOSE);	// This example show the "raw" structure of what we parse
   //listHeaderFile(&h_page, &vNTL, NO_ORDER);								// not very usefull to the end user.
   //freeHeaderFile(h_page.next);								// free the header page
   //memset(&h_page, 0x00, sizeof(struct line));						// clear struct
   //freeHeaderFile(fld_nm_lst.next);								// free field names list
   //memset(&fld_nm_lst, 0x00, sizeof(struct line));						// clear struct

   parseConfigFile(argv[1],    CREATE_STRUCT_PATHS, &h_page, &fld_nm_lst, config, &vNTL, QUIET);// create the structure paths for parsing/populating
   freeHeaderFile(h_page.next);									// free the header page
   freeConfigStructContent(config, &vNTL, QUIET);								// free the config structure
#else // create data parser
   parseConfigFile(argv[1], CREATE_STRUCT, &h_page, &fld_nm_lst, &vNTL, VERBOSE);			// parse prep work this parses the XML
   pseudo_h.next = listHeaderFile(&h_page, &vNTL, DECONSTRUCT);					// create C structures from parsed XML into pseudo headers
   vNTLapply(&vNTL, &pseudo_h);
   freevNTL(vNTL.next);
//listHeaderFile(&pseudo_h, &vNTL, NO_ORDER);
   listHeaderFile(&pseudo_h, &vNTL, GEN_PARSE_STRUCTS);						// creates the actual structures and generates confpars-structs.h for the parser

#ifdef MD5_CONFIG_CHECK
   xml_conf = readConfigFile(argv[1]);
   createMD5checksums(xml_conf);
   free(xml_conf);
#endif

//   listHeaderFile(&pseudo_h, &vNTL, NO_ORDER);                                                               // display C structures derived from XML

   freeHeaderFile(h_page.next);									// free the header page
   memset(&h_page, 0x00, sizeof(struct line));
   freeHeaderFile(fld_nm_lst.next);								// free field names list
   memset(&fld_nm_lst, 0x00, sizeof(struct line));
   freeHeaderFile(pseudo_h.next);								// free the pseudo headers
   memset(&pseudo_h, 0x00, sizeof(struct line));

   parseConfigFile(argv[1],    CREATE_STRUCT_PATHS, &h_page, &fld_nm_lst, &vNTL, VERBOSE);		// parse for access paths
   listHeaderFile(&fld_nm_lst, &vNTL, GEN_STRUCT_SWITCH);						// generate access switch
   freeHeaderFile(fld_nm_lst.next);
#endif
   }
else
#ifdef DATAPARSE
   printf("Usage: ./dataparse <XML config file>\n");
#else
   printf("Usage: ./confparse <XML blueprint file>\n");
#endif


//listHeaderFile(&h_page, &vNTL, NO_ORDER_DEBUG);
//listHeaderFile(&h_page, &vNTL, NO_ORDER);
}

/*
 * 
 *   example-4.c
 *
 *   XML config file parser example 2
 *
 *   Ron Croonenberg rocr@lanl.gov
 *   High Performance Computing (HPC-3)
 *   Los Alamos National Laboratory
 *
 *
 *   06-30-2015:	initial start rocr@lanl.gov
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
//    It would work on another config file, but just returns NULL every time.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "confpars-structs.h"
#include "confpars.h"
#include "checksum.h"
#include "./parse-inc/xml-md5-checksums.inc"


main(int argc, char *argv[])
{
char *xml_conf, *cs;

if (argc == 2) {
   xml_conf = readConfigFile(argv[1]);
   cs = md5Checksum(xml_conf);
   if (strcmp(cs, xml_md5_checksums[0]) == 0) {
      printf("The configuration file: %s, is the same as the original configuration.\n", argv[1]);
      free(cs);
      }
   else {
      printf("The configuration: %s, is different from the original configuration.\n", argv[1]);
      free(cs);

      configFileData(xml_conf);
      cs = md5Checksum(xml_conf);
      if (strcmp(cs, xml_md5_checksums[1]) == 0) {
         printf("The data in configuration: %s, is the same as the data in the original config file.\n", argv[1]);
         free(cs);
         }
      else {
         printf("The data in configuration: %s, is different from the data in the original config file.\n", argv[1]);
         free(cs);

         configFileStruct(xml_conf);
         cs = md5Checksum(xml_conf);
         if (strcmp(cs, xml_md5_checksums[2]) == 0)
            printf("The structure of configuration: %s, is the same as the structure of the original config file.\n", argv[1]);
         else
            printf("The structure of configuration: %s, is different from the structure in the original config file.\n", argv[1]);
         free (cs);
         }
      }
   free(xml_conf);
   }
else
   printf("Usage: ./example-5 <XML config file>\n");
}

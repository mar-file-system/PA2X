/*
 * 
 *   checksum.c
 *
 *   checksums for config file parser 
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
#include <openssl/md5.h>

#include "checksum.h"
#ifdef DATAPARSE
#include "./parse-inc/xml-md5-checksums.inc"
#endif



char *configFileStruct(char *blk)
{
char *c_ptr, *cw_ptr, *cr_ptr;

c_ptr = blk;

cw_ptr = strstr(c_ptr, ">");
cw_ptr++;

cr_ptr = strstr(cw_ptr, "<");
while (cr_ptr != (char *)NULL) {
   while (cr_ptr[0] != '>') {
      cw_ptr[0] = cr_ptr[0];
      cw_ptr++;
      cr_ptr++;
      }
   cw_ptr[0] = cr_ptr[0];
   cw_ptr++;
   cr_ptr++;
   cr_ptr = strstr(cw_ptr, "<");
   }
cw_ptr == 0;

return c_ptr;
}



char *configFileData(char *blk)
{
char *conf, *line, *sol_ptr, *eol_ptr, *tr_ptr;

conf = (char *)malloc(strlen(blk)+1);
line = (char *)malloc(strlen(blk)+1);
conf[0]=0;

sol_ptr = blk;
eol_ptr = strstr(sol_ptr, "\n");
while (eol_ptr != (char *)NULL) {
   strncpy(line, sol_ptr, eol_ptr - sol_ptr);
   line[eol_ptr - sol_ptr] = 0;
//   tr_ptr = strstr(line, "#");
//   if (tr_ptr != (char *)NULL)
//      tr_ptr[0] = 0;
   stripWhiteSpace(line);
   if (strlen(line) > 0)
      strcat(conf, line);
   sol_ptr = eol_ptr+1;
   eol_ptr = strstr(sol_ptr, "\n");
   }

free(line);
strcpy(blk, conf);
free(conf);

return blk;
}



char *md5Checksum(char *blk)
{
unsigned char c[MD5_DIGEST_LENGTH], data[1024];
int i;
MD5_CTX mdContext;
char *cs;

cs = (char *)malloc(2 * ((MD5_DIGEST_LENGTH * sizeof(char)) + 1));
if (cs == (char *)NULL)
   return (char *)NULL;

MD5_Init (&mdContext);

MD5_Update (&mdContext, blk, strlen(blk));

MD5_Final (c,&mdContext);
#ifdef DEBUG
printf("int: ");
#endif
for (i = 0; i < MD5_DIGEST_LENGTH; i++) {
#ifdef DEBUG
   printf("%02x", c[i]);
#endif
   sprintf(&(cs[2*i]), "%02x", c[i]);
   }
#ifdef DEBUG
printf("\n");
#endif

return cs;
}



int createMD5checksums(char *cfg)
{
FILE *f_ptr;
char *txt_config, *cs;

txt_config = (char *)malloc( (strlen(cfg) + 1) * sizeof(char));
strcpy(txt_config, cfg);

if (txt_config != (char *)NULL) {
   f_ptr = fopen("./parse-inc/xml-md5-checksums.inc", "w");

   fprintf(f_ptr, "char *xml_md5_checksums[3] = { ");
   cs = md5Checksum(txt_config);
   fprintf(f_ptr, "\"%s\",", cs);
   free(cs);

   configFileData(txt_config);
   cs = md5Checksum(txt_config);
   fprintf(f_ptr, "\"%s\",", cs);
   free(cs);

   configFileStruct(txt_config);
   cs = md5Checksum(txt_config);
   fprintf(f_ptr, "\"%s\" };", cs);
   free(cs);
   free(txt_config);
   }
}


#ifdef DATAPARSE
#ifdef MD5_CONFIG_CHECK
int configMD5Checksums(char *cfg, char *fn)
{
char *xml_conf, *cs;
int abort = 0;

xml_conf = (char *)malloc((strlen(cfg) +1) * sizeof(char));
strcpy(xml_conf, cfg);

cs = md5Checksum(xml_conf);
if (strcmp(cs, xml_md5_checksums[0]) != 0) {
   printf("WARNING: configuration: %s, is different from the original configuration.\n", fn);
   free(cs);

   configFileData(xml_conf);
   cs = md5Checksum(xml_conf);
   if (strcmp(cs, xml_md5_checksums[1]) == 0) {
      printf("WARNING: The data in configuration: %s, is the same as the data in the original config file.\n", fn);
      free(cs);
      }
   else {
      printf("WARNING: The data in configuration: %s, is different from the data in the original config file.\n", fn);
      free(cs);

      configFileStruct(xml_conf);
      cs = md5Checksum(xml_conf);
      if (strcmp(cs, xml_md5_checksums[2]) == 0)
         printf("WARNING: The structure of configuration: %s, is the same as the structure of the original config file.\n", fn);
      else
         {
         printf("ERROR: The structure of configuration: %s, is different from the structure in the original config file.\n", fn);
         abort = -1;
         }
      free (cs);
      }
   }
else
   free(cs);

free(xml_conf);

return abort;
}
#endif
#endif

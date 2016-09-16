#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DATAPARSE
#include "parse-inc/config-structs.h"
#endif

#include "confpars-structs.h"
#include "confpars.h"
#include "parse-types.h"
#include "path-switch.h"

int countSwitchPaths(struct line *plist)
{
struct line *c_ptr;
int cnt = 0;

if (plist == (struct line *)NULL)					// we're looking at the 'base' next one is 1st one
   return 0;

c_ptr = plist;

if (c_ptr->next == (struct line *)NULL)
   printf("it is pointing to NULL\n");
while (c_ptr->next != (struct line *)NULL) {
   c_ptr = c_ptr->next;
   cnt++;								// found another one.
   }

return cnt;
}


int createStructPaths(struct varNameTypeList *vNTL)
{
FILE *fs_ptr, *fn_ptr;
struct varNameTypeList *c_ptr;
char str_line[2*TMP_BUFF_SZ];
int lcnt = 0, cnt = 0;

c_ptr = vNTL;
if (c_ptr == (struct varNameTypeList *)NULL)
   return 0;

fs_ptr = fopen("./parse-inc/struct-switch.inc", "w");                                                          // this is where the path <--> struct switch goes
fn_ptr = fopen("./parse-inc/struct-names.inc", "w");

while (c_ptr->next != (struct varNameTypeList *)NULL) {
   c_ptr = c_ptr->next;
   if (c_ptr->varList == vNTL_LIST)
      cnt++;
   }
c_ptr = vNTL;

sprintf(str_line, "char *configStructs[%d] = {", cnt);                                                      //  index starts at 0
fprintf(fn_ptr, "%s\n", str_line);

while (c_ptr->next != (struct varNameTypeList *)NULL) {
   c_ptr = c_ptr->next;
   if (c_ptr->varList == vNTL_LIST) {
      fprintf(fs_ptr, "case % 3d: { \n", lcnt);
      fprintf(fs_ptr, "   ptr = malloc(sizeof(%s));\n", c_ptr->varType);
      fprintf(fs_ptr, "   memset(ptr, 0x00, sizeof(%s));\n", c_ptr->varType);
      fprintf(fs_ptr, "   return ptr;\n");
      fprintf(fs_ptr, "   break;\n   };\n");

      fprintf(fn_ptr, "\"%s\"", c_ptr->varName);                                                                   // list of paths in the structure
      if (++lcnt == cnt)
         fprintf(fn_ptr, "\n");                                                                              // last entry does not end with a comma
      else
         fprintf(fn_ptr, ",\n");           
      }
   }
fprintf(fn_ptr, "};\n");


fclose(fs_ptr);
fclose(fn_ptr);
}


int countStructRecursionLvl(char *defin)
{
int rlvl = -1;
char *srl_ptr;

srl_ptr = strstr(defin, "]]");
while (srl_ptr != (char *)NULL) {
   rlvl++;
   srl_ptr = strstr(srl_ptr+1, "]]");
   }

return rlvl;
}



int createSwitchPaths(struct line *paths, int cnt, struct varNameTypeList *vNTL)
{
FILE *fs_ptr, *fn_ptr;
struct line *c_ptr;
char str_line[2*TMP_BUFF_SZ], *br_ptr;
int lcnt = 0, srl;

if (paths == (struct line *)NULL)
   return 0;

c_ptr = paths;

fs_ptr = fopen("./parse-inc/path-switch.inc", "w");								// this is where the path <--> struct switch goes
fn_ptr = fopen("./parse-inc/path-names.inc", "w");								// this is where the 'struct path' names go

if (fs_ptr != (FILE *)NULL && fn_ptr != (FILE *)NULL) {								// we need these files for the 2nd stage compile !!
   sprintf(str_line, "char *configFields[%d] = {", cnt-1);							//  index starts at 0
   fprintf(fn_ptr, "%s\n", str_line);
   while (c_ptr->next != (struct line *)NULL) {									// as long as there's a valid next
      c_ptr = c_ptr->next;											// go to the next one
      if (lcnt != 0) {												// we don't put the 'config super struct' in this list
         fprintf(fs_ptr, "case % 3d: { \n", lcnt-1);								// open the case
         if (c_ptr->type == TYPE_CHAR) {									// we treat dynamic (char *) different than static structs
            if (strstr(c_ptr->ln, "[idx[") != (char *)NULL) {
               fprintf(fs_ptr, "   if (idxStatus == INCOMPLETE)\n");
               fprintf(fs_ptr, "      return (void *) NULL;\n");
               }
            fprintf(fs_ptr, "   return (void *)&(%s);\n", c_ptr->ln);						// for (char *) just return the pointer
            }
         if (c_ptr->type == TYPE_STRUCT) {									// structs are static for now, we don't free/alloc them
            fprintf(fs_ptr, "   if (sw_task == GET_PTR)\n");
            if (vNTLtype(c_ptr->ln, vNTL) == 1) {
               strcpy(str_line, c_ptr->ln);
               getvNTLlistName(str_line);
               fprintf(fs_ptr, "         return (void **)&(%s);\n", str_line);
               fprintf(fs_ptr, "   if (sw_task == GET_LST_MBR_PTR) {\n");
               fprintf(fs_ptr, "      if ((void **)(%s) == (void **)NULL)\n", str_line);
               fprintf(fs_ptr, "         return (void **)(%s);\n", str_line);
               fprintf(fs_ptr, "      else\n");
               fprintf(fs_ptr, "        return %s", str_line);
               srl = countStructRecursionLvl(str_line);
               fprintf(fs_ptr, "[idx[%d]];\n", srl+1);
               fprintf(fs_ptr, "      }\n");
               }
            else {
               fprintf(fs_ptr, "      return (void *)&(%s);\n", c_ptr->ln);					// return pointer for 'regular' addressing
               }
            fprintf(fs_ptr, "   if (sw_task == GET_F_PTR)\n");
            fprintf(fs_ptr, "      return (void *)NULL;\n");							// return NULL if pointer is needed for free/malloc
            }
         fprintf(fs_ptr, "   break;\n   };\n");									// case closed

         str2NamesListTemplate(c_ptr->ln, str_line);
         fprintf(fn_ptr, "\"%s\"", str_line);									// list of paths in the structure
         if (++lcnt == cnt)
            fprintf(fn_ptr, "\n");										// last entry does not end with a comma
         else
            fprintf(fn_ptr, ",\n");										// all others do
         }
      else
         lcnt++;
      }

   fprintf(fn_ptr, "};\n");
   fclose(fs_ptr);
   fclose(fn_ptr);
   }
else
   printf("Error opening include files\n");

return lcnt;
}

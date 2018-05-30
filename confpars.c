/*
 * 
 *   confpars.c
 *
 *   xml config file parser 
 *
 *   Ron Croonenberg rocr@lanl.gov
 *   High Performance Computing (HPC-3)
 *   Los Alamos National Laboratory
 *
 *
 *   06-08-2015:        initial start rocr@lanl.gov
 *   06-08-2015:        redesigned collate functions
 *
 *
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef DATAPARSE
#include "parse-inc/config-structs.h"
#endif


#include "confpars-structs.h"
#include "confpars.h"
#include "parse-types.h"
#include "path-switch.h"

#ifdef DATAPARSE
#include "parsedata.h"
#endif

#ifdef MD5_CONFIG_CHECK
#include "checksum.h"
#endif


char *stripLeadingWhiteSpace(char *line)
{
char *ln, *str_ptr;

str_ptr = line;
while (isspace(*str_ptr))										// leading whitespace  .....
   str_ptr++;

ln = (char *)malloc(strlen(line)+1);

strcpy(ln, str_ptr);
strcpy(line, ln);

free(ln);

return line;
}



char *stripTrailingWhiteSpace(char *line)
{
int i;

i=strlen(line);												// start at the end
while (isspace(line[i-- - 1]))										// trailing whitespace  .....

line[i] = 0;												// truncate line

return line;
}



char *stripWhiteSpace(char *line)
{
stripLeadingWhiteSpace(line);										// strip leading white space
stripTrailingWhiteSpace(line);										// strip trailing white space

return line;
}



char *str2NamesListTemplate(char *str, char *tmp)		// strip indexes out of brackets
{
int i = 0, j = 0, open = 0, match = 0;

for (i = 0; i < strlen(str); i++) {				// for as long as the string is
   if (str[i] == '[') {						// find starting 'index bracket'
      match++;							// when we're back at 0 again..  we found the closing one
      if (open == 0) {
         open = 1;						// we're inside index brackets
         tmp[j] = str[i];					// we only keep the first bracket
         j++;							// pointing at empty spot in templet string
         }
      }
   if (str[i] == ']') {						// found a closing bracket
      match--;							// come one down
      if (match == 0)
         open = 0;						// if last closing bracket..  we're not in an index anymore
      }
   if (open == 0) {
      tmp[j] = str[i];						// if we're not in an index bracket, copy the token.
      j++;							// point to next open spot
      }
   }
tmp[j] = 0;							// terminate the string

return tmp;
}



struct line *findClosingStructBracket(struct line *hdr)
{
struct line *c_ptr;
int cnt = 1;

if (hdr == (struct line *)NULL)
   return (struct line *)NULL;										// there is nothing to find

while (c_ptr->next != (struct line *)NULL && cnt >0) {							// as long as there are lines and we didn't find it yet
   if (strstr(c_ptr->ln, "};") != (char *)NULL)
      cnt--;												// found a struct closing bracket
   if (strstr(c_ptr->ln, "{")  != (char *)NULL)
      cnt++;												// found a struct openening bracket
   }
return c_ptr;												// report what we found
}



int checkElementName(char *str, char *buff)
{
int i;
char *reserved[33] = { "auto", "break", "case", "char", "const", "continue", "default",			// list of reserved keywords, probably not complete
                       "do", "double", "else", "entry", "enum", "extern", "float",
                       "for", "goto", "if", "int", "long", "register", "return", "short",
                       "signed", "sizeof", "static", "struct", "switch", "typedef",
                       "union", "unsigned", "void", "volatile", "while" };


for (i=0; i<RSRV_LST; i++) {
   if (strcmp(str, reserved[i]) == 0) {									// check element with reserved words
      printf("Error: An XML element can not be a C reserved word in this configuration. (%s)\n", str);
      exit(-1);											// fatal error
      }
   }


for (i=0; i<strlen(str)-1; i++) {
   if (!(isalnum(str[i]) ||  str[i] == '_')) {								// check if element name is a valid variable name
      printf("Error: non C type variable/element name detected (%s).\n", str);
      exit(-1);
      }
   }

return 0;												// everything fine
}



void countLtGtTokens(int *lt, int *gt, char *start, char *end)
{
char *c_ptr;
int lt_cnt = 0, gt_cnt = 0;

c_ptr = start;
while (c_ptr < end) { 
   if (c_ptr[0] == '<') 
      lt_cnt++;                                              // count the < tokens
   if (c_ptr[0] == '>') 
      gt_cnt++;                                              // count the > tokens
   c_ptr++;
   }
 
*lt = lt_cnt;
*gt = gt_cnt;
}



char *prepConfigFileLine(char *line, int ln_cnt)
{
int lt_cnt = 0, gt_cnt = 0;
char *cmmnt_ptr, *c_ptr;

if (line[0] == '#') {
   line[0] = 0;
   return line;
   } 

if (strlen(line) > 0) {
   while (isspace(line[strlen(line)-1]))
      line[strlen(line)-1] = 0;							// now the last '\n' is stripped from ALL lines
   }

c_ptr = line;
cmmnt_ptr = strstr(line, "#");

if (cmmnt_ptr == (char *)NULL)
   return line;

countLtGtTokens(&lt_cnt, &gt_cnt, c_ptr, cmmnt_ptr);

if ((lt_cnt == 0 && gt_cnt == 0) || (lt_cnt == 2 && gt_cnt == 2)) {		//      |#           ......   OR    |  <>  <> #
   cmmnt_ptr[0] = 0;
   }
else {
   if (lt_cnt == 1 && gt_cnt == 1) {						//      |  < > # < >
      c_ptr = line;
      cmmnt_ptr = strstr(c_ptr, "#");						// find first pound sign
      cmmnt_ptr = strstr(cmmnt_ptr+1, ">");					// find end of closing tag
      cmmnt_ptr = strstr(cmmnt_ptr, "#");					// find pound sign
      if (cmmnt_ptr != (char *) NULL) {
         countLtGtTokens(&lt_cnt, &gt_cnt, c_ptr, cmmnt_ptr);
         if ((lt_cnt == 2 && gt_cnt == 2)) {					//	|  < >  #  < >  #
            cmmnt_ptr[0] = 0;
            } 
         }
      else {
         printf("WARNING: pound sign considered data, comment would unmatch object element tag on line %d.\n", ln_cnt);
         }
      }
   }

return line;
}


char *readConfigFile(char *fn)
{
FILE *f_ptr;
long int f_size;
int ln_cnt = 0, ln_total=0;
char line[CFG_LINE_SZ],  *conf, *nl_ptr;

f_ptr = fopen(fn, "r");											// open config file
if (f_ptr != (FILE *)NULL) {										// on succesful open
   fseek(f_ptr, 0, SEEK_END);										// go to last position
   f_size = ftell(f_ptr);										// get position
   conf = (char *)malloc((size_t)f_size + 2);								// alloc 1 byte more for termination
   if (conf != (char *)NULL) {
      fseek(f_ptr, 0, SEEK_SET);									// go back to start of file
      clearerr(f_ptr);
      conf[0] = 0;											// empty config
      nl_ptr = conf;
      while (!feof(f_ptr)) {
         fgets(line, CFG_LINE_SZ, f_ptr);
         ln_cnt++;
         prepConfigFileLine(line, ln_cnt);
         sprintf(nl_ptr, "%s\n", line);
         nl_ptr += strlen(line)+1;
         line[0]=0;
         }
      }
   else {
      printf("Could not allocate memory. Aborting.\n");
      return (char *)NULL;
      }
   fclose(f_ptr);											// close config file, we opened it.
   }
else {
   printf("***Could not open configuration file (%s). Aborting.\n", fn);
   return (char *)NULL;
   }

#ifdef DEBUG
printf("%s", conf);
#endif

return conf;
}



int findLineNumber(char *pos_ptr, char *buffstart)
{
int nl_cnt = 1;
char *p_ptr;

p_ptr = pos_ptr;											// where we are in the configuration 'file'
while (p_ptr > buffstart) {										// as long as we're in the configuration file
   if (p_ptr[0] == '\n')										// and we see a newline
      nl_cnt++;												// count it
   p_ptr--;												// move one character back
   }

return nl_cnt;												// return how many lines we counted
}



int findElement(char *content_start, char *content_end, struct elm_str *Elem, char *buffstart)
{
char *el_ptr, *type_ptr, *tag_end;
int i, cnt_len, ln_nr, ret=0;



Elem->element[0] = 0;											// start with empty element name
Elem->element_ctype[0] = 0;										// start with empty element type name.
el_ptr  = strstr(content_start, "<");									// find a new element start tag
tag_end = strstr(content_start, ">"); 									// we need to have some better controls IF NULL.
if (el_ptr != (char *)NULL) {										// check to see if there is an element tag
   if (el_ptr <= content_end && el_ptr[1] != '/') {							// check it is not an end tag
      el_ptr++;												// point at token after the '<' where the name starts
      cnt_len = content_end - el_ptr;									// counter for searching only withing this content
      i=0;
      while (el_ptr[i] != '>' && el_ptr[i] != ':' && i < cnt_len) {					// end of element tag or end of content?
         Elem->element[i] = el_ptr[i];									// copy  the element name from the tag
         i++;
         }
      Elem->element[i] = 0;										// terminate it
      stripWhiteSpace(Elem->element);									// scrub it
      if (el_ptr[i] == ':') {										// with have additional tag info
         type_ptr = strstr(el_ptr + i, "type") + 4;							// we only support type now
         if (type_ptr != (char *) NULL && type_ptr < tag_end) {
            type_ptr = strstr(type_ptr, "=");								// is there something actually there?
            if (type_ptr != (char *) NULL && type_ptr < tag_end) {								// or not?
               type_ptr += 1;										// move past the '='
               i=0;
               cnt_len = content_end - type_ptr;							// set content limiters for this type
               while (type_ptr[i] != '>' && i < cnt_len) {						// end of element tag or end of content?
                  Elem->element_ctype[i] = type_ptr[i];							// copy the type info 
                  i++;
                  }
               Elem->element_ctype[i] = 0;								// terminate the element_ctype
               stripWhiteSpace(Elem->element_ctype);							// scrub it
#ifdef DEBUG
               printf("Type of variable %s is: |%s|\n", Elem->element, Elem->element_ctype);
#endif
               }
            else {
               ln_nr = findLineNumber(el_ptr, buffstart);
               printf("ERROR: incomplete tag information definition on line: %d, aborting.\n", ln_nr);
               exit(-1);
               }
            }
         else {
            ln_nr = findLineNumber(el_ptr, buffstart);
            printf("ERROR: unknown or missing tag information on line: %d, aborting.\n", ln_nr);
            exit(-1);
            }
         }
      ret = 1;
      }
   else {
      if (el_ptr <= content_end) {									// we either ran out of content (weird) OR found an illegal end tag
         strPrintTag(Elem->element, el_ptr);
         ln_nr = findLineNumber(el_ptr, buffstart);
         printf("ERROR: Found a bad tag: %s on line number: %d. Aborting parse process.\n", Elem->element, ln_nr);
         ret = -1;
         }
      Elem->element[0] = 0;											// there is no element on this level
      }
   }

if (strlen(Elem->element) > 0) {
   if (checkElementName(Elem->element, buffstart) == 1) {
      }
   }

return ret;
}



int strPrintTag(char *str_tag, char *tag_ptr)								// print element tag name to a separate string
{
char *t_ptr;
int i=0;

t_ptr = tag_ptr;											// point at the tag
while (t_ptr[0] != '>') {										// as long as we didn't find the '>' end of tag token
   str_tag[i++] = t_ptr[0];										// copy token and advance index
   t_ptr++;												// advance the ptr
   }
str_tag[i++] = t_ptr[0];										// include the '>' token
str_tag[i]   = 0;											// terminate

return i;
}



int printContent(char *content_start, char *content_end)						// for debugging purposes
{
char *str_ptr;
int i;

str_ptr = content_start;
printf("****************************\n");
printf("%p - %p\n", content_start, content_end);							// the area we'll print
printf("***** Start of Content *****\n|");
while (str_ptr <= content_end) {
   printf("%c", str_ptr[0]);										// print character at a time, because of no 0 termination.
   str_ptr++;
   i++;
   }
printf("|\n***** End of Content *****\n");

return i;
}



char *processElementContent(char *elem_tag, char *content_start, char *content_end)			// example for processing data, fill structure for example
{
char *str_ptr, *buff;
int i;

buff = (char *)malloc(strlen(elem_tag) +1 + (content_end - content_start)+1 + 1);			// buffer for the result/value, seperating tag from value with a 0 (yes you read that right)

if (buff != (char *)NULL) {
   strcpy(buff, elem_tag);										// elem_tag header
   i = strlen(elem_tag) + 1;										// move one spot after elem_tag's terminating 0
   str_ptr = content_start;										// set the content pointer to beginning
   while (str_ptr <= content_end) {									// walk through the elements content
      buff[i] = str_ptr[0];										// copy it to the buffer
      str_ptr++;											// advance content pointer
      i++;												// advance buffer index
      }
   buff[i] = 0;												// terminate buffer
   }

return buff;												// return pointer to buffer
}



char *getElemVal(char *val_buff)
{
char *val_ptr, *nv_ptr;

val_ptr = val_buff;                                                                                     // point to start of buffer
while (val_ptr[0] != 0)                                                                                 // as long as we don't see terminating 0
    val_ptr++;                                                                                          // advance the pointer

val_ptr++;                                                                                              // point at position after the 0

nv_ptr = (char *)malloc(strlen(val_ptr) +1);								// create a new buffer
strcpy(nv_ptr, val_ptr);										// copy the data in there

return nv_ptr;												// return ptr to buffer
}



char *elemValPtr(char *val_buff)
{
char *val_ptr;

if (val_buff == (char *)NULL)
   return (char *)NULL;

val_ptr = val_buff;											// point to start of buffer
while (val_ptr[0] != 0 && val_ptr+1 != (char *)NULL)											// as long as we don't see terminating 0
    val_ptr++;												// advance the pointer to point at the field data

val_ptr++;												// point at position after the 0

return val_ptr;
}


// for format reasons we keep track of recursion level		(move from global to local in a newer version);
char indent[65] = "                                                                ";
int r_lvl = 0;

void setIndent(int offset)
{
if (3*offset > 0)
   indent[3*offset] = 0;										// truncate the indent string
else
   indent[0] = 0;											// if zero or negative, no ident
}



void resetIndent(int offset)
{
if (3*offset > 0)											// fix the indent string ...
   indent[3*offset] = ' ';
else
   indent[0] = ' ';											//                       ... where we broke it before
}



char *isVarIn(char *haystack, char *needle)
{
char *ndl_ptr, *e_ptr, *ret_ptr;

ndl_ptr = (char *)malloc(strlen(needle) + 3);

strcpy(ndl_ptr, needle);

e_ptr  = haystack;
e_ptr += strlen(haystack);
if (strlen(haystack) >= strlen(needle))
   if (strcmp(e_ptr, ndl_ptr) == 0) {
      free(ndl_ptr);
      return e_ptr;
      }

strcat(ndl_ptr, " ");
ret_ptr = strstr(haystack, ndl_ptr);
if (ret_ptr != (char *)NULL) {
   free(ndl_ptr);
   return ret_ptr;											// trying 'needle ' success
   }

ndl_ptr[strlen(ndl_ptr)-1] = '.';
ret_ptr = strstr(haystack, ndl_ptr);
if (ret_ptr != (char *)NULL){
   free(ndl_ptr);
   return ret_ptr;
   }													// trying 'needle.' success

ndl_ptr[strlen(ndl_ptr)-1] = '[';
ret_ptr = strstr(haystack, ndl_ptr);
if (ret_ptr != (char *)NULL) {
   free(ndl_ptr);
   return ret_ptr;											// trying 'needle[' success
   }

ndl_ptr[strlen(ndl_ptr)-1] = '-';
strcat(ndl_ptr, ">");
ret_ptr = strstr(haystack, ndl_ptr);
if (ret_ptr != (char *)NULL) {
   free(ndl_ptr);
   return ret_ptr;											// trying 'needle->'  succes.
   }

free(ndl_ptr);

return (char *)NULL;
}



struct line *findNextLine(struct line *head, char *srch1, char *srch2)
{
struct line *ptr = (struct line *)NULL;
char *s_ptr1, *s_ptr2;

if (head == (struct line *)NULL)
   return ptr;									// there's nothing to find here

ptr = head;									// start at the base
while (ptr->next != (struct line *)NULL) {
   ptr = ptr->next;								// go to next entry
   if (ptr->ln != (char *)NULL) {						// if we have a line
      s_ptr1 = strstr(ptr->ln, srch1);						// check if first search item is there
      if (s_ptr1 != (char *)NULL) {
         s_ptr2 = strstr(s_ptr1, srch2);					// check if second search item comes after first
         if (s_ptr2 != (char *)NULL)
            return ptr;								// it does, it might be what we're looking for
         }
      }
   }
return (struct line *)NULL;							// nothing found
}



struct line *findNextOccurence(struct line *header, char *srch_str)
{
struct line *srch_ptr;

if (header == (struct line *)NULL)
   return (struct line *)NULL;							// there's nothing to find here

srch_ptr = header;
if (srch_ptr != (struct line *)NULL && strlen(srch_str) > 0) {
   srch_ptr = srch_ptr->next;							// move to next line else we find ourself again.
   srch_ptr = findNextLine(srch_ptr, srch_str, "");				// find next header line
   if (srch_ptr != (struct line *)NULL)
      if (srch_ptr->ln != (char *)NULL)
         return srch_ptr;							// found one! return it
   }
return (struct line *)NULL;							// nothing found
}



struct line *findNextStruct(struct line *header)
{
struct line *srch_ptr;
char *srch_str;

if (header == (struct line *)NULL)
   return (struct line *)NULL;							// there's nothing to find here

srch_ptr = header;
srch_str = (char *)malloc(strlen(header->ln)+1);
strcpy(srch_str, header->ln);							// create a search string
stripWhiteSpace(srch_str);							// without trailing nor leading whitespace
srch_ptr = findNextOccurence(srch_ptr, srch_str);				// get next one if there is one
free(srch_str);									// free the search string

return srch_ptr;								// return whatever result
}



struct line *findEmptyStruct(struct line *header)
{
struct line *srch_ptr, *p_ptr;

if (header == (struct line *)NULL)
   return (struct line *)NULL;							// there's nothing to find here

srch_ptr = header;
while (srch_ptr != (struct line *)NULL) {
   srch_ptr = findNextLine(srch_ptr, "struct", "{");				// get next struct, if there is one
   if (srch_ptr != (struct line *)NULL) {
      p_ptr = srch_ptr;								// remember where structure started
      srch_ptr = srch_ptr->next;						// check the next line
      if (strstr(srch_ptr->ln, "};") != (char *)NULL) {				// is it the closing line?
#ifdef DEBUG
printf("Found empty:\n%s\n%s\n", p_ptr->ln, srch_ptr->ln);
#endif
         return p_ptr;								// return the location of the empty struct
         }
      }
   }

return (struct line *)NULL;							// we didn't find an empty struct
}



int removeEmptyStruct(struct line *base, struct line *h_struct)
{
struct line *p_ptr, *c_ptr, *n_ptr; 

if (h_struct == (struct line *)NULL)
   return -1;									// there's nothing to remove

c_ptr = base;									// point at the start of the list
while (c_ptr != h_struct) {							// go to the line just before where the struct starts
   p_ptr = c_ptr;								// previous become current
   c_ptr = c_ptr->next;								// advance current pointer one position
   }
n_ptr = c_ptr->next;								// next pointer pointing at line after the struct definition

if (strstr(n_ptr->ln, "};") != (char *)NULL) {					//n_ptr should contain '};' and maybe some whitespace
   p_ptr->next = n_ptr->next;							// p_ptr->next is pointing at the struct after the one we're deleting
   free(c_ptr);									// free the definition
   free(n_ptr);									// free the last line of this struct
   n_ptr = c_ptr->next;
   }
else {
#ifdef DEBUG
   printf("Not empty, found: '%s'", n_ptr->ln);
#endif
   return -1;
   }

return 0;
}



void removeStruct(struct line *base, struct line *h_struct)
{
struct line *c_ptr, *n_ptr;

if (h_struct == (struct line *)NULL)						// there's nothing to remove
   return;

c_ptr = base;									// point at the start of the list
while (c_ptr->next != h_struct)							// go to the line just before where the struct starts
   c_ptr = c_ptr->next;								// current pointer

n_ptr = c_ptr->next;								// next pointer pointing to the struct definition
while (strstr(n_ptr->ln, "};") == (char *)NULL) {				// as long as we're not at the end of the struct, keep deleting
   c_ptr->next = n_ptr->next;							// point at the struct after the one we're deleting
   free(n_ptr);									// free, delete
   n_ptr = c_ptr->next;
   }
c_ptr->next = n_ptr->next;							// n_ptr is pointing a the closing line
free(n_ptr);									// so get rid of it.
}



struct line *findStructMember(struct line *header, char *member)
{
struct line *c_ptr;
int in_struct = 1;

if (header == (struct line *)NULL)
   return (struct line *)NULL;							// there's nothing to find here

c_ptr = header;									// this is the 'current line' pointer
//e_ptr = findClosingStructBracket(header);

#ifdef DEBUG
printf("%s - %s\n", c_ptr->ln, c_ptr->dbg);
#endif

while (c_ptr->next != (struct line *)NULL && in_struct) {
//while (c_ptr->next != (struct line *)NULL && in_struct && c_ptr < e_ptr) {

   c_ptr = c_ptr->next;								// walk down the list
#ifdef DEBUG
printf("%s - %s\n", c_ptr->ln, c_ptr->dbg);					// move to the next line
#endif
   if (strstr(c_ptr->ln, "};") == (char *)NULL) {				// stop at the end of this structure
      if (strstr(c_ptr->ln, member) != (char *)NULL) {
#ifdef DEBUG
printf("(%s == %s) returning\n", c_ptr->ln, member);
#endif
         return c_ptr;								// found it;
         }
      }
   else
      in_struct = 0;								// we're at the end of this struct
   }
return (struct line *)NULL;
}



int removeStructField(struct line *h_struct, char *srch_str)
{
struct line *p_ptr, *c_ptr, *n_ptr;
int in_struct = 1;

#ifdef DEBUG
printf("Entering: 'removeStructField'\n");
#endif

if (h_struct == (struct line *)NULL)						// there's nothing to remove
   return -1;

c_ptr = h_struct;								// this is the 'current line' pointer
while (c_ptr->next != (struct line *)NULL && in_struct) {			// as long as there are members in this struct
   p_ptr = c_ptr;
   c_ptr = c_ptr->next;								// move to the next line
   n_ptr = c_ptr->next;								// point to the line after the current line (could be NULL)
   if (strstr(c_ptr->ln, "};") == (char *)NULL) {				// stop at the end of this structure
      if (strstr(c_ptr->ln, srch_str) != (char *)NULL) {			// if this is the field/member we're looking for
#ifdef DEBUG
         printf("Removing: '%s - %s' member\n", srch_str, c_ptr->ln);
#endif
	 p_ptr->next = n_ptr;							// drop current and connect 'previous line' to 'next line'
	 free(c_ptr);		
#ifdef DEBUG
printf("Removed\n");
#endif
	 in_struct = 0;								// we delete only one field at a time
	 }
      }
   else
      in_struct = 0;								// we're at the end of this struct
   }

#ifdef DEBUG
printf("Leaving: 'removeStructField'\n");
#endif
}



int countStructMembers(struct line *h_struct)
{
struct line *c_ptr;
int field_cnt = 0, in_struct = 1;;

if (h_struct == (struct line *)NULL)						// there is nothing to count here
   return -1;

c_ptr = h_struct;
while (c_ptr->next != (struct line *)NULL && in_struct) {			// as long as we're in this struct
   c_ptr = c_ptr->next;								// go to the next line
   if (strstr(c_ptr->ln, "};") == (char *)NULL)					// if we're not at the end of the struct
	 field_cnt++;								// there's another field/member
   else
      in_struct = 0;								// end of the struct
   }

return field_cnt;
}



int countFieldOccurences(struct line *h_struct, char *srch_str)
{
struct line *c_ptr;
int in_struct = 1, field_cnt = 0;
char *st_ptr;

if (h_struct == (struct line *)NULL)						// there is nothing to count here
   return -1;

c_ptr = h_struct;
while (c_ptr->next != (struct line *)NULL && in_struct) {			// as long as we're in this struct
   c_ptr = c_ptr->next;								// go to the next line
   if (strstr(c_ptr->ln, "};") == (char *)NULL) {				// not the end of the struct yet
      st_ptr = strstr(c_ptr->ln, srch_str);
      if (st_ptr != (char *)NULL) {						// could it be the field/member we're looking for?
         st_ptr += strlen(srch_str);						// move to the character after srch_str 
         if (st_ptr[0] == ' ' ||						// a struct name or so?
             st_ptr[0] == 0   ||						// a variable name?
             st_ptr[0] == '.' ||                                                // a struct member?
            (st_ptr[0] == '-' && st_ptr[1] == '>') ||                           // a struct member?
             st_ptr[0] == '[') {						// a list?
            if (st_ptr[0] == '[') { 						// is it a list?
               field_cnt += atoi(st_ptr+1);					// yup, find out how long
#ifdef DEBUG
printf("List Counting:   %s\n", c_ptr->ln);
#endif
               }
            else {
	       field_cnt++;							// just one, add one
#ifdef DEBUG
printf("Single Counting: %s\n", c_ptr->ln);
#endif
               }
            }
         }
      }
   else
      in_struct = 0;								// end of struct
   }

#ifdef DEBUG
printf("I counted '%s' %d times,\n", srch_str, field_cnt);
#endif

return field_cnt;
}



void printStruct(struct line *hdr)
{
struct line *c_ptr;
int in_struct = 1;

if (hdr == (struct line *)NULL)
   return;

c_ptr = hdr;
if (c_ptr != (struct line *) NULL)
   if (c_ptr->ln != (char *)NULL)
      printf("%s\n", c_ptr->ln);
while (c_ptr->next != (struct line *)NULL && in_struct == 1) {
   c_ptr = c_ptr->next;
   printf("%s\n", c_ptr->ln);
   if (strstr(c_ptr->ln, "};") != (char *)NULL)
      in_struct = 0;
   }
}



struct line *collateFields(struct line *l_struct, struct line *r_struct)
{
struct line *r_ptr, *l_ptr;
char lst_cnt[6], *srch_str, *tr_ptr;
int field_cnt = 0, old_fld_cnt, in_struct = 1, i;

#ifdef DEBUG
printf("Entering: collateFields.\n");
#endif

if (l_struct == (struct line *) NULL || r_struct == (struct line *) NULL)
   return (struct line *)NULL;

srch_str = (char *)malloc(TMP_BUFF_SZ);
if (srch_str == (char *)NULL)
   printf("Memory allocation error\n");						// we should do a meaningfull return here

l_ptr = l_struct;								// the first occurence of it this structure definition
r_ptr = r_struct;								// the other occurence of it this structure definition (starts with l_struct)

#ifdef DEBUG
printf("l_struct:\n");
printStruct(l_struct);
#endif

while (l_ptr->next != (struct line *)NULL && in_struct) {			// as long as we have lines in this struct
   l_ptr = l_ptr->next;								// walk through the fields of the structure occurence we keep
   strcpy(srch_str, l_ptr->ln);							// make the field a search string ...
   stripWhiteSpace(srch_str);							//                                ... without trailing/leading white space
   tr_ptr = strstr(srch_str, "[");						// is this an array already?
   if (tr_ptr != (char *)NULL)							// does this need to be truncated"
      tr_ptr[0] = 0;								// yup, truncate it
   if (strstr(l_ptr->ln, "};") == (char *)NULL) {				// is this the last line of this structure?
      field_cnt   = countFieldOccurences(r_struct, srch_str);			// count how often the field appears in the right side occurence
      old_fld_cnt = countFieldOccurences(l_struct, srch_str);			// count how long a list we have ourself
      r_ptr = r_struct;
      for (i=0; i < field_cnt; i++) {						// IF field_cnt is 1 or larger I found one or more extra somewhere somewhere
         if (l_struct == r_struct) {						// we're checking ourself ...
            if (i != 0)								//                        ... but we need at least one occurence of the field
               removeStructField(r_ptr, srch_str);				// delete rest
            }
         else
            removeStructField(r_ptr, srch_str);					// delete all of them if we're not checking ourself
         r_ptr = findStructMember(r_struct, srch_str);				// find the left over entry
         }									// there will be at least one field left in my own (l_ptr) struct
      strcpy(lst_cnt, "");							// set to nothing, change to [n] if needed
      if (l_struct == r_struct) {						// IF we're looking at ourself, skip the first one (we're working with that
         l_ptr = findStructMember(l_struct, srch_str);				// find our only one entry again, just in case
         if (old_fld_cnt > 1)
            sprintf(lst_cnt, "[%d];", old_fld_cnt);				//                            ... old_fld_cnt is the correct one, use it
         }
      else {									// we're NOT counting ourself
         if (field_cnt > old_fld_cnt) {
            if (field_cnt > 1)
               sprintf(lst_cnt, "[%d];", field_cnt);				// we found a larger list, use the larger list
            }
         else {
            if (old_fld_cnt > 1)
              sprintf(lst_cnt, "[%d];", old_fld_cnt);				// our own list is long enough
            }
         }

      tr_ptr = strstr(l_ptr->ln, "[");                                              // is this an array already?
      if (tr_ptr != (char *)NULL)                                                  // does this need to be truncated"
         tr_ptr[0] = 0;
      stripTrailingWhiteSpace(l_ptr->ln);

      strcat(l_ptr->ln, lst_cnt);                                             // add it after the field name

#ifdef DEBUG
      if (l_struct != r_struct) {
         printf("r_struct:\n");
         printStruct(r_struct);
         }
#endif
      }
   else
      in_struct = 0;                                                            // done with this struct
   }
free(srch_str);                                                                 // cleanup

#ifdef DEBUG
printf("Leaving: collateFields\n");
#endif

return l_struct;
}




struct line *collateStructures(struct line *header)
{
struct line *l_ptr, *r_ptr;
char *definition;
int empty;


definition = (char *)malloc(TMP_BUFF_SZ);

#ifdef DEBUG
printf("Entering: collateStructures\n");
#endif

l_ptr = header;												// we start at the base
while (l_ptr != (struct line *)NULL) {
   empty = 0;												// reset it every iteration
   l_ptr = findNextLine(l_ptr, "struct", "{");								// we go through the list finding all and collate all structures
   if (l_ptr != (struct line *)NULL) {
      r_ptr = l_ptr;											// we start with ourself
      strcpy(definition, l_ptr->ln);									// copy the definition of the struct
      stripWhiteSpace(definition);									// strip all trailing and leading white space
      while (r_ptr != (struct line *)NULL) {								// as lon as we find structs to collate
         collateFields(l_ptr, r_ptr);									// compare and collate the fields in these two occurences
         r_ptr = findNextOccurence(r_ptr, definition);							// find another occurence of this structure
         }
      while (!empty) {
         r_ptr = findEmptyStruct(header);								// find an empty struct
         if (r_ptr != (struct line *)NULL) {
#ifndef DEBUG
            removeEmptyStruct(header, r_ptr);								// and remove it if tehre is one
#else
            if (removeEmptyStruct(header, r_ptr) == 0)
               printf("Removed empty '%s' structure.\n", definition);
            else
               printf("could not remove %s, not empty.\n", definition);
#endif
            }
         else {
            empty = 1;											// there are no empty structs anymore
            }
         }
      }													// we should be done here
   }


free(definition);											// free the buffer again

#ifdef DEBUG
printf("Leaving: collateStructures\n");
#endif
}



char *getStructName(char *str)
{
char *s_ptr, *e_ptr, *sn_ptr;

s_ptr=strstr(str, "struct");
if (s_ptr != (char *)NULL) {
   s_ptr += 6;
   e_ptr = &(s_ptr[strlen(s_ptr) - 1]);

   sn_ptr = (char *)malloc((e_ptr - s_ptr) + 1 + 1);							// 1 for the difference between front and end, 1 for the terminator

   strcpy(sn_ptr, s_ptr);
   sn_ptr[strlen(sn_ptr)-1] = 0;
   stripWhiteSpace(sn_ptr);
   }
else
   sn_ptr = (char *)NULL;

return sn_ptr;
}



struct line *listHeaderFile(struct line *base, struct varNameTypeList *vNTL, int l_order)
{
FILE *f_ptr;
struct line *c_ptr, *p_ptr, *n_ptr;
struct line header, *new, *cn_ptr;
char *structName, *tmp_line;
int order = 0, i, cnt;


#ifdef DEBUG
printf("Entering listHeader: ");
if (l_order == GEN_PARSE_STRUCTS)
   printf("GEN_PARSE_STRUCTS\n");
if (l_order == GEN_STRUCT_SWITCH)
   printf("GEN_STRUCT_SWITCH\n");
#endif

if (base == (struct line *)NULL)									// nothing here
   return (struct line *)NULL;

tmp_line = (char *)malloc(TMP_BUFF_SZ);

memset(&header, 0x00, sizeof(struct line));
header.next = (struct line *)NULL;
cn_ptr = &header;

if (l_order == DECONSTRUCT) {										// DECONSTRUCT get the structs out of the nested XML from highest recursion to lowest
   c_ptr = base;											// point to the base of the list
   while (c_ptr->next != (struct line *) NULL) {
      c_ptr   = c_ptr->next;										// base is never used except for attaching things to, go to next
#ifdef DEBUG
printf("===%s\n", c_ptr->ln);
#endif
      if (c_ptr->lvl > order)										// if this recursion level was higher ...
         order = c_ptr->lvl;										//                                    ... take it
      }
#ifdef DEBUG
   printf("Highest r-lvl: %d\n", order);
#endif
   for (i = order; i > 0; i--) {									// were going down, pairwise   n - n-1 ... 3-2, 2-1, 1-0
      c_ptr = base;											// current pointer pointing to the list
      while (c_ptr->next != (struct line *) NULL) {							// only end of the list should be NULL
         p_ptr = c_ptr;											// we're pointing to the previous line, if there is one
         c_ptr = c_ptr->next;										// this is our 'current' line
         if (c_ptr->next != (struct line *)NULL)
            n_ptr = c_ptr->next;									// possible next line ...
         else
            n_ptr->next = (struct line *)NULL;								//                    ... or not
         if (c_ptr->tag == 0) {										// if the line is not tagged, it's up for grabs
            if (c_ptr->lvl == i && c_ptr->ln != (char *)NULL) {						// it is the level we're looking for and there is a line
               if (c_ptr->lvl - p_ptr->lvl == 1) {							// level of previous line is one less, so it is our 'head'
                  p_ptr->tag = 0;									// but we don't tag it because the next struct needs it also

#ifdef DEBUG
                  printf("[%d] [%d] [%d] +1+ %s\n", p_ptr->lvl, p_ptr->tag, i, p_ptr->ln);		// print previous header line
#endif
                  new = (struct line *)malloc(sizeof(struct line));					// create new line for the header line
                  memset(new, 0x00, sizeof(struct line));
                  new->ln = (char *)malloc(strlen(p_ptr->ln) + 1);
                  strcpy(new->ln, p_ptr->ln);								// copy the string
                  new->next = (struct line *)NULL;							// we don't have a next line yet
                  cn_ptr->next = new;									// attach to previous line
                  cn_ptr = cn_ptr->next;								// point at new our addition
                  p_ptr->ln[strlen(p_ptr->ln)-1] = 0;							// this gets rid of the '{' bracket
                  }
               c_ptr->tag = 1;										// this is our line, tag and take it
#ifdef DEBUG
               printf("[%d] [%d] [%d] +2+ %s\n", c_ptr->lvl, c_ptr->tag, i, c_ptr->ln);			// print current header line
#endif
               new = (struct line *)malloc(sizeof(struct line));					// create new line for the header line
               memset(new, 0x00, sizeof(struct line));
               structName = getStructName(c_ptr->ln);
               if (structName != (char *)NULL)
                  new->ln = (char *)malloc(strlen(c_ptr->ln) + 1 + strlen(structName) + 2);		// allocate for line   ln + " " + structName + 0
               else
                  new->ln = (char *)malloc(strlen(c_ptr->ln) + 1);					// just the original length
               strcpy(new->ln, c_ptr->ln);								// copy the string
               stripTrailingWhiteSpace(new->ln);
               if (structName != (char *)NULL) {
                  strcat(new->ln, " ");
                  strcat(new->ln, structName);
                  free(structName);
                  }
               new->next = (struct line *)NULL;								// we don't have a next line yet
               cn_ptr->next = new;									// attach to previous line
               cn_ptr = cn_ptr->next;									// point at new our addition
               }
            }
         }												// end while.
      }													// for every recursion level higher than 0
   collateStructures(&header);										// delete all double fieds in all structs
#ifdef DEBUG
printf("##############  AND WE'RE DONE ##########\n");
#endif
   }													// end of deconstruct

if (l_order == NO_ORDER) {
   c_ptr = base;
   while (c_ptr->next != (struct line *) NULL) {
      c_ptr = c_ptr->next;
      if (c_ptr->ln != (char *)NULL) {
         strcpy(tmp_line, c_ptr->ln);
         stripWhiteSpace(tmp_line);
         if (strstr(tmp_line, "struct") != (char *)NULL && strstr(tmp_line, "{") != (char *)NULL)
            printf("%s\n", tmp_line);
         else
            printf("   %s\n", tmp_line);
         if (strstr(tmp_line, "};") != (char *)NULL)
            printf("\n");
         }
      }
   }

if (l_order == GEN_PARSE_STRUCTS) {
   f_ptr = fopen("./parse-inc/config-structs.h", "w");
   if (f_ptr != (FILE *)NULL) {
      c_ptr = base;
      while (c_ptr->next != (struct line *) NULL) {
         c_ptr = c_ptr->next;
         if (c_ptr->ln != (char *)NULL) {
            strcpy(tmp_line, c_ptr->ln);
            stripWhiteSpace(tmp_line);
            if (strstr(tmp_line, "struct") != (char *)NULL && strstr(tmp_line, "{") != (char *)NULL)
               fprintf(f_ptr, "%s\n", tmp_line);
            else {
               if (tmp_line[strlen(tmp_line)-1] == ';')
                  fprintf(f_ptr, "   %s\n", tmp_line);
               else
                  fprintf(f_ptr, "   %s;\n", tmp_line);
               }
            if (strstr(tmp_line, "};") != (char *)NULL)
               fprintf(f_ptr, "\n");
            }
         }
      fclose(f_ptr);
      }
   else
      printf("Could not open/create confpars-structs.h for output.\n");
   }

if (l_order == NO_ORDER_DEBUG) {
   c_ptr = base;
   while (c_ptr->next != (struct line *) NULL) {
      c_ptr = c_ptr->next;
      if (c_ptr->ln != (char *)NULL)
         printf("[%d] [%d] %s %s\n", c_ptr->lvl, c_ptr->tag, c_ptr->dbg, c_ptr->ln);
      }
   }


if (l_order == GEN_STRUCT_SWITCH) {
   c_ptr = base;
   cnt = countSwitchPaths(c_ptr);
   createSwitchPaths(c_ptr, cnt, vNTL);
   createStructPaths(vNTL);
   }

free(tmp_line);

#ifdef DEBUG
printf("Leaving listHeader\n");
#endif

return header.next;
}



void freeHeaderFile(struct line *base)
{
struct line *ptr;

if (base == (struct line *)NULL)
   return;

ptr = base;                                                                             		// base of the list
if (ptr->next != (struct line *)NULL)                                                     		// walk to the ...
   ptr = ptr->next;
if (ptr->ln != (char *) NULL)
   free(ptr->ln);

free(ptr);
}



struct line *addToHeaderFile(int lvl, char *ln, char *type, char *dbg, struct line *base)
{
struct line *ptr, *new;

new = (struct line *)malloc(sizeof(struct line));							// create a new entry for the list
if (new != (struct line *) NULL) {
   memset(new, 0x00, sizeof(struct line));								// reset everything to 0
   new->lvl = lvl;											// what was the rec level

   new->ln = (char *)malloc(strlen(ln) + 1);								// create space for the header line
   strcpy(new->ln, ln);											// copy header line
   strcpy(new->dbg, dbg);										// debug info
if (strlen(type) > 0)
   strcpy(new->c_type, type);										// the type definition from the element tag
   new->next = (struct line *)NULL;									// terminate list

   ptr = base;												// base of the list
   while (ptr->next != (struct line *)NULL)								// walk to the ...
      ptr = ptr->next;											//             ... end of the list

   ptr->next = new;											// attach new line
   }

return new;
}



int checkForList(char *elem_tag, char *element, char *struct_h)
{
char *e_tag, *struct_ptr, *field_ptr;
char *struct_end_ptr, *field_end_ptr, *list_ptr;

if (struct_h == (char *)NULL)
   printf("we have a NULL problem\n");

e_tag = (char *)malloc(TMP_BUFF_SZ);

strcpy(e_tag, "struct ");										// this is going to look
strcat(e_tag, elem_tag);										//       something like:
strcat(e_tag, " {");											//    'struct mystruct {'

struct_ptr = strstr(struct_h, e_tag);									// find the structure the field is a member of

if (struct_ptr != (char *)NULL) {
   struct_end_ptr = strstr(struct_ptr, "};");								// find the end of that structure
   if (struct_end_ptr != (char *)NULL) {
      field_ptr = strstr(struct_ptr, element);								// find the field/member
      if (field_ptr != (char *)NULL) {
         if (field_ptr < struct_end_ptr) {								// needs to be within the same structure
            field_end_ptr = strstr(field_ptr, ";");							// find the end of the field
            list_ptr = strstr(field_ptr, "[");								// find the square bracket
            if (list_ptr != (char *)NULL) {
               if (list_ptr < field_end_ptr) {								// if it comes before the end of the field it is always a list
                  return 1;
                  }
               }  // end '[' check

            list_ptr = strstr(field_ptr, "**");
            if (list_ptr != (char *)NULL) {
               if (list_ptr < field_end_ptr) {
                  return 1;
                  }
               }

            }
         }
      }
   }

free(e_tag);

return 0;
}



char *findLastMember(char *str)
{
char *ptr;

ptr = &(str[strlen(str)-1]);										// point at the last character

while (ptr >= str) {
   if (ptr[0] == '.' || ptr[0] == '>')
      return ++ptr;
   else
      ptr--;
      }
return (char *)NULL;											// no . found
}



int checkForStruct(char *struct_path, char *struct_h)
{
char *mbr_ptr, *br_ptr, member[64], str_name[64], *struct_ptr;

if (struct_h == (char *)NULL)
   printf("we have a NULL problem\n");

mbr_ptr = findLastMember(struct_path);

strcpy(member, mbr_ptr);
br_ptr = strstr(member, "[");
if (br_ptr != (char *)NULL)
   *br_ptr = 0;

strcpy(str_name, "struct ");
strcat(str_name, member);
strcat(str_name, " ");

struct_ptr = strstr(struct_h, str_name);                                                                   // find the structure the field is a member of

if (struct_ptr != (char *)NULL)
   return 1;

return 0;
}



int checkElemCnt(char *element, struct elmPathCnt *base)
{
struct elmPathCnt *c_ptr, *new;

c_ptr = base;								// point at the start
if (c_ptr == (struct elmPathCnt *)NULL)
   return -1;								// nothing there, return

while (c_ptr->next != (struct elmPathCnt *)NULL) {			// as long if we have 'stuff'
   c_ptr = c_ptr->next;
   if (strcasecmp(c_ptr->elem, element) == 0) {				
      c_ptr->cnt++;							// if we already have the element/type we're looking for, add one occurence
      return c_ptr->cnt;						// and return the value
      }
   }

new = (struct elmPathCnt *)malloc(sizeof(struct elmPathCnt));		// apparently we don't have this one yet, so create one.
memset(new, 0x00, sizeof(struct elmPathCnt));				// set to 0x00
new->next = (struct elmPathCnt *)NULL;					// just to make sure.
strcpy(new->elem, element);						// copy the name
new->cnt = 0;								// set count to 0
c_ptr->next = new;							// attach to end of the list

return 0;								// we always created a new one here.
}



int countPaths(struct line *paths)
{
struct line *c_ptr;
int cnt = 0;

if (paths == (struct line *)NULL)                                       // we're looking at the 'base' next one is 1st one
   return 0;

c_ptr = paths;

while (c_ptr->next != (struct line *)NULL) {
   c_ptr = c_ptr->next;
   cnt++;
   }

return cnt;
}



void *addToNamesList(char *ln, struct line *fld_nm_lst, struct varNameTypeList *vNTL, int type)
{
struct line *ptr, *new;

new = (struct line *)malloc(sizeof(struct line));							// create a new entry for the list
if (new != (struct line *) NULL) {
   memset(new, 0x00, sizeof(struct line));								// reset everything to 0
   new->lvl = 0;											// what was the rec level

   new->ln = (char *)malloc(strlen(ln) + 1);								// create space for the header line
   strcpy(new->ln, ln);											// copy header line
   strcpy(new->dbg, "");										// debug info
   new->type = type;
   new->next = (struct line *)NULL;									// terminate list

   ptr = fld_nm_lst;											// base of the list
   while (ptr->next != (struct line *)NULL) 								// walk to the ...
      ptr = ptr->next;											//             ... end of the list
 
   ptr->next = new;											// attach new line
   }

return new;
}



int nstLst(char *str)											// count nested lists
{
char *b_ptr;												// bracket pointer
int   b_cnt = 0;

#ifdef DEBUG
printf("nstLst: %s\n", str);
#endif

b_ptr = str;
while (b_ptr != (char *)NULL) {
   b_ptr = strstr(b_ptr+1, "[");
   b_cnt++;
   }

return (b_cnt-1)/2;
}



struct varNameTypeList *addTypeTovNTL(struct varNameTypeList *vNTL, struct elm_str *elem)					// keep a list of all names and types
{
struct varNameTypeList *c_ptr, *new;

if (vNTL == (struct varNameTypeList *)NULL)
   return (struct varNameTypeList *)NULL;

c_ptr = vNTL;
while (c_ptr->next != (struct varNameTypeList *)NULL) {
   c_ptr = c_ptr->next;
   if (strcmp(c_ptr->varName, elem->element) == 0) {
      return (struct varNameTypeList *)NULL;
      }
   }

new = (struct varNameTypeList *)malloc(sizeof(struct varNameTypeList));

new->next = (struct varNameTypeList *)NULL;
strcpy(new->varName, elem->element);

if (strlen(elem->element_ctype) > 0) {
   if (strcmp(elem->element_ctype, "__list") == 0)
      new->varList = vNTL_LIST;
   else {
      new->varList = vNTL_NO_LIST;
      strcpy(new->varType, elem->element_ctype);
      }
   }

c_ptr->next = new;

#ifdef DEBUG
if (new->varList == vNTL_LIST)
   printf("vNTL: %s - %s is a list.\n", new->varName, new->varType);
else
   printf("vNTL: %s - %s\n", new->varName, new->varType);
#endif

return new;
}


struct varNameTypeList *vNTLset(struct varNameTypeList *vNTL, char *vName, char *vType)
{
struct varNameTypeList *c_ptr;

c_ptr = vNTL;
while (c_ptr->next != (struct varNameTypeList *)NULL) {
   c_ptr = c_ptr->next;
   if (strcmp(c_ptr->varName, vName) == 0) {
      strcpy(c_ptr->varType, vType);
      return c_ptr;
      }
   }
return (struct varNameTypeList *)NULL;
}



struct varNameTypeList *vNTLclean(struct varNameTypeList *vNTL)
{
struct varNameTypeList *p_ptr, *c_ptr;

if (vNTL == (struct varNameTypeList *)NULL)
   return (struct varNameTypeList *)NULL;					// no vNTLs yet

p_ptr = vNTL;									// point at ...
c_ptr = vNTL->next;								//          ... the first real entry
while (c_ptr != (struct varNameTypeList *)NULL) {
   if (strstr(c_ptr->varType, "struct") != (char *)NULL) {
      p_ptr = c_ptr;								// move p_ptr to next entry
      c_ptr = p_ptr->next;							// move c_ptr to the one after p_ptr
      }
   else {
      p_ptr->next = c_ptr->next;						// point at one after current
      free(c_ptr);								// throw away the current;
      c_ptr = p_ptr->next;							// point c_ptr at the one after p_ptr;
      }
   }
return vNTL;
}



int vNTLapply(struct varNameTypeList *vNTL, struct line *base)
{
struct line *cl_ptr;
struct varNameTypeList *cv_ptr;
char *nws_ptr;
int ret=0;

cl_ptr = base;
if (cl_ptr == (struct line *)NULL)
   return -1;									// nothing here

cv_ptr = vNTL;
if (cv_ptr == (struct varNameTypeList *)NULL)
   return -1;									// nothing here

while (cv_ptr->next != (struct varNameTypeList *)NULL) {			// for every next vNTL entry not NULL
   cv_ptr = cv_ptr->next;							// go to next entry
   cl_ptr = base;								// point at base of header list
   while (cl_ptr->next != (struct line *)NULL) {				// for every header line
      cl_ptr = cl_ptr->next;							// go to next line if there is one
      nws_ptr = strstr(cl_ptr->ln, cv_ptr->varType);				// No WhiteSpace ptr, where the declaration starts
      if (nws_ptr != (char *)NULL				&&		// start of struct declaration
         strstr(cl_ptr->ln, "{") == (char *)NULL		&&		// but not a definition
         cv_ptr->varList == vNTL_LIST) {					// and the type is a list
         if (nws_ptr[strlen(cv_ptr->varType)] != ' ') {				// check if it is exactly the type, and not start the same
#ifdef DEBUG
            printf("BAD vNTLapply: |%s| -- |%s|\n", cl_ptr->ln, cv_ptr->varType);
#endif
            }
         else {
            sprintf(nws_ptr, "%s **%s", cv_ptr->varType, cv_ptr->varName);		// change to something like "struct mystruct **mstruct"
            ret++;
            }
         }
      }
   }
return ret;
}



char *getVarNameFromPath(char *path, char *varname)
{
char *c_ptr, *vn_ptr;

c_ptr = &(path[strlen(path)-1]);					// point at the last char
while (c_ptr[0] != '.' && c_ptr[0] != '>' && c_ptr > path)		// find the last '.' or '>' whichever comes first
   c_ptr--;

if (c_ptr[0] == '.' || c_ptr[0] == '>') {				// unlikely, but we could be pointing at path
   c_ptr++;								// point at first char of the variable name
   vn_ptr = varname;
   while (c_ptr[0] != 0 && c_ptr[0] != '[') {				// copy all characters of the variable name
     vn_ptr[0] = c_ptr[0];						// copy the char
     vn_ptr++;								// advance variable name pointer
     c_ptr++;								// advance current pointer
     }
   vn_ptr[0] = 0;							// terminate string
   }
return varname;
}



int vNTLtype(char *path, struct varNameTypeList *vNTL)
{
struct varNameTypeList *c_ptr;
char varname[EL_NAME_SIZE];

getVarNameFromPath(path, varname);				// separate variable name from the path

#ifdef DEBUG
#ifdef DATAPARSE
printf("vNTLtype: %s - %s,", varname, path);
#endif
#endif

c_ptr = vNTL;							// point at what we have

if (c_ptr == (struct varNameTypeList *)NULL)
   return 0;							// there's nothing here.

while (c_ptr->next != (struct varNameTypeList *)NULL) {		// as long as there are entries
   c_ptr = c_ptr->next;
   if (strcmp(varname, c_ptr->varName) == 0) {			// does the name compare
      if (c_ptr->varList ==  vNTL_LIST) {			// is it a variable length list?
#ifdef DEBUG
#ifdef DATAPARSE
printf(" is a variable type list.\n");
#endif
#endif
         return 1;						// yup
         }
#ifdef DEBUG
#ifdef DATAPARSE
printf("\n");
#endif
#endif
      }
   }

return 0;							// nope. or not in the vNTL list.
}



char *vNTLtypeByPath(char *path, struct varNameTypeList *vNTL)
{
struct varNameTypeList *c_ptr;
char varname[EL_NAME_SIZE];

getVarNameFromPath(path, varname);

c_ptr = vNTL;

if (c_ptr == (struct varNameTypeList *)NULL)
   return 0;

while (c_ptr->next != (struct varNameTypeList *)NULL) {
   c_ptr = c_ptr->next;
   if (strcmp(varname, c_ptr->varName) == 0) {
      return c_ptr->varType;
      }
   }

return (char *)NULL;
}



char *getvNTLlistName(char *path)
{
char *br_ptr;
int  cnt = 0;


br_ptr = &(path[strlen(path)-1]);			// point to last char in path

while (br_ptr[0] != ']' && br_ptr > path)		// find the last bracket after start of string
   br_ptr--;

if (br_ptr == path)					// start of string, there is no bracket
   return path;

cnt++;
while (cnt != 0 && br_ptr > path) {			// find the matching start bracket
   br_ptr--;
   if (br_ptr[0] == ']')				// another pair here
      cnt++;
   if (br_ptr[0] == '[')
      cnt--;
   if (cnt == 0)					// we are pointing at the matching bracket now
      br_ptr[0] = 0;
   }
return path;
}




#ifdef DATAPARSE
char *parseElementContent(char *element_start, char *element_end, char *elem_tag,			// definition for parser in data parse 'mode'
                          char *buffstart, char **par_elem_data, int *prcss_cntnt,
                          int task, struct line *base, char *struct_h, 
                          struct line *fld_nm_lst, char *c_struct_path, struct config *config, struct varNameTypeList *vNTL, int verbose)
#else
char *parseElementContent(char *element_start, char *element_end, char *elem_tag,			// definition of parser not in data parse "mode'
                          char *buffstart, char **par_elem_data, int *prcss_cntnt,
                          int task, struct line *base, char *struct_h,
                          struct line *fld_nm_lst, char *c_struct_path, struct varNameTypeList *vNTL, int verbose)
#endif
{
char start_tag[EL_NAME_SIZE+4], end_tag[EL_NAME_SIZE+4], hf_line[128],
     *content_start, *content_end, *next_content, *elem_data, *elem_val_ptr, new_cs_path[256],
      tmp_cs_path[256], *ncp_ptr, **field, varName[32], vNTLvarName[32], *sw_ptr;
int  parsing=1, nest_elem=0, processed=0, process_content=0, fld_idx, sw_idx;
struct elmPathCnt elemCntBase;
struct elm_str Elem;
void *new, **structList;

memset(&elemCntBase, 0x00, sizeof(struct elmPathCnt));

// increase recursionlevel
r_lvl++;										// we should make that local instead of global ....


elem_data = (char *) NULL;
Elem.element[0] = 0;									// make sure element is initially an empty string
Elem.element_ctype[0] = 0;
new_cs_path[0] = 0;

sprintf(start_tag, "<%s", elem_tag);							// create element start tag
sprintf(end_tag, "</%s>", elem_tag);							// create element end tag

if (element_start != (char *)NULL) {
   content_start = strcasestr(element_start, start_tag);				// point at start of current content
   if (content_start != (char *)NULL) {
      content_start = strstr(content_start, ">") + 1;
      content_end   = strcasestr(content_start, end_tag);				// point at end of current content
      if (content_end != (char *)NULL) {
         content_end--;									// content ends before the end tag
         next_content = strstr(content_end, ">");					// find next chunk of content
         if (next_content != (char *)NULL) {
            next_content++;
            }										// content start after a tag if there is any
         }
      else {
         printf("Missing %s element\n", end_tag);
         }
      }
   else {
      content_end = (char *)NULL;
      printf("Missing %s element\n", start_tag);
      }
   }
else
   {
   content_start = (char *)NULL;							// element start was NULL
   content_end   = (char *)NULL;							// didn't see an end tag
   }

if (content_start != (char *)NULL && content_end != (char *)NULL) {
   while (parsing) {
      parsing = findElement(content_start, content_end, &Elem, buffstart);		// as long as we keep finding a next element in this element/content
      if (strlen(Elem.element) > 0) {
         if (strlen(Elem.element_ctype) == 0)
            strcpy(Elem.element_ctype, DEFAULT_TYPE);
         addTypeTovNTL(vNTL, &Elem);							// keep track of the vNTL types
#ifdef DEBUG
         printf("findElement found variable '%s' of type '%s' [%d-%d-%d]\n", Elem.element, Elem.element_ctype, parsing, processed, process_content);
#endif
         }
      if (parsing == 1) {								// IF there is an element in this one we won't have element data
         nest_elem = 1;									//  ... on this level when done
         if (processed == 0 && process_content == 0) {
            switch (task) {
               case DISPLAY: {
                  printf("Populating new structure.\n");
                  break;
                  }
               case CREATE_STRUCT_PATHS:
               case CREATE_STRUCT: {
                  setIndent(r_lvl-1);
                  sprintf(hf_line, "struct %s", elem_tag);
                  vNTLset(vNTL, elem_tag, hf_line);
                  sprintf(hf_line, "%sstruct %s {\n", indent, elem_tag);
#ifdef DEBUG
if (strlen(Elem.element_ctype) > 0)
   printf("ATH-1: %s [%s - %s]\n", hf_line, Elem.element, Elem.element_ctype);
else
   printf("ATH-1: %s\n", hf_line);
#endif
                  stripTrailingWhiteSpace(hf_line);
                  addToHeaderFile(r_lvl-1, hf_line, Elem.element_ctype, "-1-", base);
                  resetIndent(r_lvl-1);
                  break;
                  }
               default: {
                  break;
                  }
               } 
            }

         if (task == CREATE_STRUCT_PATHS) {
            strcpy(new_cs_path, c_struct_path);						// copy the path
            if (strcmp(new_cs_path, "config") == 0)
               strcat(new_cs_path, "->");
            else {
#ifdef DATAPARSE
#ifdef DEBUG
printf("vNTLtype reference: %s\n", new_cs_path);
#endif
#endif
#ifdef DATAPARSE
               getVarNameFromPath(new_cs_path, vNTLvarName);
               if (structIdx(vNTLvarName) >= 0)
#else
               if (vNTLtype(new_cs_path, vNTL) == 1)
#endif
                  strcat(new_cs_path, "->");
               else
                  strcat(new_cs_path, ".");
               }
            strcat(new_cs_path, Elem.element);						// new field/element
            ncp_ptr = &(new_cs_path[strlen(new_cs_path)]);				// point at string terminator
            if (checkForList(elem_tag, Elem.element, struct_h) == 1) {			// check if 'struct field' is a list'
               fld_idx = checkElemCnt(Elem.element, &elemCntBase);			// check to see how often we saw it on this level
#ifdef DATAPARSE
               sprintf(ncp_ptr, "[%d]", fld_idx);					// add index/occurence
#ifdef DEBUG
printf("New Path: %s\n", new_cs_path);
#endif

               getVarNameFromPath(new_cs_path, varName);
               structList = (void **)getMemberPtr(new_cs_path, config);			// get the pointer again
#ifdef DEBUG
printf("The value of structList is:     %p\n", structList);
printf("The direction of structList is: %p\n", &structList);
#endif
               new = structAlloc(varName);						// allocate memory fopr this new thing
               *structList = addStruct(*structList, new);				// add it to the config tree
#else
               sw_ptr = new_cs_path;
               sw_idx = -1;
               while (sw_ptr != (char *)NULL) {
                  sw_ptr = strstr(++sw_ptr, "[idx[");
                  sw_idx++;
                  }
               sprintf(ncp_ptr, "[idx[%d]]", sw_idx);					// add index/occurence, passed through an array
#ifdef DEBUG
printf("ncp_ptr: %s\n", new_cs_path);
#endif
#endif
               }
            if (checkForStruct(new_cs_path, struct_h) == 1)
               addToNamesList(new_cs_path, fld_nm_lst, vNTL, TYPE_STRUCT);		// this pretty crude
            else
               addToNamesList(new_cs_path, fld_nm_lst, vNTL, TYPE_CHAR);		// and when types are implemented we probably have to do better.
            }

#ifdef DATAPARSE
         content_start = parseElementContent(content_start, content_end, Elem.element, buffstart,	// call in data parse 'mode'
                                             &elem_data, &process_content, task, base, struct_h,	// recursively find elements/objects, these are structures part of a 'super structure'
                                             fld_nm_lst, new_cs_path, config, vNTL, verbose);		// part of a 'super structure'
#else
         content_start = parseElementContent(content_start, content_end, Elem.element, buffstart,	// call when not in data parse 'mode'
                                             &elem_data, &process_content, task, base, struct_h,
                                             fld_nm_lst, new_cs_path, vNTL, verbose);			// recursively find elements/objects, these are structures part of a 'super structure'
#endif
#ifdef DEBUG
         if (elem_data != (char *) NULL) {
            if (processed == 0 || process_content == 1) {
               if (strlen(new_cs_path) > 0) {
                  elem_val_ptr = elemValPtr(elem_data);
                  str2NamesListTemplate(new_cs_path, tmp_cs_path);
                  printf("Path: %s -- [%s]\n", tmp_cs_path, elem_val_ptr);
                  }
               }
            }
         else {
            str2NamesListTemplate(new_cs_path, tmp_cs_path);
            printf("Path: %s\n", tmp_cs_path);
            }
#endif

         if (processed == 0 || process_content == 1) {
            elem_val_ptr = elemValPtr(elem_data);
#ifdef DEBUG
printf("elem_data - elem_val_ptr: %s - %s\n", elem_data, elem_val_ptr);
#endif
            switch (task) {
               case DISPLAY: {
                  if (elem_val_ptr != (char *)NULL)
                     printf("Parent name is %s, the element name is %s (%s) and the content is %s\n", elem_tag, elem_data, Elem.element, elem_val_ptr);
                  else
                     printf("Parent name is %s, the element name is %s (%s) and there is no content.", elem_tag, elem_data, Elem.element);
                  break;
                  }
               case CREATE_STRUCT: {
                  if (elem_data != (char *)NULL) {
                     setIndent(r_lvl);
                     sprintf(hf_line, "%schar *%s\n", indent, elem_data);
                     stripTrailingWhiteSpace(hf_line);
                     strcat(hf_line, ";");
#ifdef DEBUG
                     if (strlen(Elem.element_ctype) > 0)
                        printf("ATH-2: %s [%s]\n", hf_line, Elem.element_ctype);
                     else
                        printf("ATH-2: %s\n", hf_line);
#endif
                     addToHeaderFile(r_lvl, hf_line, Elem.element_ctype, "-2-", base);
                     resetIndent(r_lvl);
                     }
#ifdef DEBUG
                  else
                     printf("elem_data is NULL\n");
#endif
                  break;
                  }
#ifdef DATAPARSE
               case POPULATE_STRUCT: {			// same as CREATE_STRUCT_PATHS, we need to change that
                  if (elem_data != (char *) NULL) {
                     if (processed == 0 || process_content == 1) {
                        if (strlen(new_cs_path) > 0) {
                           if (verbose == VERBOSE)
                              printf("Populating:  %s: %s\n", new_cs_path, elem_val_ptr);
                           elem_val_ptr = getElemVal(elem_data);					// pointer to data of element
                           field = (char **)getMemberPtr(new_cs_path, config);				// get the pointer to char * in a structure
                           if (field == (char **)NULL)							// this should fail for a list like   a.b.c.d[..]
                              getvNTLlistName(new_cs_path);						// get rid of the last index
                           else 
                              *field = elem_val_ptr;							// change it OR create another struct if it is a struct !!!
                           }
                        }
                     else {		// not processed  and not process_content
                        if (processed == 0 || process_content == 0) {
printf("0 - 0: %s\n", new_cs_path);
                           }
                        }
                     }
                  break;
                  }
#endif
               default: {
                  break;
                  }
               }
            processed = 1;										// to prevent multiple evaluations of same data from happening
            process_content = 0;									// force evaluation from data we know for sure doesn't have nested elements
            }
         }
      else {
         if (parsing == -1) {										// found a bad tag
            free(buffstart);										// be nice and release memory
            exit(-1);											// just bail out
            }
         if (nest_elem == 0) {										// no nested elements, it's data (fill struct fields here).
            *par_elem_data = processElementContent(elem_tag, content_start, content_end);		// process the content of element 'elem_tag'
            *prcss_cntnt = 1;										// we need to let the previous process know there was no tag and needs processed
            }
         }
      }
   }
else
   {
   if (content_start == (char *)NULL && content_end != (char *)NULL)					// found an end tag without a start tag
      printf("No element start tag found. (<%s>).\n", elem_tag);
   if (content_start != (char *)NULL && content_end == (char *)NULL)					// found a start tag without an end tag.
      printf("No matching element end tag found. (</%s>)\n", elem_tag) ;
   }

if (elem_data != (char *)NULL)										// if elem_data is not pointing "ins blaue hinein"
   free(elem_data);											// free it.

if (nest_elem == 1 && task == CREATE_STRUCT) {
   setIndent(r_lvl);
   sprintf(hf_line, "%s}\n", indent);
   stripTrailingWhiteSpace(hf_line);
   strcat(hf_line, ";");
#ifdef DEBUG
if (strlen(Elem.element_ctype) > 0)
   printf("ATH-3: %s [%s]\n", hf_line, Elem.element_ctype);
else
   printf("ATH-3: %s\n", hf_line);
#endif
   addToHeaderFile(r_lvl, hf_line, Elem.element_ctype, "-3-",  base);					// was a typo,  this was r_lvl-1   in the closing bracket section !!!!
   resetIndent(r_lvl);
   }

// decrease recursionlevel
r_lvl--;

return next_content;
}



void freevNTL(struct varNameTypeList *vNTL)
{
struct varNameTypeList *c_ptr;

if (vNTL == (struct varNameTypeList *)NULL)
   return;

c_ptr = vNTL;
if (c_ptr->next != (struct varNameTypeList *)NULL)
   freevNTL(c_ptr->next);

#ifdef DEBUG
if (c_ptr->varList == vNTL_LIST)
   printf("vNTL: %s - %s is a list.\n", c_ptr->varName, c_ptr->varType);
else
   printf("vNTL: %s - %s\n", c_ptr->varName, c_ptr->varType);
#endif

free(c_ptr);
}



#ifdef DATAPARSE
int parseConfigFile(char *cfg_file, int task, struct line *base, struct line *fld_nm_lst, struct config *config, struct varNameTypeList *vNTL, int verbose)		// parsing config file, in data parse 'mode'
#else
int parseConfigFile(char *cfg_file, int task, struct line *base, struct line *fld_nm_lst, struct varNameTypeList *vNTL, int verbose)					// parsing config file when not in data parse 'mode'
#endif
{
struct line *new;
char *txt_config, *struct_h, *end_ptr, *root_data, config_tag[16], conf_path[] = "config";
int prc_cnt, lst_sz;

#ifdef MD5_CONFIG_CHECK
int abort;
char *xml_conf;
#endif

root_data = (char *)NULL;										// is recursively used by parseElemContent, not here.
struct_h  = (char *) NULL;										// buffer for structire header file

memset(vNTL, 0x00, sizeof(struct varNameTypeList));

strcpy(config_tag, "config");		//     !!!!!  Do NOT change !!! 				// A configuration is ALWAYS in an element named this.
txt_config = readConfigFile(cfg_file);									// read in the config file
if (txt_config !=  (char *)NULL) {
   end_ptr = txt_config;										// also point at the last byte ...
   end_ptr += strlen(txt_config);									//                        ... of this buffer

   if (task == CREATE_STRUCT_PATHS)
      struct_h = readConfigFile("./parse-inc/config-structs.h");

#ifdef DATAPARSE
#ifdef MD5_CONFIG_CHECK
abort = configMD5Checksums(txt_config, cfg_file);
   if (abort != 0) {
      printf("FATAL: configuration mismatch. Aborting.\n");
      exit(abort);
      }
#endif
   parseElementContent(txt_config, end_ptr, config_tag, txt_config, &root_data,				// parse the content of an element from txt_config to end_ptr in data parse 'mode'
                       &prc_cnt, task, base, struct_h, fld_nm_lst, conf_path, config, vNTL, verbose);
#else
   if (task == CREATE_STRUCT_PATHS)
      addToNamesList("config", fld_nm_lst, vNTL, TYPE_STRUCT);
   parseElementContent(txt_config, end_ptr, config_tag, txt_config, &root_data,				// parse the content of an element from txt_config to end_ptr
                       &prc_cnt, task, base, struct_h, fld_nm_lst, conf_path, vNTL, verbose);
#endif

   free(txt_config);											// free the config txt buffer
   if (struct_h != (char *)NULL)
      free(struct_h);											// free structures header file buffer
   if (root_data != (char *) NULL)
      free(root_data);
   vNTLclean(vNTL);											// just keep the structures
   }
else
   return -1;

return 0;
}

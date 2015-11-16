/*
 * 
 *   confpar-structs.h
 *
 *   config file parser header
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

#include "defaults.h"

struct line {
   int lvl;
   int tag;
   int type;	// 1 = char
   char c_type[32];
   char *ln;
   char dbg[5];
   struct line *next;
   };

struct elmPathCnt {
   char elem[32];
   int  cnt;
   struct elmPathCnt *next;
   };

struct elm_str {
   char element[EL_NAME_SIZE];
   char element_type[EL_NAME_SIZE];
   char element_ctype[EL_NAME_SIZE];
   };

struct varNameTypeList {
   char varName[EL_NAME_SIZE];
   char varType[EL_NAME_SIZE];
   char varList;
   struct varNameTypeList *next;
   };

/*
 * 
 *   confpar.h
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


char *stripLeadingWhiteSpace(char *);
char *stripTrailingWhiteSpace(char *);
char *stripWhiteSpace(char *);
char *str2NamesListTemplate(char *, char *);
struct line *findClosingStructBracket(struct line *);
int checkElementName(char *, char *);
void countLtGtTokens(int *, int *, char *, char *);
char *prepConfigFileLine(char *, int);
char *readConfigFile(char *);
int  findLineNumber(char *, char *);
int  findElement(char *, char *, struct elm_str *, char *);
int  strPrintTag(char*, char *);
int  printContent(char *, char *);
char *processElementContent(char *, char *, char *);
char *elemValPtr(char *);
void setIndent(int);
void resetIndent(int);
struct line *findNextLine(struct line *, char *, char *);
struct line *findNextOccurence(struct line *, char *);
struct line *findNextStruct(struct line *);
struct line *findEmptyStruct(struct line *);
int removeEmptyStruct(struct line *, struct line *);
void removeStruct(struct line *, struct line *);
struct line *findStructMember(struct line *, char *);
int removeStructField(struct line *, char *);
int countStructMembers(struct line *);
int countStructFieldsOccurences(struct line *, char *);
struct line *collateFields(struct line *, struct line *);
struct line *collateStructures(struct line *);
char *getStructName(char *);
struct line *listHeaderFile(struct line *, struct varNameTypeList *, int);
void freeHeaderFile(struct line *);
struct line *addToHeaderFile(int, char *, char *, char *, struct line *);
int checkForList(char *, char *, char *);
int checkForStruct(char *, char *);
int checkElemCnt(char *, struct elmPathCnt *);
int countPaths(struct line *);
void *addToNamesList(char *, struct line *, struct varNameTypeList *, int);
struct varNameTypeList *addTypeTovNTL(struct varNameTypeList *, struct elm_str *);
int vNTLapply(struct varNameTypeList *, struct line *);
void freevNTL(struct varNameTypeList *);
char *getVarNameFromPath(char *, char *);
int vNTLtype(char *, struct varNameTypeList *);
char *vNTLtypeByPath(char *, struct varNameTypeList *);
char *getvNTLlistName(char *);
#ifdef DATAPARSE
struct line *freeConfigStructContent(struct config *, struct varNameTypeList *, int);
char *parseElementContent(char *, char *, char *, char *, char **, int *, int, struct line *, char *, struct line *, char *, struct config *, struct varNameTypeList *, int);
int  parseConfigFile(char *, int, struct line *, struct line *, struct config *, struct varNameTypeList *, int);
#else
char *parseElementContent(char *, char *, char *, char *, char **, int *, int, struct line *, char *, struct line *, char *, struct varNameTypeList *,  int);
int  parseConfigFile(char *, int, struct line *, struct line *, struct varNameTypeList *, int);
#endif

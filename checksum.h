/*
 * 
 *   checksum.h
 *
 *   md5 checksums for config file parser 
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

char *configFileStruct(char *);
char *configFileData(char *);
char *md5Checksum(char *);
int   createMD5checksums(char *);
int   configMD5Checksums(char *, char *);


/*
 * 
 *   path-switch.h
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

int countSwitchPaths(struct line *);
int createStructPaths(struct varNameTypeList *);
int createSwitchPaths(struct line *, int, struct varNameTypeList *);

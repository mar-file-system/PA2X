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

#define CFG_LINE_SZ	256
#define TMP_BUFF_SZ     128
#define EL_NAME_SIZE     64                                             // max length of the element/tag name
#define RSRV_LST         33                                             // length of reserved words list

#define IDX_LLEN	 16						// index list length

#define	VALID		 0
#define	EOL		-1						// end of list
#define	INCOMPLETE	-2						// missing index

#define	EOL_ERR		-1
#define NULL_PATH	-2

#define VNTL_EMPT	 0
#define VNTL_ACTV	 1 

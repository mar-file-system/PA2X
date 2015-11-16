SHELL = /bin/bash
CC = gcc

LOG_FLAGS = -lm
MD5_FLAGS = -lssl -lcrypto

#LIBS = -pthread
LIBS =
MD5_OPTIONS = -DMD5_CONFIG_CHECK -D_GNU_SOURCE -g
#OPTIONS = -D_GNU_SOURCE -g -Wall
OPTIONS = -D_GNU_SOURCE -g
PARSE_OPT = -DDATAPARSE
DEBUG   = -DDEBUG

all:		datapars
md5-all:	md5-datapars
base:		confpars
md5-base:	md5-confpars
debug:  confpars-d datapars-d

examples: example-1 example-2 example-3 example-4 
md5-examples: example-5 example-6

confpars:
	$(CC) -c confpars.c -o confpars.o $(OPTIONS)
	$(CC) -c path-switch.c -o path-switch.o $(OPTIONS)
	$(CC) -c mainpars.c -o mainpars.o $(OPTIONS)
	$(CC) $(LIBS) confpars.o path-switch.o mainpars.o -o confpars $(OPTIONS)
	-rm *.o

md5-confpars:
	$(CC) -c confpars.c -o confpars.o $(OPTIONS)
	$(CC) -c path-switch.c -o path-switch.o $(OPTIONS)
	$(CC) -c checksum.c -o checksum.o $(MD5_OPTIONS)
	$(CC) -c mainpars.c -o mainpars.o $(MD5_OPTIONS)
	$(CC) $(LIBS) confpars.o path-switch.o checksum.o mainpars.o -o md5-confpars $(MD5_OPTIONS) $(MD5_FLAGS)
	-rm *.o


datapars:
#	$(shell ./confpars '$(conf)')
	$(CC) -c confpars.c -o valpars.o $(OPTIONS) $(PARSE_OPT)
	$(CC) -c path-switch.c -o pars-path-switch.o $(OPTIONS) $(PARSE_OPT)
	$(CC) -c parsedata.c -o parsedata.o $(OPTIONS) $(PARSE_OPT)
	$(CC) -c mainpars.c -o maindatapars.o $(OPTIONS) $(PARSE_OPT)
	$(CC) $(LIBS) valpars.o pars-path-switch.o parsedata.o maindatapars.o -o datapars
	-rm *.o

so-datapars:
	$(CC) -c confpars.c -o valpars.o $(OPTIONS) $(PARSE_OPT)
	$(CC) -c path-switch.c -o pars-path-switch.o $(OPTIONS) $(PARSE_OPT)
	$(CC) -c parsedata.c -o parsedata.o $(OPTIONS) $(PARSE_OPT)
	$(CC) -c mainpars.c -o maindatapars.o $(OPTIONS) $(PARSE_OPT)
	$(CC) $(LIBS) valpars.o pars-path-switch.o parsedata.o maindatapars.o -o datapars
	-rm *.o

md5-datapars:
	$(shell ./md5-confpars '$(conf)')
	$(CC) -c confpars.c -o valpars.o $(OPTIONS) $(PARSE_OPT)
	$(CC) -c path-switch.c -o pars-path-switch.o $(OPTIONS) $(PARSE_OPT)
	$(CC) -c parsedata.c -o parsedata.o $(OPTIONS) $(PARSE_OPT)
	$(CC) -c checksum.c -o checksum.o $(MD5_OPTIONS)
	$(CC) -c mainpars.c -o maindatapars.o $(OPTIONS) $(PARSE_OPT)
	$(CC) $(LIBS) valpars.o pars-path-switch.o parsedata.o checksum.o maindatapars.o -o md5-datapars $(MD5_FLAGS)
	-rm *.o

confpars-d:
	$(CC) -c confpars.c -o confpars.o $(OPTIONS) $(DEBUG)
	$(CC) -c path-switch.c -o path-switch.o $(OPTIONS) $(DEBUG)
	$(CC) -c mainpars.c -o mainpars.o $(OPTIONS) $(DEBUG)
	$(CC) $(LIBS) confpars.o path-switch.o mainpars.o -o confpars $(DEBUG)
	-rm *.o

datapars-d:
	$(CC) -c confpars.c -o valpars.o $(OPTIONS) $(PARSE_OPT) $(DEBUG)
	$(CC) -c mainpars.c -o maindatapars.o $(OPTIONS) $(PARSE_OPT) $(DEBUG)
	$(CC) $(LIBS) valpars.o maindatapars.o -o datapars
	-rm *.o

example-1:
	$(CC) -c confpars.c -o confpars.o $(OPTIONS) $(PARSE_OPT)
	$(CC) -c path-switch.c -o path-switch.o $(OPTIONS) $(PARSE_OPT)
	$(CC) -c example-1.c -o example-1.o
	$(CC) -c parsedata.c -o parsedata.o $(OPTIONS) $(PARSE_OPT)
	$(CC) $(LIBS) confpars.o path-switch.o example-1.o parsedata.o -o example-1
	-rm example-1.o parsedata.o

example-2:
	$(CC) -c confpars.c -o confpars.o $(OPTIONS) $(PARSE_OPT)
	$(CC) -c path-switch.c -o path-switch.o $(OPTIONS) $(PARSE_OPT)
	$(CC) -c parsedata.c -o parsedata.o $(OPTIONS) $(PARSE_OPT)
	$(CC) -c example-2.c -o example-2.o $(OPTIONS) $(PARSE_OPT)
	$(CC) $(LIBS) confpars.o parsedata.o path-switch.o example-2.o -o example-2
	-rm *.o

example-3:
	$(CC) -c confpars.c -o confpars.o $(OPTIONS) $(PARSE_OPT)
	$(CC) -c path-switch.c -o path-switch.o $(OPTIONS) $(PARSE_OPT)
	$(CC) -c parsedata.c -o parsedata.o $(OPTIONS) $(PARSE_OPT)
	$(CC) -c example-3.c -o example-3.o $(OPTIONS) $(PARSE_OPT)
	$(CC) $(LIBS) confpars.o parsedata.o path-switch.o example-3.o -o example-3
	-rm *.o

example-4:
	$(CC) -c confpars.c -o confpars.o $(OPTIONS) $(PARSE_OPT)
	$(CC) -c path-switch.c -o path-switch.o $(OPTIONS) $(PARSE_OPT)
	$(CC) -c parsedata.c -o parsedata.o $(OPTIONS) $(PARSE_OPT)
	$(CC) -c example-4.c -o example-4.o $(OPTIONS) $(PARSE_OPT)
	$(CC) $(LIBS) confpars.o parsedata.o path-switch.o example-4.o -o example-4
	-rm *.o

example-5:
	$(CC) -c confpars.c -o confpars.o $(OPTIONS)
	$(CC) -c path-switch.c -o path-switch.o $(OPTIONS)
	$(CC) -c checksum.c -o checksum.o $(MD5_OPTIONS)
	$(CC) -c example-5.c -o example-5.o $(MD5_OPTIONS)
	$(CC) $(LIBS) confpars.o path-switch.o checksum.o example-5.o -o example-5 $(MD5_FLAGS)
	-rm *.o

example-6:
	$(CC) -c confpars.c -o confpars.o $(MD5_OPTIONS) $(PARSE_OPT)
	$(CC) -c path-switch.c -o path-switch.o $(MD5_OPTIONS) $(PARSE_OPT)
	$(CC) -c parsedata.c -o parsedata.o $(MD5_OPTIONS) $(PARSE_OPT)
	$(CC) -c checksum.c -o checksum.o $(MD5_OPTIONS) $(PARSE_OPT)
	$(CC) -c example-6.c -o example-6.o $(MD5_OPTIONS) $(PARSE_OPT)
	$(CC) $(LIBS) confpars.o parsedata.o path-switch.o checksum.o example-6.o -o example-6 $(MD5_FLAGS)
	-rm *.o

clean:
	-rm *.o datapars md5-datapars example-1 example-2 example-3 example-4 example-5 example-6

pristine:
	-rm *.o confpars md5-confpars datapars md5-datapars example-1 example-2 example-3 example-4 example-5 example-6
	-rm -rf ./parse-inc/*

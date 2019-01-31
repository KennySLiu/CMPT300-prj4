CC=gcc
CCFLAGS= -std=c99 -D _GNU_SOURCE -static

all: processAncestors arrayStats	

processAncestors: cs300_test_processancestors.c
	$(CC) $(CCFLAGS) cs300_test_processancestors.c -o processAncestors.o

arrayStats: cs300_test_arraystats.c
	$(CC) $(CCFLAGS) cs300_test_arraystats.c -o arrayStats.o


transfer: arrayStats.o processAncestors.o
	scp -P 2222 ./arrayStats.o root@localhost:~
	scp -P 2222 ./processAncestors.o root@localhost:~

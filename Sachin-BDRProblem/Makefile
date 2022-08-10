CC=g++
CFLAGS=-c -Wall -g
LDFLAGS = -lrt -lpthread -lboost_system -std=c++11

all: Dictionary TrieStore MemoryMgr

Dictionary: Dictionary.o TrieStore.o MemoryMgr.o
	$(CC) $(LDFLAGS) Dictionary.o TrieStore.o MemoryMgr.o -o Dictionary

Dictionary.o: Dictionary.cpp
	$(CC) $(CFLAGS) Dictionary.cpp $(LDFLAGS)

TrieStore.o: TrieStore.cpp
	$(CC) $(CFLAGS) TrieStore.cpp $(LDFLAGS)

MemoryMgr.o: MemoryMgr.cpp
	$(CC) $(CFLAGS) MemoryMgr.cpp $(LDFLAGS)

clean:
	rm -f *.o Dictionary

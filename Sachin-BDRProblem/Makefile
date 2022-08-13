CC=g++
CFLAGS=-c -Wall -g
LDFLAGS = -lrt -lpthread -lboost_system -std=c++11

all: Dictionary

Dictionary: main.o Dictionary.o TrieStore.o MemoryMgr.o
	$(CC) $(LDFLAGS) main.o Dictionary.o TrieStore.o MemoryMgr.o -o Dictionary

main.o: main.cpp
	$(CC) $(CFLAGS) main.cpp $(LDFLAGS)

Dictionary.o: Dictionary.cpp
	$(CC) $(CFLAGS) Dictionary.cpp $(LDFLAGS)

TrieStore.o: TrieStore.cpp
	$(CC) $(CFLAGS) TrieStore.cpp $(LDFLAGS)

MemoryMgr.o: MemoryMgr.cpp
	$(CC) $(CFLAGS) MemoryMgr.cpp $(LDFLAGS)

clean:
	rm -f *.o Dictionary

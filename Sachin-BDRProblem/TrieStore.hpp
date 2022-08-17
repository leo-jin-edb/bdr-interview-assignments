
#pragma once

#include "MemoryMgr.hpp"

#define IndexOf(ch) (ch - 'a')

const UInt8 MAX_NEXT_TSNODES = 26;


class TrieStore {

public:

	TrieStore(bool init);	

	DicStatus InsertWord(UInt32 i, const string word, const string definition);

	DicStatus DeleteWord(UInt32 i, const string word);

	DicStatus SearchWord(UInt32 i, const string word, string & definition);

private:

		pthread_mutex_t			mutex;

		UInt32 					nextTS[MAX_NEXT_TSNODES]; // 26*4 = 104 bytes

		// self word (at current location)
	
		UInt32					wordPtr;
		UInt16					defLen;
};


class TrieStoreMgr {

public:

	TrieStoreMgr(DicConfig * config);

	~TrieStoreMgr() {
		MemoryMgr::DeInitialize();
	}

	DicStatus InsertWord(string word, const string definition);
	DicStatus DeleteWord(string word);
	DicStatus SearchWord(string word, string & definition);

private:

	/* Head Trie node of each alphabet. This is stored/retried using MemMgr as AppData
	* */

	TrieStore * tsHead[MAX_NEXT_TSNODES];
};


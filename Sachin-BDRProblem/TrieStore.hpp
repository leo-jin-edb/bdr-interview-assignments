
#pragma once

#include "MemoryMgr.hpp"

#define IndexOf(ch) (ch - 'a')

const UInt8 MAX_NEXT_TSNODES = 26;

// TODO: Check if all state variables below need to be defined as volatile. Doesn't seem to be a need as of now, because every data access is only with 'SyncFlag' which is volatile.

/*
* 1) Holds specific store's info. Mapped in Shared Memory space.
* 2) When any word is deleted, we free 'selfWordPtr' memory & just mark it null. We don't delete Trie Node.
*
*/

class TrieStore {

public:

	// TODO: extra parameters added for debugging. Delete later.
	TrieStore(TrieStore * parent, char c);
	// TODO: ~TrieStore(); call MemMgr DeInitialize.
		
	DicStatus InsertWord(UInt32 i, const string word, const string definition);

	// TODO: memory leakage here is not handled as MemMgr do now support defragmented memory currently. 
	// This can be improved by adding free chunks management using buckets of size 32/64/128/256/512/etc. 12 to 16 such buckets(of lists of chunks) should suffice.
	DicStatus DeleteWord(UInt32 i, const string word);

	DicStatus SearchWord(UInt32 i, const string word, string & definition);
	

private:

		//interprocess_mutex 	mutex;
		pthread_mutex_t			mutex;

		TrieStore *			nextTS[MAX_NEXT_TSNODES]; // 26*4 = 104 bytes

		// self word (at current location)
	
		BPtr				wordPtr;
		UInt16				defLen;

		// TODO: debugging purpose. Delete later.
		TrieStore *			parent; 
		char 				c;
};





class TrieStoreMgr {

public:

	TrieStoreMgr(DicConfig * config);

	DicStatus InsertWord(string word, const string definition);
	DicStatus DeleteWord(string word);
	DicStatus SearchWord(string word, string & definition);

private:

	/* Head Trie node of each alphabet. This is stored/retried using MemMgr as AppData
	* */

	TrieStore * tsHead[MAX_NEXT_TSNODES];
};


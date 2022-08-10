
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
class TrieStoreHead;

class TrieStore {

public:

	TrieStore(DicConfig& config);

	DicStatus InsertWord(const string word, const string definition, TrieStoreHead * tsHead);
	DicStatus DeleteWord(const string word, TrieStoreHead* tsHead);
	DicStatus SearchWord(const string word, string & definition);
	

private:

		interprocess_mutex 	mutex;

		TrieStore * nextTS[MAX_NEXT_TSNODES]; // 26*4 = 104 bytes

				// self word (at current location)
	
		BPtr		selfWordPtr;
		UInt16		selfDefLen;
};


/*
* 1) Holds all store's info. Mapped in Shared Memory space.
*	a) Num Stores?
*	b) Starting Mem Address of each store?
* 2) How/where do we add new word?
* 3) Stores batch of free memory  ~ 64KB for each of 26 stores. This is to increase the write concurrency.
	a) If avg size of each word (word + it's definition) is 256 bytes,
		we can store at most 256 new words in one batch. Some extra space would be required for Trie nodes.
	b) Ignore last few bytes which won't be enough to fit current word plus possibly new Trie DS?
	c) Avg memory wastage under 256 bytes per cycle of 64k (~ 256 words). Can this be minimized?
* 4) sync flag for each 26 stores
* 5) TODO: We could maintain this (free memory batch & hence sync flags) till 2 levels.
	a) This way, we can support 26*26 = 676 cuncurrent write operations.
*
*
*/
class TrieStoreMgr;

class TrieStoreHead {

public:

	// for every new TrieNode or new word definition string, this will be called by TrieStore.
	// It always allocates new blocks sequentially. Hence new writes always keep getting appended to store level block.
	// TODO: What if we don't have large enough block to allocate even at MemMgr?
	DicStatus AllocMemBlock(BPtr& ptr, UInt32 size);

	// DeAlloc simply calls MemMgr DeAllocData & doesn't manage fragmented mem blocks.
	DicStatus DeAllocMemBlock(BPtr& ptr, UInt32 size);

	friend class TrieStoreMgr;

private:
		TrieStore* headTS;

	// Taken Exl sync when any memory is asked within the store.
		SyncFlag	syncSelf;
		BPtr		storeMemPtr;
		UInt32		totalMem;
		UInt32		offset;
}; // 20 bytes

/*
* This is process defined & initializes when process starts from shared memory
*
*/
class TrieStoreMgr {

public:
	DicStatus InsertWord(const string word, const string definition);
	DicStatus DeleteWord(const string word);
	DicStatus SearchWord(const string word, string & definition);

private:

	/*
	* Gets updated during initial MemMgr::Initialize() & remains unchanged there after.
	* This is stored/retried using MemMgr as AppData (size: ).
	* */
	TrieStoreHead* tsPtrs[MAX_NEXT_TSNODES];

};


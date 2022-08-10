
#include "TrieStore.hpp"

TrieStore::TrieStore() {

	memset(nextTS, 0, sizeof(TrieStore*) * MAX_NEXT_TSNODES);
	wordPtr = nullptr;
}

DicStatus TrieStore::InsertWord(UInt32 i, const string word, const string definition) {

	// TODO: There is a possible partial write issue here if write partial data & process crashes abruptly. Not handled as of now.

	DicStatus rc = DIC_SUCCESS;
	TrieStore* nextPtr = nullptr;

	{
		scoped_lock<interprocess_mutex> lock(mutex);

		if (i == word.size() - 1) {

			// reached destination TS node.
			if (wordPtr == nullptr) {

				defLen = definition.size();

				wordPtr = MemoryMgr::AllocMem(defLen);

				if (wordPtr) {

					memcpy(wordPtr, definition.c_str(), defLen);
				}
			}
			else {

				rc = TST_WORD_ALREADY_EXISTS;
			}
				
		}
		else {
			char ch = word[i + 1];
			BPtr ptr = nullptr;

			if (!nextTS[IndexOf(ch)]) {

				ptr = MemoryMgr::AllocMem(sizeof(TrieStore));

				if (ptr) {

					nextTS[IndexOf(ch)] = new (ptr) TrieStore;
				}
			}

			// if mem allocation have failed, nextPtr would be still null
			nextPtr = (TrieStore *)nextTS[IndexOf(ch)];
		}
	} // lock released.

	// if next pointer exist
	if (nextPtr) {
		rc = nextPtr->InsertWord(i + 1, word, definition);
	}

	return rc;
}

DicStatus TrieStore::DeleteWord(UInt32 i, const string word) {

	DicStatus rc = DIC_SUCCESS;
	TrieStore* nextPtr = nullptr;

	{
		scoped_lock<interprocess_mutex> lock(mutex);

		if (i == word.size() - 1) {

			// reached destination TS node.
			if (wordPtr != nullptr) {

				wordPtr = nullptr;

				// TODO: memory leakage here is not handled as MemMgr do now support defragmented memory currently. 
				// This can be improved by adding free chunks management using buckets of size 32/64/128/256/512/etc. 12 to 16 such buckets(of lists of chunks) should suffice.
			}
			else
				rc = TST_WORD_DOESNOT_EXIST;
		}
		else {
			char ch = word[i + 1];
			nextPtr = (TrieStore*)nextTS[IndexOf(ch)];

			if (nextPtr == nullptr)
				rc = TST_WORD_DOESNOT_EXIST;
		}
	} // lock released.

	// if next pointer exist
	if (nextPtr) {
		rc = nextPtr->DeleteWord(i + 1, word);
	}

	return rc;
}

DicStatus TrieStore::SearchWord(UInt32 i, const string word, string & definition) {

	DicStatus rc		= DIC_SUCCESS;
	TrieStore* nextPtr	= nullptr;

	{
		scoped_lock<interprocess_mutex> lock(mutex);

		if (i == word.size() - 1) {

			// reached destination TS node.
			if (wordPtr != nullptr)
				memcpy(&definition, wordPtr, defLen);
			else
				rc = TST_WORD_DOESNOT_EXIST;
		}
		else {
			char ch = word[i + 1];
			nextPtr = (TrieStore*)nextTS[IndexOf(ch)];

			if (nextPtr == nullptr)
				rc = TST_WORD_DOESNOT_EXIST;
		}
	} // lock released.

	// if we valid next pointer exist
	if (nextPtr) { 
		rc = nextPtr->SearchWord(i+1, word, definition);
	}
		
	return rc;
}


///////////// TrieStoreMgr APIs ////////////////////////

TrieStoreMgr::TrieStoreMgr(DicConfig& config) {

	DicStatus rc = MemoryMgr::Initialize(config);

	if (rc) {

		BPtr mptr = MemoryMgr::AllocMem(sizeof(interprocess_mutex));

		if (config.shCreate)
			mutex = new (mptr) interprocess_mutex;
		else
			mutex = (interprocess_mutex*)mptr;

		scoped_lock<interprocess_mutex> lock(*mutex);

		for (int i = 0; i < MAX_NEXT_TSNODES; i++) {

			TrieStore* ptr = (TrieStore*)MemoryMgr::AllocMem(sizeof(TrieStore));

			if (config.shCreate) {

				tsHead[i] = new (ptr) TrieStore;
			}
			else
				tsHead[i] = ptr;
		}
	}
	else
		exit(EXIT_FAILURE);
}

DicStatus
TrieStoreMgr::InsertWord(const string word, const string definition) {

	char ch = word[0];
	return tsHead[IndexOf(ch)]->InsertWord(0, word, definition);
}

DicStatus
TrieStoreMgr::DeleteWord(const string word) {

	char ch = word[0];
	return tsHead[IndexOf(ch)]->DeleteWord(0, word);
}

DicStatus TrieStoreMgr::SearchWord(const string word, string & definition) {

	char ch = word[0];
	return tsHead[IndexOf(ch)]->SearchWord(0, word, definition);
}



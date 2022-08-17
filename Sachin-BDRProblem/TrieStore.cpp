
#include "TrieStore.hpp"

TrieStore::TrieStore(bool init) {

	// TODO: added extra param 'init' for debugging purpose. Remove it later.
	if (init) {

		memset(nextTS, 0, sizeof(nextTS[0]) * MAX_NEXT_TSNODES);
		wordPtr = 0;
	}
}

DicStatus TrieStore::InsertWord(UInt32 i, const string word, const string definition)
{

	// TODO: There is a possible partial write issue here if write partial data & process crashes abruptly. Not handled as of now.

	DicStatus rc = DIC_SUCCESS;
	TrieStore *nextPtr = nullptr;

	{
		//scoped_lock<interprocess_mutex> lock(mutex);

		if (pthread_mutex_lock(&mutex))
			return PTHREAD_LOCK_ERROR;

		if (i == word.size() - 1)
		{

			// reached destination TS node.
			if (wordPtr == 0)
			{

				defLen = definition.size();
				BPtr ptr;

				wordPtr = MemoryMgr::Obj()->AllocMem(defLen);
				ptr = (BPtr)MemoryMgr::Obj()->Ptr(wordPtr);

				memcpy(ptr, definition.c_str(), defLen);
			}
			else
			{

				rc = TST_WORD_ALREADY_EXISTS;
			}
		}
		else {
			char ch = word[i + 1];
			BPtr ptr = nullptr;
			bool init = false;

			if (!nextTS[IndexOf(ch)]) {

				nextTS[IndexOf(ch)] = MemoryMgr::Obj()->AllocMem(sizeof(TrieStore));
				init = true;
			}			

			ptr = (BPtr)MemoryMgr::Obj()->Ptr(nextTS[IndexOf(ch)]);

			nextPtr = new (ptr) TrieStore(init);

			// if mem allocation have failed, nextPtr would be still null
			//nextPtr = static_cast<TrieStore *>(nextTS[IndexOf(ch)]);
		}

		if (pthread_mutex_unlock(&mutex))
			return PTHREAD_LOCK_ERROR;

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
		//scoped_lock<interprocess_mutex> lock(mutex);

		if (pthread_mutex_lock(&mutex))
			return PTHREAD_LOCK_ERROR;

		if (i == word.size() - 1) {

			// reached destination TS node.
			if (wordPtr != 0) {

				wordPtr = 0;

				// TODO: memory leakage here is not handled as MemMgr do now support defragmented memory currently. 
				// This can be improved by adding free chunks management using buckets of size 32/64/128/256/512/etc. 12 to 16 such buckets(of lists of chunks) should suffice.
			}
			else
				rc = TST_WORD_DOESNOT_EXIST;
		}
		else {
			char ch = word[i + 1];
			BPtr ptr;

			if (nextTS[IndexOf(ch)] == 0)
				rc = TST_WORD_DOESNOT_EXIST;
			else {

				ptr = (BPtr)MemoryMgr::Obj()->Ptr(nextTS[IndexOf(ch)]);
				nextPtr = new (ptr) TrieStore(false);
			}
		}

		if (pthread_mutex_unlock(&mutex))
			return PTHREAD_LOCK_ERROR;

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
		//scoped_lock<interprocess_mutex> lock(mutex);
		if (pthread_mutex_lock(&mutex))
			return PTHREAD_LOCK_ERROR;

		if (i == word.size() - 1) {

			// reached destination TS node.
			if (wordPtr != 0) {

				BPtr ptr = (BPtr)MemoryMgr::Obj()->Ptr(wordPtr);

				definition = string(ptr, ptr + defLen);
			}
			else
				rc = TST_WORD_DOESNOT_EXIST;
		}
		else {
			char ch = word[i + 1];
			BPtr ptr;

			if (nextTS[IndexOf(ch)] == 0)
				rc = TST_WORD_DOESNOT_EXIST;
			else {

				ptr = (BPtr)MemoryMgr::Obj()->Ptr(nextTS[IndexOf(ch)]);
				nextPtr = new (ptr) TrieStore(false);
			}
		}

		if (pthread_mutex_unlock(&mutex))
			return PTHREAD_LOCK_ERROR;
	} // lock released.

	// if we valid next pointer exist
	if (nextPtr) { 
		rc = nextPtr->SearchWord(i+1, word, definition);
	}
		
	return rc;
}

///////////// TrieStoreMgr APIs ////////////////////////

TrieStoreMgr::TrieStoreMgr(DicConfig * config) {

	MemoryMgr * memMgr = MemoryMgr::Obj(config);

	// TODO: race conditions related to parallel processing, 2 processes trying to create initial TrieStore blocks.
	// can maintain one mutex for this.

	if (memMgr) {

		for (int i = 0, offset = 0; i < MAX_NEXT_TSNODES; i++) {

			if (config->shCreate) {

				UInt32 offset = memMgr->AllocMem(sizeof(TrieStore));

				TrieStore *ptr = (TrieStore *)memMgr->Ptr(offset);

				tsHead[i] = new (ptr) TrieStore(true);
			}
			else {

				tsHead[i] = (TrieStore *)memMgr->GetAppDataBuff(offset);
				offset += sizeof(TrieStore);
			}
		}
	}
	else
		exit(EXIT_FAILURE);
}

DicStatus
TrieStoreMgr::InsertWord(string word, const string definition) {

	transform(word.begin(), word.end(), word.begin(), ::tolower);

	char ch = word[0];
	return tsHead[IndexOf(ch)]->InsertWord(0, word, definition);
}

DicStatus
TrieStoreMgr::DeleteWord(string word) {

	transform(word.begin(), word.end(), word.begin(), ::tolower);

	char ch = word[0];
	return tsHead[IndexOf(ch)]->DeleteWord(0, word);
}

DicStatus TrieStoreMgr::SearchWord(string word, string & definition) {

	transform(word.begin(), word.end(), word.begin(), ::tolower);

	char ch = word[0];
	return tsHead[IndexOf(ch)]->SearchWord(0, word, definition);
}



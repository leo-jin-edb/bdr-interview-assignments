
#include "TrieStore.hpp"

DicStatus 
TrieStore::InsertWord(const string word, const string definition, TrieStoreHead* tsHead) {

	// All inputs are valid.

	DicStatus ret = DIC_SUCCESS;

	while (syncSelf.AccessExclusive() == false); // TODO: We could add Timed Wait here & some retries instead of indefinite loop.

	if (word.empty()) {
		// reached leaf TS node.

		if (selfWordPtr == nullptr) {

			selfDefLen = sizeof(definition);

			ret = tsHead->AllocMemBlock((BPtr &)selfWordPtr, selfDefLen); 

			if (!ret)
				memcpy(selfWordPtr, definition.c_str(), selfDefLen);
		}
		else
			ret = TST_WORD_ALREADY_EXISTS;
	}
	else {

		// We need to drill down further
		
		char ch = word[0];
		TrieStore* & next = nextTS[IndexOf(ch)];

		if (next == nullptr) {
			// create new TrieNode & update the respective chil pointer.
			// Below func resets the pointer before inside before returning. So we don't need to reset again.
			ret = tsHead->AllocMemBlock((BPtr&)next, sizeof(TrieStore));
		}
		syncSelf.ReleaseExclusive();

		if (!ret) {
			ret = next->InsertWord(string(word.begin() + 1, word.end()), definition, tsHead); // trim down first char in 'word' & send rest
		}

		return ret;
	}

	syncSelf.ReleaseExclusive();
	return ret;
}

DicStatus 
TrieStore::DeleteWord(const string word, TrieStoreHead* tsHead) {

	// All inputs are valid.

	DicStatus ret = DIC_SUCCESS;

	while (syncSelf.AccessExclusive() == false); // TODO: We could add Timed Wait here & some retries instead of indefinite loop.

	if (word.empty()) {
		// reached leaf TS node.

		if (selfWordPtr != nullptr) {

			ret = tsHead->DeAllocMemBlock((BPtr&)selfWordPtr, selfDefLen);

			if (!ret)
				selfWordPtr = nullptr;
		}
		else
			ret = TST_WORD_DOESNOT_EXIST;
	}
	else {

		// We need to drill down further

		char ch = word[0];
		TrieStore*& next = nextTS[IndexOf(ch)];

		if (next == nullptr) {
			ret = TST_WORD_DOESNOT_EXIST;
		}
		syncSelf.ReleaseExclusive();

		if (!ret) {
			ret = next->DeleteWord(string(word.begin() + 1, word.end()), tsHead); // trim down first char in 'word' & send rest
		}

		return ret;
	}

	syncSelf.ReleaseExclusive();
	return ret;
}

DicStatus TrieStore::SearchWord(const string word, string & definition) {

	DicStatus ret = DIC_SUCCESS;

	while (syncSelf.AccessShared() == false); // TODO: We could add Timed Wait here & some retries instead of indefinite loop.

	if (word.empty()) {
		// reached leaf TS node.

		if (selfWordPtr != nullptr) {

			memcpy((void *)definition.c_str(), selfWordPtr, selfDefLen);
		}
		else
			ret = TST_WORD_DOESNOT_EXIST;
	}
	else {

		// We need to drill down further

		char ch = word[0];
		TrieStore*& next = nextTS[IndexOf(ch)];

		if (next == nullptr) {
			ret = TST_WORD_DOESNOT_EXIST;
		}
		syncSelf.ReleaseShared();

		if (!ret) {
			ret = next->SearchWord(string(word.begin() + 1, word.end()), definition); // trim down first char in 'word' & send rest
		}

		return ret;
	}

	syncSelf.ReleaseShared();
	return ret;
}


DicStatus 
TrieStoreHead::AllocMemBlock(BPtr& ptr, UInt32 size) {

	/*
	* 1) TODO: If memory goes below some threshold say '256' bytes, get new memblock from SHM for this store. 
		Caller will remain under Excl lock till then.
	  2) Reset the memory before returning.
	*/

	return DIC_SUCCESS;
}

// DeAlloc simply calls MemMgr DeAllocData & doesn't manage fragmented mem blocks.
DicStatus 
TrieStoreHead::DeAllocMemBlock(BPtr& ptr, UInt32 size) {

	return DIC_SUCCESS;
}
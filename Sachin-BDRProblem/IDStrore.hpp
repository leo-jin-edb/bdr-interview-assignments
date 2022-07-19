
#pragma once

// This is alternate method to TrieStore using IDs to words & hash to find out the IDs during searchWord.

// TODO: StoreID not yet implemented.
#ifdef USE_IDSTORE
#include "MemoryMgr.hpp"

/*
	1) Each Store Operations.
	2) Store is segregated based on 
		a) Just first characters of the word
	3) Starting alphabet is only used Use Index to get the ID & & fetch word in specific store (starting alphabet)

*/


// Used to get the Key ID & then goto IDStore to fetch the word.
class HashIDStore {

};

/*
* holds all store's info
*	1) Num Stores?
*	2) Starting Mem Address of each store?
*	3)
*
*/
class IDStoreMgr {

public:

private:

};

/*
* holds specific store's info
*	1) Starting Mem Address of the store
*	2) Starting alphabet
*
*/
class IDStore {

public:

};
#endif


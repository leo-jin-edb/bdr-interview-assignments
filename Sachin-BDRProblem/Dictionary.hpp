#pragma once

#include "TrieStore.hpp"

#define DEFAULT_SHM_SIZE					1048576 // 2^20 

class Dictionary {

    public:

        Dictionary(string pCreateShm);

        DicStatus InsertWord(const string word, const string definition);
        DicStatus DeleteWord(const string word);
        DicStatus SearchWord(const string word, string & definition); 

    private:

        TrieStoreMgr* tsMgr;
};
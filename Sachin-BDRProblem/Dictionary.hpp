#pragma once

#include "TrieStore.hpp"

#define DEFAULT_SHM_SIZE					134217728 // 2^27 

class Dictionary {

    public:

        Dictionary(bool createSHM);

        DicStatus InsertWord(const string word, const string definition);
        DicStatus DeleteWord(const string word);
        DicStatus SearchWord(const string word, string & definition); 

    private:

        TrieStoreMgr* tsMgr;
};
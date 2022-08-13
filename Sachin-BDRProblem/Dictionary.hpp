#pragma once

#include "TrieStore.hpp"

class Dictionary {

    public:

        Dictionary(DicConfig & config);

        DicStatus InsertWord(const string word, const string definition);
        DicStatus DeleteWord(const string word);
        DicStatus SearchWord(const string word, string & definition); 

    private:

        TrieStoreMgr* tsMgr;
};
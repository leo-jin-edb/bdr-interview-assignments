#pragma once

#include "IDStrore.hpp"
#include "TrieStore.hpp"

class Dictionary {

    public:
        DicStatus InsertWord(const string word, const string definition);
        DicStatus DeleteWord(const string word);
        DicStatus SearchWord(const string word, string & definition); 

    private:

};
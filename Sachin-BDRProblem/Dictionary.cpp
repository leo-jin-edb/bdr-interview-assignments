#include "Dictionary.hpp"

Dictionary::Dictionary(bool createSHM) {

    DicConfig config = {
                createSHM,
                "SachinRSharedMemory",
                DEFAULT_SHM_SIZE
            };

    tsMgr = new TrieStoreMgr(&config);
}

DicStatus Dictionary::InsertWord(const string word, const string definition) {

    if (word.empty() || definition.empty())
        return DIC_INVALID_IP_PARAM;

    if (tsMgr == nullptr)
        return MEM_INIT_ERROR;
    
    return tsMgr->InsertWord(word, definition);
}

DicStatus Dictionary::DeleteWord(const string word) {

    if (word.empty())
        return DIC_INVALID_IP_PARAM;

    if (tsMgr == nullptr)
        return MEM_INIT_ERROR;

    return tsMgr->DeleteWord(word);
}

DicStatus Dictionary::SearchWord(const string word, string& definition) {

    if (word.empty() )
        return DIC_INVALID_IP_PARAM;

    if (tsMgr == nullptr)
        return MEM_INIT_ERROR;

    return tsMgr->SearchWord(word, definition);
}
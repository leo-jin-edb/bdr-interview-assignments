#include "Dictionary.hpp"

Dictionary::Dictionary(DicConfig & config) {

    if (config.shName[0] == '\0')
        exit(EXIT_FAILURE);

    tsMgr = nullptr;

    tsMgr = new TrieStoreMgr(config);
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

    if (word.empty() || definition.empty())
        return DIC_INVALID_IP_PARAM;

    if (tsMgr == nullptr)
        return MEM_INIT_ERROR;

    return tsMgr->SearchWord(word, definition);
}

int main (int argc, char *argv[]) {

    cout << "******* Dictionary app ******* " << endl;

    if (argc != 2) {
        cerr << "Usage:  " << argv[0] << " create/open" << endl;
        return -1;
    }
    bool createSHM = false;

    if (strcmp(argv[1], "create") == 0)
        createSHM = true;

    DicConfig config = {
        createSHM,
        "SachinRSharedMemory",
        DEFAULT_SHM_SIZE
    };

    Dictionary dict(config);

    DicStatus rc;

    string word = "sample";
    string def = "A small part or quantity intended to show what the whole is like: investigations involved analyzing samples of handwriting";

    rc = dict.InsertWord(word, def);
    if(!rc)
        cout << "Insert Success for word: " << word << endl;
    else
        cout << "Insert failed\n";

    word = "hint";
    def = "a slight or indirect indication or suggestion: he has given no hint of his views";

    rc = dict.InsertWord(word, def);
    if (!rc)
        cout << "Insert Success for word: " << word << endl;
    else
        cout << "Insert failed\n";

    def = "";
    
    rc = dict.SearchWord(word, def);
    if (!rc)
        cout << "Meaning of '" << word << "': " << def << endl;
    else
        cout << "Search failed\n";

    return 0;
}
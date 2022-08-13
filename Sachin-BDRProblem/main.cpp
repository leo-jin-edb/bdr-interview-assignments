
#include "Dictionary.hpp"

// for ease of testing.

Dictionary *dict;
DicStatus rc;

void initialize(bool createSHM) {

    DicConfig config = {
        createSHM,
        "SachinRSharedMemory",
        DEFAULT_SHM_SIZE
    };

    dict = new Dictionary(config);

}

void insert(string word, string def) {

    rc = dict->InsertWord(word, def);
    if (!rc)
        cout << "Insert Success for word: " << word << endl;
    else
        cout << "Insert failed for " << word << endl;
}

void delete(string word) {

    cout << "Deleting word: " << word << endl;
    rc = dict->DeleteWord(word);
    if (!rc)
        cout << "Word Deleted successfully\n";
    else
        cout << "Delete failed\n";
}

void search(string word){

    string def;
    
    cout << "searching for: " << word << endl;
    rc = dict->SearchWord(word, def);
    if (!rc)
        cout << "Word: " << word << ", found.\n Def: " << def << endl;
    else
        cout << "Word: " << word << " doesn't exist in dictionary.\n";
}

void TestBasicCases() {

    DicStatus rc;
    
    // insert new word
    string word = "sample";
    string def = "A small part or quantity intended to show what the whole is like: investigations involved analyzing samples of handwriting";
    insert(word, def);

    // insert word that is already inserted earlier.
    insert(word, def);

    // insert new word
    word = "hint";
    def = "a slight or indirect indication or suggestion: he has given no hint of his views";
    insert(word, def);

    // search existing word
    word = "sample";
    search(word);

    // search word that is not yet inserted
    word = "sachin";
    search(word);        

    // search existing word
    word = "hint";
    search(word);
    
    word = "hint";
    delted(word);

    // search deleted word.
    word = "hint";
    search(word);

    // insert same deleted word with larger length of 'def'
    word = "hint";
    def = "a slight or indirect indication or suggestion: he has given no hint of his views.\n It is a noun.\n E.g.\"Give me a 'hint'!\"\n";
    insert(word, def);
    
    // search newly inserted word (which was deleted earlier)
    word = "hint";
    search(word);
}
/*
void TestParallelProcessCases() {

    int i;

    do {

        cout << "Enter Your Choice: \n";

        cout << ""


    }while (i != 0);


}*/

int main (int argc, char *argv[]) {

    cout << "******* Dictionary app ******* " << endl;

    if (argc != 2) {
        cerr << "***** Wrong Usage! *****\n. Correct Usage is:  " << argv[0] << " create/open" << endl;
        return -1;
    }
    
    bool createSHM = false;

    if (strcmp(argv[1], "create") == 0)
        createSHM = true;

    initialize(createSHM);

    TestBasicCases();

    //TestParallelProcessCases();

    return 0;
}
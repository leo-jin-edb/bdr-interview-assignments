
#include "Dictionary.hpp"

// for ease of testing.

Dictionary *dict;
DicStatus rc;

void initialize(bool createSHM) {

    cout << "================  Initializing  ==================" << endl;

    DicConfig config = {
        createSHM,
        "SachinRSharedMemory",
        DEFAULT_SHM_SIZE
    };

    dict = new Dictionary(config);

}

void insert(string word, string def) {

    cout << "===========================================" << endl;

    cout << "Inserting word: " << word << ".\nDefinition:\n" << def << endl;

    cout << "------\n\n";

    rc = dict->InsertWord(word, def);
    if (!rc)
        cout << "Insert Success." << endl;
    else
        cout << "Insert failed." << endl;

    cout << endl;
}

void deletew(string word) {

    cout << "===========================================" << endl;
    cout << "Deleting word: " << word << endl;
    
    cout << "------\n\n";

    rc = dict->DeleteWord(word);
    if (!rc)
        cout << "Deleted successfully\n";
    else
        cout << "Delete failed\n";

    cout << endl;
}

void search(string word){

    cout << "===========================================" << endl;

    string def;
    
    cout << "searching for: " << word << endl;

    cout << "------\n\n";

    rc = dict->SearchWord(word, def);
    if (!rc)
        cout << "Word found!\nDefinition:\n\n\"" << def << "\"\n" << endl;
    else
        cout << "Word doesn't exist in dictionary.\n";

    cout << endl;
}

void TestBasicCases() {

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
    word = "Sachin";
    search(word);        

    // search existing word
    word = "hint";
    search(word);
    
    // delete word with some alphabets in upper case
    word = "HinT";
    deletew(word);

    // search deleted word.
    word = "hint";
    search(word);

    // insert same deleted word with larger length of 'def'. Some alphabets in upper case
    word = "Hint";
    def = "a slight or indirect indication or suggestion: he has given no hint of his views. E.g.->> Give me a 'hint'! <--";
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

    if (argc != 2) {
        cerr << "\n**************** Wrong Usage! ****************\n\nCorrect Usage is:  " << argv[0] << " create/open.\n" << endl;
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
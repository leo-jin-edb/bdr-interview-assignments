
#include "Dictionary.hpp"


Dictionary *dict;

void initialize(bool createSHM) {

    DicConfig config = {
        createSHM,
        "SachinRSharedMemory",
        DEFAULT_SHM_SIZE
    };

    dict = new Dictionary(config);

}

void search(string word, string def){

}

void TestBasicCases() {

    DicStatus rc;
    
    string word = "sample";
    string def = "A small part or quantity intended to show what the whole is like: investigations involved analyzing samples of handwriting";

    rc = dict->InsertWord(word, def);
    if(!rc)
        cout << "Insert Success for word: " << word << endl;
    else
        cout << "Insert failed for " << word << endl;

    word = "hint";
    def = "a slight or indirect indication or suggestion: he has given no hint of his views";

    rc = dict->InsertWord(word, def);
    if (!rc)
        cout << "Insert Success for word: " << word << endl;
    else
        cout << "Insert failed for " << word << endl;

    word = "sample";
    cout << "searching for: " << word << endl; 
    rc = dict->SearchWord(word, def);
    if (!rc)
        cout << "Word: " << word << ", found.\n Def: " << def << endl;
    else
        cout << "Word: " << word << " doesn't exist in dictionary.\n";


    word = "sachin";
    cout << "searching for: " << word << endl; 
    rc = dict->SearchWord(word, def);
    if (!rc)
        cout << "Word: " << word << ", found.\n Def: " << def << endl;
    else
        cout << "Word: " << word << " doesn't exist in dictionary.\n";
        

    word = "hint";
    cout << "searching for: " << word << endl; 
    rc = dict->SearchWord(word, def);
    if (!rc)
        cout << "Word: " << word << ", found.\n Def: " << def << endl;
    else
        cout << "Word: " << word << " doesn't exist in dictionary.\n";
    
    word = "hint";
    cout << "Deleting word: " << word << endl; 
    rc = dict->DeleteWord(word);
    if (!rc)
        cout << "Word Deleted successfully\n";
    else
        cout << "Delete failed\n";

    word = "hint";
    cout << "searching for: " << word << endl; 
    rc = dict->SearchWord(word, def);
    if (!rc)
        cout << "Word: " << word << ", found.\n Def: " << def << endl;
    else
        cout << "Word: " << word << " doesn't exist in dictionary.\n";

    word = "hint";
    def = "a slight or indirect indication or suggestion: he has given no hint of his views.\n It is a noun.\n E.g.\"Give me a 'hint'!\"\n";

    rc = dict->InsertWord(word, def);
    if (!rc)
        cout << "Insert Success for word: " << word << endl;
    else
        cout << "Insert failed for " << word << endl;

    
    word = "hint";
    cout << "searching for: " << word << endl; 
    rc = dict->SearchWord(word, def);
    if (!rc)
        cout << "Word: " << word << ", found.\n Def: " << def << endl;
    else
        cout << "Word: " << word << " doesn't exist in dictionary.\n";

}

void TestParallelProcessCases() {

    int i;

    do {

        cout << "Enter Your Choice: \n";

        cout << ""


    }while (i != 0);


}

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

    TestParallelProcessCases();

    return 0;
}
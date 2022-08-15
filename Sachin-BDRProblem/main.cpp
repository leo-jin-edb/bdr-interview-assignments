
#include "Dictionary.hpp"

// for ease of testing.

Dictionary *dict;
DicStatus rc;

void initialize(UInt64 handle) {

    cout << "================  Initializing  ==================" << endl;

    dict = new Dictionary(handle);

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

/*
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
    */
}


void TestParallelProcessCases() {

    int i;
    bool exitapp = false;

    do {

        string word, def;

        cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;

        cout << "1: Insert Word\n";
        cout << "2: Delete Word\n";
        cout << "3: Search Word\n";
        cout << "0: Exit application\n";

        cout << "Enter Your Choice: ";

        cin >> i;

        switch (i) {

            case 0:
                exitapp = true;
                break;

            case 1:
                cout << "Enter word to insert:\n";
                cin >> word;
                cout << "Enter definition for the word:\n";

                cin.ignore(100, '\n');
                getline(std::cin, def);
                insert(word, def);

                break;

            case 2:
                cout << "Enter word to delete:\n";
                cin >> word;
                deletew(word);

                break;

            case 3:
                cout << "Enter word to search:\n";
                cin >> word;
                search(word);

                break;

            default:
                cout << "Enter correct choice... Try Again.\n";
        }

    }while (exitapp == false);


}

int main (int argc, char *argv[]) {

    if (argc != 2) {
        cerr << "\n**************** Wrong Usage! ****************\n\nCorrect Usage is:  " << argv[0] << " create/open.\n" << endl;
        return -1;
    }
    
    UInt32 handle;

    //Obtain handle value
    std::stringstream s;
    s << argv[1];
    s >> handle;

    cout << "Input Handle: " << handle << endl;

    initialize(handle);

    TestBasicCases();

    TestParallelProcessCases();

    return 0;
}
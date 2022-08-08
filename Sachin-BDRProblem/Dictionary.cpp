
#include "Dictionary.hpp"


int main () {

    int* aptr = nullptr;

    string a = "Hey Sachin!";
    string b = string(a.begin()+2, a.end());

    cout << "******* Dictionary app 18 july ******* " << b << endl;

    
    MemoryMgr::AllocData((BPtr &)aptr, 1);


    return 0;
}
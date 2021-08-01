/** This class manages the English word dictionary.
  * Words may be added, removed, or searched for.
  * The dictionary may be saved to disk.
  **/
 #include <thread>
 #include <algorithm>
 #include <stdio.h>
 #include <mutex>
 #include <list>
 #include <iostream>
 #include <cstring>
 #include "dictmgr.hpp"

 using namespace std;
 
 /* The dictionary is kept in memory as linked lists of words within a hash table.
  * This enables more parallel processing by locking only the list of words for
  * a given hash value. 
  */
 #define READSIZE 512
static int HASHSIZE = 1024; // with a good hash algorithm, each list should be no more than 1000 words long
static const string defaultDict("./dictionary.dct");
DictMgr::DictMgr(void) : DictMgr(defaultDict) {};
/*{
    this(defaultDict);
}*/

// Initialize hash table, mutexes, etc. and read in the dictionary
DictMgr::DictMgr(string dictpath)
: filename(dictpath) {
    wordlist = new list<string>[HASHSIZE];
    FILE* dictfile = fopen(filename.c_str(), "r");
    char readbuf[READSIZE];
    if (dictfile == NULL) {
        printf("No dictionary present.\n");
    } else {
        printf("Initializing ... Reading existing dictionary ... ");
        int count = 0;
        while (!feof(dictfile)) {
            if (fgets(readbuf, READSIZE, dictfile) == NULL)
                break;
            int len = strlen(readbuf);
            for (--len; readbuf[len] == '\r' || readbuf[len] == '\n'; --len) {
            	readbuf[len] = '\0';
            }
            insertNoLock(readbuf);
            count++;
        }
        fclose(dictfile);
        printf("%d words read\n", count);
    }
    // Now initialize the mutexes for the hash buckets
    mutlist = new mutex[HASHSIZE];
}

bool DictMgr::save(void) {
    // write dictionary to temp file; then rename
    string tmpfname = filename + ".tmp";
    FILE* dictfile = fopen(tmpfname.c_str(), "w");
    bool rval;
    if (dictfile == NULL) {
        printf("Couldn't open file to save");
        rval = false;
    } else {
        for (int i = 0; i < HASHSIZE; i++) {
            mutlist[i].lock();
            list<string>* words = &wordlist[i];
            for (list<string>::iterator it = words->begin(); it != words->end(); ++it) {
                fputs((*it).c_str(), dictfile);
                fputc('\n', dictfile);
            }
            mutlist[i].unlock();
        }
        fclose(dictfile);
        if (rename(tmpfname.c_str(), filename.c_str()) == 0) {
            rval = true;
        } else {
            rval = false;
            printf("Could not replace existing dictionary\n");
        }
    }
    return rval;
}

bool DictMgr::insert(string word) {
    int hashval = hash<string>{}(word) % HASHSIZE;
    mutlist[hashval].lock();
    bool rval = insertNoLock(word);
    mutlist[hashval].unlock();
    return rval;
}

bool DictMgr::insertNoLock(string word, int hashval) {
	if (hashval < 0) {
		hashval = hash<string>{}(word) % HASHSIZE;
	}
    bool rval;
    list<string>* bucket = &(wordlist[hashval]);
    if (searchcommon(word, bucket)) {
        // word is already in the dict
        rval = true;
        cout << word << " already in dictionary\n";
    } else {
        bucket->push_back(word);
        cout << "Inserted " << word << endl;
        rval = true;
    }
    return rval;
}

bool DictMgr::remove(string word) {
    int hashval = hash<string>{}(word) % HASHSIZE;
    bool rval;
    mutlist[hashval].lock();
    list<string>* bucket = &wordlist[hashval];
    list<string>::iterator it;
    if (searchcommon(word, bucket, &it)) {
        it = bucket->erase(it);
        rval = true;
    } else {
        printf("%s not found in dictionary\n", word.c_str());
        rval = false;
    }
    mutlist[hashval].unlock();
    return rval;
}

bool DictMgr::find(string word) {
    int hashval = hash<string>{}(word) % HASHSIZE;
    bool rval;
    mutlist[hashval].lock();
    list<string>* bucket = &wordlist[hashval];
    rval = searchcommon(word, bucket);
    if (rval == false) {
        printf("%s not found in dictionary\n", word.c_str());
    }
    mutlist[hashval].unlock();
    return rval;
}

bool DictMgr::searchcommon(string word, list<string>* words, list<string>::iterator* pit) {
    list<string>::iterator it = std::find(words->begin(), words->end(), word);
    if (pit != NULL) {
        *pit = it;
    }
    return (it != words->end());
}

DictMgr::~DictMgr() {
	cout << "Destroying memory dict\n";
    delete[] wordlist;
    delete[] mutlist;
}



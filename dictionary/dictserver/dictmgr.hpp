#ifndef _DICTMGR_HPP_
#define _DICTMGR_HPP_
// Public interface for management of English word dictionary
#include <string>
#include <list>
#include <mutex>

class DictMgr {
   public:
     bool insert(std::string str);  // returns true on successful insertion
     bool remove(std::string str);  // returns true on successful removal
     bool find(std::string str);    // returns true if found; false otherwise
     bool save(void);               // returns true if successful; false otherwise
     DictMgr();
     DictMgr(const std::string dictpath);
     ~DictMgr();
     
    private:
      std::string filename; // file to read on initialize; write on exit
      std::list<std::string> *wordlist; // pointer to array of buckets
      std::mutex *mutlist;
      bool searchcommon(std::string word, std::list<std::string>* wordlist, std::list<std::string>::iterator* foundpos = NULL);
      bool insertNoLock(std::string str, int hashval = -1);
};
#endif // _DICTMGR_HPP_

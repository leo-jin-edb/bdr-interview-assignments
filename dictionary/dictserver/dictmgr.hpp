#ifndef _DICTMGR_HPP_
// Public interface for management of English word dictionary
#include <string>
#include <list>
typedef class dictmgr {
   public:
     bool insert(std::string str);  // returns true on successful insertion
     bool remove(std::string str);  // returns true on successful removal
     bool find(std::string str);    // returns true if found; false otherwise
     dictmgr();
     dictmgr(std::string dictpath)
        : filename(dictpath) {};

    private:
      std::string filename; // file to read on initialize; write on exit
      std::list<std::string> *wordlist; // pointer to array of buckets



} DictMgr;
#endif // _DICTMGR_HPP_
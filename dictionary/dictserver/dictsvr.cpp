/** This server program is used to manage a dictionary of English words
  * Words may be added, removed, and searched for.
  * The dictionary is saved to a flat file upon normal exit and
  * restored on restart.
  * The dictionary is managed through receiving requests on a TCP/IP socket.
  * The requests are then put in a queue to be concurrently handled by a 
  * thread pool.
  **/
  #include <map>
  #include <cstdio>
  #include "dictmgr.hpp"

  int main() {
      dictmgr *mgr;
      mgr = new dictmgr(std::string("dictionary"));
  }

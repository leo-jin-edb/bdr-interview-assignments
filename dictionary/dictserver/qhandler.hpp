/** This class supports handling dictionary requests via a queue of requests and a thread pool
  * of handlers for the requests.
  **/
 #ifndef _QHANDLER_HPP_
 #define _QHANDLER_HPP_

 #include <queue>
 #include <iostream>
 #include <thread>
 #include <condition_variable>
 #include <Winsock2.h>
 #include "dictmgr.hpp"

 class QHandler {
   public:
    typedef enum ACTION {
      INSERT,
      REMOVE,
      SEARCH,
      SAVE,
      QUIT,
      INVALID = -1
    } ACTION;
    class DictRequest {
      public:
      SOCKET socket;
      ACTION dictaction;
      std::string dictword;

      DictRequest(ACTION action, int sock, std::string word)
        : socket(sock),
          dictaction(action),
          dictword(word) {};
    };
    QHandler(DictMgr* dict)
      : dictionary(dict) {};
    ~QHandler();
    bool queueReq(ACTION action, int sock, std::string word);
    bool isFinished() { return finished;}

   private: 
    std::queue<DictRequest*> *dictq;
    std::thread *hdlrs;
    DictMgr* dictionary;
    std::condition_variable qcv;
    std::mutex qmut;
    bool finished;
    void run();
    QHandler(); // disallow default constructor
 };
 #endif // _QHANDLER_HPP_

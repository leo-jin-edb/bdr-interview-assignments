/** Implementation of the qhandler class **/
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include "qhandler.hpp"

using namespace std;

#define NUM_THREADS 5

QHandler::QHandler(DictMgr* dict)
	: dictionary(dict) {
    finished = false;
    dictq = new queue<DictRequest*>();
    hdlrs = new thread[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        hdlrs[i] = thread(&run, this);
    }
}

bool QHandler::queueReq(ACTION action, int sock, string word) {
    DictRequest* req = new DictRequest(action, sock, word);
    cout << "Got the dict request: " << word << endl;
    {
        unique_lock<mutex> lck(qmut);
        dictq->push(req);
        lck.unlock();
    }
    qcv.notify_one();
    return true;
}

void QHandler::run() {
    // thread needs to get the queue semaphore and then wait until there is something in the queue
    // Once something is in the queue, it pops it, releases the lock and then processes the request
    // When the request is done, it returns success or failure on the socket listed in the request
    while (!finished) {
        unique_lock<mutex> qlock(qmut);
       // qlock.lock();
        while (dictq->empty() && !finished)
        {
            qcv.wait(qlock);
        }
        DictRequest *req = dictq->front();
        dictq->pop();
        qlock.unlock(); // no lock needed anymore
        bool rval;
        switch (req->dictaction)
        {
        case INSERT:
            rval = dictionary->insert(req->dictword);
            break;
        case REMOVE:
            rval = dictionary->remove(req->dictword);
            break;
        case SEARCH:
            rval = dictionary->find(req->dictword);
            break;
        case SAVE:
            rval = dictionary->save();
            break;
        case QUIT:
            rval = true;
            finished = true;
            qcv.notify_all();   // let all threads know it's time to exit
            break;
        default:
            rval = false; // error
            break;
        }
        // now send result back to client
        static const char *success = "success";
        static const char *failure = "failed";
        // send the null at the end of the string
        if (rval)
        {
            send(req->socket, success, (int)strlen(success) + 1, 0);
        }
        else
        {
            send(req->socket, failure, (int)strlen(failure) + 1, 0);
        }
        close(req->socket);
    }
    return;
}

QHandler::~QHandler() {
	cout << "Destroying queue and handlers" << endl;
    if (!finished) {
        finished = true;
        qcv.notify_all();
        for (int i = 0; i < NUM_THREADS; i++) {
            hdlrs->join();
        }
        delete[] hdlrs;
        delete dictq;
    }
}

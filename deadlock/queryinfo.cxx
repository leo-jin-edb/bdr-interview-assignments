#include "queryinfo.h"
#include <iostream>
using namespace std;


QueryInfo::~QueryInfo()
{

    for(auto itr = _objectList.begin(); itr != _objectList.end(); itr++)
    {
        ObjectInfo *objInfo = itr->second;
 //       _objectList.erase(itr);
        delete objInfo;
    }
    _objectList.clear();
}

bool QueryInfo::addObjectInfo(std::string node, std::string object, unsigned int lockQueryId, unsigned int pendingQueryId)
{
    bool rval = true;
    auto info = _objectList.find(object);
    if(info == _objectList.end())
    {
       ObjectInfo *objInfo = new ObjectInfo();
       auto out = _objectList.insert(std::make_pair<std::string, ObjectInfo*>(std::string(object), new ObjectInfo));
       info = out.first;
    }
    info->second->_lockQueryId.push_back(lockQueryId);
    info->second->_pendingQueryId.push_back(pendingQueryId);

    return true;
}

void QueryInfo::GenerateQueryWaitGraph()
{
    for(auto itr = _objectList.begin(); itr != _objectList.end(); itr++)
    {
        ObjectInfo *obj = itr->second;
        std::cout << " Object " << itr->first; 
        for(auto& pendingId : obj->_pendingQueryId)
        {
            std::cout << " pendingQueryId :-  " << pendingId << " lockQueryId :- ";
            auto info = _queryPendingInfo.find(pendingId);
            for(auto& lockId : obj->_lockQueryId)
            {
                std::cout << " " << lockId;
                if(info == _queryPendingInfo.end())
                {
                    std::list<LockHeldQueryId>* tmp = new std::list<LockHeldQueryId>;
                    tmp->push_back(LockHeldQueryId(lockId));
                    auto out = _queryPendingInfo.insert({pendingId, tmp});
                    info = out.first;
                }
                else 
                {
                    info->second->push_back(LockHeldQueryId(lockId));
                }
                std::cout << std::endl;
        
            }
        }
    }
    for(auto itr = _queryPendingInfo.begin(); itr != _queryPendingInfo.end(); itr++)
    {
        std::list<LockHeldQueryId> *tmp = itr->second;
        std::cout << " query " << itr->first << " wait list " << std::endl;
        for(auto itr2 = tmp->begin(); itr2 != tmp->end(); itr2++)
        {
            LockHeldQueryId& tmp2 = *itr2 ;
            std::cout << " " << tmp2._queryId << " visited " << tmp2._visited;
    //        tmp2._visited = true;
        }
        std::cout << std::endl;
    }

/*
    for(auto itr = _queryPendingInfo.begin(); itr != _queryPendingInfo.end(); itr++)
    {
        std::list<LockHeldQueryId>* tmp = itr->second;
        std::cout << " updated query " << itr->first << " wait list " << std::endl;
        for(auto itr2 = tmp->begin(); itr2 != tmp->end(); itr2++)
        {
            LockHeldQueryId& tmp2 = *itr2 ;
            std::cout << " " << tmp2._queryId << " visited " << tmp2._visited;
        }
        std::cout << std::endl;
    }
*/

}


bool QueryInfo::detectCycle(unsigned int queryId, unsigned int currId, bool isStarted)
{
    bool ret = false;
    if(!isStarted && (queryId == currId))
    {
        ret = true;
    }

    auto queryItr = _queryPendingInfo.find(currId);
    if(queryItr != _queryPendingInfo.end())
    {
        std::list<LockHeldQueryId>* tmp = queryItr->second;
        for(auto itr = tmp->begin(); itr != tmp->end(); itr++)
        {
            LockHeldQueryId& tmp2 = *itr ;
            if(!tmp2._visited)
            {
                tmp2._visited = true;
                ret = detectCycle(queryId, tmp2._queryId, false);
                if(ret)
                {
                    std::cout << tmp2._queryId << " -> " ;
                    break;
                }
            }
        }
    }

    return ret;
}

bool QueryInfo::detectDeadlock()
{
    GenerateQueryWaitGraph();
    bool isDeadlockSeen = false;

    for(auto itr = _queryPendingInfo.begin(); itr != _queryPendingInfo.end(); itr++)
    {
        bool ret = detectCycle(itr->first, itr->first, true);    
        if(ret)
        {
            std::cout << "Deeadlock started from " << itr->first << std::endl;
            isDeadlockSeen = true;
        }
    }

    if(!isDeadlockSeen)
            std::cout << "No Deeadlock seen" << std::endl;

    return false;
}

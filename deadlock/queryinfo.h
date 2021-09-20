#ifndef _QUERY_INFO_H_
#define _QUERY_INFO_H_

#include <unordered_map>
#include <list>
#include <vector>
#include <string>
#include <unordered_set>

struct ObjectInfo
{
    std::list<unsigned int> _lockQueryId;
    std::list<unsigned int> _pendingQueryId;
};

struct LockHeldQueryId
{
    LockHeldQueryId(unsigned int queryId) : _queryId(queryId), _visited(false)
    {

    }

    unsigned int _queryId;
    bool _visited;
};

class QueryInfo
{
public:
    QueryInfo() {} 
    ~QueryInfo();
    bool addObjectInfo(std::string node, std::string object, unsigned int lockQueryId, unsigned int pendingQueryId);
    bool detectDeadlock();

private:
    typedef std::unordered_map<unsigned int, std::list<LockHeldQueryId>*> QUERY_PENDING_INFO;
    typedef QUERY_PENDING_INFO::iterator QUERY_PENDING_INFO_ITERATOR;

    void GenerateQueryWaitGraph();
    bool detectCycle(unsigned int queryId, unsigned int currId, bool isStarted);

    QUERY_PENDING_INFO      _queryPendingInfo;
    std::unordered_map<std::string, ObjectInfo*> _objectList;
    std::unordered_set<unsigned int> _deadLockList;
};

#endif

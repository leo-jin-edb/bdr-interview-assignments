/** This class manages a map of queries and which objects they have locked **/
#ifndef _QUERYMAP_HPP_
#define _QUERYMAP_HPP_

#include <map>
#include <list>
#include <string>

class QueryMap {
public:
	typedef std::map<int, std::list<int>> DeadlockMap;
	QueryMap();
	~QueryMap();
	bool addLock(int query, std::string node, std::string object);
	bool addPending(int query, std::string node, std::string object);
	bool addList(std::string node, std::string objName, std::list<int> queryIds);
	bool detectDeadlocks(DeadlockMap* pdeadlocks);
private:
	typedef std::map<std::string, int> LockMap;  // Key is object name, value is query id
	typedef std::map<int, std::list<int>> PendMap; // Key is query id, value is list of pending queries
	typedef std::map<int, std::list<std::string>> TmpMap;
	LockMap locksHeld;
	PendMap locksPending;
	TmpMap tmpPending;
	DeadlockMap dmap;
	bool movePends(); // move all objects from tmp map to pend map
	bool addPending(int queryId, int lockerId);
	bool searchForCycles(int queryId, int otherQuery);
	bool addDeadlock(int q1Id, int q2Id);
};
#endif // _QUERYMAP_HPP_

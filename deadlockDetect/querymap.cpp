#include <algorithm>
#include "querymap.hpp"

using namespace std;

QueryMap::QueryMap() {

}

QueryMap::~QueryMap() {

}

bool QueryMap::addLock(int queryId, string nodeName, string objName) {
	bool rval = true;
	string nodeObjName = nodeName + objName;

	if (locksHeld.find(nodeObjName) != locksHeld.end()) {
		rval = false;		// Object is already locked -- shouldn't be possible
	} else {
		auto res = locksHeld.insert({nodeObjName, queryId});
		rval = res.second;
	}

	return rval;
}

bool QueryMap::addPending(int queryId, string nodeName, string objName) {
	bool rval = true;
	string nodeObjName = nodeName + objName;

	LockMap::iterator lockInfo;
	lockInfo = locksHeld.find(nodeObjName);
	if (lockInfo == locksHeld.end()) {
		// we don't know the locker yet, so put in temp pending map
		// we'll move it later
		TmpMap::iterator tmpInfo;
		if ((tmpInfo = tmpPending.find(queryId)) == tmpPending.end()) {
			// the query isn't in the table yet, so add it with object name
			list<string> tmp({nodeObjName});
			auto res = tmpPending.insert({queryId, tmp});
			rval = res.second;
		} else {
			// query is in the table: add objName to its pending list
			tmpInfo->second.push_back(nodeObjName);
		}
	} else {
		// we know the locker, so put in pending locks map.
		rval = addPending(queryId, lockInfo->second);
	}
	return rval;
}

bool QueryMap::addPending(int queryId, int lockerId) {
	auto pendInfo = locksPending.find(queryId);
	int rval = true;
	if (pendInfo == locksPending.end()) {
		// the query isn't in the table yet, so add it
		list<int> tmp({lockerId});
		auto res = locksPending.insert({queryId, tmp});
		rval = res.second;
	} else {
		// Query is in the table: add query it's pending on
		pendInfo->second.push_back(lockerId);
	}
	return rval;
}

/* Add list of all locks to the maps:
 *   First item in list is the query holding the lock
 *   Other items in list are the queries waiting for the lock
 */
bool QueryMap::addList(string nodeName, string objName, list<int> queryIds) {
	bool rval = true;

	if (!queryIds.empty()) {
		rval = addLock(queryIds.front(), nodeName, objName);
		queryIds.pop_front();
		while (rval && !queryIds.empty()) {
			rval = addPending(queryIds.front(), nodeName, objName);
			queryIds.pop_front();
		}
	}
	return rval;
}

bool QueryMap::movePends() {
	bool rval = true;
	TmpMap::iterator tmpInfo;
	TmpMap errorMap;
	bool error = false;
	for (tmpInfo = tmpPending.begin(); tmpInfo != tmpPending.end(); ++tmpInfo) {
		auto plist = &tmpInfo->second;

		list<string> errorList;
		while (!plist->empty()) {
			string objId = plist->front();
			auto lockInfo = locksHeld.find(objId);
			if (lockInfo == locksHeld.end()) {
				errorList.push_front(objId);
			} else {
				addPending(tmpInfo->first, lockInfo->second);
			}
			plist->pop_front();
		}
		if (!errorList.empty()) {
			error = true;
			errorMap.insert({tmpInfo->first, errorList});
		}
	}
	tmpPending.clear();
	// if there is anything that we couldn't match it's an error, but don't discard the info
	if (error) {
		tmpPending = errorMap;
		rval = false;
	}
	return rval;
}

bool QueryMap::detectDeadlocks(DeadlockMap *pDeadlocks) {
	bool rval = false;  // assume no deadlocks are found

	movePends();  // don't care about errors for now
	for (auto pendInfo = locksPending.begin(); pendInfo != locksPending.end(); ++pendInfo) {
		printf("%d is waiting for:\n", pendInfo->first);
		for (auto lockIt = pendInfo->second.begin(); lockIt != pendInfo->second.end(); ++lockIt) {
			printf("  %d", *lockIt);
		}
		printf("\n");
	}

	// Do a depth-first walk through the pending locks map to look for cyclic dependencies
	for (auto pendInfo = locksPending.begin(); pendInfo != locksPending.end(); ++pendInfo) {
		for (auto lockIt = pendInfo->second.begin(); lockIt != pendInfo->second.end(); ++lockIt) {
			printf("Checking between %d and %d\n", pendInfo->first, *lockIt);
			bool locrval = searchForCycles(pendInfo->first, *lockIt);
			if (locrval) {
				addDeadlock(pendInfo->first, *lockIt);
				rval = true;
			}
		}
	}
	if (rval) {
		*pDeadlocks = dmap;
	}
	return rval;
}

bool QueryMap::searchForCycles(int queryId, int lockerId) {
	bool rval = false;   // assume no cycles
	if (queryId == lockerId) {
		return true;
	}
	auto pendInfo = locksPending.find(lockerId);
	if (pendInfo == locksPending.end()) {
		return false;	// no dependencies, so no deadlocks
	}
	for (auto nextPend = pendInfo->second.begin(); nextPend != pendInfo->second.end(); ++nextPend) {
		if (dmap.find(*nextPend) != dmap.end()) {
			// we already found a deadlock here, so this is deadlocked also
			return true;
		}
		rval = searchForCycles(queryId, *nextPend);
		if (rval) {
			addDeadlock(queryId, *nextPend);
			printf("found deadlock between %d and %d\n", queryId, lockerId);
		}
	}
	return rval;
}


bool QueryMap::addDeadlock(int q1Id, int q2Id) {
	// deadlock detected between queries q1Id and q2Id
	// add them (both directions) to the deadlock map
	DeadlockMap::iterator dit = dmap.find(q1Id);
	bool rval = true;
	if (q1Id == q2Id) {
		printf("Trying to add %d deadlocked to itself\n", q1Id);
		return rval;
	}
	if (dit == dmap.end()) {
		auto res = dmap.insert({q1Id, {q2Id}});
		rval = res.second;
	} else {
		if (find(dit->second.begin(), dit->second.end(), q2Id) == dit->second.end()) {
			dit->second.push_back(q2Id);
		}
	}
	dit = dmap.find(q2Id);
	if (dit == dmap.end()) {
		auto res = dmap.insert({q2Id, {q1Id}});
		rval = res.second && rval;
	} else {
		if (find(dit->second.begin(), dit->second.end(), q1Id) == dit->second.end()) {
			dit->second.push_back(q1Id);
		}
	}
	return rval;
}

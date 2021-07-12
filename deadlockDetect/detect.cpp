/** This is the main file for the deadlock detector.
 *  It reads data that describes the set of locks taken and requested for a set of query
 *  operations which make queries distributed across a set of nodes.
 *  The data is ordered by node, objects locked on that node, and which queries own or
 *  requested a lock on that object (as a comma separated list with the first entry being
 *  the locking query on that node and the rest being queries with pending locks on that
 *  object on that node.
 *
 *  The input data is parsed and put in tables organized by query.
 *  For each query, the objects it has locked are saved in one table.
 *  For each query, the objects that it has requested locks on is also maintained.
 *  It is assumed that a given object may be successfully locked separately on each node,
 *  so that Query 1 can lock object 1 from node 1, while Query 5 can lock object 1 from node 2.
 */
#include <cstdio>
#include <iostream>
#include "querymap.hpp"

using namespace::std;

int main(int argc, char* argv[]) {
	QueryMap qmap;
	qmap.addList("node2", "obj1", list<int>({5,4}));
	qmap.addList("node1", "obj1", list<int>({1,3}));
	qmap.addList("node1", "obj2", list<int>({4,10}));
	qmap.addList("node3", "obj2", list<int>({3,5}));
	qmap.addList("node4", "obj1", list<int>({3,1}));
	QueryMap::DeadlockMap dmap;
	cout << "Starting deadlock detection...\n";
	bool rval = qmap.detectDeadlocks(&dmap);
	if (!rval) {
		cout << "No deadlocks detected\n";
	} else {
		cout << "Deadlocks detected:\n";
		for (QueryMap::DeadlockMap::iterator dit = dmap.begin(); dit != dmap.end(); ++dit) {
			int qid = dit->first;
			cout << "   ";
			cout << qid;
			cout << " with:" << endl;
			for (auto it2 = dit->second.begin(); it2 != dit->second.end(); ++it2) {
				cout << "      " << *it2 << endl;
			}
		}
	}
	return 0;
}

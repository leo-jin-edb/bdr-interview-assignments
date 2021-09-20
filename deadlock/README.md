Implementation details -
class QueryInfo maintain data structure to hold query pending information (QUERY_PENDING_INFO). This data structure contain list of pending query ID. And each pending query ID maintain list of query ID which already holding lock on object. We can consider it as adjacency list which form graph.

API detectDeadlock() will traverse though pending query ID list. On each query ID , it will use DFS to traverse to lock ID list to check if there is cycle present or not. But detecting cycle is no sufficient to confirm deadlock. We need to consider total resources available. So even if cycle found , try to visit each node belongs to query ID , lock ID. And if we found atlead 1 node which hold resource and it not wait for any resource then we can say no deadlock.
e.g.
Resource	lock_held_query 	 pending_query
obj1 		3			1
obj2		4			2
obj4		2			3
obj3		1			4
obj1		5			6

Here there is cycle => 1->4->2->3->1
But there is query 5 which hols obj1 and it is not waiting for any resource. So even if we have cycle, query 5 will release obj1 and it can be utilize to break cycle. So no deadlock.

How to run :-
 $> make clean
 $> make
 $> ./deadlock


How to resolve :-
We can revoke resources from query which hold less number of resouces to break cycle. Assumption here is that query having less number of resources has done less amount of work.

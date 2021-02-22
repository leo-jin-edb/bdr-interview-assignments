#include <mutex>
#include <atomic>

/* This code fragment supports deadlock detection in a distributed
   lock manager. Go to sleep waiting to be awoken when request can be
   granted. Interrupt sleep after DEADLOCK_TIMEOUT seconds have elapsed
   and call deadlockScan(request} with our lock request. The deadlock
   target is the request that has waited the least amount of time as
   defined by time() seconds (seconds since Jan. 1, 1970).

    bool waitForGrant(LockReq* request) {
    LockReq* target;
    ...
    	if ((target = deadlockScan(request)) {
	    return (target != request);	// don't dereference target;
	}
    ...
    }
*/

typedef int NodID;
typedef int QryID;
typedef int ReqID;
typedef int LckID;

struct Node {
    NodID id;
    string name;
    socket lockConn;	/* normal lock request connection */
    socket dlckConn;	/* deadlock scan connection */
};

struct Query {
    QryID id;
    string textOfQuery;
    std::Dict<ReqID, LockReq*> grantedRequests;
    LockReq* waitRequest;
};

struct Lock {
    LckID id;
    NodID nodeID
    string name;
    std::Dict<QryID, LockReq*> requests;
    char state;		/* highest granted state */
    char dataLength;	/* application-specific state */
    char data[1];
};

enum REQF {REQF_visited = 1, REQF_deadlock = 2};

struct LockReq {
    ReqID id;
    QryId queryID;
    string lockName;
    int	waitTime;	// populated by time(). Deadlock victim is youngest
    char grantedState;
    char requestedState;
    char flags;
};

class DistLockMgr {
    static const int MAX_NODES = 1000;
    static const int MAX_QUERIES = 1000;
    static const int DEADLOCK_TIMEOUT = 30 // seconds

    Dict<NodID, Node> nodes;		// DLM member nodes;
    Dict<QryID, Query> queries;
    Dict<String, Lock> locks;
    NodID localNodeID;			// this(local) node
    Node localNode;
    std::atomic<int> cmdSequence;

    void deadlockScanner(DistLockMgr dlm);
    LockReq* deadlockScan (LockReq&);
    Status deadlockPrepare (bool leader);
    LockReq* deadlockWalk (LockReq&);
    void deadlockComplete();
};

/* Lock client entrypoint for requestin a deadlock scan */

LockReq* DistLockMgr::deadlockScan (LockReq* request)
{
    CMD cmd;

    /* Send deadlock scan request to the node that masters
       this lock. */

    Lock lock = locks.search(request->lockName);
    Node& node = lock.search(lock->nodeID);	// get lock master node
    cmd.command = DEADLOCK_SCAN;
    cmd.sequence = ++cmdSequence;
    cmd.request = *request;
    sendTo(node.dlck_conn, &cmd);
    recvFrom(node.dlck_conn, &cmd);
    if (cmd.status == DLCK_NOT_FOUND) return NULL;
    return (cmd.status == DLCK_FOUND_US) ? request : &cmd_request;    
}

/* Lock scanner thread entrypoint to serve deadlock requests */

void DistLockMgr::deadlockScanner(DistLockMgr dlm)
{
    CMD cmd;
    STATUS status;
    LockReq *victim;
    Node *node;

recvLoop:	// should have had while(true) loop

    node = recvFrom(localNode->dlck_conn, &cmd);
    switch (cmd.command) {
	case DEADLOCK_SCAN:
	    status = deadlockPrepare(true, localNodeID);
	    if (status == DLCK_SUCCESS) {
		if ((victim = deadlockWalk(&cmd_request))) {
		    cmd.status = (victim == &cmd_request) ? DLCK_FOUND_US : DLCK_FOUND;
		    cmd.request = *victim;
		    sendto (node->dlck_conn, &cmd);
		}
		else {
		    cmd.status = DLCK_NOT_FOUND;
		    sendto (node->dlck_conn, &cmd);
		}
	    }
	    deadlockComplete (true);
	    break;

	case DEADLOCK_PREPARE:
	    if (coordinatorID)
		status = DLCK_BUSY;	// busy with another deadlock request
	    else
	        status = deadlockPrepare(false, cmd.nodeID);
	    cmd.status = status;
	    sendto (node->dlck_conn, &cmd);
	    break;

	case DEADLOCK_WALK:
	    status = deadlockWalk(&cmd.request);
	    cmd.status = status;
	    sendto (node->dlck_conn, &cmd);
	    break;

	case DEADLOCK_COMPLETE:
	    deadlockComplete (false);	// no response
	    break;

	default:
	    assert(false);
	    break;
    }

    goto recvLoop;
}
	    
Status DistLockMgr::deadlockPrepare (bool leader, NodID nodeID)
{
/* Clears deadlock detection flags for all waiting
   requests across the DLM. These flags prevent cycles
   and detect the request involved in deadlock. */

    Node* node;
    CMD cmd;

    /* Request other nodes to prepare for deadlock scan */

    if (leader) {
    	for (int i = 0; i < MAX_NODES; ++i) {
	    node = nodes.iterate(&i);
	    if (node.id == localNodeID)
	    	continue;
	    cmd.command = DEADLOCK_PREPARE;
	    cmd.sequence = cmdSequence;
	    cmd.nodeID = nodeID;
	    sendTo (node.dlck_conn, &cmd);
	}
    }

    /* Do on the local node what we just broadcast to the other
       DLM nodes to do. That is, clear deadlock flags for queries
       that are waiting for a lock request to be granted.*/

    for (int j = 0; j < MAX_QUERIES; ++j) {
	Query* query = queries.iterate(&j);
	if (query->waitRequest)
	    query->waitRequest->flags &= ~(REQF_visited | REQF_deadlock);
    }

    /* Consume responses from other nodes */

    Status status = DLCK_SUCCESS;
    if (leader) {
    	for (int i = 0; i < MAX_NODES; ++i) {
	    node = nodes.iterate(&i);
	    if (node.id == localNodeID)
	    	continue;
	    recvFrom (node.dlck_conn, &cmd);
	    if (cmd.status != DLCK_SUCCESS)
		error = cmd.status;
	}

	/* If there was an error cancel the DEADLOCK_PREPARE */

	if (status != DLCK_SUCCESS)
	    deadlockComplete (true);
    }

    if (status == DLCK_SUCCESS)
	coordinatorID = nodeID;

    return status;
}
 
void DistLockMgr::deadlockComplete (bool leader)
{
    Node node;
    CMD	cmd;

    coordinatorID = 0;

    if (leader) {
    	for (int i = 0; i < MAX_NODES; ++i) {
	    node = nodes.iterate(&i);
	    if (node.id == localNodeID)
	    	continue;
	    cmd.command = DEADLOCK_COMPLETE;
	    cmd.sequence = cmdSequence;
	    sendto (node.dlck_conn, &cmd);
	    /* No need to receive response; it won't be sent */
	}
    }
} 

LockReq* DistLockMgr::deadlockWalk (LockReq*)
{
    Lock& lock;
    LockReq *other, *waitReq;
    Query *query;

    /* Prevent cycle if already scanned in wait-for search */

    if (request->flags & REQF_visited)
	return NULL;

    if (request->flags & REQF_deadlock)
	return request;

    request->flags |= REQF_deadlock;

    /* Check for deadlock on DLM node where that lock is mastered. */

    if (!(lock = locks.search(request->lockName)))
	return NULL;

    if (lock.nodeID != localNodeID) {
	node = nodes.search(lock.nodeID);
	cmd.command = DEADLOCK_WALK;
	cmd.sequence = cmdSequence;
	cmd.lockNodeID = lock.nodeID;
	cmd.request = *request;
	sendTo (node.dlck_conn, &cmd);

	/* While waiting for response to our deadlock walk, we handle
           deadlock walk callbacks */

	while (true) {
	    node2 = recvFrom (localNode->dlck_conn &cmd);
	    if (cmd.lockNodeID != localNodeID)
	    	if (cmd.status == DLCK_COMPLETE)
		    return NULL;
		else if (cmd.status == DLCK_FOUND || cmd.status == DLCK_NOT_FOUND)
		    break;
		
	    if (cmd.lockNodeID == localNodeID) {
		if (cmd.status == DLCK_COMPLETE) {
		    cmd.sequence = cmdSequence;
		    sendto (node2->dlck_conn, &cmd);
		    return NULL;
		    }
	    	if ((victim = deadlockWalk (&cmd.request))) {
		    cmd.status = DLCK_FOUND;
		    cmd.sequence = cmdSequence;
		    cmd.request = *victim;
		    sendto (node2->dlck_conn, &cmd);
		else {
		    cmd.status = DLCK_NOT_FOUND;
		    cmd.sequence = cmdSequence;
		    sendto (node2->dlck_conn, &cmd);
  		}
	    }		

	return (cmd.status == DLCK_FOUND) ? &cmd.request : NULL;
    }

    /* Otherwise handle to local locks mastered by this node */

    for (int i = 0; i < MAX_QUERIES; ++i) {
	other = lock.requests.iterate(&i);

	/* Skip our own request and if our requested state is
	   compatible with the other request's granted state. */

	if (other == request || compatible(request->requestedState, other->grantedState))
	    continue;

	/* If the other query is waiting on a pending request then
	   walk that wait-for branch. */

        query = queries.search(other->qryID);
	if ((waitReq = query.waitReq)) {

	    /* Found a deadlock. Favor the request that has been waiting
	       the longest by choosing the victim who has been waiting
	       the least amount of time. This assures some degree of
	       distributed liveliness */

	    if (waitReq = deadlockWalk(waitReq)) {
		if (waitReq->reqTime >= request->reqTime)
		    waitReq->flags |= REQF_deadlock;
		    return waitReq;
		}
		else {
		    request->flags |= REQF_deadlock;
		    return request;
		}
	    }
	}
    }

    /* No deadlock detected on this branch of the wait-for graph.
       Mark it as scanned. */

    request->flags &= ~REQF_deadlock;
    request->flags |= REQF_visited;
    return NULL;
}


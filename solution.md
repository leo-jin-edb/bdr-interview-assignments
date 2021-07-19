#requirements
1. All deadlocks should be found
2. Should take minimum time
3. Deadlock resolution


#Assumptions
1. We need global view of the (lock,query, object) status in JSON format
2. Each node will have a local JSON view which can be collected in one of the node-formed signed views, Assuming it is available
3. The  queryID got a lock for the same obj on different nodes, assuming they are shared locks
4. Lock granularity at Object level and not Node level


typedef struct {
        char node[64];
        char obj[64];
        char  waits_on[125]; // each bit represents queryID it depends on, Assuming 1000 Nodes
        // Bit is made on for queryid if it depends on
}Query;

Query *list[1000] = {NULL}

int create_WFG(char *jason_path)
{
        // Parse JSON file
        for each Node {
                for each obj {
                        // wait query from object e.g. obj1:[1, 3]
                        // 3 is waiting for 1 on obj1

                        // check query is already on list
                        if not then only
                                Query *q = alloc();
                                list[3] = q;
                        else
                                Query *q = list[3]

                        // Init node and obj

                        test_set_bit(q, 1);
                }
        }

        return 0;
}

/ Stack is required in DFS to keep track
int stack[1000] = {0};
int top = -1;

void push(int num)
{
        top++;
        stack[top] = num;
}

int pop()
{
        int val = stack[top];
        top--;

        return val;
}

// kind of Depth first search(DFS), not handled all cases
int search(int index, int skip_bits)
{
        q = list[index];
        for each bit in q->waits_on after skip_bitis {
                // e.g. 100 bit
                if (test_bit(q, 100)) {
                        // Found wait
                        if index == 100 { // index and bit matches to query
                                // found cycle
                                // Print the queryIds from stack
                                // set stack top = -1
                                return 100
                        } else {
                                push(100);
                                search(100, 0);
                        }
                }
        }

        return 0;
}

void find_cycles()
{
        for each query 1 to 1000 {
                current_query = queryID // examples
                int ret = 0;

                while(1) {
                        ret = search(queryId, ret);
                        // ret > 0, run again all bits are not checked
                        // ret = 0, break, all bits are checked
                        if (ret == 0) break;
                }
        }
}


# Deadlock resolution could be done by cancelling the query
1. Based on priority if defined
2. Based on maximum waits_on for query
3. Based on sequence number/QueryID , last query which forms the cycle


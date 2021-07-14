#include "graph.h"

using namespace std;

// Add an edge from u to v.
void Graph::addEdge(int u, int v) {
    graph[u].insert(v);
}


// Finds all the simple cycles using Tarjan's method, a
// modification to the depth first search by incorporating an additional
// stack (markedStack) to trace the nodes in the recursion sequence
// (and ensure the discovered cycles are simple---no nodes expcet the first
// and the last one repeat in the cycle).
vector<vector<int>> Graph::getAllSimpleCycles() {
    unordered_set<int> visited, // For dfs.
                       markedSet; // To check membership in markedStack.
    vector<int> st;  // To track the loop or visiting nodes in call stacks.
   
    // Track all the nodes visited during the traversal starting from the 
    // current origin node.
    stack<int> markedStack;  
   
    vector<vector<int>> res;

    for (auto& ent : graph) {
        enumerateCycles(ent.first, ent.first, visited, markedSet,
                        st, markedStack, res);
        visited.insert(ent.first);
        
        // These nodes did not form any cycle with the previous origin,
        // but they might form cycles with the future ones. So, remove
        // from the marked set to traverse the nodes from the subsequent
        // origin node.
        while (!markedStack.empty()) {
            int u = markedStack.top();
            markedStack.pop();
            markedSet.erase(u);
        }
    }

    return res;
}

// Find all the cycles starting (and ending) at 'origin'.
bool Graph::enumerateCycles(int origin, int cur,
                            unordered_set<int>& visited,
                            unordered_set<int>& markedSet,
                            vector<int>& st,
                            stack<int>& markedStack,
                            vector<vector<int>>& out) {
    bool foundCycle = false;

    markedSet.insert(cur);
    st.push_back(cur);
    markedStack.push(cur);
   
    for (int next : this->graph[cur]) {
        if (visited.count(next)) continue;
        
        // Found a cycle starting at 'origin'.
        if (next == origin) {
            foundCycle = true;
            st.push_back(next);
            
            vector<int> cycle;
            // Don't repeat the origin node at the end.
            for (int i=st.size()-2; i >=0 ; i--) {
                cycle.push_back(st[i]);
            }

            st.pop_back();
            out.push_back(cycle);
        } else if (!markedSet.count(next)) {
            foundCycle |= enumerateCycles(origin, next, visited, markedSet,
                                   st, markedStack, out);
        }
    }
   
    // Clean up the marked nodes as the call stack for the node 'cur'
    // is wiped out. This is to make way for those nodes to discover more 
    // cycles in subsequent traversals from the nodes ancestor to 'cur'.
    if (foundCycle) {
        while(markedStack.top() != cur) {
            markedSet.erase(markedStack.top());
            markedStack.pop();
        }
        markedStack.pop();
        markedSet.erase(cur);
    }

   st.pop_back();
   return foundCycle;
}

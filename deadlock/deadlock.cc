#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <tuple>
#include <functional>

#include "graph.h"

using namespace std;
using Tuple=tuple<int, int, string>;  // holder, requestor, object.

struct Node {
    string name;
    vector<Tuple> waits;
};

Graph parseInput(string& input) {
    string str;
    vector<Node> nodeList;

    istringstream issGraph(input);
    while(getline(issGraph, str)) {
        Node node;
        int holder, requestor;
        char ch; 

        istringstream issNode(str);
        getline(issNode, node.name, ':');  // Node name.
        for (string obj; (issNode >> ch >> holder >> ch >> requestor >> ch) 
                          && getline(issNode, obj, ')'); ) { 
            // Depedency edge/tuple.
            node.waits.push_back(make_tuple(holder, requestor, obj));
            if (!issNode.eof()) issNode >> ch;
        }

        nodeList.push_back(move(node));
    }

    Graph graph;
    // Generate the wait-for graph: the requestor is waiting for the holder.
    for (auto& node : nodeList) {
        for (auto [holder, requestor, obj] : node.waits) {
            graph.addEdge(requestor, holder);
        }
    }
    
    return graph;
}

/** 
 * Use greedy algorithm to solve the minimum set cover problem:
 *  select queries to cover all the cycles; here, each query represents
 *  a subset of the cycles it can break;
 * With cost values assoticated with each queries, the problem become a
 * weighted set cover problem.
 * */
vector<int> selectVictimQueries(vector<vector<int>>& cycles) {
    int n = cycles.size(), coveredCount = 0;
    unordered_map<int, unordered_set<int>> queryToCycleMap;

    for (int i=0; i < n; i++) {
        for (int q : cycles[i]) {
            queryToCycleMap[q].insert(i);
        }
    }
        
    vector<int> result;
    while (coveredCount < n) {  // All cycles are not covered.
        int maxQ = n, maxUncoveredCycles = 0;
        for (auto& ent : queryToCycleMap) {
            if (maxUncoveredCycles < ent.second.size()) {
                maxUncoveredCycles = ent.second.size();
                maxQ = ent.first;
            }
        }
        assert(maxQ != n);
        coveredCount += maxUncoveredCycles;
       
        // Prune the covered cycles from the map entries.
        for (int cycle : queryToCycleMap[maxQ]) {
            for (auto& ent : queryToCycleMap) {
                if (ent.first == maxQ) continue;
                ent.second.erase(cycle);
            }
        }

        queryToCycleMap.erase(maxQ);
        result.push_back(maxQ);
    }

    return result;
}


int main() {  
    // (holder, requestor, obj)
    string str = "node1: (2, 1, obj1), (2, 2, obj1), (4, 3, obj4), (3, 2, obj3), (2, 4, obj2), (3, 4, obj2), (1, 4, obj1), (1, 3, obj2), (2, 3, obj2) \n \
                  node2: (6, 7, a1), (7, 6, a2)";
    Graph graph = parseInput(str);
    vector<vector<int>> cycles = graph.getAllSimpleCycles();
    assert(cycles.size() == 7);
    cout << "Cycles.." << endl;
    for (auto& v : cycles) {
        for (int n : v) {
            cout << n << " ";
        }
        cout << endl;
    }

    vector<int> victims = selectVictimQueries(cycles);
    
    cout << "Victims.." << endl;
    for (int n : victims) {
        cout << n << " ";
    }
    cout << endl;
}

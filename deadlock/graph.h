#include<vector>
#include<stack>
#include<unordered_map>
#include<unordered_set>

class Graph {
public:
    Graph(){}
    void addEdge(int u, int v);
    std::vector<std::vector<int>> getAllSimpleCycles();
 
private:
    // Find all the cycles starting (and ending) at 'origin'.
    bool enumerateCycles(int origin, int cur, std::unordered_set<int>& visited,
            std::unordered_set<int>& markedSet, std::vector<int>& st,
            std::stack<int>& markedStack, std::vector<std::vector<int>>& out);

    std::unordered_map<int, std::unordered_set<int>>  graph;
};

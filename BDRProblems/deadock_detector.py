from collections import defaultdict

# For each strongly connected component, find all elementary
# cycles. 
def simple_cycles(G, sccs):
    G = {v: set(nbrs) for (v,nbrs) in G.items()} # make a copy of the graph
    while sccs:
        scc = sccs.pop()
        startnode = scc.pop()
        if (startnode in G):
            remove_node(G, startnode)
        H = subgraph(G, set(scc))
        scc_temp = strongly_connected_components(H)
        if (len(scc_temp) == 0):
            print("query to kill: {}".format(startnode))
            return
        sccs.extend(scc_temp)

# Returns all strongly connected components in a graph
# This would allow isolating a set of queries that are
# dependent on each other from other sets that have no
# impact on this set
def strongly_connected_components(graph):
    # Tarjan's algorithm for finding SCC's
    # Robert Tarjan. "Depth-first search and linear graph algorithms." SIAM journal on computing. 1972.
    # Code by Dries Verdegem, November 2012
    # Downloaded from http://www.logarithmic.net/pfh/blog/01208083168

    index_counter = [0]
    stack = []
    lowlink = {}
    index = {}
    result = []
    
    def _strong_connect(node):
        index[node] = index_counter[0]
        lowlink[node] = index_counter[0]
        index_counter[0] += 1
        stack.append(node)
        successors = []
    
        if (node in graph):
            successors = graph[node]
        for successor in successors:
            if successor not in index:
                _strong_connect(successor)
                lowlink[node] = min(lowlink[node],lowlink[successor])
            elif successor in stack:
                lowlink[node] = min(lowlink[node],index[successor])

        if lowlink[node] == index[node]:
            connected_component = []

            while True:
                successor = stack.pop()
                connected_component.append(successor)
                if successor == node: break
            if (len(connected_component) > 1):
                result.append(connected_component[:])
    
    for node in graph:
        if node not in index:
            _strong_connect(node)
    return result

def remove_node(G, target):
    # Completely remove a node from the graph
    # Expects values of G to be sets
    del G[target]
    for nbrs in G.values():
        nbrs.discard(target)

def subgraph(G, vertices):
    # Get the subgraph of G induced by set vertices
    # Expects values of G to be sets
    return {v: G[v] & vertices for v in vertices}



##example:

dict = {
    # "node1": { "obj1": [1, 2, 5], "obj2": [2, 3], "obj3": [3, 1, 2, 4, 6], "obj4": [4, 5], "obj5": [5, 2], "obj6": [6, 4]}
    "node1": { "obj1": [1, 2, 5], "obj2": [2, 3], "obj3": [3, 1, 2, 4, 6], "obj4": [4, 5], "obj5": [5, 2], "obj6": [6, 4], "obj7": [8, 9], "obj8": [9, 8], "obj10": [7] }
    # "node2": { "obj1": [5, 4] },
    # "node3": { "obj2": [3, 5] }
}

# dict = {
#     "node1": { "obj1": [1, 3], "obj2": [4, 10] },
#     "node2": { "obj1": [5, 4] },
#     "node3": { "obj2": [3, 5, 1] }
# }

graph = {}

for node, objs in dict.items():
    for obj, qids in objs.items():
        graph[qids[0]] = qids[1:]
sccs = strongly_connected_components(graph)
for scc in sccs:
    simple_cycles(graph, [scc])



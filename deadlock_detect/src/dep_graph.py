'''
Implementation of dependency graph
'''

import logging
from collections import namedtuple, deque

log = logging.getLogger(__name__)
log.setLevel(level="INFO")


GraphEdge = namedtuple('GraphEdge', ['src', 'dst', 'obj', 'node'])


class Graph(object):
    '''
    Dependency graph representation
    '''

    def __init__(self):
        '''
        Dependency graph constructor
        '''
        self.nodes = set()
        self.edges = set()

    @property
    def num_nodes(self):
        return len(self.nodes)

    @property
    def num_edges(self):
        return len(self.edges)

    def node_add(self, node_id: int):
        '''
        Add graph node
        :param node_id: Integer ID of node to be added
        '''
        if node_id in self.nodes:
            log.debug(f"Node {node_id} already in graph")
        else:
            self.nodes.add(node_id)
            log.debug(f"Added node {node_id} to graph")

    def node_delete(self, node_id: int):
        '''
        Delete graph node
        :param node_id: Integer ID of node to be deleted
        '''
        if node_id in self.nodes:
            self.nodes.remove(node_id)
            # Delete add node edges
            rem_edges = set()
            for edge in self.edges:
                if not (edge.src == node_id or edge.dst == node_id):
                    rem_edges.add(edge)
            self.edges = rem_edges
            log.debug(f"Deleted node {node_id} from graph")
        else:
            log.warning(f"Node {node_id} not in graph")

    def edge_add(self, edge: GraphEdge):
        '''
        Add graph edge
        :param edge: named tuple representing the graph edge ('src', 'dst', 'obj', 'node')
        '''
        if edge in self.edges:
            log.debug(f"Edge {edge} already in graph")
        else:
            # Add edge nodes if they do not exist
            if edge.src not in self.nodes:
                self.nodes.add(edge.src)
                log.debug(f"Added node {edge.src} to graph")
            if edge.dst not in self.nodes:
                self.nodes.add(edge.dst)
                log.debug(f"Added node {edge.dst} to graph")
            # Add edge
            self.edges.add(edge)
            log.debug(f"Added edge {edge} to graph")

    def edge_delete(self, edge: GraphEdge):
        '''
        Delete graph edge
        :param edge: named tuple representing the graph edge ('src', 'dst', 'obj', 'node')
        '''
        if edge in self.edges:
            self.edges.remove(edge)
            log.debug(f"Deleted edge {edge} from graph")
        else:
            log.warning(f"Edge {edge} not in graph")

    def _get_adj_list(self):
        '''
        Build graph adjacency list
        :return: Graph agjacency list (dictionary of lists)
        '''
        adj_list = {}
        # Init ajd list nodes
        for node in self.nodes:
            adj_list[node] = []
        # Add edges
        for edge in self.edges:
            adj_list[edge.src].append(edge.dst)

        return adj_list

    def _dfs_subgraph(self, adj_list: dict,
                      stack: deque,
                      visited: set,
                      cycles: list):
        '''
        Traverse a sub-graph in depth-first-search (DFS) order
        Add cycles found during graph traversal to "cycles" list parameter
        '''
        if stack:
            start_node = stack[-1]
            for node in adj_list[start_node]:
                if node in stack:
                    # Found cycle
                    node_index = stack.index(node)
                    cycle = list(stack)[node_index:]
                    log.info(f"DFS: Found cycle {cycle}")
                    cycles.append(cycle)
                elif node not in visited:
                    stack.append(node)
                    visited.add(node)
                    self._dfs_subgraph(adj_list=adj_list,
                                       stack=stack,
                                       visited=visited,
                                       cycles=cycles)
            log.debug(f"DFS: Visited node {start_node}")
            visited.add(start_node)
            stack.pop()

    def find_cycles(self):
        '''
        Find cycles in the directed graph
        :return; Return a list of cycles found in graph
        '''
        # Init adjacency list
        adj = self._get_adj_list()
        # Set of visited nodes
        visited = set()
        # Graph cycles
        cycles = []

        # Trverse graph nodes
        for node in self.nodes:
            if node not in visited:
                stack = deque()
                stack.append(node)
                visited.add(node)
                sub_cycles = self._dfs_subgraph(adj_list=adj,
                                                stack=stack,
                                                visited=visited,
                                                cycles=cycles)
                if sub_cycles:
                    cycles = cycles + sub_cycles

        return cycles

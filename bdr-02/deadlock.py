#!/usr/bin/env python

import json
import argparse

import networkx as nx
import matplotlib.pyplot as plt # XXX: needed?


def main():
    # parse command line arguments
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '-i', '--input-file',
        type=open,
        help="JSON input file")

    args = parser.parse_args()

    # load query data
    data = json.loads(args.input_file.read())

    # create the query graph
    # https://networkx.org/documentation/stable/reference/classes/multidigraph.html
    query_graph = nx.MultiDiGraph()

    for node_id, node_data in data.items():
        for object_id, query_ids in node_data.items():
            locking_query, waiting_query = query_ids[0], query_ids[1]

            # XXX: a query could lock more than one object across multiple nodes
            query_graph.add_node(locking_query, type='query', color='red')
            query_graph.add_node(waiting_query, type='query', color='red')
            query_graph.add_edge(waiting_query, locking_query, color='red')

            query_graph.add_node(object_id, type='object', color='blue')
            query_graph.add_edge(locking_query, object_id, type='query-object', color='blue')
            query_graph.add_edge(object_id, locking_query, type='object-query', color='blue')

            query_graph.add_node(node_id, type='node', color='black')
            query_graph.add_edge(object_id, node_id, type='object-node', color='black')
            query_graph.add_edge(node_id, object_id, type='node-object', color='black')

    # XXX
    # https://networkx.org/documentation/stable/tutorial.html#drawing-graphs
    # https://networkx.org/documentation/stable/reference/drawing.html
    nx.draw(query_graph, with_labels=True)
    plt.show()


if __name__ == '__main__':
    main()

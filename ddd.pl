#!/usr/bin/perl

use strict;
use warnings;


use JSON;
use Path::Tiny;
use Data::Dumper;
use List::Util;

my $locks;
# just quick and dirty load of the source data
sub slurp_input {
    my ($filename) = @_;
    my $lock_file_data = Path::Tiny::path($filename)->slurp || die($!);
    $locks = decode_json $lock_file_data || die ($!);
}

# print %$locks;

# Notes:
# 1. We are looking for a deadlock; and that means a cycle in the directed graph of locks.
# 2. The graph may be disconnected (and usually is).
# 3. There can be more cycles. They can even be connected.
# 4. We find the longest cycle (as solving that will unblock the most queries and will
#    give us the best choice of queries to select from); note that here "cycle" includes
#    alternate paths, too (forks and joins). 
# 5. pick the query to kill:
#    a. either pick the one that blocks the most other queries - this will unblock
#       more other queries
#    b. or pick one that blocks the least. 
#    The idea is that query (a) will be probably a large one and rolling it back is 
#    expensive and will take long. So killing (b) will be faster and will allow the
#    cycle to unwind more naturally.

# graph vertex: (queryId), data = aux data: visited, etc.
# graph edge: (from, to), data = weight = count of edges in the source lock file
# for (5), just add weights of incoming edges

my %edges;
my %vertices;

sub load_graph {
    # create vertices and edges from the source lock file
    for my $node (keys %$locks) {
        my %node_list = % {%$locks{$node}};
        for my $obj (keys %node_list) {
            my $lock_list = $node_list{$obj};
            my ($holder, $waiter) = @$lock_list;
            $vertices{$holder} = { "visited" => 0 };
            $vertices{$waiter} = { "visited" => 0 };
            $edges{$waiter}{$holder}++;
        }
    }
}


# We do a depth-first traversal, vertex by vertex. First, we finding any cycle.
# But this may be coming from a joining branch; we don't want to include such a branch.
# Thus we take a vertex we now know is a part of the cycle; and then we redo the traversal again.
# Now with cutting of branches going off the cycle (we traverse them, but not )
# but since we want to find all cycles, "visited" is not a boolean but rather a 
# "generation". While traversin, we stop when we see the same generation. And we don't abort
# the traversal; we try other paths, too. We find the largest "cycle component".
#
# Note: we want only the cycle itself; not branches eventually joining the cycle.
# (Although that could also be an interesting area to explore - perhaps include it in costing
# the query to kill.)
# So once we find a cycle, we run the traverse once again, but starting from a node we know is
# on the cycle itself.

my @cycles;

# exit early, find just any node on the cycle
sub traverse_vertex_dry {
    my ($vertex)= @_;
    if ($vertices{$vertex}{"visited"}) { return 1;}
    $vertices{$vertex}{"visited"} = -1;
    for my $next_vertex (keys %{$edges{$vertex}}) {
        # print "traverse dry next: $next_vertex at []\n";
        my $n = traverse_vertex_dry($next_vertex);
        if ($n) {return $n;}
    }
    return undef;
}

# exhaustive search - find the whole cycle component; cut off any branches coming out of the cycle
sub traverse_vertex_for_cycle {
    my ($vertex, $generation) = @_;
    # If we really want the largest cycle even in complex case, we have to check we have seen this in this
    # particular traversal.
    # But we could cheat a bit and stop if we encounter any previous cycle. This will be faster, but will fail
    # to find cases like two cycles linked together, if we saw one of them before (we would find them each
    # separately, but not together as one big one.) In this mode, we treat such an old cycle as a dead-end branch off
    # the cycle we are traversing now.
    # if ($vertices{$vertex}{"visited"} > 0) {
    #     return ($vertices{$vertex}{"visited"} == $generation) + 0; 
    # }
    if ($vertices{$vertex}{"visited"} == $generation) { return 1; }
    $vertices{$vertex}{"visited"} = $generation;
    my $edge_weights = 0;
    my $found_any_cycle = 0;
    for my $next_vertex (keys %{$edges{$vertex}}) {        
        my $found_cycle = traverse_vertex_for_cycle($next_vertex, $generation);
        if ($found_cycle) {
            $edge_weights += $edges{$vertex}{$next_vertex};
            $found_any_cycle = 1;
        }
    }
    # this is a branch off the cycle. We don't want it in the cycle component. But keep it marked as visited, so we don't
    # run the full traversal from here again.
    if ($found_any_cycle == 0) {
        $vertices{$vertex}{"visited"} = -1;
    }
    $vertices{$vertex}{"cycle_weight"} = $edge_weights;
    return $found_any_cycle;
}


my %longest_cycle;
my $longest_cycle_weight = -1;

sub store_cycle {
    my ($generation) = @_;
    my @cycle_vertices = grep { $vertices{$_}{visited} == $generation} keys %vertices;
    my %new_cycle = map { $_ => $vertices{$_}{cycle_weight}} @cycle_vertices;
    my $weight = List::Util::sum( values %new_cycle );

    if ($weight > $longest_cycle_weight) {
        %longest_cycle = %new_cycle;
    }
}

sub traverse_all_vertices {
    my $generation = 1;
    for my $vertex (keys %vertices) {
        # traverse each vertex; but skip it if it's already been processed
        unless ($vertices{$vertex}{"visited"}) {
            my $found_vertex = traverse_vertex_dry($vertex);
            if ($found_vertex) {
                my $found = traverse_vertex_for_cycle($found_vertex, $generation);
                if ($found) {store_cycle($generation++);}
            }
        }   
    }
}


# Find the most expensive query. If there are more than one, pick at random.
sub pick_most_blocking_query {
  my $max_cost = List::Util::max( values %longest_cycle );
  my @max_queries =  grep { $longest_cycle{$_} == $max_cost} keys %longest_cycle;
  return $max_queries[0];
}

# Find the least expensive query. If there are more than one, pick at random.
sub pick_least_blocking_query {
  my $min_cost = List::Util::min( values %longest_cycle );
  my @min_queries =  grep { $longest_cycle{$_} == $min_cost} keys %longest_cycle;    
  return $min_queries[0];
}

######################
sub main {
    slurp_input("locks.json");
    load_graph();
    traverse_all_vertices();

    print "Most  blocking query: ", pick_most_blocking_query(), "\n";
    print "Least blocking query: ", pick_least_blocking_query(), "\n";
    # print Data::Dumper::Dumper(\%edges);
    # print Data::Dumper::Dumper(\%vertices);
    print "Longest cycle: ", Data::Dumper::Dumper(\%longest_cycle);
}

main();

# Deadlock

Distributed deadlock detection.

TODO:

#. [ ] Detect deadlocks.
  * Store data in a digraph with vertices for objects and queries and edges for
    depedencies between the queries.
  * Any cycles that go through two objects are a deadlock.
#. [ ] Break deadlocks efficiently.
  * What is `efficient` in this case?
  * One approach: Kill the query that breaks the cycle into the smallest
    dependency chains. For example, given the sample in the problem statement,
    you would kill query `5` (resulting in `1<-3, 4<-10`) instead of `3` (
    resulting in `1, 5<-4<-10`) This is based on the assumption that the most
    efficient execution is based on processing as many queries as quickly as
    possible.

## Quick Start

Create a Python 3 virtual environment and install requirements:

```bash
python -m venv env
source env/bin/activate
pip install -r requirements.txt
```

TODO: Generate a sample depedency graph.

Run the script against the depedency graph file:

```bash
./deadlock.py -i sample.json
```

# Notes

1. We are looking for a deadlock; and that means a cycle in the directed graph of locks.
2. The graph may be disconnected (and usually is).
3. There can be more cycles. They can even be connected.
4. We find the longest cycle (as solving that will unblock the most queries and will
   give us the best choice of queries to select from); note that here "cycle" includes
   alternate paths, too. Or more cycles connected together. But not branches going into the cycle or from it.
5. Pick the query to kill:
   * either pick the one that blocks the most other queries - this will unblock
      more other queries
   * or pick one that blocks the least.

   Although the first option sounds as a clear winner, such a query will be probably a large one and rolling it back is going to be expensive and will take a long time. So choosing the second option will be faster to kill and cleanup and will allow the cycle to unwind more naturally. Think a user holding a single row lock, blocking a large batch job.

# Usage
  main() is hardcoded to read `locks.json`.
  Basic tests are present (using `locks_test.json`), not invoked by default. (Change `main()` to test().)
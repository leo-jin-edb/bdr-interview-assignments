I see this as two problems to address:

1) efficient memory structure and algorithms for the dictionary lookup and modifications
2) sharing the memory between multiple processes; this includes sharing the memory as well as locking.

For 1), a trie is a good option. This is a well known problem and once can spend up quite some time devising the most effective implementation. Or even better, look around for an exising implementation.
I use a simple approach with reduced alphabet. And a plain trie, not a radix tree.

For 2) shared memory and IPC semaphores.
In this implementation there is a mutex on every item; in real-world, one would want to limit this a bit. Perhaps by limiting the trie depth protected by mutexes (mutex in the deepest level protects everything below, too).

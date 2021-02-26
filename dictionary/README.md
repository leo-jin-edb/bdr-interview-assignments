# Notes

I see this as two problems to address:

1) efficient memory structure and algorithms for the dictionary lookup and modifications;
2) sharing the memory between multiple processes; this includes sharing the memory as well as locking.

For 1), a trie is a good option. This is a well known problem and one can spend up quite some time devising the most effective implementation. Or even better, look around for an exising library.

I use a simple approach with reduced alphabet (lowercase 26 letters). And a plain trie, not a radix tree.

For 2) I use shared memory (shm_open + mmap) and IPC semaphores (sem_open). To make things simpler, the user first starts a "server" that loads data from disk (or in this case, there is some sample data hardcoded in the `load_some_sample_data()` function).

A (very, very dumb) custom allocator is implemented to allocate the trie nodes in the shared memory. There are more concurrency friendly allocators (you really want to use an existing library and not reinvent the wheel).

To keep things simple, just 2 global locks are used to prevent race conditions; one for the allocator, second for trie access, traversal and modifications.

The trie locking could be granular. RW mutex per trie node is probably an overkill; locking just a few top levels would be enough, although it complictes the code. (This is a tradeoff between conurrency and time spent in the critical sections - mutexes are not free.)

Pthread mutexes (pthread_rwlock_t) would be a good fit (since we already have shared memory where to put them). Read concurrency is easy; writes are not (imagine search for an item that is just being deleted - delete must read-lock the path, then find top of the subtrie being deleted and relock for write). Inserts a bit easier, as the new subtrie can be created before joining it to the full trie, so only one node needs to be write locked.

As with any system, there is always room for improvement. But one should stop soon enough to ask what are really the requirements (expected load, use case etc.)

# Note on the server

The only purpose of the "server" is really to setup the shared memory and load sample data. The search/insert/delete does not need it. However, someone needs to cleanup the memory and semaphores when they are no longer needed. The server would do this on shutdown.

Another approach would be to have a `dict setup` and `dict destroy` commands for the shmem setup and destroy.

Even better, `dict setup` can be included in `search|insert|delete`. Use semaphore/mutex to prevent race condition (so that the initial setup is done exactly once). But the `dict destroy` still needs to be done explicitly.

This all really depeneds on business requirements; such dictionary would be part of a bigger project and that would determine the best approach among the options.

# Usage

`make` builds the `./dict` executable.

* `dict server` loads sample data into memory, sets up shared memory;
* `dict [search|insert|delete] word` modifies the dictionary;
* `dict test` runs unit tests (local, does not use the shared memory);
* Any other invocation shows a very simple usage prompt.
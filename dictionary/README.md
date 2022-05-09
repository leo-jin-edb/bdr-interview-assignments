# Alan Estrada's Thoughts On Problem 1 - Dictionary

## Introduction

Most technical assessments are not only about testing the abilities of the developer
but also to get an understanding of their thought processes. So I'm adding this
to help you guys get what's going on in my head.

## Deciding on An Architecture

After reading the problem for the first time, my first thought was that it'd be great
as a service-based architecture. I've been coding in mostly JavaScript for the last
few years so I'd set a server up. It'd add requests to a queue, which would spawn
a child process to handle each request on the queue. I'd lock the hashtable/dictionary
object on operations. We could do some optimizations (e.g. allow multiple readers to
read) to speed things up.

But after re-reading the problem, I think what this specific question is trying to test
is making sure we can coordinate multiple processes running concurrently. After all,
the data structure implementation is being hand-waved, no performance metrics are given,
and the requirements in this program are centered around that.

With that in mind, I'm going to go for something simple: a set in shared memory.

**Design Decisions**:

- The set will be a black box to us and will have insert(), delete(), and has() functionality
- The set will contain just strings
- Set will store unique elements only (e.g. calling an insert on the same string twice will cause the second call to be a no-op)
- The longest word I can think of is "antidisestablishmentarianism", which is 28 characters so I'll round up and make the max word length be 31 so with null byte, it'll be 32.
- We'll use a named semaphore to coordinate our processes

I added two commands for my own benefit, `init` and `end`. `init` will allocate the shared memory and create the named semaphore and `end` will unlink them.

I just assume you're going to name the executable `dict` so I really only parse the second and third arguments for the type of command, `search | insert | delete` and the word we're doing the operation on.

## Problems

Well, the implementation for Set could've been imperfect and failed during an insert or delete and we could've incorrectly incremented/decremented the number of words. However, I think this is out of the scope of the problem.

There's also the chance that the process just randomly crashes during the critical section, leaving every other process waiting on a semaphore that will never be incremented back to 1. We could fix this by just using a file lock, which will be released however way the process ends, e.g.

```
int fd = open("
  /tmp/.lock_dict",
  O_CREAT | O_WRONLY | O_EXLOCK, // exclusive lock
  0666
);
```

However, I'll stick with using the semaphore in v1 as it's usually the "go-to" lock. In v2, I'd use a file-lock instead.

## Optimizations

A lot could be optimized here. The first would probably be not to hand-wave the implementation of the Set. The Set could be a version of a hash-table, where the key would be hashed to figure out the bucket it goes to. Then, we could
have a semaphore/lock for each of the buckets. This would be v3.

A final optimization would be implementing a queue in the shared memory where we could potentially allow multiple readers to read at the same time. This'd be v4.

## Feedback

- A short gif or sample output showing what you guys are looking for would have been great,
  just as guidance to what you're looking for.

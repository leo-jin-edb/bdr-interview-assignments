# BDR Home Assignments

## Option 1 - Dictionary

A dictionary consists of english words stored in memory.
Maximum number of words in the dictionary is 1M.

Write a command-line program that allows concurrent (by multiple processes):

 * search for existence of a word in the dictionary
 * insert a word in the dictionary, if it does not exist
 * delete a word from the dictionary

The command should be invoked as following:
```
dict {insert|search|delete} <word>
```

Languages to chose from: C or C++
Assume APIs for data structures like queue, hash table, linked list etc. are available.
Document the APIs (no implementation required) as comments.

How to compile and use this app:
g++ -o dict dict_mp.cc
./dict <insert|search|delete> <word>



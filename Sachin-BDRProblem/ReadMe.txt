
Assumptions:
1) System would be read (Search) heavy. Probably 80-20% split. 20% could be Insert or Delete.
	a) Because in code, we are taking exclusive access at trie node level if we are modifying (Insert/Delete) anything at that node level. 
	b) We take Shared lock at Trie Node level, if we are reading anything at given node level.
2) 


System Weakness/Further Improvements:
1) MemoryMgr & SyncAtomic layers are not implemented.
	a) Even after implementation, as per current logic, small memory fragments (lets say upto 256 bytes after a cycle of few 100s of word inserts). Can be further fine tuned a little.

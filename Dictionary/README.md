
## Instructions

1. To compile go to src and execute make
2. To execute give below command
	$./MemoryMaster &
	$ dict {insert|search|delete} <word>



## Approch

MemoryMaster will create share memory and keep running. it can takes command to check the data

Current Memory layout is like

=================================
| SM_META| DATA_NODE|DATA_NODE|...
================================

DATA_NODE has Node meta and data with mutex object

TODO:
1. create data node index for faster searching
=================================
| SM_META| DATANODE_INDEX| DATA_NODE|DATA_NODE|...
================================
2. Use custom mutex for higher concrrency
3. Documentation
4. Test script
5. Bug fix
6. self review


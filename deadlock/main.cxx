#include "queryinfo.h"

int main()
{
    QueryInfo obj;
	obj.addObjectInfo("node2", "obj1", 5, 4);
	obj.addObjectInfo("node1", "obj1", 1, 3);
	obj.addObjectInfo("node1", "obj2", 4, 10);
	obj.addObjectInfo("node3", "obj2", 3, 5);
//Add below entry to add deadlock.
	obj.addObjectInfo("node4", "obj1", 3, 1);

/*
    obj.addObjectInfo("node2", "obj1", 3,1);
    obj.addObjectInfo("node1", "obj2", 4,2);
    obj.addObjectInfo("node2", "obj4", 2,3);
    obj.addObjectInfo("node1", "obj3", 1,4);
//    obj.addObjectInfo("node1", "obj1", 5,6);
*/
    obj.detectDeadlock();
}

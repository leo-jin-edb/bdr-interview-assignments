#include "Common.h"
#include "MemoryManager.h"
const char *VERSION = "0.0.1";

int main(int argc, char *argv[])
{

    /* read config from default path*/
    Conf::config.readAllValues(NULL);

    MemoryManager memMgr;
    printf ("Starting Memory Manager \n");
    memMgr.createSharedMemory();
    
    do{
    	printf("Press 0 for quit\n");
    	printf("Press 1 for statistics \n");
    	printf("\n \n");
    	int choice = -1;
    	scanf ("%d", &choice);
	switch (choice)
	{
		case 0:
			memMgr.closeSharedMemory();
			memMgr.deleteSharedMemory();
			break;
		case 1:
			memMgr.printStatistics();
			break;
		default:
			printf ("Invalid Option %d\n", choice);
			break;
                
	}
	if (0 == choice)
	{
		break;
	}
    }while (1);

    return 0;
}


#include <stdio.h>
#include <string.h>


#define  MAX_RESOURCES_CNT   2
#define  MAX_QUERY_CNT       5


int main()
{
    /* we have considered shared example from the problem 
     * This matrix is made from the 
     * Resouce allocation graph please find the 
     * doc in the same folder.
     *       Allocated Resources         Remaining Need
              |obj1 |obj2 |               |obj1 |obj2|
         -------------------------------------------------------    
         Q1   |1   |   0 |                |0   |  0 |
         -----------------------------------------------------
         Q3   |0   |   1 |                | 1  |  0 |
         -----------------------------------------------------
         Q4   |0   |  1  |                | 1  | 0  |
         -----------------------------------------------------
         Q5   | 1  | 0   |                | 0  | 1  |
         -----------------------------------------------------
         Q10  | 0   |  0 |                | 0  | 1  |
         -----------------------------------------------------
       
     */

    /* This means that there are 2 resources in the system having 
     * 2 instances each
     */
	int maxResources[] = {2, 2};

	const char *queryList[MAX_QUERY_CNT] = {"Q1", "Q3", "Q4",
	                                       "Q5", "Q10"
										 };

	/* This Matrix is created on the basis of Resouce Allocation Graph
	 * Allocated Edges i.e qeuries those assigned the resources
	 */
    int AllocatedResources[MAX_QUERY_CNT][MAX_RESOURCES_CNT] = {
                                                                {1, 0},
                                                                {0, 1},
                                                                {0, 1},
                                                                {1, 0},
                                                                {0, 0}
                                                            };    

    /* This matrix created on the basic of Resouce Allocation Graph
     * Request Edges, i.e query those are waiting for the resouces
     */
    int RemaingNeed[MAX_QUERY_CNT][MAX_RESOURCES_CNT] = {
                                                            {0, 1},
                                                            {1, 0},
                                                            {1, 0},
                                                            {0, 1},
                                                            {1, 1}
                                                        };

    														
    int  avilableResources[MAX_RESOURCES_CNT] = {0};
	int  usedResourceCnt = 0;
	bool queryExec = false;

	int  running_query_cnt = 	MAX_QUERY_CNT;
	int  runningQueryList[MAX_QUERY_CNT];


	bool bIsUnsafeSate = false;
	int  kill_query_idx = -1;

    //consider all the query running 
	memset((void*)runningQueryList, 1, sizeof(runningQueryList));

    /* calculate total avilable resouces */
	for (int resource_cnt = 0; resource_cnt < MAX_RESOURCES_CNT; resource_cnt++)
	{
          for (int query_cnt = 0; query_cnt < MAX_QUERY_CNT; query_cnt++)
    	  {
	           usedResourceCnt += AllocatedResources[query_cnt][resource_cnt];
          }
		  avilableResources[resource_cnt] = maxResources[resource_cnt] - usedResourceCnt;
		  usedResourceCnt = 0;
		  
	}


	/* check each query Remaining Resources need against the avilableResources
     * if avilableResources >= Query Remaining Resource Need 
     *    a. current query will execute successfully.
          b. handover the allocated resource to avilable resource list.
          
       else:
           continue for the next query.
	 */

    while (running_query_cnt)
    {
		for (int query_idx = 0; query_idx < MAX_QUERY_CNT; query_idx++)
		{
              /* query is excuted, check for next active query */
		      if (!runningQueryList[query_idx])
			  	  continue;

			  bIsUnsafeSate = false;
			  kill_query_idx = -1;
	        
	          queryExec = true;
	          for (int resource_cnt = 0; resource_cnt < MAX_RESOURCES_CNT; resource_cnt++)
	          {
	               /* Avilable resources are less than Remaining resource. */
	               if (avilableResources[resource_cnt] < RemaingNeed[query_idx][resource_cnt])
	               {
	                   queryExec = false;
					   bIsUnsafeSate = true;
					   kill_query_idx = query_idx;
	                   break;
	               }
	          }

			  if (queryExec)
			  {
	              printf("Executing the Query: %s\n", queryList[query_idx]);

				  for (int idx = 0; idx < MAX_RESOURCES_CNT; idx++)
				  {
	                   avilableResources[idx] += AllocatedResources[query_idx][idx];
				  }
				  running_query_cnt -= 1;
				  runningQueryList[query_idx] = 0; // it means it's executed
				  
			  }
	    
		}

		if (bIsUnsafeSate)
		{
           printf("DeadLock detected in the system..\n");
           printf("kill the query: %s\n", queryList[kill_query_idx]);
		   break;
		}
    }

    if (!bIsUnsafeSate)
	   printf("System is not in deadlock condition.\n");
	
    return 0;
}


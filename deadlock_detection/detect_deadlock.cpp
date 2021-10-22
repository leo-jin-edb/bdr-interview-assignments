#include <stdio.h>
#include <string.h>
#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <fstream>

using namespace std;


#define  MAX_NODES_IN_CLUSTER 1000
#define  MAX_QUERY_PER_NODE 1000

/* This function read the json file.
 * which contain cluster info in json format
 * for Example:-
 *
 * 
 * {
 *     "node1": { "obj1": [1, 3], "obj2": [4, 10] }
 *     "node2": { "obj1": [5, 4] }
 *     "node3": { "obj2": [3, 5] }
 * }
 *
 * o/p:- create map from this. 
 *
 */ 
bool  ReadClusterInfoFromJsonFile(const char* filename, 
                         map<string, map<string, vector<int>>> &cluster_info)
{
      
     ifstream inFile;
     string line;

     /* open the json file */
     inFile.open(filename);
     if (inFile.fail()) 
     {
         cout << "failed to open the file: " << filename << endl;
         return false;
     }

     /* we have assumed that, this current function will 
      * parse the json and store the cluster information 
      * in the cluster_info object.
      */  
     while (getline(inFile, line))
     {
         //parse the line and those info in the cluster_info map
         
     }     

     inFile.close();
     return true;
}

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

     map<string, map<string, vector<int>>> cluster_info;
	 
     if (false == ReadClusterInfoFromJsonFile("cluster_info.json", cluster_info))
     {
         cout << "failed to read the clusterinfo.\n";
         return -1;
     }
  
     /* it will give for {"node1" :
	      {{"obj1", {1, 2}}, {"obj2", {3, 4}}}
		*/
	 /* For testing purpose json open can be commented and 
	    enable this below comment code.
	    */
     /*cluster_info["node1"]["obj1"] = {1,3};
     cluster_info["node1"]["obj2"] = {4,10};
     
     cluster_info["node2"]["Obj1"] = {5,4};
     
     cluster_info["node1"]["obj2"] = {3,5};*/
     
    /* validate total number of nodes in the cluster*/
    if (cluster_info.size() > MAX_NODES_IN_CLUSTER)
    {
       cout << "Nodes counts: " << cluster_info.size()<< " is more then the supprted in cluster." << endl; 
       return -1;
    }

    /* validate total number of query in the cluster*/
   /*
    *  {
    *    {"node1", {{"obj1", {1, 2}}, {"obj2", {3, 4}}},
    *    {"node2", { "obj1", {5, 4}},
    *    {"node3", { "obj2", {3, 5}}
    *  };
    */
   set<int> unique_query_per_node;
   set<string> unique_resource_name;
   map<int, map<string, int>> allocated_resources;
   map<int, map<string, int>> need_resources;
   map<string, int> max_resources;
   set<int> cluster_unique_query;

    //int  avilableResources[MAX_RESOURCES_CNT] = {0};
    map<string, int> avilableResources;
	int  usedResourceCnt = 0;
	bool queryExec = false;

	int  running_query_cnt = cluster_unique_query.size();
	int  runningQueryList[running_query_cnt];


	bool bIsUnsafeSate = false;
	int  kill_query_no = 0;
	int  cur_running_query = 0;
	
	
   for (auto ech_node_info:cluster_info)
   {
       /* it will give for "node1" 
	      {{"obj1", {1, 2}}, {"obj2", {3, 4}}}
		*/
        for (auto resource_query_info:ech_node_info.second)
        {
             //collect unique resource from the each query to make resource allocation matrix
             unique_resource_name.insert(resource_query_info.first);
         
		      max_resources[resource_query_info.first] += 1;
			  
             /*for (auto ech_query:resource_query_info.second)
             {
                 unique_query_per_node.insert(ech_query); 
                 
             }*/
             vector<int> query_list = resource_query_info.second;
			 
			 /* Already Resource allocated for this query */
             unique_query_per_node.insert(query_list[0]); 
             
			 //Query need resource
			 unique_query_per_node.insert(query_list[1]); 

             // total unique query in cluster
             cluster_unique_query.insert(query_list[0]);
			 cluster_unique_query.insert(query_list[1]);
			 
             allocated_resources[query_list[0]][resource_query_info.first] += 1; // create resource allocation matrix
             
			 need_resources[query_list[1]][resource_query_info.first] += 1; // Remaining Need marix
			 
			 
             if (unique_query_per_node.size() > MAX_QUERY_PER_NODE)
             {
                 cout << "Total numbers quries: " << unique_query_per_node.size() << " are more than we support per node." << endl;
                 return -1;
             }
             unique_query_per_node.clear();         
             
        }
 
   }

    //consider all the query running 
	memset((void*)runningQueryList, 1, sizeof(runningQueryList));
	
     /*This map contain allocated resources in
       the below example format:- 
       {{1, {{"Obj1", 2}, {"Obj2", 0}},
    	{2, {"Obj1", 2}}...
       }
     */
     for (auto ech_query : allocated_resources)
     {
         for (auto ech_resource: ech_query.second)
	     {
			 //usedResourceCnt += ech_resource.second;
			 //allocated_resources[ech_query.first][]
			 /* calculate the avilable resources */
		 avilableResources[ech_resource.first] = max_resources[ech_resource.first] - ech_resource.second;
         //usedResourceCnt = 0;
		 }
		 
		 
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
		//for (int query_idx = 0; query_idx < MAX_QUERY_CNT; query_idx++)
			
		/* {{1, {"obj1", 1}}, {1, "obj2", 0} ... }*/	
		for (auto ech_query :need_resources)
		{
              /* query is excuted, check for next active query */
		      if (!runningQueryList[ech_query.first - 1])
			  	  continue;

			  bIsUnsafeSate = false;
			  kill_query_no = -1;
	        
	          queryExec = true;
			  cur_running_query = ech_query.first;
			  
	          //for (int resource_cnt = 0; resource_cnt < MAX_RESOURCES_CNT; resource_cnt++)
			  for (auto ech_resource : ech_query.second)
	          {
	               /* Avilable resources are less than Remaining resource. */
				   if (avilableResources[ech_resource.first] < ech_resource.second)
	               {
	                   queryExec = false;
					   bIsUnsafeSate = true;
					   kill_query_no = ech_query.first; 
	                   break;
	               }
	          }

			  if (queryExec)
			  {
	              printf("Executing the Query-%d\n", cur_running_query);
				  for (auto ech_resource: ech_query.second)
				  {
					  avilableResources[ech_resource.first] += 
					    allocated_resources[ech_query.first][ech_resource.first];
					    
				  }
				  running_query_cnt -= 1;
				  runningQueryList[ech_query.first - 1] = 0; // it means it's executed
				  
			  }
	    
		}

		if (bIsUnsafeSate)
		{
           printf("DeadLock detected in the system..\n");
           printf("kill the query:-%d\n", kill_query_no);
		   break;
		}
    }

    if (!bIsUnsafeSate)
	   printf("System is not in deadlock condition.\n");
	
    return 0;
}


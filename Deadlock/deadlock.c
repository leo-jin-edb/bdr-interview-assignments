#include <stdio.h>
#include <stdlib.h>

int main(){

int no_process[9], no_machines, i, m1_p, m2_p, p_id, m_no;

printf("Enter the number of machines present \n");
scanf("%d",&no_machines);

for(i = 0; i < no_machines; i++){
        printf("Enter total number of processes in machine%d\n", i+1);
        scanf("%d",&no_process[i]); }

for(i = 0; i < no_machines; i++){
        printf("Total number of processes in eacH machine %d are %d \n", i+1, no_process[i]);}

printf("Enter the machine no. and process id for which deadlock detection should be initiated\n");
scanf("%d %d",&m_no, &p_id);

printf("Enter the processes of two different machines connected with requested edge\n");
scanf("%d %d", &m1_p, &m2_p);

printf("Probe message is (%d, %d, %d)", p_id, m1_p, m2_p);
if(p_id == m2_p)
        printf("Deadlock detected\n");
else
        printf("Deadlock not present\n");

return 0;

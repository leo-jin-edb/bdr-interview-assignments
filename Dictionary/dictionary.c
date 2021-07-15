#include <stdio.h>
#include <stdlib.h>

#define HASHTABLE_SIZE 23

typedef struct node{
	char* value;
	struct node* nxt;
}node_t;

void insert_node(node_t** head_ref, char* str){
	node_t* new_node = malloc(sizeof(node_t*));
	new_node->value = str;
	new_node->nxt = NULL;

	if(*head_ref == NULL){
		*head_ref = new_node;
	} 
	else {
		node_t* current = *head_ref;
		while(current->nxt != NULL){
			if(strcmp(current->value, str) != 0){
			current = current->nxt;
			}
			else{
			printf("Key: %s is already exists\n", current->value);
			exit(1); }
		}
		if(strcmp(current->value, str) == 0){
			printf("Key: %s is already exists\n", current->value);
			exit(1);
			}
		else
			current->nxt = new_node;
	}
}

void delete_node(node_t** head_ref, char *str){
	if(*head_ref == NULL){
		printf("Table is empty\n");
		exit(1);
	} else {
		node_t* current = *head_ref;
		node_t* prev = NULL;
		while(strcmp(current->value, str) != 0){
			prev = current;
			current = current->nxt;
		}
		printf("%s Key found \n", current->value);
		if(current->nxt == NULL){
			node_t* temp = current;
			current = prev;
			current->nxt = NULL;
			free(temp);
		}

		else{	
			node_t* temp = current->nxt;
			current->value = temp->value;
			current->nxt = temp->nxt;
			free(temp);
		}
	}
	printf("%s value deleted \n", str); 
		
}

void search_node(node_t** head_ref, char* str){
	node_t* temp = *head_ref;
	while(strcmp(temp->value, str) != 0){
		temp = temp->nxt;	
	}
	printf("Key:%s found \n", temp->value);
}

int hash_fun(char* str){
	int i, sum = 0;
	for(i = 0; i < strlen(str); i++){
		 sum = sum + str[i];
	}
	return sum % HASHTABLE_SIZE ;
}

node_t** create_HT(){
	node_t** HashTable = calloc(HASHTABLE_SIZE, sizeof(node_t*));
	return HashTable;
}

void insert_HT(node_t** HashTable, char* str){
	int index = hash_fun(str);

	if(HashTable[index] == NULL){
		HashTable[index] = malloc(sizeof(node_t*));
		node_t* head = NULL;
		insert_node(&head, str); 		
		
		HashTable[index] = head;
		}
	else {
		node_t* head = HashTable[index];
		insert_node(&head, str);
		HashTable[index] = head;
	}
}

void delete_HT(node_t** HashTable, char* str){
	int index = hash_fun(str);
	if(HashTable[index] == NULL){
		printf("Requested key is not available to delete \n");
		exit(1);
	}
	else {
		node_t* head =  HashTable[index];
		delete_node(&head, str);
		HashTable[index] = head;
	}
}

void search_HT(node_t** HashTable, char* str){
	int index = hash_fun(str);
	if(HashTable[index] == NULL){
		printf("Requested Key: %s not found\n", str);
		exit(1);	
		}
	else {
		if(strcmp(HashTable[index], str) == 0){
			printf("Requested Key: %s found \n", str);
			return;
			}
		else {
			node_t* head =  HashTable[index];
			search_node(&head, str);
			}
	}
}

void display_table(node_t** table){
	int i;
	for(i = 0; i < HASHTABLE_SIZE; i++){
		node_t* head = table[i];
		printf("%d: ",  i);
	
	if(head == NULL)
		printf("NULL");
	else {
		node_t* current = head;
		while(current != NULL){
			printf("%s ", current->value);
			current = current->nxt;
		}

	}
	printf("\n");
	}
}

void free_table(node_t** table) {
    node_t* temp = *table;
    while(temp != NULL){
	node_t* temp1 = *table;
	temp = temp->nxt;
	temp1 = temp;
	free(temp1);
	}
  free(table);
}

int main(int argc, char** argv){
	int choice;
	node_t** table  = create_HT();
	insert_HT(table, "candidate");
	insert_HT(table, "eu");
	insert_HT(table, "20");
	insert_HT(table, "100");
	insert_HT(table, "js");
	insert_HT(table, "sj");
	

	if(argc < 2){
		printf("Insuffient Input\n");
		exit(1);
	}
	
	if(strcmp(argv[1], "insert") == 0)
		choice = 1;	
	else if(strcmp(argv[1], "delete") == 0)
		choice = 2;
	else if(strcmp(argv[1], "search") == 0)
		choice = 3;
	else if(strcmp(argv[1], "display") == 0)
		choice = 4;
	else
		printf("Invalid Input\n");

	switch(choice){

	case 1: 
		insert_HT(table, argv[2]);
		display_table(table);
		break;

	case 2:
		delete_HT(table, argv[2]);
		display_table(table);
		break;

	case 3:
		search_HT(table, argv[2]);
		display_table(table);
		break;

	case 4:
		display_table(table);
		break;

	default:
		printf("In default\n");
		break;
	}
	free_table(table);

return 0;
}

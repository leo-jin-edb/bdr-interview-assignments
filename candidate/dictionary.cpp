#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mutex>
using namespace std;

#define WORD_SIZE (sizeof(char)+255)
#define SHM_SIZE (sizeof(int) + (WORD_SIZE * 1000000))
#define SHM_NAME "dictionary"

#define INSERT 1
#define SEARCH 2
#define DELETE 3
#define EXIT   4

#define TRUE '1'
#define FALSE '0'

int count = 0;
char *headShmPtr = NULL;
char word[255];
char wordDeleted = FALSE;
char* tempPtr = NULL;

mutex ilock;
mutex slock;
mutex dlock;

bool searchDictionary(char* head, char* str);
void addData2Shm(char* head, char* str);
int createSharedMemory();
int getCmdArgs(int argc, char* argv[]);
bool remWordFromDict(char* head, char* str);

void addData2Shm(char* head, char* str)
{
    char* sPtr = head;
    int tempCount = count;
    bool slotExists = false;
    if(!searchDictionary(sPtr, str))
    {
        lock_guard<mutex> isrt(ilock);
        wordDeleted =  FALSE;
        sPtr = head;
        while(tempCount > 0)
        {
            if(*(sPtr+255) == TRUE)
            {
                slotExists = true;
                break;
            }
            sPtr = sPtr + WORD_SIZE;
            tempCount --;

        }

        if(slotExists)
        {
            head = sPtr;
        }
        else
        {
            head = head + (count*WORD_SIZE);
        }

        sprintf(headShmPtr,"%d",++count);
        memset(head, 0x00, WORD_SIZE);
        memcpy(head, str,strlen(str));
        *(head+strlen(str)) = '\0';
        *(head+255) = FALSE;
        cout << "Word " << str << " inserted in dictionary." << endl;
    }
}

bool searchDictionary(char* head, char* str)
{
    int tempCount = count;
    tempPtr = head;
    bool found = false;

    lock_guard<mutex> srch(slock);
    while(tempCount > 0)
    {
        if( memcmp(tempPtr, str, strlen(str)) != 0 )
        {
            tempPtr = tempPtr + WORD_SIZE;
        }
        else
        {
            wordDeleted = *(tempPtr+255);
            if(wordDeleted == TRUE)
            {
                break;
            }

            cout << "Word " << str << " found in dictionary." << endl;
            found = true;
            break;
        }
        tempCount --;
    }

    if(!found)
    {
        cout << "Word " << str << " not found in dictionary." << endl;
    }

    return found;
}

bool remWordFromDict(char* head, char* str)
{
    bool deleted = false;
    if(searchDictionary(head, str))
    {
        head = tempPtr;
        lock_guard<mutex> dlte(dlock);

        memset(head, 0x00, WORD_SIZE);
        *(head+255) = TRUE;
        cout << "Word " << str << " deleted from dictionary." << endl;
        deleted = true;
    }
    else
    {
        cout << "Word " << str << " not found in dictionary." << endl;
    }
    return deleted;
}

int createSharedMemory()
{
    // ftok to generate unique key
    key_t key = ftok(SHM_NAME,811);

    // shmget returns an identifier in shmid
    int shmid = shmget(key,SHM_SIZE,0666|IPC_CREAT);
    if (shmid < 0)
    {
        printf("shmget error\n");
        exit(1);
    }

    // shmat to attach to shared memory
    headShmPtr = (char*) shmat(shmid,(void*)0,0);

    return shmid;
}

int getCmdArgs(int argc, char* argv[])
{
    int retCode = 0;
    if((argc > 3) || (argc < 3))
    {
        cout << "Too many or too less arguments." << endl;
        cout << "dictionary {insert|search|delete|exit} <word>" << endl;
    }
    else
    {
        if(!memcmp(argv[1], "insert", 6))
        {
            retCode = 1;
        }
        else if(!memcmp(argv[1], "search", 6))
        {
            retCode = 2;
        }
        else if(!memcmp(argv[1], "delete", 6))
        {
            retCode = 3;
        }
        else if(!memcmp(argv[1], "exit", 4))
        {
            retCode = 4;
        }
        else
        {
            cout << "Invalid command." << endl;
        }
    }
    if(retCode)
    {
        memset(word, 0x00, 255);
        memcpy(word, argv[2],strlen(argv[2]));
    }
    return retCode;
}

int main(int argc, char *argv[])
{
    int argsRc = 0;
    int shmid = 0;
    char *shmPtr = NULL;

    shmid = createSharedMemory();
    shmPtr = headShmPtr;

    count = atoi(shmPtr);
    shmPtr = shmPtr+sizeof(int);


    argsRc = getCmdArgs(argc, argv);
    switch(argsRc)
    {
        case INSERT:
            addData2Shm(shmPtr, word);
            break;
        case SEARCH:
            searchDictionary(shmPtr, word);
            break;
        case DELETE:
            remWordFromDict(shmPtr, word);
            break;
        case EXIT:
            //detach from shared memory
            shmdt(headShmPtr);

            // destroy the shared memory
            shmctl(shmid,IPC_RMID,NULL);
            break;
        default:
            break;
    }

    return 0;
}

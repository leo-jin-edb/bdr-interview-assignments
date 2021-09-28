#include <stdio.h>
#include <iostream>
#include <string>
#include <string.h>
#include <algorithm>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

using namespace std;

#define MAX_CHILDREN_PER_NODE  26 // we are assuming only 26 alphabets
#define MAX_SUPPORT_WORDS      1000000 // right now total word we support is 1M
typedef struct MapHdr
{
    int maxWordsCnt;
    int curWordsCnt;
    pthread_mutex_t lock;

}MapHdr;

typedef struct DictNode
{
  DictNode *children[MAX_CHILDREN_PER_NODE];
  bool     bWordEnd;

}DictNode;

typedef enum operationType
{
    DO_INSERT = 0x0001,
    DO_SRCH = 0x0002,
    DO_DELETE = 0x0004,
    DO_NOOP = 0xFFFF
}eOperationType;



/* create Dictionary Node */
DictNode* createNode(void** pBasePtr)
{
   MapHdr *pMapHdr = (MapHdr*)(*pBasePtr);
   DictNode *rootPtr = (DictNode*)((char*)pBasePtr + sizeof(MapHdr));

   if (pMapHdr->curWordsCnt >= pMapHdr->maxWordsCnt)
   {
       fprintf(stderr, "dictionary only support maximum: [%d] words\n",  pMapHdr->maxWordsCnt);
       return NULL;
   }
   pthread_mutex_lock(&(pMapHdr->lock));
   pMapHdr->curWordsCnt += 1;
   DictNode *newNode = &rootPtr[pMapHdr->curWordsCnt];
   pthread_mutex_unlock(&(pMapHdr->lock));

#if 0
   DictNode *newNode = (DictNode*) malloc(sizeof(DictNode));
   if (NULL == newNode)
   {
       fprintf(stderr, "failed to allocate memory for trienode.\n");
       return NULL;
   }
#endif
   
   for (int idx = 0; idx < MAX_CHILDREN_PER_NODE; idx++)
   {
        newNode->children[idx] = NULL;
   }
   newNode->bWordEnd = false; 
   return newNode; 
}

/*insert word into dictionary */
bool insertWordIntoDict(void *pBasePtr, const char *word)
{

   MapHdr *pMapHdr = (MapHdr*)pBasePtr;
   DictNode *rootNode = (DictNode*)((char*)pBasePtr + sizeof(MapHdr));
   DictNode *pCurrNode = rootNode;

   /* inserting  a word but rootnode of dict is NULL*/
  /* if (!pCurrNode)
   {
       pCurrNode = createNode();
       if (NULL == pCurrNode)
       {
           bCheck = false;
           return NULL; 
       }
       rootNode  = pCurrNode; // make this rootnode               
   }*/
   
  

   /* process each character from the input word and add to dict */
   for (int idx = 0; idx < strlen(word); idx++)
   {
       if (NULL == pCurrNode->children[word[idx] - 'a'])
       {
            pCurrNode->children[word[idx] - 'a'] = createNode(&pBasePtr);
            if (!pCurrNode->children[word[idx] - 'a'])
            {
                fprintf(stderr, "failed to crate intermediatry node for dict.\n");
                return false;       
            }
       }
       pCurrNode = pCurrNode->children[word[idx] - 'a'];
   } 
   pCurrNode->bWordEnd = true;
   return true;
}


/*search element from dict */
bool findWordInDict(void *pBasePtr, const char* word)
{
   MapHdr *pMapHdr = (MapHdr*)pBasePtr;
   DictNode *rootNode = (DictNode*)((char*)pBasePtr + sizeof(MapHdr));
   DictNode *pCurrNode = rootNode;
   int idx = 0;
  // DictNode *pCurrNode = NULL;

   if (!rootNode)
   {
      fprintf(stderr, "dictionary context is not created.\n");
      return false;
   }

   pCurrNode = rootNode;  
   while (idx < strlen(word))
   {
        pCurrNode = pCurrNode->children[word[idx] - 'a'];
        if (!pCurrNode)
           return false;
        idx += 1;
   } 

   return (pCurrNode->bWordEnd == true);

}
/*
 * check there is child exist for the
 * input node.
 *
 */

bool hasChildren(DictNode *rootNode)
{
   for(int idx = 0; idx < MAX_CHILDREN_PER_NODE; idx++)
   {
        if (rootNode->children[idx])
           return true; 
   }
   return false;
}   

/*Delete Word from Dictonary 
 *
 *ex:- word:-1 <abc>
 *           2 <ab>
 *           3. <abcd>
 * */
bool DeleteWordFromDict(DictNode** rootNode, const char* word,
                       int cur_idx, bool &bFoundWord)
{
    bool ret = true;

   
     if (!*rootNode)
     {
        fprintf(stderr, "Delete word: [%s] doesn't exist in dictionary.\n\n", word);
        return false;
     }

    if ((*rootNode) 
      && (word[cur_idx] == '\0'))
       return true;

    /*come to end of the word */
    if (word[cur_idx]) //do recursivly still we reach end of the word
    {
        rootNode = &((*rootNode)->children[word[cur_idx] - 'a']);
        ret = DeleteWordFromDict(rootNode, word, cur_idx + 1, bFoundWord);       
        if (!ret)
        {
           return ret;
        }
        //rootNode->children[word[cur_idx] - 'a'] = pChildNode;
   
        if (!bFoundWord)
        {
           bFoundWord = true;
           if (false == ((*rootNode)->bWordEnd))
           {
              fprintf(stderr, "Delete word: [%s] doesn't exist in dictionary.\n\n", word);
              return false;
           }
           (*rootNode)->bWordEnd = false;
           
        }

        if (!hasChildren(*rootNode))
        {
           //free(*rootNode);
           memset((*rootNode), 0, sizeof(DictNode));
           *rootNode = NULL;
           //pChildNode = NULL;
           //rootNode->children[word[cur_idx] - 'a'] = NULL;
        }
    } 
   return true;
}

/*intialize the shared memory during process bootup */
void* initalizeMapCtx(const char* filepath, int maxWrdsSupport)
{
#if 0
   int fd = open(filepath, O_CREAT |O_RDWR |O_SYNC, 0666);
   if (fd < 0)
   {
       perror("failed to open the file");
       return NULL;
   }
#endif
   int total_size = ((sizeof(DictNode) * maxWrdsSupport) + sizeof(MapHdr));

   void *pBaseAddr = mmap(NULL, total_size, PROT_READ|PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
   if(pBaseAddr == MAP_FAILED)
   {
        perror("Mapping Failed\n");
        return NULL;
    }
#if 0
    if (ftruncate(fd, total_size) < 0)
    {
        perror("failed to truncate the file.\n");
        return NULL;
    }

    close(fd);
#endif
    memset(pBaseAddr, 0, total_size);

    /* update share meemory header */
    MapHdr stMapHdr;
    stMapHdr.maxWordsCnt = maxWrdsSupport;
    stMapHdr.curWordsCnt = 0;
    pthread_mutex_init(&(stMapHdr.lock), NULL);
    memcpy(pBaseAddr, &stMapHdr, sizeof(MapHdr));
    return pBaseAddr;
}

#define MAX_INPUT_STR_LEN 200
#define VALIDATE_INPUT_WORD(pInputWord) \
   if (!pInputWord)\
        {\
            fprintf(stderr, "input dictinary word is not provided.\n\n");\
            fprintf(stdout, "please povide: <insert/delete/search/quit> <word>\n");\
            break;\
        }


int main(int argc, char** argv)
{
   eOperationType opType = DO_NOOP;
   char inputStr[MAX_INPUT_STR_LEN] = {0};
   char *sOpration = NULL;
   char *pInputWord = NULL;
   DictNode* pRootNode = NULL;
   bool      bWordEnd = false;
   DictNode *pChildNode = NULL;

   void* pBasePtr = initalizeMapCtx("dict_map", MAX_SUPPORT_WORDS);
   MapHdr *pMapHdr = (MapHdr*)pBasePtr;
   //pMapHdr->maxWordsCnt = MAX_SUPPORT_WORDS;
   //pMapHdr->curWordsCnt = 0; 

   if (pMapHdr->curWordsCnt == 0)
   {
      pRootNode = createNode(&pBasePtr);  
      if (!pRootNode)
      {
         fprintf(stderr, "Failed to allocate memory for dictionary node.\n");
         return -1;
      }
   }

  fprintf(stdout, "please povide: <insert/delete/search/quit> <word>\n");
  while (true)
  {
 
     fgets(inputStr, MAX_INPUT_STR_LEN, stdin);
     sOpration = strtok(inputStr, " ");
     pInputWord = strtok(NULL, " ");

     if (pInputWord && pInputWord[strlen(pInputWord) - 1] == '\n')
     {
         pInputWord[strlen(pInputWord) - 1] = '\0';
     }
 
     if (!strncmp(sOpration, "insert", min(strlen("insert"), strlen(sOpration)))) 
     {
        VALIDATE_INPUT_WORD(pInputWord);

        pthread_mutex_lock(&(pMapHdr->lock));        
        if (insertWordIntoDict(pBasePtr, pInputWord))
            fprintf(stdout, "[%s]: word is inserted successfully.\n\n", pInputWord);
        else
            fprintf(stderr, "[%s]: word insert failed.\n\n", pInputWord);
        pthread_mutex_unlock(&(pMapHdr->lock));

     }
     else if (!strncmp("delete", sOpration, min(strlen("delete"), strlen(sOpration))))
     {
         bWordEnd = false;
         VALIDATE_INPUT_WORD(pInputWord);
         pRootNode = (DictNode*)((char*)pBasePtr + sizeof(MapHdr));
      
        pthread_mutex_lock(&(pMapHdr->lock));
        if (DeleteWordFromDict(&pRootNode, pInputWord, 0, bWordEnd))
            fprintf(stdout, "word: [%s] is deleted successfully.\n\n", pInputWord);
        else 
            fprintf(stdout, "word: [%s] deletion failed.\n\n", pInputWord);
        pthread_mutex_unlock(&(pMapHdr->lock));
     }
     else if (!strncmp("search", sOpration, min(strlen("search"), strlen(sOpration))))
     {
        
        VALIDATE_INPUT_WORD(pInputWord);
         
        pthread_mutex_lock(&(pMapHdr->lock));
        if (findWordInDict(pBasePtr, pInputWord))
        {
           fprintf(stdout, "[%s]: word found in dictionary.\n\n", pInputWord);
        }
        else
        {
           fprintf(stdout, "[%s]: word dosnot exist in dictionary.\n\n", pInputWord);
        }
        pthread_mutex_unlock(&(pMapHdr->lock));

    }
    else if (!strncmp("quit", sOpration, min(strlen("quit"), strlen(sOpration))))
    {
        fprintf(stdout, "Quiting from dictionary.\n\n");
        break;
    } 
    else
    {
      fprintf(stderr, "wrong operation is provided\n\n");
      fprintf(stdout, "please povide: <insert/delete/search/quit> <word>\n\n");
    }
   }

   if (pBasePtr)
   {
      int total_size = ((sizeof(DictNode) * MAX_SUPPORT_WORDS) + sizeof(MapHdr));
      pthread_mutex_destroy(&(pMapHdr->lock));
      (void*)munmap(pBasePtr, total_size);
   }
   return 0;

}

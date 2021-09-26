#include <stdio.h>
#include <iostream>
#include <string>
#include <string.h>
#include <algorithm>

using namespace std;

#define MAX_CHILDREN_PER_NODE  26 // we are assuming only 26 alphabets

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



/* create trienode */
DictNode* createNode()
{

   DictNode *newNode = (DictNode*) malloc(sizeof(DictNode));
   if (NULL == newNode)
   {
       fprintf(stderr, "failed to allocate memory for trienode.\n");
       return NULL;
   }
   
   for (int idx = 0; idx < MAX_CHILDREN_PER_NODE; idx++)
   {
        newNode->children[idx] = NULL;
   }
   newNode->bWordEnd = false; 
   return newNode; 
}

/*insert word into dictionary */
bool insertWordIntoDict(DictNode *rootNode, const char *word)
{

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
            pCurrNode->children[word[idx] - 'a'] = createNode();
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
bool findWordInDict(DictNode *rootNode, const char* word)
{
   int idx = 0;
   DictNode *pCurrNode = NULL;

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
           free(*rootNode);
           *rootNode = NULL;
           //pChildNode = NULL;
           //rootNode->children[word[cur_idx] - 'a'] = NULL;
        }
    } 
   return true;
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

  pRootNode = createNode();  
  if (!pRootNode)
  {
     fprintf(stderr, "Failed to allocate memory for dictionary node.\n");
     return -1;
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
        if (insertWordIntoDict(pRootNode, pInputWord))
            fprintf(stdout, "[%s]: word is inserted successfully.\n\n", pInputWord);
        else
            fprintf(stderr, "[%s]: word insert failed.\n\n", pInputWord);
     }
     else if (!strncmp("delete", sOpration, min(strlen("delete"), strlen(sOpration))))
     {
         bWordEnd = false;
         VALIDATE_INPUT_WORD(pInputWord);
        
        if (DeleteWordFromDict(&pRootNode, pInputWord, 0, bWordEnd))
            fprintf(stdout, "word: [%s] is deleted successfully.\n\n", pInputWord);
        else 
            fprintf(stdout, "word: [%s] deletion failed.\n\n", pInputWord);
     }
     else if (!strncmp("search", sOpration, min(strlen("search"), strlen(sOpration))))
     {
        
        VALIDATE_INPUT_WORD(pInputWord);
        if (findWordInDict(pRootNode, pInputWord))
        {
           fprintf(stdout, "[%s]: word found in dictionary.\n\n", pInputWord);
        }
        else
        {
           fprintf(stdout, "[%s]: word dosnot exist in dictionary.\n\n", pInputWord);
        }

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
   return 0;

}

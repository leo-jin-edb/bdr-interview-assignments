#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "dictionary.h"
#include "shedmemory.h"

int main(int argc , char* argv[])
{
    int opt;
    bool removeMem = false;
    char* word;
    bool alreadyExist = false;
    ShedMemory shm(sizeof(Dictionary));
    Dictionary* dict = 0;
    char* ptr =  shm.GetShedMemory(alreadyExist);
    if(!alreadyExist)
    {
        dict = new(ptr) Dictionary();
    } else {
        dict = (Dictionary*) ptr;
    }
    dict->Init(alreadyExist);
    
    while ((opt = getopt(argc, argv, "i:d:s:r")) != -1) {
           switch (opt) {
           case 'd':
               dict->deleteword(optarg);
               break;
           case 's':
               dict->searchword(optarg);
               break;
           case 'i':
               dict->insertword(optarg);
               break;
           case 'r':
               removeMem = true;
               shm.ReleaseShedMemory();
               break;
           default: /* '?' */
               fprintf(stderr, "Usage: %s [-i word] [-d word] [-s word] [-r]\n",
                       argv[0]);
               exit(EXIT_FAILURE);
           }
       }
    if(!removeMem)
        dict->Print();
    return 0;

}

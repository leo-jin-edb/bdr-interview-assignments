#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "dictionary.h"
#include "shedmemory.h"

void usage()
{
     fprintf(stderr, "\nUsage: ./dict [-i word] [-d word] [-s word] [-r]\n");
     fprintf(stderr, "-i word : insert word in dictionary\n");
     fprintf(stderr, "-d word : delete word in dictionary\n");
     fprintf(stderr, "-s word : search word in dictionary\n");
     fprintf(stderr, "-r : erase shared memory\n");
     fprintf(stderr, "-p : Print dictionary\n");
     exit(EXIT_FAILURE);

}

int main(int argc , char* argv[])
{
    int opt;
    bool removeMem = false;
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
    
    while ((opt = getopt(argc, argv, "i:d:s:rp")) != -1) {
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
           case 'p':
               dict->Print();
               break;
           default: /* '?' */
               usage();
           }
       }
    return 0;
}

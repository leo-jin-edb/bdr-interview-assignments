#ifndef CONFIG_H
#define CONFIG_H

#include "Os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define DEFAULT_CONFIG_FILE "word.conf"


class ConfigValues
{
    public:
     
    int shmKey;
    

    int mutexSecs;
    int mutexUSecs;
    int mutexRetries;
	
    ConfigValues()
    {
        shmKey = 2222;
 
	mutexSecs=0;
        mutexUSecs=10;
        mutexRetries = 10;
    }
};

class Config
{
    ConfigValues cVal;
    bool isLoaded;
    
    int readLine(FILE *fp, char * buffer);
    int storeKeyVal(char *key, char *val);
    int validateValues();

    public:
    
    Config() { isLoaded = false; }
    int readAllValues(const char *filename);
    void print();
   
    inline int getShmKey() { return cVal.shmKey; }
	
    inline int getMutexSecs() { return cVal.mutexSecs; }
    inline int getMutexUSecs() { return cVal.mutexUSecs; }
    inline int getMutexRetries() { return cVal.mutexRetries; }
};

class Conf
{
    public:
    static Config config;
};

#endif

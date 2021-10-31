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
    
    int lockSecs;
    int lockUSecs;
    int lockRetries;


    int mutexSecs;
    int mutexUSecs;
    int mutexRetries;
	
    ConfigValues()
    {
        shmKey = 2222;
 
        lockSecs =0;
        lockUSecs = 10;
        lockRetries = 10;
		
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
	
    inline int getLockSecs() { return cVal.lockSecs; }
    inline int getLockUSecs() { return cVal.lockUSecs; }
    inline int getLockRetries() { return cVal.lockRetries; }
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

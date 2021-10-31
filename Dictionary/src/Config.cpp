#include "Config.h"
//TODO: Documentation
Config Conf::config;

int Config::readLine(FILE *fp, char * buffer)
{
  char c =0;
  int count =0;
  while (true)
  {
      c = fgetc(fp);
      if (c == '\n') break;
      if (c == EOF) return EOF;
      buffer[count++] = c;
  }
  return count;
}

int Config::storeKeyVal(char *key, char *value)
{
    if (strcasestr(key, "SHM_KEY") != NULL)
           { cVal.shmKey = atoi(value);  }
    else if (strcasestr(key, "MUTEX_TIMEOUT_SECS") != NULL)
           { cVal.mutexSecs = atoi(value);  }
    else if (strcasestr(key, "MUTEX_TIMEOUT_USECS") != NULL)
           { cVal.mutexUSecs = atoi(value);  }
    else if (strcasestr(key, "MUTEX_TIMEOUT_RETRIES") != NULL)
           { cVal.mutexRetries = atoi(value);  }
    else if (strcasestr(key, "LOCK_TIMEOUT_SECS") != NULL)
           { cVal.lockSecs = atoi(value);  }
    else if (strcasestr(key, "LOCK_TIMEOUT_USECS") != NULL)
           { cVal.lockUSecs = atoi(value);  }
    else if (strcasestr(key, "LOCK_TIMEOUT_RETRIES") != NULL)
           { cVal.lockRetries = atoi(value);  }
    else  return 1;
    return 0;
}
int Config::validateValues()
{
    
    if (cVal.shmKey < 10 || cVal.shmKey > 8192)
    {
        printf ("SHM_KEY should be >= 10 and <= 8192\n");
        return 1;
    }
    
    if (cVal.lockSecs < 0 || cVal.lockSecs > 360)
    {
        printf (  "LOCK_TIMEOUT_SECS should be >= 0 and <= 360\n");
        return 1;
    }
    if (cVal.lockUSecs < 0 || cVal.lockUSecs > 1000000)
    {
        printf (  "LOCK_TIMEOUT_USECS should be >= 0 and <= 1000000\n");
        return 1;
    }
    if (cVal.lockRetries < 0 || cVal.lockRetries > 100)
    {
        printf (  "LOCK_TIMEOUT_RETRY should be >= 0 and <= 100\n");
        return 1;
    }
    if (cVal.mutexSecs < 0 || cVal.mutexSecs > 360)
    {
        printf (  "MUTEX_TIMEOUT_SECS should be >= 0 and <= 360\n");
        return 1;
    }
    if (cVal.mutexUSecs < 0 || cVal.mutexUSecs > 1000000)
    {
        printf (  "MUTEX_TIMEOUT_USECS should be >= 0 and <= 1000000\n");
        return 1;
    }
    if (cVal.mutexRetries < 0 || cVal.mutexRetries > 100)
    {
        printf (  "MUTEX_TIMEOUT_RETRY should be >= 0 and <= 100\n");
        return 1;
    }
    return 0;
}

int Config::readAllValues(const char *fileName)
{
    if (isLoaded) 
    {
    	return 0;
    }
    FILE *fp = NULL;
    if (fileName == NULL || 0 == strcmp(fileName, "")) 
    	fileName = DEFAULT_CONFIG_FILE;
    fp = fopen(fileName,"r");
    if( fp == NULL ) {
        printf ("Invalid path/filename\n");
	return 1;
    }

    int hasData = 1;
    char buffer[1024];
    char key[1024];
    char value[1024];
    while (hasData)
    {
        memset(buffer, 0, 1024);
        //int ret = fscanf(fp,"%s\r",buffer);
        int ret = readLine(fp, buffer);
        if (ret == EOF) break;
        bool isComment= false;
        int posEqual =0;
        for (int i = 0; i <1024; i++)
        {
              if (buffer[i] == '=' ) posEqual=i;
              else if (buffer[i] == '#' ) { isComment = true; break; }
              else if (buffer[i] == '\n') { break; }
              else if (buffer[i] == '\0') { break; }
        }
      if (isComment) continue;
      if (!posEqual) continue;
      strncpy(key, buffer, posEqual);
      key[posEqual] = '\0';
      posEqual++;
      strcpy(value, &buffer[posEqual]);
      storeKeyVal(key, value);
    }
    fclose(fp);
    if (validateValues())
    {
        return 1;
    }
    isLoaded = true;
    return 0;
}

void Config::print()
{
    printf("ConfigValues\n");
    printf(" getShmKey %d\n", getShmKey());
    printf(" getLockSecs %d\n", getLockSecs());
    printf(" getLockUSecs %d\n", getLockUSecs());
    printf(" getLockRetries %d\n", getLockRetries());
    printf(" getMutexSecs %d\n", getMutexSecs());
    printf(" getMutexUSecs %d\n", getMutexUSecs());
    printf(" getMutexRetries %d\n", getMutexRetries());
}

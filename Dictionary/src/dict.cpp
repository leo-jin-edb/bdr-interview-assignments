#include "Common.h"
#include "WordManager.h"
const char *VERSION = "0.0.1";

/**
 * printUsage: print the application version and usage
 */
void printUsage()
{
	printf("\ndict version : %s\n",VERSION);
	printf("\nUsage:  dict {insert|search|delete} <word>\n\n");
}

int main(int argc, char *argv[])
{

        InputOption opt = END_OPT;

        /* Read config from default path*/
        Conf::config.readAllValues (NULL);	

	if( argc != TOTAL_ARGS )
	{
		printf("Invalid Arguments\n");
                printUsage(); 
		return (1);
	}

	/* Get operation to perform */
	if(0 == strcmp(argv[1], "search"))
        {
		opt = SEARCH_OPT;
	}
	else if(0 == strcmp(argv[1], "insert"))
	{
		opt = INSERT_OPT;
	} 
	else if(0 == strcmp(argv[1], "delete"))
	{
		opt = DELETE_OPT;
	}

	if( END_OPT == opt )
	{
		printf("Invalid Arguments\n");
                printUsage(); 
		return (1);
	}

	/* Get Word */
	if( strlen(argv[2]) > WORD_LENGTH_LENGTH-1 )
	{
		printf("Word is too long. bigger than %d\n", WORD_LENGTH_LENGTH-1 );
                printUsage(); 
		return (1);
	}
        
        WordManager wordMrg;
	RetVal ret = OK;
	/* Perform requested operation */
	switch( opt )
	{
		case SEARCH_OPT:
                        ret = wordMrg.searchWord(argv[2]);
                        if (ret != OK)
                        {
				printf ("Not Found\n");
                        }
			else
			{
				printf ("Found\n");
			}
			break;

		case INSERT_OPT:
                        ret = wordMrg.insertWord(argv[2]);
			if (OK == ret)
			{
				printf("Inserted successfully\n");
			}
			break;

		case DELETE_OPT:
                        ret = wordMrg.deleteWord(argv[2]);
			if (OK == ret)
			{
				printf("Deleted successfully\n");
			}
			break;

		default:
			break;
	}

	return 0;
}


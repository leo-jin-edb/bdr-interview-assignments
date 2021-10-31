#ifndef WORD_MANAGER_H
#define WORD_MANAGER_H
#include "MemoryManager.h" 

class WordManager
{
	MemoryManager m_mManager;
	
	public:
	WordManager();
	~WordManager();
	RetVal insertWord(char *word);
	RetVal deleteWord(char *word);
	RetVal searchWord(char *word);
};


#endif

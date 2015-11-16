#ifndef PRIMO_LIB_H
#define PRIMO_LIB_H
#include<stdio.h>
#include<stdarg.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<limits.h>
//#include<float.h>

class Memory;
class Generic{};


//ENUM for different types of memories (they are freed/deleted accordingly). 
//Objects are cast to Generic (Everything mine, no existing structures)
typedef enum
{
	NONE, MALLOC, NEW, NEWARR
} MEMTYPE;

extern int debuglevel;
extern Memory* memory;

void debug(int level, const char* format, ...);
long getfilesize(const char* filename);
//bool equalstrings(const char* s1, const char* s2);

class MemoryBlock
{
	public:
		void* p;
		MEMTYPE type;
		MemoryBlock(void* p, MEMTYPE type);
		~MemoryBlock();
};

class Memory
{
	private:
		int max;
		int* maxes;
		
	public:
		int* counts;
		MemoryBlock*** mems;

		Memory(int max);
		~Memory();
		void More(int bucket);
		bool Add(void* m, MEMTYPE type, bool exitonfail, const char* msgonfail);
		void FreeAll();
		void Free(void* m);
		void FreeThis();
		int GetHash(void* p);
};

#endif

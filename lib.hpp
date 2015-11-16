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

class MemoryBlock //holds one pointer
{
	public:
		void* p; //pointer to memory
		MEMTYPE type; //type of allocation method
		MemoryBlock(void* p, MEMTYPE type);
		~MemoryBlock();
};

class Memory
{
	private:
		int max; //number of buckets
		int* maxes; //maximum number of blocks in each bucket
		
	public:
		int* counts; //actual number of blocks in each bucket
		MemoryBlock*** mems; //hashmap of pointers to memory blocks

		Memory(int max);
		~Memory();
		void More(int bucket); //bucket resizing method
		bool Add(void* m, MEMTYPE type, bool exitonfail, const char* msgonfail); //memory block addition
		void FreeAll(); //all pointers deallocation
		void Free(void* m); //single pointer deallocation
		void FreeThis(); //all pointers deallocation (including this structure)
		int GetHash(void* p); //hash computation
};

#endif

#include "lib.hpp"

Generic::~Generic(){}

void debug(int level, const char* format, ...)
{
	//print info if wanted
	if(level<=debuglevel)
	{
		va_list args;
		va_start(args, format);
		fprintf(stdout, "D%d> ", level);
		vfprintf(stdout, format, args);
		fprintf(stdout, "\n");
		va_end(args);
	}
}

long getfilesize(const char* filename)
{
	//get size of file (not good for files 4GB+)
	FILE *f = fopen(filename, "rt");
	if(f==NULL)
	{
		printf("E: Unable to open file '%s'\n", filename);
		delete memory;
		return 0;
	}
	fseek(f, 0, SEEK_END);
	long size=ftell(f);
	if(fclose(f)==EOF)
	{
		printf("E: Unable to close file '%s'\n", filename);
		return 0;
	}	
	return size;
}

// - - - - - - - - - - - - - - - - - - - - - 

MemoryBlock::MemoryBlock(void* p, MEMTYPE type)
{
	this->p=p;
	this->type=type;
}

MemoryBlock::~MemoryBlock()
{
	//freeing memory based on alloc type
	if(type==MALLOC)
		free(p);
	else if(type==NEW)
		delete (Generic*)p;
	else if(type==NEWARR)
		delete [] (Generic*)p;
	else 
		debug(9, "Attempt to free memory with unknown type.");
}
// - - - - - - - - - - - - - - - - - - - - - 

Memory::Memory(int max)
{
	debug(9, "Creating memory structure.");
	this->max=max;
	//create hashmap
	mems = (MemoryBlock***)malloc(max*sizeof(MemoryBlock**));
	if(mems==NULL)
	{
		printf("E: Cannot allocate memory structure, exiting!\n");
		exit(1);
	}
	//create counts array
	counts=(int*)malloc(max*sizeof(int));
	if(counts==NULL)
	{
		printf("E: Cannot allocate memory counts structure. Exiting!");
		free(mems);
		exit(1);
	}
	//create maxes array
	maxes = (int*)malloc(max*sizeof(int));
	if(maxes==NULL)
	{
		printf("E: Cannot allocate memory maxes structure. Exiting!");
		free(mems);
		free(counts);
		exit(1);
	}
	//create buckets, initialize maxes and counts
	int maxesvalue=max/8;
	for(int i=0; i<max; i++)
	{
		maxes[i]=maxesvalue;
		counts[i]=0;
		mems[i]=(MemoryBlock**)malloc(maxesvalue*sizeof(MemoryBlock*));
		if(mems[i]==NULL)
		{
			printf("E: Cannot allocate memory bucket structure. Exiting!");
			for(int j=0; j<i; j++)
				free(mems[i]);
			free(maxes);
			free(counts);
			free(mems);
			exit(1);
		}
	}
}

Memory::~Memory()
{
	FreeThis();
}

void Memory::More(int bucket)
{
	debug(9, "Resizing memory bucket %d from %d to %d\n.", bucket, maxes[bucket], maxes[bucket]*2);
	maxes[bucket]*=2;
	//resize bucket
	MemoryBlock** tmpmems=(MemoryBlock**)realloc(mems[bucket], maxes[bucket]*sizeof(MemoryBlock*));
	if(tmpmems==NULL)
	{
		printf("E: Resizing memory failed. Exiting...\n");
		FreeThis();
		exit(1);
	}
	mems[bucket]=tmpmems;
}

bool Memory::Add(void* m, MEMTYPE type, bool exitonfail, const char* msgonfail)
{
	debug(9, "___Adding memory pointer into memory structure (%p).", m);
	//allocation failed?
	if(type==MALLOC && m==NULL)
	{
		printf("E: %s\n", msgonfail);
		if(exitonfail)
		{
			FreeThis();
			exit(1);
		}
		return false;
	}
	//determine bucket
	int hash = GetHash(m);
	//resize bucket if necessary
	if(counts[hash]>maxes[hash]-1)
		More(hash);
	//create MemoryBlock
	MemoryBlock* newblock = new MemoryBlock(m, type);
	//add into bucket
	mems[hash][counts[hash]]=newblock;
	counts[hash]++;
	return true;
}

void Memory::FreeAll()
{
	for(int i=0; i<max; i++)
	{
		for(int j=0; j<counts[i]; j++)
		{
			if(mems[i][j]==NULL)
				continue;
			//delete Memory Block (will free/delete/delete[] its content)
			debug(9, "~~~Cleaning memory %p :", mems[i][j]->p);
			delete mems[i][j];
			mems[i][j]=NULL;
		}
		counts[i]=0;
	}
}

void Memory::Free(void* m)
{
	int hash = GetHash(m);
	//find MemoryBlock to free
	for(int i=0; i<counts[hash]; i++)
	{
		if(mems[hash][i]==NULL)
			continue;
		if(mems[hash][i]->p==m)
		{
			//delete Memory Block (will free/delete/delete[] its content)
			debug(9, "~~~Cleaning specific memory %p :", mems[hash][i]->p);
			delete mems[hash][i];
			//re-organize bucket
			mems[hash][i]=mems[hash][counts[hash]-1];
			mems[hash][counts[hash]-1]=NULL;
			counts[hash]--;
			break;
		}
	}
}

void Memory::FreeThis()
{
	//first free every Memory Block
	FreeAll();
	//free buckets, hashmap, counts and maxes
	for(int i=0; i<max; i++)
		free(mems[i]);
	free(mems);
	free(counts);
	free(maxes);
	debug(9, "Memory structure is clean and it is being deleted.");
}

int Memory::GetHash(void* p)
{
	// abs((pointer value)%(number of buckets))
	int hash = *((int*)(&p))%max;
	if(hash<0)
		hash=-hash;
	debug(9, "###Memory hash for %p: %d", p, hash);
	return hash;
}



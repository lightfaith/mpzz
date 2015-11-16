#include "lib.hpp"

void debug(int level, const char* format, ...)
{
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
	FILE *f = fopen(filename, "rt");
	if(f==NULL)
	{
		printf("E: Unable to open file '%s'\n", filename);
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
	if(type==MALLOC)
		free(p);
	else if(type==NEW)
		delete (Generic*)p;
	else if(type==NEWARR)
		delete [] (Generic*)p;
	else 
		debug(4, "Attempt to free memory with unknown type.");
}
// - - - - - - - - - - - - - - - - - - - - - 

Memory::Memory(int max)
{
	debug(9, "Creating memory structure.");
	this->max=max;
	mems = (MemoryBlock***)malloc(max*sizeof(MemoryBlock**));
	if(mems==NULL)
	{
		printf("E: Cannot allocate memory structure, exiting!\n");
		exit(1);
	}
	counts=(int*)malloc(max*sizeof(int));
	if(counts==NULL)
	{
		printf("E: Cannot allocate memory counts structure. Exiting!");
		free(mems);
		exit(1);
	}
	maxes = (int*)malloc(max*sizeof(int));
	if(maxes==NULL)
	{
		printf("E: Cannot allocate memory maxes structure. Exiting!");
		free(mems);
		free(counts);
		exit(1);
	}
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
	debug(9, "Adding memory pointer into memory structure (%p).", m);
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
	int hash = GetHash(m);
	if(counts[hash]>maxes[hash]-1)
		More(hash);
	MemoryBlock* newblock = new MemoryBlock(m, type);
	mems[hash][counts[hash]]=newblock;
	counts[hash]++;
	return true;
}

void Memory::FreeAll()
{
	debug(9, "~~~Cleaning memory:");
	for(int i=0; i<max; i++)
	{
		for(int j=0; j<counts[i]; j++)
		{
			delete mems[i][j];
			mems[i][j]=NULL;
		}
		counts[i]=0;
	}
}

void Memory::Free(void* m)
{
	int hash = GetHash(m);
	for(int i=0; i<counts[hash]; i++)
		if(mems[hash][i]->p==m)
		{
			delete mems[hash][i];
			mems[hash][i]=mems[hash][counts[hash]-1];
			counts[hash]--;
			break;
		}
}

void Memory::FreeThis()
{
	FreeAll();
	for(int i=0; i<max; i++)
		free(mems[i]);
	free(mems);
	free(counts);
	free(maxes);
	debug(9, "Memory structure is clean and it is being deleted.");
}

int Memory::GetHash(void* p)
{
	int hash = ((int)p)%max;
	if(hash<0)
		hash=-hash;
	debug(8, "Memory hash for %p: %d", p, hash);
	return hash;
}



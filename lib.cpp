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

/*
bool equalstrings(const char* s1, const char* s2)
{
	int counter=0;
	while(1)
	{
		//same length && change not detected => equal
		if(s1[counter]==0 && s2[counter]==0)
			return true;
		//first shorter
		if(s1[counter]==0 && s2[counter]!=0)
			return false;
		//second shorter
		if(s1[counter]!=0 && s2[counter]==0)
			return false;
		//byte is different
		if((s1[counter]^s2[counter])!=0)
			return false;
		counter++;
	}
}
*/

Memory::Memory()
{
	debug(9, "Creating memory structure.");
	max=32;
	count=0;
	mems = (void**)malloc(max*sizeof(void*));
	if(mems==NULL)
	{
		printf("E: Cannot allocate memory structure, exiting!\n");
		exit(1);
	}
	types = (MEMTYPE*)malloc(max*sizeof(MEMTYPE));
	if(types==NULL)
	{
		printf("E: Cannot allocate memory type structure, exiting!\n");
		exit(1);
	}
}

Memory::~Memory()
{
	FreeAll();
	free(mems);
	free(types);
	debug(9, "Memory structure is clean and it is being deleted.");
}

void Memory::More()
{
	debug(9, "Resizing memory from %d to %d\n.", max, max*2);
	max*=2;
	void** tmpmems=(void**)realloc(mems, max*sizeof(void*));
	if(tmpmems==NULL)
	{
		printf("E: Resizing memory failed. Exiting...\n");
		FreeAll();
		exit(1);
	}
	mems=tmpmems;
	MEMTYPE* tmptypes = (MEMTYPE*)realloc(types, max*sizeof(MEMTYPE));
	if(types==NULL)
	{
		printf("E: Resizing memory type failed. Exiting...\n");
		FreeAll();
		exit(1);
	}
	types=tmptypes;
}

bool Memory::Add(void* m, MEMTYPE type, bool exitonfail, const char* msgonfail)
{
	debug(9, "Adding memory pointer into memory structure (%p).", m);
	if(m==NULL)
	{
		printf("E: %s\n", msgonfail);
		if(exitonfail)
		{
			FreeAll();
			exit(1);
		}
		return false;
	}
	if(count>max-1)
		More();
	mems[count]=m;
	types[count]=type;
	count++;
	return true;
}

void Memory::FreeAll()
{
	debug(9, "~~~Cleaning memory:");
	for(int i=0; i<count; i++)
		if(mems[i]!=NULL)
		{
			debug(9, "~~~  Freeing memory %p.", mems[i]);
			if(types[i]==MALLOC)
				free(mems[i]);
			else if(types[i]==NEW)
				delete (Generic*)mems[i];
			else
				delete[] (Generic*)mems[i];
			mems[i]=NULL;
			types[i]=NONE;
		}
}

void Memory::Free(void* m)
{
	for(int i=0; i<count; i++)
	{
		if(mems[i]==m)
		{
			debug(9, "~~~  Freeing memory %p.", m);
			if(types[i]==MALLOC)
				free(m);
			else if(types[i]==NEW)
				delete (Generic*)mems[i];
			else
				delete[] (Generic*)mems[i];
			count--;
			mems[i]=mems[count];
			mems[count]=NULL;
			types[i]=types[count];
			types[count]=NONE;
			break;
		}
	}
}

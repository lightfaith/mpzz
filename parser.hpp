#ifndef PRIMO_PARSER_H
#define PRIMO_PARSER_H
#include "classes.hpp"
#include "lib.hpp"

class FileReader : Generic
{
	private:
		long length;
		long counter;
		const char* filename;
		char* content;
		bool inmemory;
		FILE *f;
	public:
		
		FileReader(const char* filename);
		~FileReader();
		long GetFileSize();
		char GetNext();
		void Reset();
		long GetLength();
};

Nodes* gmlparse(const char* file);

#endif

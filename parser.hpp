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
		
		FileReader();
		~FileReader();
		void Init(const char* filename);
		long GetFileSize();
		char GetNext();
		void Reset();
		long GetLength();
};

Nodes* gmlparse(const char* file);
Nodes* csvparse(const char* file);


#endif

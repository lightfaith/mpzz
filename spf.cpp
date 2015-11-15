#include "classes.hpp"
#include "lib.hpp"
#include "parser.hpp"

int debuglevel=0;
Memory* memory = new Memory();

void printusage(const char* program)
{
		printf("Usage: %s <inputfile> <outputfile> <root> [<debuglevel> (0-9)]\n", program);
		exit(1);	
}

int main(int argc, char** argv)
{
	if(argc<4)
		printusage(argv[0]);
	if(argc>4)
	debuglevel=atoi(argv[4]);
	if(debuglevel<0 || debuglevel>9)
		printusage(argv[0]);

	Nodes* nodes = gmlparse(argv[1]);
	if(nodes==NULL)
	{
		printf("E: Parsing failed. Exiting...\n");
		memory->FreeAll();
		exit(1);
	}
	debug(2, "Size: %d\n", nodes->count);
	if(nodes->count<100)
		nodes->Print();

	Node* root = nodes->Find(argv[3]);
	if(root==NULL)
	{
		printf("E: You must choose valid node id!\n");
		memory->FreeAll();
		return 1;
	}
	nodes->SPF(root, argv[2]);
	delete memory;
	return 0;
}

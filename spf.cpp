#include "classes.hpp"
#include "lib.hpp"
#include "parser.hpp"

int debuglevel=0; //verbosity
Memory* memory = new Memory(100); //memory management structure

void printusage(const char* program)
{
		printf("Usage: %s <inputfile> <outputfile> <root> [<debuglevel> (0-9)]\n", program);
		exit(1);	
}

int main(int argc, char** argv)
{
	if(argc<4) //not enough arguments
		printusage(argv[0]);
	if(argc>4) //debug level provided, parse
	debuglevel=atoi(argv[4]);
	if(debuglevel<0 || debuglevel>9) //debug level weird
		printusage(argv[0]);

	Nodes* nodes = gmlparse(argv[1]); //parse nodes as gml
	if(nodes==NULL) 
	{
		//something got wrong
		printf("E: Parsing failed. Exiting...\n");
		delete memory;
		exit(1);
	}

	if(nodes->count<100)
		nodes->Print();

	//find root by name
	Node* root = nodes->Find(argv[3]);
	if(root==NULL)
	{
		printf("E: You must choose valid node id!\n");
		delete memory;
		return 1;
	}

	//compute Dijkstra
	nodes->SPF(root, argv[2]);

	//delete memory structure (will free everything)
	delete memory;
	return 0;
}

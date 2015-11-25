#include "classes.hpp"
#include "lib.hpp"
#include "parser.hpp"

int debuglevel=0; //verbosity
Memory* memory;  //memory management structure

void printusage(const char* program)
{
		printf("Usage: %s <inputfile> (gml|csv) <outputfile> <root> [<debuglevel> (0-9)]\n", program);
		exit(1);	
}

int main(int argc, char** argv)
{
	if(argc<5) //not enough arguments
		printusage(argv[0]);
	if(argc>5) //debug level provided, parse
	debuglevel=atoi(argv[5]);
	if(debuglevel<0 || debuglevel>9) //debug level weird
		printusage(argv[0]);
	memory = new Memory(100); //memory management structure

	//choose method
	int method=-1;
	if(strncmp(argv[2], "gml", 3)==0)
		method=0;
	else if(strncmp(argv[2], "csv", 3)==0)
		method=1;
	
	Nodes* nodes=NULL;
	switch(method)
	{
		case 0:
		{
			nodes = gmlparse(argv[1]); //parse nodes as gml
			break;
		}
		case 1:
		{
			nodes = csvparse(argv[1]);	
			break;
		}
		default:
		{
			printusage(argv[0]);
		}
	}
	
	if(nodes==NULL) 
	{
		//something got wrong
		printf("E: Parsing failed. Exiting...\n");
		delete memory;
		exit(1);
	}
	//for(int i=0; i<nodes->hashmap->max; i++)
	//	printf("Bucket %d: %d items\n", i, nodes->hashmap->counts[i]);
	if(nodes->count<100)
		nodes->Print();
	debug(2, "------------------------");
	//find root by name
	Node* root = nodes->Find(argv[4]);
	if(root==NULL)
	{
		printf("E: You must choose valid node id!\n");
		delete memory;
		return 1;
	}

	//compute Dijkstra
	nodes->SPF(root, argv[3]);

	//delete memory structure (will free everything)
	delete memory;
	return 0;
}

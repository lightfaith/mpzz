/*
	This tool generates complete graphs (with random edges missing) in various formats:
	- gml
*/
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<time.h>
#define MISSRATIO 50

void gml(int nodes, const char* filename)
{
	FILE *f = fopen(filename, "w");	
	if(f==NULL)
		exit(1);
	fprintf(f, "graph\n[\n");
	//generate nodes
	for(int i=0; i<nodes; i++)
	{
		fprintf(f, "  node\n  [\n    id %d\n  ]\n", i);
	}
	//generate edges
	for(int i=0; i<nodes; i++)
	{
		//skip sometimes to gain longer paths
		if((rand()%100)<MISSRATIO)
			continue;
		for(int j=i+1; j<nodes; j++)
		{
			fprintf(f, "  edge\n  [\n    source %d\n    target %d\n    value %d\n  ]\n", i, j, rand()%500+1);
		}
	}
	fprintf(f, "]\n"); //end of graph
	if(fclose(f)==EOF)
		exit(1);
}


int main(int argc, char**argv)
{
	if(argc<3)
	{
		printf("Usage: %s gml <nodes> <outputfile>\n", argv[0]);
		exit(1);
	}
	
	srand((unsigned)time(NULL));
	int nodes = atoi(argv[2]);
	if(strncmp(argv[1], "gml", 3)==0) //create gml file
	{
		gml(nodes, argv[3]);
		return 0;
	}
}

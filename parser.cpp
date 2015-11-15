#include "classes.hpp"
#include "lib.hpp"
//#define STACKSIZE 256
#define BUFFSIZE 256

Nodes* gmlparse(const char* file)
{
	debug(1, "FILE %s will be parsed as GML...", file);
	long inputsize=getfilesize(file);
	char* input = (char*)malloc(sizeof(char)*inputsize);
	memory->Add(input, MALLOC, true, "Cannot allocate input buffer.");
	long inputcount=0;
	Nodes* result = new Nodes();
	memory->Add(result, NEW, true, "Cannot allocate node list.");
	char* buffer = (char*)malloc(BUFFSIZE*sizeof(char));
	memory->Add(buffer, MALLOC, true, "Cannot allocate temporary buffer.");
	
	int wordlen=0;
	int c;
	int state=0;
	int edgecount=0;
	
	char* tmpid = (char*)malloc(BUFFSIZE*sizeof(char));
	memory->Add(tmpid, MALLOC, true, "Cannot allocate temp id buffer.");
	char* tmpsourceid = (char*)malloc(BUFFSIZE*sizeof(char));
	memory->Add(tmpsourceid, MALLOC, true, "Cannot allocate temp srcid buffer.");
	char* tmptargetid = (char*)malloc(BUFFSIZE*sizeof(char));
	memory->Add(tmptargetid, MALLOC, true, "Cannot allocate temp dstid buffer.");
	float tmpmetric;

	for(int i=0; i<BUFFSIZE; i++)
	{
		buffer[i]=0;
		tmpid[i]=0;
		tmpsourceid[i]=0;
		tmptargetid[i]=0;
	}

	FILE *f =  fopen(file, "r");
	if(f==NULL)
	{
		printf("E: Cannot open input file '%s'.\n", file);
		memory->FreeAll();
		exit(1);
	}
	fread(input, 1, inputsize, f);
	fclose(f);
	
	debug(1, "Parsing started.");
	
	while(state<11)
	{
		c=input[inputcount++];//fgetc(f);
		if(c==0) //end of input? stop this madness
			break;
		else if(c==' ' || c=='\t' || c=='\n' || c=='\r' || c=='\v') //whitespace, part complete
		{
			if(wordlen==0)
				continue;
			buffer[wordlen]=0;
			
			switch(state)
			{
				case 0: //start, look for 'graph' keyword
				{
					if(strncmp(buffer, "graph", 5)==0)
					{
						//filestack[stcount++]='g';
						state=1;
						debug(6, "Correct keyword '%s'. Transiting to %d", buffer, state);
					}
					else
					{
						debug(9, "Wrong keyword '%s' in state %d", buffer, state);
					}
					break;
				}
				case 1: //in graph, look for [
				{
					if(strncmp(buffer, "[",1)==0)
					{
						//filestack[stcount++]='b';
						state=2;
						debug(6, " Correct keyword '%s'. Transiting to %d", buffer, state);
					}
					else 
					{
						debug(9, "Wrong keyword '%s' in state %d", buffer, state);
					}
					break;
				}
				case 2: //int graph[, look for node, edge or ]
				{
					if(strncmp(buffer, "node", 4)==0)
					{
						//filestack[stcount++]='n';
						state=3;
						debug(6, "  Correct keyword '%s'. Transiting to %d", buffer, state);
					}
					else if(strncmp(buffer, "edge", 4)==0)
					{
						//filestack[stcount++]='e';
						state=6;
						debug(6, "  Correct keyword '%s'. Transiting to %d", buffer, state);
					}
					else if(strncmp(buffer, "]", 1)==0)
					{
						//filestack[--stcount]=0;
						state=11; //this is the end
						debug(6, " Correct keyword '%s'. Transiting to %d", buffer, state);
					}
					else
					{
						debug(9, "Wrong keyword '%s' in state %d", buffer, state);
					}
					break;
				}
				case 3: //in node, wait for [
				{
					if(strncmp(buffer, "[", 1)==0)
					{
						//filestack[stcount++]='b';
						state=4;
						debug(6, "   Correct keyword '%s'. Transiting to %d", buffer, state);
					}
					else 
					{
						debug(9, "Wrong keyword '%s' in state %d", buffer, state);
					}
					break;
				}
				case 4: // in node[, wait for id or ]
				{
					if(strncmp(buffer, "id",2)==0)
					{
						//filestack[stcount++]='i';
						state=5;
						debug(6, "    Correct keyword '%s'. Transiting to %d", buffer, state);
					}
					else if(strncmp(buffer, "]", 1)==0)
					{
						//filestack[--stcount]=0;
						if(tmpid[0]!=0 /*&& tmpnode==NULL && tmpconn==NULL*/)
						{
							Node* tmpnode = new Node(tmpid);
							memory->Add(tmpnode, NEW, true, "Cannot alloc tmpnode.");
							Connections* tmpconn = new Connections();
							memory->Add(tmpconn, NEW, true, "Cannot alloc tmpconn.");
							debug(9, "HIER!");
							tmpnode->neighbors=tmpconn;
							result->Add(tmpnode);
						}
						else
							debug(9, "   Wrong data in node, ignoring...");
						state=2;
						debug(6, "   Correct keyword '%s'. Transiting to %d", buffer, state);
					}
					else 
					{
						debug(9, "   Wrong keyword '%s' in state %d", buffer, state);
					}
					break;
				}
				case 5: //in node[id, load identifier (string)
				{
					//filestack[stcount++]='i';
					state=4;
					strncpy(tmpid, buffer, BUFFSIZE);
					tmpid[BUFFSIZE-1]=0;
					debug(6, "    Loaded id of new node: %s. Transiting to %d", tmpid, state);
					break;
				}
				case 6: //in edge, wait for [
				{
					if(strncmp(buffer, "[",1)==0)
					{
						//filestack[stcount++]='b';
						state=7;
						debug(6, "   Correct keyword '%s'. Transiting to %d", buffer, state);
					}
					else 
					{
						debug(9, "   Wrong keyword '%s' in state %d", buffer, state);
					}
					break;
				}
				case 7: //in edge[, wait for source, target, value or ]
				{
					if(strncmp(buffer, "source", 6)==0)
					{
						//filestack[stcount++]='s';
						state=8;
						debug(6, "    Correct keyword '%s'. Transiting to %d", buffer, state);
					}
					else if(strncmp(buffer, "target", 6)==0)
					{
						//filestack[stcount++]='t';
						state=9;
						debug(6, "    Correct keyword '%s'. Transiting to %d", buffer, state);
					}
					else if(strncmp(buffer, "value", 5)==0)
					{
						//filestack[stcount++]='v';
						state=10;
						debug(6, "    Correct keyword '%s'. Transiting to %d", buffer, state);
					}
					else if(strncmp(buffer, "]", 1)==0)
					{
						//filestack[--stcount]=0;
						state=2;
						Node* tmpsource = result->Find(tmpsourceid);
						Node* tmptarget = result->Find(tmptargetid);
						if(tmpsource==NULL || tmptarget==NULL || tmpmetric<=0)
						{
							debug(6, "    Edge found is weird. Skipping...");
						}
						else
						{
							tmpsource->neighbors->Add(tmptarget, tmpmetric);
							edgecount++;
						}
						if(tmpsource==NULL)
						{
							debug(9, "     tmpsource NULL");
						}
						if(tmptarget==NULL)
						{
							debug(9, "     tmptarget NULL");
						}
						
						debug(6, "   Correct keyword '%s'. Transiting to %d", buffer, state);
					}
					else 
					{
						debug(9, "   Wrong keyword '%s' in state %d", buffer, state);					
					}
					break;
				}
				case 8: //in edge[source, load id (string)
				{
					//filestack[--stcount]=0;
					state=7;
					strncpy(tmpsourceid, buffer, BUFFSIZE);
					tmpsourceid[BUFFSIZE-1]=0;
					debug(6, "     Loaded source of new edge: %s. Transiting to %d", buffer, state);
					break;
				}
				case 9: //in edge[target, load id (string)
				{
					//filestack[--stcount]=0;
					state=7;
					strncpy(tmptargetid, buffer, BUFFSIZE);
					tmptargetid[BUFFSIZE-1]=0;
					debug(6, "     Loaded target of new edge: %s. Transiting to %d", buffer, state);
					break;
				}
				case 10: //in edge[value, load metric (float)
				{
					//filestack[--stcount]=0;
					state=7;
					tmpmetric=atof(buffer);
					debug(6, "     Loaded metric of new edge: %.3f. Transiting to %d", tmpmetric, state);
					break;
				}
			} //end of state switch
			wordlen=0;
		}
		else //normal character, treat as keyword or value part
		{
			buffer[wordlen++]=c;	
		}
		
	}
	debug(1, "Data successfully loaded (%d nodes, %d edges).", result->count, edgecount);
	memory->Free(buffer);
	//free(filestack);
	memory->Free(input);
	memory->Free(tmpid);
	memory->Free(tmpsourceid);
	memory->Free(tmptargetid);
	return result;
}



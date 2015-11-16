#include "classes.hpp"
#include "lib.hpp"
#define BUFFSIZE 256

Nodes* gmlparse(const char* file)
{
	debug(1, "FILE %s will be parsed as GML...", file);
	long inputsize=getfilesize(file); //size of input file
	char* input = (char*)malloc(sizeof(char)*inputsize); //whole input string
	memory->Add(input, MALLOC, true, "Cannot allocate input buffer.");
	long inputcount=0; //char counter
	Nodes* result = new Nodes(); //result Nodes structure
	memory->Add(result, NEW, true, "Cannot allocate node list.");
	char* buffer = (char*)malloc(BUFFSIZE*sizeof(char)); //temporary buffer
	memory->Add(buffer, MALLOC, true, "Cannot allocate temporary buffer.");
	
	int wordlen=0; //length of keywords
	int c; //single character
	int state=0; //parsing state
	long edgecount=0; //number of edges
	
	int tmpidlen=0; //length of temporary sid
	char* tmpid = (char*)malloc(BUFFSIZE*sizeof(char)); //temporary sid
	memory->Add(tmpid, MALLOC, true, "Cannot allocate temp id buffer.");
	char* tmpsourceid = (char*)malloc(BUFFSIZE*sizeof(char)); //temporary id of source
	memory->Add(tmpsourceid, MALLOC, true, "Cannot allocate temp srcid buffer.");
	char* tmptargetid = (char*)malloc(BUFFSIZE*sizeof(char)); //temporary id of target
	memory->Add(tmptargetid, MALLOC, true, "Cannot allocate temp dstid buffer.");
	float tmpmetric; //temporary metric

	//null those buffers
	for(int i=0; i<BUFFSIZE; i++)
	{
		buffer[i]=0;
		tmpid[i]=0;
		tmpsourceid[i]=0;
		tmptargetid[i]=0;
	}

	debug(1, "Parsing started.");
	
	//read file
	FILE *f =  fopen(file, "r");
	if(f==NULL)
	{
		printf("E: Cannot open input file '%s'.\n", file);
		memory->FreeAll();
		exit(1);
	}
	fread(input, 1, inputsize, f);
	fclose(f);	
	while(state<11)
	{
		//c=fgetc(f);
		c=input[inputcount++];
		if(c==EOF)//if(c==0) //end of input? stop this madness
			break;
		else if(c==' ' || c=='\t' || c=='\n' || c=='\r' || c=='\v') //whitespace, part complete
		{
			if(wordlen==0) //weird, skip
				continue;
			buffer[wordlen]=0;
			
			switch(state)
			{
				case 0: //start, look for 'graph' keyword
				{
					if(strncmp(buffer, "graph", 5)==0)
					{
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
						state=3;
						debug(6, "  Correct keyword '%s'. Transiting to %d", buffer, state);
					}
					else if(strncmp(buffer, "edge", 4)==0)
					{
						state=6;
						debug(6, "  Correct keyword '%s'. Transiting to %d", buffer, state);
					}
					else if(strncmp(buffer, "]", 1)==0)
					{
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
						state=5;
						debug(6, "    Correct keyword '%s'. Transiting to %d", buffer, state);
					}
					else if(strncmp(buffer, "]", 1)==0)
					{
						if(tmpid[0]!=0)
						{
							//create new node
							char* sid=(char*)malloc((tmpidlen+1)*sizeof(char));
							//memcpy(sid, tmpid, tmpidlen);
							strncpy(sid, tmpid, tmpidlen);
							sid[tmpidlen]=0;
							memory->Add(sid, MALLOC, true, "Cannot allocate node sid.");
							Node* tmpnode = new Node(sid);
							memory->Add(tmpnode, NEW, true, "Cannot alloc tmpnode.");
							Connections* tmpconn = new Connections();
							memory->Add(tmpconn, NEW, true, "Cannot alloc tmpconn.");
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
					state=4;
					strncpy(tmpid, buffer, wordlen+1);
					tmpid[wordlen]=0;
					tmpidlen=wordlen;
					debug(6, "    Loaded id of new node: %s. Transiting to %d", tmpid, state);
					break;
				}
				case 6: //in edge, wait for [
				{
					if(strncmp(buffer, "[",1)==0)
					{
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
						state=8;
						debug(6, "    Correct keyword '%s'. Transiting to %d", buffer, state);
					}
					else if(strncmp(buffer, "target", 6)==0)
					{
						state=9;
						debug(6, "    Correct keyword '%s'. Transiting to %d", buffer, state);
					}
					else if(strncmp(buffer, "value", 5)==0)
					{
						state=10;
						debug(6, "    Correct keyword '%s'. Transiting to %d", buffer, state);
					}
					else if(strncmp(buffer, "]", 1)==0)
					{
						//add new edge
						state=2;
						Node* tmpsource = result->Find(tmpsourceid);
						Node* tmptarget = result->Find(tmptargetid);
						if(tmpsource==NULL || tmptarget==NULL || tmpmetric<=0)
						{
							debug(6, "    Edge found is weird. Metric=%d. Skipping...", tmpmetric);
						}
						else
						{
							tmpsource->neighbors->Add(tmptarget, tmpmetric);
							edgecount++;
						}
						if(tmpsource==NULL)
						{
							debug(9, "     tmpsource NULL (looked for '%s')", tmpsourceid);
						}
						if(tmptarget==NULL)
						{
							debug(9, "     tmptarget NULL (looked for '%s')", tmptargetid);
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
					state=7;
					strncpy(tmpsourceid, buffer, BUFFSIZE);
					tmpsourceid[BUFFSIZE-1]=0;
					debug(6, "     Loaded source of new edge: %s. Transiting to %d", buffer, state);
					break;
				}
				case 9: //in edge[target, load id (string)
				{
					state=7;
					strncpy(tmptargetid, buffer, BUFFSIZE);
					tmptargetid[BUFFSIZE-1]=0;
					debug(6, "     Loaded target of new edge: %s. Transiting to %d", buffer, state);
					break;
				}
				case 10: //in edge[value, load metric (float)
				{
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
			buffer[wordlen]=c;
			//we don't want any buffer overflow
			if(wordlen<BUFFSIZE-2)
				wordlen++;
			else
			{
				buffer[BUFFSIZE-1]=0;
				debug(3, "Keyword '%s' too long. Will be malformed.", buffer);
			}
		}
		
	} //end of while

	//close file, free unnecessary buffers
	//fclose(f);
	debug(1, "Data successfully loaded (%d nodes, %d edges).", result->count, edgecount);
	memory->Free(buffer);
	//free(filestack);
	memory->Free(input);
	memory->Free(tmpid);
	memory->Free(tmpsourceid);
	memory->Free(tmptargetid);
	return result;
}



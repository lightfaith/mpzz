#include "parser.hpp"
#define BUFFSIZE 256
#define REINDEXVALUE 1200

FileReader::FileReader(const char* filename)
{
	memory->Add(this, NEW, true, "Cannot create FileReader!");
	this->filename=filename;
	//get file size for allocation
	length = GetFileSize();
	if(length==0)
	{
		printf("E: File is empty or broken. Exiting.\n");
		delete memory;
		exit(1);
	}
	counter=0;
	inmemory=true;
	f=NULL;
	//try to allocate space
	content = (char*)malloc(length*sizeof(char)+1);
	memory->Add(content, MALLOC, false, "Cannot allocate memory for file. File will be read by character.");
	if(content==NULL)
	{
		//read by char, open file
		inmemory=false;
		f = fopen(filename, "r");
	    if(f==NULL)
		{   
			printf("E: Unable to open file '%s' for fgetc. Error %d: %s\n", filename, errno, strerror(errno));
			delete memory;
			exit(1);
		}
	}
	else	
	{
		//copy file content
		inmemory=true;
		f = fopen(filename, "r");
	    if(f==NULL)
		{   
			printf("E: Unable to open file '%s' for copy. Error %d: %s\n", filename, errno, strerror(errno));
			delete memory;
			exit(1);
		}
		fread(content, 1, length, f);
		content[length]=0;
		fclose(f);
		f=NULL;
	}
}

FileReader::~FileReader()
{
	if(f!=NULL)
		fclose(f);
	//if(inmemory)
	//	memory->Free(content);
}

long FileReader::GetFileSize()
{
    //get size of file (not good for files 4GB+)
    FILE *f = fopen(filename, "r");
    if(f==NULL)
    {   
        printf("E: Unable to open file '%s' for file size. Error %d: %s\n", filename, errno, strerror(errno));
        delete memory;
        exit(1);
    }   
    fseek(f, 0, SEEK_END);
    long size=ftell(f);
    if(fclose(f)==EOF)
    {   
        printf("E: Unable to close file '%s'\n", filename);
		delete memory;
        exit(1);
    }   
    return size;
}

char FileReader::GetNext()
{
	//at the end
	if(counter>=length-1)
		return 0;
	if(inmemory)
		return content[counter++]; //next char of array
	else
		return fgetc(f); //next char from file
}

void FileReader::Reset()
{
	if(inmemory)
		counter=0;
	else
		fseek(f, 0, SEEK_SET);
}

long FileReader::GetLength()
{
	return length;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - -


Nodes* gmlparse(const char* file)
{
	debug(1, "FILE %s will be parsed as GML...", file);
	//long inputsize=getfilesize(file); //size of input file
	//char* input = (char*)malloc(sizeof(char)*inputsize); //whole input string
	//memory->Add(input, MALLOC, true, "Cannot allocate input buffer.");
	//long inputcount=0; //char counter
	Nodes* result = new Nodes(); //result Nodes structure
	memory->Add(result, NEW, true, "Cannot allocate node list.");
	char* buffer = (char*)malloc(BUFFSIZE*sizeof(char)); //temporary buffer
	memory->Add(buffer, MALLOC, true, "Cannot allocate temporary buffer.");
	
	int wordlen=0; //length of keywords
	int c; //single character
	int state=0; //parsing state
	long edgecount=0; //number of edges
	bool optimized=false;

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
	FileReader *fr = new FileReader(file); //adds itself to memory management
	while(state<11)
	{
		c=fr->GetNext();
		if(c==0) //end of input? stop this madness
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
						debug(8, "Wrong keyword '%s' in state %d", buffer, state);
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
						debug(8, "Wrong keyword '%s' in state %d", buffer, state);
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
						debug(8, "Wrong keyword '%s' in state %d", buffer, state);
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
						debug(8, "Wrong keyword '%s' in state %d", buffer, state);
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
							memcpy(sid, tmpid, tmpidlen);
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
						debug(8, "   Wrong keyword '%s' in state %d", buffer, state);
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
					//hashmap not optimized? do it now!
					if(!optimized)
					{
						result->Reindex();
						optimized=true;
					}
					
					if(strncmp(buffer, "[",1)==0)
					{
						state=7;
						debug(6, "   Correct keyword '%s'. Transiting to %d", buffer, state);
					}
					else 
					{
						debug(8, "   Wrong keyword '%s' in state %d", buffer, state);
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
							debug(8, "    Edge found is weird. Metric=%d. Skipping...", tmpmetric);
						}
						else
						{
							tmpsource->neighbors->Add(tmptarget, tmpmetric);
							edgecount++;
						}
						if(tmpsource==NULL)
						{
							debug(8, "     tmpsource NULL (looked for '%s')", tmpsourceid);
						}
						if(tmptarget==NULL)
						{
							debug(8, "     tmptarget NULL (looked for '%s')", tmptargetid);
						}
						
						debug(6, "   Correct keyword '%s'. Transiting to %d", buffer, state);
					}
					else 
					{
						debug(8, "   Wrong keyword '%s' in state %d", buffer, state);					
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
			if(state==10 && c==',')//decimal point for metric
				buffer[wordlen]='.';
			else 
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

	debug(1, "Data successfully loaded (%d nodes, %d edges).", result->count, edgecount);
	memory->Free(fr);
	memory->Free(buffer);
	memory->Free(tmpid);
	memory->Free(tmpsourceid);
	memory->Free(tmptargetid);
	return result;
}



Nodes* csvparse(const char* file)
{
	debug(1, "FILE %s will be parsed as CSV...", file);
	Nodes* result = new Nodes();
	memory->Add(result, NEW, true, "Cannot allocate node list.");
	char* buffer = (char*)malloc(BUFFSIZE*sizeof(char));
	memory->Add(buffer, MALLOC, true, "Cannot allocate temporary buffer.");

	int edgecount=0;
	int wordlen=0;	
	int c; //single character
	int state=0; //parsing state
	char* tmpid1 = (char*)malloc(BUFFSIZE*sizeof(char));
	memory->Add(tmpid1, MALLOC, true, "Cannot allocate temporary buffer for first ID.");
	int tmpid1len;
	char* tmpid2 = (char*)malloc(BUFFSIZE*sizeof(char));
	memory->Add(tmpid2, MALLOC, true, "Cannot allocate temporary buffer for second ID.");
	int tmpid2len;
	float tmpmetric;
	Node* lastsource=NULL;

	//null those buffers
	for(int i=0; i<BUFFSIZE; i++)
	{
		buffer[i]=0;
		tmpid1[i]=0;
		tmpid2[i]=0;
	}

	debug(1, "Parsing started.");
	FileReader *fr = new FileReader(file); //adds itself to memory management
	while(1)
	{
		c=fr->GetNext();
		if(c==0 && wordlen==0) //end of input? stop this madness!
			break;
		else if(c==0 || c==' '|| c=='\t' || c=='\n' || c=='\r' || c=='v') //whitespace, treat as new info
		{
			if(state==2)
			{
				buffer[wordlen]=0;
				state=0;
				tmpmetric=atof(buffer);
				debug(6, "    Loaded metric of new edge: %.3f. Transiting to %d.", tmpmetric, state);
				//now find associated nodes and update changes
				
				Node* source=NULL;
				if(lastsource!=NULL)
					if(strlen(lastsource->sid)==tmpid1len && strncmp(lastsource->sid, tmpid1, tmpid1len)==0)
						source=lastsource;
				if(source==NULL)
					source = result->Find(tmpid1);
				lastsource=source;
				//brand new?
				if(source==NULL)
				{
					char* sid=(char*)malloc((tmpid1len+1)*sizeof(char));
					memcpy(sid, tmpid1, tmpid1len);
					sid[tmpid1len]=0;
					memory->Add(sid, MALLOC, true, "Cannot allocate first node sid.");
					source = new Node(sid);
					memory->Add(source, NEW, true, "Cannot create new node.");
					Connections* c = new Connections();
					memory->Add(c, NEW, true, "Cannot create connections for node.");
					source->neighbors=c;
					result->Add(source);
					if(result->count%REINDEXVALUE==0)
					{
						result->Reindex();
					}
				}
				Node* target = result->Find(tmpid2);
				//brand new?
				if(target==NULL)
				{
					char* sid=(char*)malloc((tmpid2len+1)*sizeof(char));
					memcpy(sid, tmpid2, tmpid2len);
					sid[tmpid2len]=0;
					memory->Add(sid, MALLOC, true, "Cannot allocate first node sid.");
					target = new Node(sid);
					memory->Add(target, NEW, true, "Cannot create new node.");
					Connections* c = new Connections();
					memory->Add(c, NEW, true, "Cannot create connections for node.");
					target->neighbors=c;
					result->Add(target);
					if(result->count%REINDEXVALUE==0)
					{
						result->Reindex();
					}
				}
				source->neighbors->Add(target, tmpmetric);
				edgecount++;
				wordlen=0;
			}
		}
		else if(c==';') //part complete
		{
			buffer[wordlen]=0;
			switch(state)
			{
				case 0: //loading first node id
				{
					state=1;
					strncpy(tmpid1, buffer, wordlen+1);
					tmpid1[wordlen]=0;
					tmpid1len=wordlen;
					debug(6, "Loaded source of edge: %s, transiting to %d.", buffer, state);
					break;
				}
				case 1: //loading second node id
				{
					state=2;
					strncpy(tmpid2, buffer, wordlen+1);
					tmpid2[wordlen]=0;
					tmpid2len=wordlen;
					debug(6, "Loaded target of edge: %s, transiting to %d.", buffer, state);
					break;
				}
			}
			wordlen=0;
		}
		else //normal character
		{
			if(c==',' && state==2) //decimal point for metric
				buffer[wordlen]='.';
			else
				buffer[wordlen]=c;
			if(wordlen<BUFFSIZE-2)
				wordlen++;
			else
			{
				buffer[BUFFSIZE-1]=0;
				debug(3, "Keyword '%s' too long. Will be malformed.", buffer);
			}
		}
	} //end of while
	result->Reindex(); //probably not as effective here...
	debug(1, "Data successfully loaded (%d nodes, %d edges).", result->count, edgecount);
	memory->Free(fr);
	memory->Free(buffer);
	memory->Free(tmpid1);
	memory->Free(tmpid2);
	return result;
}

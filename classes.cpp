#include "classes.hpp"

Node::Node(string sid)
{
	this->sid=sid;
	debug(9, "Creating node %s", sid.c_str());
	predecessor=NULL;
	hopcount=INT_MAX;
}

Node::~Node()
{
	//debug("Freeing node %s (#%d)", sid.c_str(), id);
	debug(9, "Freeing node %s", sid.c_str());
	memory->Free(neighbors);
}

void Node::Print()
{
	//printf("  Node %s (#%d)\n", sid.c_str(), id);
	debug(2, "  Node %s", sid.c_str());
	for(int i=0; i<neighbors->count; i++)
		//printf("    Neighbor %s (#%d), distance %.3f\n", neighbors->nodes[i]->sid.c_str(), 		neighbors->nodes[i]->id, neighbors->metrics[i]);
		debug(2, "    Neighbor %s, distance %.3f", neighbors->nodes[i]->sid.c_str(), neighbors->metrics[i]);
}

void Node::SetPredecessor(Node* n, float metric)
{
	predecessor=n;
	totalmetric=metric;
	if(n==NULL) //that should be root
		hopcount=0;
	else
		hopcount=n->hopcount+1;		
}

Node** Node::GetPath()
{
	Node** result = (Node**)malloc(hopcount*sizeof(Node*));
	if(!memory->Add(result, MALLOC, false, "Cannot get path for node."))
		return NULL;
	Node* p = this;
	debug(8, "Getting path for %s:", p->sid.c_str());
	for(int i=hopcount-1; i>=0; i--)
	{
		debug(8, "  hop #%d: %s", hopcount-i-1, p->sid.c_str());
		result[i]=p;
		p=p->predecessor;
	}
	if(p->predecessor!=NULL)
		debug(1, "  Some problem with getpath()...");
	
	debug(8, "Path for %s gathered.", sid.c_str());
	return result;
}


Connections::Connections()
{
	debug(9, "Creating connections...");
	max=16;
	count=0;
	nodes = (Node**)malloc(max*sizeof(Node*));
	memory->Add(nodes, MALLOC, false, "Cannot create list of neighbors.");
	metrics = (float*)malloc(max*sizeof(float));
	memory->Add(metrics, MALLOC, false, "Cannot create list of neighbor distances.");
	debug(9, "Connections created.");
}

Connections::~Connections()
{
	debug(9, "Freeing connections...");
	memory->Free(nodes); nodes=NULL;
	memory->Free(metrics); metrics=NULL;
}

void Connections::Add(Node* n, float m)
{
	//debug(9, "Adding new connection: %s (%d) with metric of %.3f", n->sid.c_str(), n->id, m);
	debug(9, "Adding new connection: %s with metric of %.3f", n->sid.c_str(), m);
	if(count>=max-1)
		More();
	nodes[count]=n;
	metrics[count++]=m;
}

void Connections::More()
{
	debug(9, "Resizing connections from %d to %d", max, max*2);
	max*=2;
	Node** newnodes = (Node**)malloc(max*sizeof(Node*));
	memory->Add(newnodes, MALLOC, true, "Cannot resize neighbor list.");
	float* newmetrics = (float*)malloc(max*sizeof(float));
	memory->Add(newmetrics, MALLOC, true, "Cannot resize neighbor distance list.");
	for(int i=0; i<count; i++)
	{
		newnodes[i]=nodes[i];
		newmetrics[i]=metrics[i];
	}
	memory->Free(nodes);
	memory->Free(metrics);
	nodes = newnodes;
	metrics= newmetrics;
	//nodes = (Node**)realloc(nodes, max);
	//metrics = (float*)realloc(metrics, max);
}

Nodes::Nodes()
{
	debug(5, "Node list is being created...");
	max=64;
	count=0;
	nodes = (Node**)malloc(max*sizeof(Node*));
	memory->Add(nodes, MALLOC, true, "Cannot create node list.");
	hashmap = new HashMap();
	memory->Add(hashmap, NEW, true, "Cannot create HashMap.");
}

Nodes::~Nodes()
{
	debug(5, "Node list is being destroyed...");
	//Free(true);
	memory->Free(nodes); nodes=NULL;
	memory->Free(hashmap); hashmap=NULL;
}

void Nodes::Add(Node *n)
{
	debug(8, "Adding new node into list: %s", n->sid.c_str());
	if(count>=max-1)
		More();
	nodes[count]=n;
	hashmap->Add(n);
	count++;
}

void Nodes::More()
{
	debug(5, "Resizing node list from %d to %d", max, max*2);
	max*=2;
	Node** newnodes = (Node**)malloc(max*sizeof(Node*));
	memory->Add(newnodes, MALLOC, true, "Cannot resize node list");
	for(int i=0; i<count; i++)
		newnodes[i]=nodes[i];
	memory->Free(nodes);
	nodes = newnodes;
	
	//nodes = (Node**)realloc(nodes, max);
}

/*void Nodes::Free(bool all=true)
{
	if(all)
		for(int i=0; i<count; i++)
		{
			free(nodes[i]); nodes[i]=NULL;
		}
	free(nodes); nodes=NULL;
}*/

void Nodes::Print()
{
	for(int i=0; i<count; i++)
	{
		nodes[i]->Print();
	}
}

Node* Nodes::Find(const char* sid)
{
	return hashmap->Find(sid);
}


void Nodes::SPF(Node* root, const char* filename)
{
	Node** used = (Node**)malloc(count*sizeof(Node*));
	memory->Add(used, MALLOC, true, "Cannot prepare 'used' list for SPF.");
	Node** usable = (Node**)malloc(count*sizeof(Node*));
	memory->Add(usable, MALLOC, true, "Cannot prepare 'usable' list for SPF.");
	//Node** unusable = (Node**)malloc(count*sizeof(Node*));
	float* usablem = (float*)malloc(count*sizeof(float));
	memory->Add(usablem, MALLOC, true, "Cannot prepare 'usablem' list for SPF.");
	int usedcount=0;
	int usablecount=0;
	//int unusablecount=0;
	int bestid=0;
	float bestmetric;
	bool found;
	
	debug(1, "SPF Algorithm for root '%s' started.", root->sid.c_str());
	
	//mark root usable //and others unusable
	usablem[usablecount]=0;
	usable[usablecount]=root;
	usable[usablecount]->SetPredecessor(NULL, 0);
	usablecount++;
	
	debug(9, "Root marked usable.");
	//move best into used, move neighbors into usable etc...
	while(usablecount>0)
	{
		//find best
		bestmetric=usablem[0];
		for(int i=0; i<usablecount; i++)
		{
			if(usablem[i]<=bestmetric)
			{
				bestmetric=usablem[i];
				bestid=i;
			}
		}
		
		debug(6, "Found best: %s (metric %.3f).", usable[bestid]->sid.c_str(), bestmetric);
		
		//add it into used, its neighbors into usable
		used[usedcount]=usable[bestid];
		//remove from usable
		usable[bestid]=usable[usablecount-1];
		usablem[bestid]=usablem[usablecount-1];
		usablecount--;
		
		debug(8, "Usables before update:");
		for(int i=0; i<usablecount; i++)
		{
			debug(8, "  %s (metric %.3f)", usable[i]->sid.c_str(), usablem[i]);
		}
		//add neighbors into usable / update metric
		for(int i=0; i<used[usedcount]->neighbors->count; i++)
		{
			Node* nei = used[usedcount]->neighbors->nodes[i];
			float met = used[usedcount]->neighbors->metrics[i];
			found=false;
			for(int j=0; j<usablecount; j++)
			{
				//found? update metric
				if(nei==usable[j])
				{
					if(bestmetric+met<usablem[j])
					{
						usablem[j]=bestmetric+met;
						usable[j]->SetPredecessor(used[usedcount], usablem[j]);
					}
					found=true;
				}
			}
			//not found? check used...
			if(!found)
			{
				for(int j=0; j<usedcount+1; j++)
					if(nei==used[j])
					{
						found=true;
						break;
					}
			}
			
			//still not found? add new :)
			if(!found)
			{
				usablem[usablecount]=bestmetric+met;
				usable[usablecount]=nei;
				usable[usablecount]->SetPredecessor(used[usedcount], usablem[usablecount]);
				usablecount++;
			}
		}
		
		debug(8, "Usables after update:");
		for(int i=0; i<usablecount; i++)
		{
			debug(8, "  %s (metric %.3f)", usable[i]->sid.c_str(), usablem[i]);
		}
		usedcount++;
	}
	

	//paths have been found, print results into file
	debug(1, "Paths have been found. Writing output file...");
	FILE *f = fopen(filename, "w");
	if(f==NULL)
	{
		printf("E: Cannot open output file '%s'. Exiting...", filename);
		memory->FreeAll();
		exit(1);
	}
	for(int i=0; i<usedcount; i++)
	{
		debug(7, "Writing info about %s", used[i]->sid.c_str());
		Node** path = used[i]->GetPath();
		
		fprintf(f, "(%s)->(%s) = %.3f: (%s)", root->sid.c_str(), used[i]->sid.c_str(), used[i]->totalmetric, root->sid.c_str());
		debug(7, "  Writing path for %s (hopcount=%d, metric=%.3f)", used[i]->sid.c_str(), used[i]->hopcount, used[i]->totalmetric);
		for(int j=0; j<used[i]->hopcount; j++)
			if(path[j]!=NULL)
				fprintf(f, "->(%s)", path[j]->sid.c_str());
		fprintf(f, "\n");
		debug(7, "Info about %s written.", used[i]->sid.c_str());
	}
	if(usedcount<count)
		fprintf(f, "It seems that some nodes are not connected to '%s'. They are not shown in this file...\n", root->sid.c_str());
	fclose(f);
	
	debug(1, "SPF is complete. Check '%s' for results.", filename);
	memory->Free(used);
	memory->Free(usable);
}



HashMap::HashMap()
{
	max=1;
	maxes = (int*)malloc(max*sizeof(int));
	memory->Add(maxes, MALLOC, true, "Cannot create 'maxes' list for HashMap.");
	counts = (int*)malloc(max*sizeof(int));
	memory->Add(counts, MALLOC, true, "Cannot create 'counts' list for HashMap.");
	buckets = (Node***)malloc(max*sizeof(Node**));
	memory->Add(buckets, MALLOC, true, "Cannot create 'buckets' list for HashMap.");
	for(int i=0; i<max; i++)
	{
		maxes[i]=2;
		counts[i]=0;
		buckets[i]=(Node**)malloc(maxes[i]*sizeof(Node*));
		memory->Add(buckets[i], MALLOC, true, "Cannot create bucket for HashMap.");
	}
}


HashMap::~HashMap()
{
	memory->Free(maxes); maxes=NULL;
	memory->Free(counts); counts=NULL;
	for(int i=0; i<max; i++)
	{
		memory->Free(buckets[i]); buckets[i]=NULL;
	}
	memory->Free(buckets);
	
}


void HashMap::Add(Node* n)
{
	debug(8, "Adding new node into hashmap.");
	int hash = GetHash(n);
	if(counts[hash]>=maxes[hash]-1)
		More(hash);
	buckets[hash][counts[hash]]=n;
	counts[hash]++;
}


void HashMap::More(int bucket)
{
	debug(5, "Resizing hashmap bucket %d from %d to %d", bucket, maxes[bucket], maxes[bucket]*2);
	maxes[bucket]*=2;
	Node** newbucket = (Node**)malloc(maxes[bucket]*sizeof(Node*));
	memory->Add(newbucket, MALLOC, true, "Cannot resize bucket in HashMap.");
	for(int i=0; i<counts[bucket]; i++)
		newbucket[i]=buckets[bucket][i];
	memory->Free(buckets[bucket]);
	buckets[bucket]=newbucket;
	
	//Node** more=(Node**)realloc(buckets[bucket], maxes[bucket]);
	//debug(9, "more: %p", more);
	//buckets[bucket]=more;
	//debug(5, "Bucket %d resized to %d.", bucket, maxes[bucket]);
}


/*void HashMap::Free()
{
	free(maxes); maxes=NULL;
	free(counts); counts=NULL;
	for(int i=0; i<max; i++)
	{
		free(buckets[i]); buckets[i]=NULL;
	}
	free(buckets);
}*/
		
		
		
int HashMap::GetHash(Node* n)
{
	int result=0;
	for(string::size_type i=0; i<n->sid.size(); ++i)
		result+=n->sid[i];
	return result%max;
}

int HashMap::GetHash(const char* name)
{
	int counter=0;
	int result=0;
	while(1)
	{
		if(name[counter]==0)
			break;
		result+=name[counter++];
	}
	return result%max;
}

Node* HashMap::Find(const char* name)
{
	int hash = GetHash(name);
	debug(9, "hash: %d", hash);
	for(int i=0; i<counts[hash]; i++)
	{
		debug(9, "hash=%d, i=%d, b[h][i]=%s, maxes[h]=%d", hash, i, buckets[hash][i]->sid.c_str(), maxes[hash]);
		if(strcmp(buckets[hash][i]->sid.c_str(), name)==0)
			return buckets[hash][i];
	}
	return NULL;
}





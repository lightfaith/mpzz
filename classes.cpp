#include "classes.hpp"

Node::Node(char* sid)
{
	//set ID
	this->sid=sid;
	debug(5, "Creating node %s", sid);
	//set NULL predecessor and "infinite" distance
	predecessor=NULL;
	hopcount=INT_MAX;
	used=false;
	qi=NULL;
}

Node::~Node()
{
}

void Node::Print()
{
	//print node id
	debug(2, "  Node %s", sid);
	//print neighbors
	for(int i=0; i<neighbors->count; i++)
		debug(2, "    Neighbor %s, distance %Lf", neighbors->nodes[i]->sid, neighbors->metrics[i]);
}

void Node::SetPredecessor(Node* n, long double metric)
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
	if(hopcount==0) //root
		return NULL;
	//prepare array of pointers
	Node** result = (Node**)malloc(hopcount*sizeof(Node*));
	memory->Add(result, MALLOC, true, "Cannot get path for node.");
	//start with this node
	Node* p = this;
	debug(5, "    Getting path for %s:", p->sid);
	for(int i=hopcount-1; i>=0; i--)
	{
		debug(5, "      hop #%d: %s", hopcount-i-1, p->sid);
		//add to list
		result[i]=p;
		//move to predecessor
		p=p->predecessor;
	}
	if(p->predecessor!=NULL)
		debug(3, "  Some problem with getpath()...");

	p=NULL;	
	//return path
	return result;
}


Connections::Connections()
{
	debug(5, " Creating connections...");
	//set default max and count
	max=16;
	count=0;
	nodes = (Node**)malloc(max*sizeof(Node*));
	memory->Add(nodes, MALLOC, false, "Cannot create list of neighbors.");
	metrics = (long double*)malloc(max*sizeof(long double));
	memory->Add(metrics, MALLOC, false, "Cannot create list of neighbor distances.");
}


void Connections::Add(Node* n, long double m)
{
	debug(5, " Adding new connection to %s with metric of %Lf", n->sid, m);
	//resize if necessary
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
	long double* newmetrics = (long double*)malloc(max*sizeof(long double));
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
}

Nodes::Nodes()
{
	debug(5, "Node list is being created...");
	//set max and count
	max=64;
	count=0;
	//allocate array of nodes
	nodes = (Node**)malloc(max*sizeof(Node*));
	memory->Add(nodes, MALLOC, true, "Cannot create node list.");
	//create hashmap
	hashmap = new (nothrow) NodeHashMap(100);
	memory->Add(hashmap, NEW, true, "Cannot create HashMap.");
}

Nodes::~Nodes()
{
	debug(9, "Node list is being destroyed...");
}

void Nodes::Add(Node *n)
{
	debug(5, " Adding new node into list: %s", n->sid);
	//resize if necessary
	if(count>=max-1)
		More();
	nodes[count]=n;
	hashmap->Add(n);
	count++;
}

void Nodes::More()
{
	debug(9, "Resizing node list from %d to %d", max, max*2);
	max*=2;
	Node** newnodes = (Node**)malloc(max*sizeof(Node*));
	memory->Add(newnodes, MALLOC, true, "Cannot resize node list");
	for(int i=0; i<count; i++)
		newnodes[i]=nodes[i];
	memory->Free(nodes);
	nodes = newnodes;
}

void Nodes::Print()
{
	for(int i=0; i<count; i++)
	{
		nodes[i]->Print();
	}
}

Node* Nodes::Find(const char* sid)
{
	//use hashmap for search
	return hashmap->Find(sid);
}

void Nodes::Reindex()
{
	//resize hash map (count/2 => cca 10-25% of total time for Find(), which is probably fine...)
	debug(5, "Reindexing hash map.");
	NodeHashMap* newhashmap = new (nothrow) NodeHashMap(count/2);
	memory->Add(newhashmap, NEW, true, "Cannot reindex hashmap.");
	for(int i=0; i<count; i++)
		newhashmap->Add(nodes[i]);
	memory->Free(hashmap);
	hashmap=newhashmap;
	debug(5, "New hashmap distribution:");
	for(int i=0; i<hashmap->max; i++)
		debug(5, "  Bucket %d: %d", i, hashmap->counts[i]);
}

void Nodes::SPF(Node* root, const char* filename)
{
	//list of used nodes
	Node** used = (Node**)malloc(count*sizeof(Node*));
	memory->Add(used, MALLOC, true, "Cannot prepare 'used' list for SPF.");
	//priority queue of usable (reachable) nodes
	Queue* usable = new (nothrow) Queue();
	memory->Add(usable, NEW, true, "Cannot prepare 'usable' queue for SPF.");
	//number of used
	int usedcount=0;
	
	debug(1, "SPF Algorithm for root '%s' started.", root->sid);
	
	//mark root usable
	usable->Add(root, NULL, 0);
	usable->recent->node->SetPredecessor(NULL, 0);
	
	debug(6, "  Root marked usable.");

	//move best into used, its neighbors into usable etc...
	//until no usable exists (finished or separated nodes)
	while(!usable->Empty())
	{
		//find best
		usable->Print();
		QueueItem* best=usable->Get();
		
		if(best!=NULL)
		{
			debug(4, "  Found best: %s (metric %Lf).", best->node->sid, best->metric);
		
			//add it into used
			used[usedcount]=best->node;
			best->node->used=true;
		
			//add neighbors into usable / update metric
			for(int i=0; i<used[usedcount]->neighbors->count; i++)
			{
				//neighbor
				Node* nei = used[usedcount]->neighbors->nodes[i];
				//its metric
				long double met = used[usedcount]->neighbors->metrics[i];

				//if neighbor not used
				if(!nei->used)
				{
					//try to update metric / add
					if(usable->Update(nei, best->node, met+best->metric))
					{
						debug(6, "    %s is new predecessor of %s.", best->node->sid, nei->sid);
					}
				}
			}
			usedcount++;
			memory->Free(best);
			debug(6, "");
		}
	}
	
	//paths have been found, print results into file
	debug(1, "Paths have been found. Writing output file...");
	FILE *f = fopen(filename, "w");
	if(f==NULL)
	{
		printf("E: Cannot open output file '%s'. Exiting...", filename);
		delete memory;
		exit(1);
	}
	
	for(int i=0; i<usedcount; i++)
	{
		//get path
		Node** path = used[i]->GetPath();
		
		//print metric and root
		fprintf(f, "(%s)->(%s) = %Lf: (%s)", root->sid, used[i]->sid, used[i]->totalmetric, root->sid);
		debug(3, "  Writing path for %s (hopcount=%d, metric=%Lf)", used[i]->sid, used[i]->hopcount, used[i]->totalmetric);
		//print following nodes
		for(int j=0; j<used[i]->hopcount; j++)
			if(path[j]!=NULL)
				fprintf(f, "->(%s)", path[j]->sid);
		fprintf(f, "\n");
	}
	
	debug(3, "Found paths to %d/%d nodes.", usedcount,count);
	//not every node in used?
	if(usedcount<count)
		fprintf(f, "It seems that some nodes are not connected to '%s'. They are not shown in this file...\n", root->sid);
	
	fclose(f);
	debug(1, "SPF is complete. Check '%s' for results.", filename);
	memory->Free(used);
	memory->Free(usable);
}



NodeHashMap::NodeHashMap(int max)
{
	this->max=max;
	//create array of max bucket sizes
	maxes = (int*)malloc(max*sizeof(int));
	memory->Add(maxes, MALLOC, true, "Cannot create 'maxes' list for HashMap.");
	//create array of actual bucket sizes
	counts = (int*)malloc(max*sizeof(int));
	memory->Add(counts, MALLOC, true, "Cannot create 'counts' list for HashMap.");
	//create hashmap
	buckets = (Node***)malloc(max*sizeof(Node**));
	memory->Add(buckets, MALLOC, true, "Cannot create 'buckets' list for HashMap.");
	int maxesvalue=3;
	//create buckets, init maxes and counts
	for(int i=0; i<max; i++)
	{
		maxes[i]=maxesvalue;
		counts[i]=0;
		buckets[i]=(Node**)malloc(maxesvalue*sizeof(Node*));
		memory->Add(buckets[i], MALLOC, true, "Cannot create bucket for HashMap.");
	}
}

void NodeHashMap::Add(Node* n)
{
	debug(5, " Adding new node into hashmap.");
	int hash = GetHash(n);
	//resize if needed
	if(counts[hash]>=maxes[hash]-1)
		More(hash);
	buckets[hash][counts[hash]]=n;
	counts[hash]++;
}


void NodeHashMap::More(int bucket)
{
	debug(9, "Resizing hashmap bucket %d from %d to %d", bucket, maxes[bucket], maxes[bucket]*2);
	maxes[bucket]*=2;
	Node** newbucket = (Node**)malloc(maxes[bucket]*sizeof(Node*));
	memory->Add(newbucket, MALLOC, true, "Cannot resize bucket in HashMap.");
	for(int i=0; i<counts[bucket]; i++)
		newbucket[i]=buckets[bucket][i];
	memory->Free(buckets[bucket]);
	buckets[bucket]=newbucket;
}

int NodeHashMap::GetHash(Node* n)
{
	return GetHash(n->sid);
}

int NodeHashMap::GetHash(const char* name)
{
 	//djb2: http://www.cse.yorku.ca/~oz/hash.html
	int result=5381;
	int counter=0;
	while(1)
	{
		if(name[counter]==0)
			break;
		result=result*33+name[counter];
		counter++;
	}
	return result%max;
}

Node* NodeHashMap::Find(const char* name)
{
	int hash = GetHash(name);
	//for every node in bucket
	for(int i=0; i<counts[hash]; i++)
	{
		if(strcmp(buckets[hash][i]->sid, name)==0)
			return buckets[hash][i];
	}
	debug(4, "Node '%s' could not be found.", name);
	return NULL;
}

// - - - - - - - - - - - - - - - - -
QueueItem::QueueItem(Node* node, long double metric, QueueItem* prev, QueueItem* next)
{
	this->node=node;
	this->metric=metric;
	this->prev=prev;
	this->next=next;
}

// - - - - - - - - - - - - - - - - - - - - 

Queue::Queue()
{
	prehead = new (nothrow) QueueItem(NULL, 0, NULL, NULL);
	memory->Add(prehead, NEW, true, "Cannot create queue because of memory issuses.");
	recent=NULL;
	count=0;
}

Queue::~Queue()
{
	//free every queueitem
	QueueItem* i = prehead;
	QueueItem* old;
	while(1)
	{
		old=i;
		if(old==NULL)
			break;
		i=i->next;	
		memory->Free(old);
	}	
}

bool Queue::Empty()
{
	return count==0;
}

void Queue::Add(Node* node, Node* predecessor, long double metric)
{
	QueueItem* prev = prehead;
	QueueItem* next;
	QueueItem* qi;
	
	//find correct position based on metric
	while(1)
	{
		next=prev->next;
		//at the end => worst
		if(next==NULL)
		{
			qi = new (nothrow) QueueItem(node, metric, prev, NULL);
			break;
		}
		//better than next
		if(metric<next->metric)
		{
			qi = new (nothrow) QueueItem(node, metric, prev, next);
			break;
		}
		prev=prev->next;
	}
	//alter those pointers 
	if(prev!=NULL)
		prev->next=qi;
	if(next!=NULL)
		next->prev=qi;
	//update info
	node->SetPredecessor(predecessor, metric);
	memory->Add(qi, NEW, true, "Cannot create queue item!");
	
	//remember
	recent=qi;
	//set node qi (can be found in O(1))
	node->qi=qi;
	count++;
}

QueueItem* Queue::Get()
{
	//return first (after prehead)
	QueueItem* result = prehead->next;
	if(prehead->next!=NULL)
	{
		//alter those pointers
		if(prehead->next->next!=NULL)
			prehead->next->next->prev=prehead;
		prehead->next=prehead->next->next;
	}
	count--;
	if(result==NULL)
		debug(6, "Returning null item from queue.");
	return result;
}

bool Queue::Update(Node* node, Node* predecessor, long double metric)
{
	debug(7, "   Trying to update metric of node %s to %Lf", node->sid, metric);
	if(node->qi==NULL) //new, cannot update
	{
		Add(node, predecessor, metric);
		return true;
	}
	
	if(node->qi->metric<metric) //new metric is worse, end
	{
		debug(6, "    Not updating metric for %s, worse (%Lf).", node->sid, metric);
		return false;
	}
	
	//update pointers for neighbors (set actual queueitem free)
	QueueItem* newprev=node->qi->prev;
	if(node->qi->prev!=NULL)
		node->qi->prev->next=node->qi->next;
	if(node->qi->next!=NULL)
		node->qi->next->prev=node->qi->prev;

	while(newprev!=NULL)
	{
		if(newprev->metric>metric)
		{
			//holding better metric, keep searching
			newprev=newprev->prev;
			continue;
		}
		else
		{
			//found, link queue items together
			node->qi->next=newprev->next;
			node->qi->prev=newprev;
			newprev->next=node->qi;
			if(node->qi->next!=NULL)
				node->qi->next->prev=node->qi;

			//update data
			node->SetPredecessor(predecessor, metric);
			node->qi->metric=metric;
			return true;
		}
	}
	return true;
}

void Queue::Print()
{
	debug(6, "  Actual queue:");
	QueueItem* q = prehead->next;
	while(q!=NULL)
	{
		debug(6, "    Node %s, metric %Lf", q->node->sid, q->metric);
		q=q->next;
	}
}

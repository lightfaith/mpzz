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
		debug(2, "    Neighbor %s, distance %.3f", neighbors->nodes[i]->sid, neighbors->metrics[i]);
}

void Node::SetPredecessor(Node* n, double metric)
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
	debug(5, "Getting path for %s:", p->sid);
	for(int i=hopcount-1; i>=0; i--)
	{
		debug(5, "  hop #%d: %s", hopcount-i-1, p->sid);
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
	metrics = (double*)malloc(max*sizeof(double));
	memory->Add(metrics, MALLOC, false, "Cannot create list of neighbor distances.");
}

Connections::~Connections()
{
}

void Connections::Add(Node* n, double m)
{
	debug(5, "Adding new connection to %s with metric of %.3f", n->sid, m);
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
	double* newmetrics = (double*)malloc(max*sizeof(double));
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
	hashmap = new NodeHashMap(100);
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
	NodeHashMap* newhashmap = new NodeHashMap(count/2);
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
	//list of usable (reachable) nodes
	//Node** usable = (Node**)malloc(count*sizeof(Node*));
	//memory->Add(usable, MALLOC, true, "Cannot prepare 'usable' list for SPF.");
	Queue* usable = new Queue();
	memory->Add(usable, NEW, true, "Cannot prepare 'usable' queue for SPF.");
	//metric for usable nodes
	//float* usablem = (float*)malloc(count*sizeof(float));
	//memory->Add(usablem, MALLOC, true, "Cannot prepare 'usablem' list for SPF.");
	//number of used
	int usedcount=0;
	//number of usable
	//int usablecount=0;
	//int bestid=0; //best node from usables
	//float bestmetric; //metric of best node
	
	debug(1, "SPF Algorithm for root '%s' started.", root->sid);
	
	//mark root usable
	//usablem[usablecount]=0;
	usable->Add(root, NULL, 0);
	//usable[usablecount]=root;
	//usable[usablecount]->SetPredecessor(NULL, 0);
	usable->recent->node->SetPredecessor(NULL, 0);
	//usablecount++;
	debug(9, "Root marked usable.");

	//move best into used, its neighbors into usable etc...
	//until no usable exists (finished or separated nodes)
	//while(usablecount>0)
	while(!usable->Empty())
	{
		//find best
		/*bestmetric=usablem[0];
		for(int i=0; i<usablecount/2+1; i++)
		{
			if(usablem[i]<=bestmetric)
			{
				bestmetric=usablem[i];
				bestid=i;
			}
			if(usablem[usablecount-i-1]<=bestmetric)
			{
				bestmetric=usablem[usablecount-i-1];
				bestid=usablecount-i-1;
			}
		}
		*/
		usable->Print();
		QueueItem* best=usable->Get();
		//debug(4, "  Found best: %s (metric %lf).", usable[bestid]->sid, bestmetric);
		if(best!=NULL)
		{
			debug(4, "  Found best: %s (metric %lf).", best->node->sid, best->metric);
		
		//add it into used
		//used[usedcount]=usable[bestid];
		used[usedcount]=best->node;
		best->node->used=true;
		//remove from usable
		//usable[bestid]=usable[usablecount-1];
		//usablem[bestid]=usablem[usablecount-1];
		//usablecount--;
		
		/*debug(6, "    Usables before update:");
		for(int i=0; i<usable[usablecount]; i++)
		{
			debug(6, "      %s (metric %lf)", usable[i]->sid, usablem[i]);
		}
		*/
		//add neighbors into usable / update metric
			for(int i=0; i<used[usedcount]->neighbors->count; i++)
			{
				//neighbor
				Node* nei = used[usedcount]->neighbors->nodes[i];
				//its metric
				float met = used[usedcount]->neighbors->metrics[i];

			/*for(int j=0; j<usablecount; j++)
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
			}*/
			
				//if neighbor not used
				if(!nei->used)
				{
					//try to update metric
					if(usable->Update(nei, best->node, met+best->metric))
					{
						debug(6, "%s is predecessor of %s.", best->node->sid, nei->sid);
					}
				}
			}
		/*	
			//still not found? add into usables :)
			if(!found)
			{
				usablem[usablecount]=bestmetric+met;
				usable[usablecount]=nei;
				usable[usablecount]->SetPredecessor(used[usedcount], usablem[usablecount]);
				usablecount++;
			}
		*/
		
		
		debug(6, "    Usables after update:");
		usable->Print();
		/*for(int i=0; i<usablecount; i++)
		{
			debug(6, "      %s (metric %lf)", usable[i]->sid, usablem[i]);
		}
		*/
		usedcount++;
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
		fprintf(f, "(%s)->(%s) = %.3f: (%s)", root->sid, used[i]->sid, used[i]->totalmetric, root->sid);
		debug(3, "  Writing path for %s (hopcount=%d, metric=%.3f)", used[i]->sid, used[i]->hopcount, used[i]->totalmetric);
		//print following nodes
		for(int j=0; j<used[i]->hopcount; j++)
			if(path[j]!=NULL)
				fprintf(f, "->(%s)", path[j]->sid);
		fprintf(f, "\n");
	}
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


NodeHashMap::~NodeHashMap()
{
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
QueueItem::QueueItem(Node* node, double metric, QueueItem* prev, QueueItem* next)
{
	this->node=node;
	this->metric=metric;
	this->prev=prev;
	this->next=next;
}


void QueueItem::Update(double metric)
{
	this->metric=metric;
}

// - - - - - - - - - - - - - - - - - - - - 
Queue::Queue()
{
	prehead = new QueueItem(NULL, 0, NULL, NULL);
	memory->Add(prehead, NEW, true, "Cannot create queue because of memory issuses.");
	//head=NULL;
	recent=NULL;
	count=0;
}

Queue::~Queue()
{
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

void Queue::Add(Node* node, Node* predecessor, double metric)
{
	QueueItem* prev = prehead;
	QueueItem* next;
	QueueItem* qi;
	
	while(1)
	{
		next=prev->next;
		//at the end => worst
		if(next==NULL)
		{
			qi = new QueueItem(node, metric, prev, NULL);
			break;
		}
		//better than next
		if(metric<next->metric)
		{
			qi = new QueueItem(node, metric, prev, next);
			break;
		}
		prev=prev->next;
	}
	if(prev!=NULL)
		prev->next=qi;
	if(next!=NULL)
		next->prev=qi;
	node->SetPredecessor(predecessor, metric);
	memory->Add(qi, NEW, true, "Cannot create queue item!");
	recent=qi;
	count++;
	node->qi=qi;
}

QueueItem* Queue::Get()
{
	QueueItem* result = prehead->next;
	if(prehead->next!=NULL)
	{
		if(prehead->next->next!=NULL)
			prehead->next->next->prev=prehead;
		prehead->next=prehead->next->next;
	}
	count--;
	if(result==NULL)
		debug(6, "Returning null item from queue.");
	else
		debug(6, "Returning item %s (metric %lf) from queue.", result->node->sid, result->metric);
	return result;
}

bool Queue::Update(Node* node, Node* predecessor, double metric)
{
	debug(7, "Trying to update metric of node %s to %lf", node->sid, metric);
	if(node->qi==NULL) //new, cannot update
	{
		Add(node, predecessor, metric);
		return true;
	}
	
	if(node->qi->metric<metric)
	{
		debug(6, "Not updating metric, worse.");
		return false;
	}
	
	QueueItem* newprev=node->qi->prev;
	if(node->qi->prev!=NULL)
		node->qi->prev->next=node->qi->next;
	if(node->qi->next!=NULL)
		node->qi->next->prev=node->qi->prev;
	while(newprev!=NULL)
	{
		if(newprev->metric>metric)
		{
			newprev=newprev->prev;
			continue;
		}
		else
		{
			node->qi->next=newprev->next;
			node->qi->prev=newprev;
			newprev->next=node->qi;
			if(node->qi->next!=NULL)
				node->qi->next->prev=node->qi;
			node->SetPredecessor(predecessor, metric);
			return true;
		}
	}
	/*{
		
		if(newprev->next==NULL) //end?? this is bad
		{
			debug(6, "Cannot update queue, something is broken.");
			return false;
		}
		
		if(newprev->next->node==node) //found myself, just update metric&pred
		{
			newprev->next->metric=metric;
			node->SetPredecessor(predecessor, metric);
			debug(6, "Node metric updated to %lf (queue not altered).", metric);
			return true;
		}
		if(newprev->next->metric<metric) 
		{
			newprev=newprev->next;
			continue;
		}
		
		//if(newprev->next->node!=node) //searching for new place
		//	newprev=newprev->next;
		//if(prev->next->node!=node) //searching for actual place, else go for next
		//{
		//	prev=prev->next;
		//	continue;
		//}
		
		//found everything
		//debug(1, "Qiparent of %s = qi of %s", node->sid, node->qiparent->node->sid);
		QueueItem* actual=node->qiparent->next;
		node->qiparent->next=actual->next;
		actual->next=newprev->next;
		newprev->next=actual;
		
		//prev->next=prev->next->next;
		//actual->next=newprev->next;
		//newprev->next=actual;
		actual->node->SetPredecessor(predecessor, metric);
		debug(6, "Node metric updated to %lf (and queue altered).", actual->metric);
		break;
	}
	*/
	return true;
}

void Queue::Print()
{
	debug(6, "Actual queue:");
	QueueItem* q = prehead->next;
	while(q!=NULL)
	{
		debug(6, "  Item %s, metric %lf", q->node->sid, q->metric);
		q=q->next;
	}
}

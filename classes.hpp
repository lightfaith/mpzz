#ifndef PRIMO_CLASSES_H
#define PRIMO_CLASSES_H

#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include "lib.hpp"

using namespace std;

class Node;
class Connections;
class Nodes;
class NodeHashMap;
class QueueItem;
class Queue;

class Node : Generic
{
	public:
		char* sid; //ID of node
		Connections* neighbors; 
		Node* predecessor; //pointer to predecessor (towards root)
		int hopcount; //number of nodes to root 
		double totalmetric; //distance from root
		bool used; //used in SPF
		QueueItem* qi; //used in SPF

		Node(char* sid);
		~Node();
		void Print();
		void SetPredecessor(Node* n, double metric);
		Node** GetPath(); // get all nodes from root to this Node
};


class Connections : Generic
{
	private:
		int max; //max number of nodes
	public:
		int count; //actual number of nodes
		Node** nodes; //array of pointers to nodes
		double* metrics; //array of distance to each node
		
		Connections();
		~Connections();
		void Add(Node* n, double m);
		void More(); //resizing method
};


class Nodes : Generic
{
	private:
		int max; //maximum number of all nodes
	public:
		int count; //actual number of all nodes
		Node** nodes; //list of nodes
		NodeHashMap* hashmap; //hashmap of nodes
		
		Nodes();
		~Nodes();
		void Add(Node* n);
		void More(); //resizing method
		void Print();
		Node* Find(const char* sid); //search by ID method
		void Reindex(); //rebuild hashmap based on node count
		void SPF(Node* root, const char* filename); //this is the black magic
};

class NodeHashMap : Generic
{
	private:
		int* maxes; //max size of buckets
	public:
		int max; //max number of buckets
		int* counts; //actual size of buckets
		Node*** buckets; //hashmap of pointers to nodes
		
		NodeHashMap(int max);
		~NodeHashMap();
		void Add(Node* n);
		void More(int bucket); //resizing function
		void Free(); 
		int GetHash(Node* n); //hash computation method
		int GetHash(const char* name); //hash computation method
		Node* Find(const char* name); //search by ID method
};

class QueueItem : Generic
{
	public:
		Node* node;
		double metric;
		QueueItem* prev;
		QueueItem* next;

		QueueItem(Node* node, double metric, QueueItem* prev, QueueItem* next);
		//~QueueItem();
		void Update(double metric);
};

class Queue : Generic
{
	private:
		//QueueItem* head;
		QueueItem* prehead;
	public:
		int count;
		QueueItem* recent;

		Queue();
		~Queue();
		bool Empty();
		void Add(Node* node, Node* predecessor, double metric);
		QueueItem* Get();
		bool Update(Node* node, Node* predecessor, double metric);
		void Print();
};
#endif

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

class Node : Generic
{
	public:
		//int id;
		char* sid;
		Connections* neighbors;
		Node* predecessor;
		int hopcount;
		float totalmetric;
		
		Node(char* sid);
		~Node();
		void Print();
		void SetPredecessor(Node* n, float metric);
		Node** GetPath();
};


class Connections : Generic
{
	private:
		int max;
	public:
		int count;
		Node** nodes;
		float* metrics;
		
		Connections();
		~Connections();
		void Add(Node* n, float m);
		void More();
};


class Nodes : Generic
{
	private:
		int max;
	public:
		int count;
		Node** nodes;
		NodeHashMap* hashmap;
		Nodes();
		~Nodes();
		
		void Add(Node* n);
		void More();
		void Print();
		Node* Find(const char* sid);
		void SPF(Node* root, const char* filename);
};

class NodeHashMap : Generic
{
	private:
		int max;
		int* maxes;
	public:
		//int count; //max automatically
		int* counts;
		Node*** buckets;
		
		NodeHashMap(int max);
		~NodeHashMap();
		void Add(Node* n);
		void More(int bucket);
		void Free();
		int GetHash(Node* n);
		int GetHash(const char* name);
		Node* Find(const char* name);
};
#endif

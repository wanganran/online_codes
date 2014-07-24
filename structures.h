#ifndef __STRUCTURES_H
#define __STRUCTURES_H
#include <set>

template<typename T>
struct block{
	T* data;
	int node_num;
	int node_xor;
};
/*
template<typename T>
struct node{
	T* val;
	node* next;
};*/
#endif
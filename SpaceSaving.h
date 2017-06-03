#ifndef _SPACESAVING_H
#define _SPACESAVING_H

#include <cstdio>
#include <cstring>
#include "params.h"
#include "SpaceSavingHeap.h"
using namespace std;

class SpaceSaving
{
public:
	SpaceSavingHeap heap;
	
	SpaceSaving(uint _k, uint _hsize);
	~SpaceSaving();
	
	void insert(const char *key);
	void GetTopK(char **Ans) const;
	void GetFreq(uint limit, char **Ans) const;
	uint query(const char *key) const;
	
private:
	SpaceSaving();
	SpaceSaving(const SpaceSaving &);
	SpaceSaving &operator =(const SpaceSaving &);
};


SpaceSaving :: 
SpaceSaving(uint _k, uint _hsize): heap(_k, _hsize){
}

SpaceSaving :: 
~SpaceSaving(){
}

void SpaceSaving ::
insert(const char *key){
	char *tkey = new char[100 + 1];	
	strcpy(tkey, key);
	heap.insert(tkey);
}

void SpaceSaving ::
GetTopK(char **Ans) const{
	heap.GetTopK(Ans);
}

void SpaceSaving ::
GetFreq(uint limit, char **Ans) const{
	heap.GetFreq(limit, Ans);
}

uint SpaceSaving ::
query(const char *key) const{
	return heap.query(key);
}

#endif//_SPACESAVING_H
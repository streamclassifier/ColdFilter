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
	
	void Insert(uint key, int f);
	void GetTopK(char **Ans) const;
	void GetFreq(uint limit, char **Ans) const;
	uint Query(const char *key) const;
	
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
Insert(uint key, int f){
	// char *tkey = new char[100 + 1];	
	// strcpy(tkey, key);
	heap.insert(key, f);
}

void SpaceSaving ::
GetTopK(uint *Ans) const{
	heap.GetTopK(Ans);
}

void SpaceSaving ::
GetFreq(uint limit, uint *Ans) const{
	heap.GetFreq(limit, Ans);
}

uint SpaceSaving ::
Query(uint key) const{
	return heap.Query(key);
}

#endif//_SPACESAVING_H
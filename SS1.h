#ifndef _SS1_H
#define _SS1_H

#include <algorithm>
#include <cstring>
#include <string.h>
#include "BOBHash64.h"
#include <iostream>
#include <string>
#include "params.h"
#include "SPA.h"

using namespace std;

class SS1: public SPA
{
private:
	uint *ID;
	int *counter;

	int K;
	int heapsize;

public:
	SS1(int _K, int _heapsize)
	{
		K = _K;
		heapsize = _heapsize;

		ID = new uint[heapsize];
		counter = new int[heapsize];

		memset(ID, 0, sizeof(uint) * heapsize);
		memset(counter, 0, sizeof(int) * heapsize);
	}
	void Insert(uint key, int f, int seq)
	{
		int min_value = 1 << 30;
		int min_index = 0;
		bool FLAG = false; 

		for(int i = 0; i < heapsize; i++)
		{
			if(ID[i] == key)
			{
				counter[i] += f;
				return;
			}
		}
		for(int i = 0; i < heapsize; i++)
		{
			if(ID[i] == 0)
			{
				ID[i] = key;
				counter[i] += f;
				return;	
			}
		}
		for(int i = 0; i < heapsize; i++)
		{
			if(counter[i] < min_value) 
			{
				min_value = counter[i]; 
				min_index = i;
			}
		}
		ID[min_index] = key;
		counter[min_index] += f;
		return;
	}
	uint * Get_ID()
	{
		return ID;
	}
	int * Get_count()
	{
		return counter;
	}
	~SS1()
	{
		delete []counter;
		delete []ID;
	}
};
#endif//_SS1_H

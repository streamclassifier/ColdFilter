#ifndef _SS1_H
#define _SS1_H

#include <algorithm>
#include <cstring>
#include <string.h>
#include "BOBHash64.h"
#include <iostream>
#include <string>
#include "params.h"

using namespace std;

class SS1
{
private:
	uint *ID;
	int *counter;
	int d;

public:
	SS1(int _d)
	{
		d = _d;
		ID = new uint[d];
		counter = new int[d];

		memset(ID, 0, sizeof(uint) * d);
		memset(counter, 0, sizeof(int) * d);
	}
	void Insert(uint ip)
	{
		int min_value = 1 << 30;
		int min_index = 0;
		bool FLAG = false; 

		for(int i = 0; i < d; i++)
		{
			if(ID[i] == ip)
			{
				counter[i] ++;
				return;
			}
		}
		for(int i = 0; i < d; i++)
		{
			if(ID[i] == 0)
			{
				ID[i] = ip;
				counter[i] ++;
				return;	
			}
		}
		for(int i = 0; i < d; i++)
		{
			if(counter[i] < min_value) 
			{
				min_value = counter[i]; 
				min_index = i;
			}
		}
		ID[min_index] = ip;
		counter[min_index] ++;
		return;
	}
	uint Query(const int x)
	{
		return ID[x];
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

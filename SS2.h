#ifndef _SS2_H
#define _SS2_H

#include <algorithm>
#include <cstring>
#include <string.h>
#include "params.h"
#include "BOBHash64.h"
#include "BOBHash32.h"
#include <iostream>
#include <string>

using namespace std;

class SS2
{
private:
	BOBHash32 * bobhash[MAX_HASH_NUM];
	int index[MAX_HASH_NUM];
	int *counter_l[MAX_HASH_NUM];

	int *counter;
	uint *ID;

	int d, d_l, w_l;
	uint64_t hash_value;
	int MAX_CNT;
public:
	SS2(int _d, int _y, int _x, int cs)
	{
		d_l = _x; 
		w_l = _y;
		d = _d;
		MAX_CNT = (1 << cs) - 1;

		ID = new uint[d];
		counter = new int[d];

		memset(ID, 0, sizeof(uint) * d);
		memset(counter, 0, sizeof(int) * d);

		for(int i = 0; i < d_l; i++)
		{
			counter_l[i] = new int[w_l];
			memset(counter_l[i], 0, sizeof(int) * w_l);
		}
		for(int i = 0; i < d_l; i++)
		{
			bobhash[i] = new BOBHash32(i + 1000);
		}
	}
	void Insert(uint ip)
	{	
		int min_value = 1 << 30;
		int temp2;

		for(int i = 0; i < d_l; i++)
		{
			index[i] = (bobhash[i]->run((const char *)(&ip), 8)) % w_l;
			temp2 = counter_l[i][index[i]];
			min_value = temp2 < min_value ? temp2 : min_value;
		}
		if (min_value != MAX_CNT)
		{
			for(int i = 0; i < d_l; i++)
			{
				if(counter_l[i][index[i]] == min_value)
					counter_l[i][index[i]] ++;
			}
		} 
		else
		{
			min_value = 1 << 30;
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
	}
	uint Query(const int x)
	{
		return ID[x];
	}
	int * Get_count()
	{
		for(int i = 0; i < d; i++)
		{
			counter[i] += MAX_CNT;
		}
		return counter;
	}
	~SS2()
	{
		delete []counter;
		delete []ID;
		for(int i = 0; i < d_l; i++)
		{
			delete []counter_l[i];
		}
		for(int i = 0; i < d_l; i++)
		{
			delete []bobhash[i];
		}
	}
};
#endif//_SS2_H

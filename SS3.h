#ifndef _SS3_H
#define _SS3_H

#include <algorithm>
#include <cstring>
#include <string.h>
#include "params.h"
#include "BOBHash64.h"
#include "BOBHash32.h"
#include <iostream>
#include <string>

#define GetHigh(a, x) ((a >> x) & 1)

using namespace std;

class SS3
{
private:
	BOBHash32 * bobhash[MAX_HASH_NUM];
	int index[MAX_HASH_NUM];
	int *counter_l[MAX_HASH_NUM];
	int *counter;
	string *HASH;
	int d, d_l, w_l, T;
	uint64_t hash_value;
	int MAX_CNT;
	int counter_size;
public:
	SS3(int _d, int _y, int _x, int cs, int threshold)
	{
		d_l = _x; 
		w_l = _y;
		d = _d;
		T = threshold;
		counter_size = cs;
		MAX_CNT = (1 << cs) - 1;

		HASH = new string[d];
		counter = new int[d];
		for (int i = 0; i < d; i++) 
			HASH[i] = "";
		memset(counter, 0, sizeof(int) * d);

		for(int i = 0; i < d_l; i++)
		{
			counter_l[i] = new int[w_l];
			memset(counter_l[i], 0, sizeof(int) * w_l);
		}
		for(int i = 0; i < d_l * 2; i++)
		{
			bobhash[i] = new BOBHash32(i + 1000);
		}
	}
	int get_sim(int a, int b)
	{
		int cnt = 0;
		for(int i = counter_size - 1; i >= 0; i--)
		{
			if(GetHigh(a, i) == GetHigh(b, i))
			{
				cnt++;
			}
			else
			{
				break;
			}
		}	
		return cnt;
	}
	void set_diff(int & a, int & b)
	{
		for(int i = 0; i < counter_size; i++)
		{
			if(GetHigh(a, i) != GetHigh(b, i))
			{
				a = ((a >> (i + 1)) << (i + 1)) & (b & (~((int64_t)(0xFFFFFFFFFFFFFFFF) << (i + 1))));	
				return;
			}
		}
	}
	int get_median(int * res, int d)
	{
		sort(res, res + d);
		int r;
		if(d % 2 == 0)
		{
			r = (res[d / 2] + res[d / 2 - 1]) / 2;
		}
		else
		{
			r = res[d / 2];
		}
		return r;
	}
	void Insert(const char * str)
	{	
		int min_sim = 1 << 30;
		int temp2;
		int fp[MAX_HASH_NUM];
		int res[MAX_HASH_NUM];

		for(int i = 0; i < d_l; i++)
		{
			index[i] = (bobhash[i]->run(str, strlen(str))) % w_l;

			fp[i] = (bobhash[i + d_l]->run(str, strlen(str))) & (~((int64_t)(0xFFFFFFFFFFFFFFFF) << counter_size));
			// printf("%x\n", fp[i]);

			temp2 = counter_l[i][index[i]];
			// min_sim = temp2 < min_sim ? temp2 : min_sim;
			// min_sim = min(get_sim(fp[i], temp2), min_sim);
			res[i] = get_sim(fp[i], temp2);
		}
		min_sim = get_median(res, d_l);


		//larger min_sim means more likely big flow.
		if (min_sim <= T)
		{
			for(int i = 0; i < d_l; i++)
			{
				if(counter_l[i][index[i]] == 0)
				{
					counter_l[i][index[i]] = fp[i];
				}
				else
				{
					set_diff(counter_l[i][index[i]], fp[i]);
				}
			}
		}
		else
		{
			bool FLAG = false;
			int XX;
			string temp = string(str);
			for(int i = 0; i < d; i++)
			{
				if(counter[i] < min_sim) 
				{
					min_sim = counter[i]; 
					XX = i;
				}
				if(HASH[i] == temp) 
				{
					FLAG = true; 
					counter[i]++; 
					break;
				}
			}
			if(!FLAG) 
			{
				HASH[XX] = temp;
				counter[XX]++;
			}
		}
	}
	string Query(const int x)
	{
		return HASH[x];
	}
	int * Get_count()
	{
		for(int i = 0; i < d; i++)
		{
			counter[i] += MAX_CNT;
		}
		return counter;
	}
	~SS3()
	{
		delete []counter;
		delete []HASH;
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
#endif//_CUSketch_H

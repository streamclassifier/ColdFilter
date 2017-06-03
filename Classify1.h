#ifndef _CLASSIFY1_H
#define _CLASSIFY1_H

#include <algorithm>
#include <cstring>
#include <string.h>
#include "BOBHash32.h"
#include "BOBHash64.h"
#include <iostream>
#include <string>
#include <unordered_map>

#define LAST

class classify1
{
private:
	BOBHash32 * bobhash[MAX_HASH_NUM * 10];
	
	int w_low, d_low;
	int w_high, d_high;
	int low_counter_size, high_counter_size;

	int MAX_CNT_LOW;
	int MAX_CNT_HIGH;

	int * counter_low;
	int * counter_high;

	char last_flow_id[500];
	int last_counter_index;
	int last_identifier;//0->low_Layer, 1->high_layer, 2->elephant flow;
	int last_hash_index;

	unordered_map<string, int> unmp;

public:
	int N;

	int man_insert, man_query;
	classify1(int wl, int wh, int dl, int dh, int lcs, int hcs)
	{
		w_low = wl;
		w_high = wh;
		d_low = dl;
		d_high = dh;
		low_counter_size = lcs;
		high_counter_size = hcs;

		counter_low = new int[w_low];
		counter_high = new int[w_high];

		memset(counter_low, 0, sizeof(int) * w_low);
		memset(counter_high, 0, sizeof(int) * w_high);

		MAX_CNT_LOW = (1 << low_counter_size) - 1;
		MAX_CNT_HIGH = (1 << high_counter_size) - 1;

		for(int i = 0; i < d_low + d_high; i++)
		{
			bobhash[i] = new BOBHash32(i + 500);
		}

		last_flow_id[0] = '\0';
		last_counter_index = -1;
		last_identifier = -1;
		last_hash_index = -1;

		N = 0;
		unmp.clear();
	}
	void Insert(const char * str)
	{
		int index_low[MAX_HASH_NUM];
		int index_high[MAX_HASH_NUM];

#ifdef LAST
		if(strcmp(str, last_flow_id) == 0)
		{
			if(last_identifier == 0)
			{
				man_insert ++;
				if(counter_low[last_counter_index] != MAX_CNT_LOW)
				{
					counter_low[last_counter_index]++;
					return;	
				}
				else
				{
					for(int i = last_hash_index + 1; i < d_low; i++)
					{
						man_insert ++;
						index_low[i] = (bobhash[i]->run(str, strlen(str))) % w_low;
						if(counter_low[index_low[i]] != MAX_CNT_LOW)
						{
							counter_low[index_low[i]] ++;
							last_counter_index = index_low[i];
							last_hash_index = i;
							return;
						}
					}
				}
				for(int i = 0; i < d_high; i++)
				{
					man_insert ++;
					index_high[i] = (bobhash[i + d_low]->run(str, strlen(str))) % w_high;
					if(counter_high[index_high[i]] != MAX_CNT_HIGH)
					{
						last_identifier = 1;
						last_counter_index = index_high[i];
						last_hash_index = i;
						counter_high[index_high[i]] ++;
						return;
					}
				}
				last_identifier = 2;
			}
			else if(last_identifier == 1)
			{
				man_insert ++;
				if(counter_high[last_counter_index] != MAX_CNT_HIGH)
				{
					counter_high[last_counter_index]++;
					return;
				}
				else
				{
					for(int i = last_hash_index + 1; i < d_high; i++)
					{
						man_insert ++;
						index_high[i] = (bobhash[i + d_low]->run(str, strlen(str))) % w_high;
						if(counter_high[index_high[i]] != MAX_CNT_HIGH)
						{
							counter_high[index_high[i]] ++;
							last_counter_index = index_high[i];
							last_hash_index = i;
							return;
						}
					}
				}	
				last_identifier = 2;
			}
			else if(last_identifier == 2)
			{
				return;		
			}
			return;
		}
		strcpy(last_flow_id, str);
#endif

		for(int i = 0; i < d_low; i++)
		{
			man_insert ++;
			index_low[i] = (bobhash[i]->run(str, strlen(str))) % w_low;
			if(counter_low[index_low[i]] != MAX_CNT_LOW)
			{
				counter_low[index_low[i]] ++;
				last_identifier = 0;
				last_counter_index = index_low[i];
				last_hash_index = i;
				return;
			}
		}

		unmp[str] ++;
		N = unmp.size();

		for(int i = 0; i < d_high; i++)
		{
			man_insert ++;
			index_high[i] = (bobhash[i + d_low]->run(str, strlen(str))) % w_high;
			if(counter_high[index_high[i]] != MAX_CNT_HIGH)
			{
				counter_high[index_high[i]] ++;
				last_identifier = 1;
				last_counter_index = index_high[i];
				last_hash_index = i;
				return;
			}
		}
		last_identifier = 2;
		return;
	}

	//true -> elephant flow;
	bool Query(const char * str)
	{		
		int index_low[MAX_HASH_NUM];
		int index_high[MAX_HASH_NUM];

		for(int i = 0; i < d_low; i++)
		{
			man_query ++;
			index_low[i] = (bobhash[i]->run(str, strlen(str))) % w_low;
			if(counter_low[index_low[i]] != MAX_CNT_LOW)
				return false;
		}

		for(int i = 0; i < d_high; i++)
		{
			man_query ++;
			index_high[i] = (bobhash[i + d_low]->run(str, strlen(str))) % w_high;
			if(counter_high[index_high[i]] != MAX_CNT_HIGH)
				return false;
		}
		return true;
	}
};

#endif //_CLASSIFY1_H
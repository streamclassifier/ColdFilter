#ifndef _SKETCH_SIEVING_H
#define _SKETCH_SIEVING_H

#include <unordered_map>
#include <iterator>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <algorithm>
#include "BOBHash32.h"
#include "params.h"

using namespace std;

class sketch_sieving
{
private:
	double simple_rate;
	int threshold;
	int g;
	int m;
	unordered_map<unsigned int, int> * flow_size;
	unordered_map<unsigned int, int> * ele_flow;

	int * cusketch_mice;
	double memory_mice;
	int counter_size_mice;
	int MAX_COUNT;
	int d, cell_num_mice;
	
	int * cusketch_ele;
	double memory_ele;
	int counter_size_ele;
	int cell_num_ele;
	

	int ip_mice[50000];
	int cnt_mice;
	int ip_ele[5000];
	int cnt_ele;

	BOBHash32 * bobhash[MAX_HASH_NUM];

public:
	sketch_sieving(double sr, int _g)
	{
		simple_rate = sr;
		threshold = RAND_MAX * simple_rate;
		g = _g;
		m = 0;
		d = 3;
		cnt_ele = cnt_mice = 0;


		memory_mice = 1;
		counter_size_mice = 12;

		MAX_COUNT = (1 << counter_size_mice) - 1;

		cell_num_mice = memory_mice * 1024 * 1024 * 8 / counter_size_mice;
		
		cusketch_mice = new int[cell_num_mice];
		memset(cusketch_mice, 0, sizeof(int) * cell_num_mice);



		memory_ele = 0.1;
		counter_size_ele = 16;

		cell_num_ele = memory_ele * 1024 * 1024 * 8 / counter_size_ele;
		
		cusketch_ele = new int[cell_num_ele];
		memset(cusketch_ele, 0, sizeof(int) * cell_num_ele);



		srand(time(NULL));
		flow_size = new unordered_map<unsigned int, int>();
		ele_flow = new unordered_map<unsigned int, int>();

		for (int i = 0; i < d * 2; i++)
		{
			bobhash[i] = new BOBHash32(i + 500);
		}
	}
	
	bool is_sample()
	{
		int key = rand() % RAND_MAX + 1;

		if(key <= threshold)
			return true;

		return false;
	}

	int Insert_mice(unsigned int ip)
	{
		int index[MAX_HASH_NUM];
		int min_value = (1 << 30);
		int my_ip = ip;
		for(int i = 0; i < d; i++)
		{
			index[i] = (bobhash[i]->run((const char *) &my_ip, 4)) % cell_num_mice;
			int temp = cusketch_mice[index[i]];
			min_value = min_value < temp ? min_value : temp;
		}
		if(min_value == MAX_COUNT)
			return 1;

		for(int i = 0; i < d; i++)
		{
			if(cusketch_mice[index[i]] == min_value)
				cusketch_mice[index[i]]++;
		}
		return 0;
	}	
	int Query_mice(unsigned int ip)
	{
		int index[MAX_HASH_NUM];
		int my_ip = ip;
		int min_value = (1 << 30);
		for(int i = 0; i < d; i++)
		{
			index[i] = (bobhash[i]->run((const char *) &my_ip, 4)) % cell_num_mice;
			int temp = cusketch_mice[index[i]];
			min_value = min_value < temp ? min_value : temp;
		}
		return min_value;
	}

	int Insert_ele(unsigned int ip)
	{
		int index[MAX_HASH_NUM];
		int min_value = 1 << 30;
		for(int i = 0; i < d; i++)
		{
			index[i] = (bobhash[i + d]->run((const char *) &ip, 4)) % cell_num_ele;
			int temp = cusketch_ele[index[i]];
			min_value = min_value < temp ? min_value : temp;
		}

		for(int i = 0; i < d; i++)
		{
			if(cusketch_ele[index[i]] == min_value)
				cusketch_ele[index[i]]++;
		}
		return 0;
	}	
	int Query_ele(unsigned int ip)
	{
		int index[MAX_HASH_NUM];
		int min_value = 1 << 30;
		for(int i = 0; i < d; i++)
		{
			index[i] = (bobhash[i + d]->run((const char *) &ip, 4)) % cell_num_ele;
			int temp = cusketch_ele[index[i]];
			min_value = min_value < temp ? min_value : temp;
		}
		return min_value;
	}



	void Insert(unsigned int ip)
	{
		m++;

		Insert_mice(ip);

		if(is_sample())
		{
			if(Query_mice(ip) == MAX_COUNT)
			{
				(*flow_size)[ip] ++;
				(*ele_flow)[ip] ++;

				if((*ele_flow)[ip] == 1)
				{
					int temp = (*flow_size)[ip];
					(*flow_size)[ip] += temp;
				}
			}
			else
			{
				(*flow_size)[ip] ++;
			}
		}
		else
		{
			if(flow_size->find(ip) != flow_size->end())
			{
				(*flow_size)[ip] ++;
			}
		}
	}
	double Query()
	{
		unordered_map<unsigned int, int>::iterator it = flow_size->begin();
		
		for(int i = 0; i < flow_size->size(); i++, it++)
    	{
    		unsigned int ip = it->first;
	    	int size = it->second;

    		if(ele_flow->find(ip) != flow_size->end())
    		{
    			m -= size;
    		}
    	}
    	it = flow_size->begin();



		int mice_num = flow_size->size() - ele_flow->size();

		int z = mice_num / g;


		printf("mice_num = %d, ele_num = %d\n", mice_num, ele_flow->size());

		// printf("z = %d\n", z);



		double sum_e = 0.0;
		double * sum_m = new double[g + 10];
		memset(sum_m, 0, sizeof(double) * (g + 10));

		int cnt = 0;
    	for(int i = 0; i < flow_size->size(); i++, it++)
    	{
    		unsigned int ip = it->first;
	    	int size = it->second;

    		if(ele_flow->find(ip) == flow_size->end())
    		{
    			if(size >= 2)
    			{
    				sum_m[cnt / z] += m * (size * log(size) / log(2.0) - (size - 1) * log(size - 1) / log(2.0)) / z;
    			}
    			cnt++;
    		}
    		else
    		{
    			if(size >= 2)
    			{
    				sum_e += size * log(size) / log(2.0);
    			}
    		}
    	}
	

		// printf("cnt = %d\n", cnt);

		sort(sum_m, sum_m + g);
		double r;
		if(g % 2 == 0)
		{
			r = (sum_m[g / 2] + sum_m[g / 2 - 1]) / 2;
		}
		else
		{
			r = sum_m[g / 2];
		}

		printf("sum_m = %lf, sum_e = %lf\n", r, sum_e);
		return r + sum_e;
	}
};


#endif//_SKETCH_SIEVING_H
#ifndef _ORI_SIEVING_H
#define _ORI_SIEVING_H

#include <unordered_map>
#include <iterator>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <algorithm>

using namespace std;

class ori_sieving
{
private:
	double simple_rate;
	int threshold;
	int g;
	int m;
	unordered_map<unsigned int, int> * flow_size;
	unordered_map<unsigned int, int> * ele_flow;

public:
	ori_sieving(double sr, int _g)
	{
		simple_rate = sr;
		threshold = RAND_MAX * simple_rate;
		g = _g;
		m = 0;

		flow_size = new unordered_map<unsigned int, int>();
		ele_flow = new unordered_map<unsigned int, int>();
		srand(time(NULL));
	}
	
	bool is_sample()
	{
		int key = rand() % RAND_MAX + 1;

		if(key <= threshold)
			return true;

		return false;
	}
	void Insert(unsigned int ip)
	{
		m++;

		if(is_sample())
		{
			if(flow_size->find(ip) != flow_size->end())
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


#endif//_ORI_SIEVING_H
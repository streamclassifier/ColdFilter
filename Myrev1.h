#ifndef _MYREV1_H
#define _MYREV1_H

#include <algorithm>
#include <cstring>
#include <string.h>
#include "params.h" 
#include "BOBHash32.h"
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <iterator>
#include "SPA.h"
#include "SC.h"

using namespace std;

class Myrev1: public SPA
{
private:
	BOBHash32 * bobhash[MAX_HASH_NUM * 10];
	int w_bf, w_ibf;
	int d_bf, d_ibf;
	struct node 
	{
		uint flowx;
		int flowc;
		int packetc;
	};

	uint64_t * bf[2];
	node * ibf[2];
	unordered_map <uint, int> unmp[2];
	unordered_map <uint, int> unmp_total;

	int threshold;
public:
	int num_flow = 0;
	
	Myrev1(double Mbf, double Mibf, int dbf, int dibf, int _threshold)
	{
		w_bf = Mbf * 1024 * 1024 * 8.0 / 1;
		w_ibf = Mibf * 1024 * 1024 / sizeof(node);
		printf("w_ibf = %d\n", w_ibf);

		d_bf = dbf;
		d_ibf = dibf;
		threshold = _threshold;

		bf[0] = new uint64_t[w_bf >> 6];
		memset(bf[0], 0, sizeof(uint64_t) * (w_bf >> 6));
		ibf[0] = new node[w_ibf];
		memset(ibf[0], 0, sizeof(node) * w_ibf);


		bf[1] = new uint64_t[w_bf >> 6];
		memset(bf[1], 0, sizeof(uint64_t) * (w_bf >> 6));
		ibf[1] = new node[w_ibf];
		memset(ibf[1], 0, sizeof(node) * w_ibf);

		unmp[0].clear();
		unmp[1].clear();

		for (int i = 0; i < d_bf + d_ibf; i++)
		{
			bobhash[i] = new BOBHash32(i + 500);
		}
	}
	void Insert(uint key, int f, int seq)
	{
	    int FLAG = 1;
	    int index[MAX_HASH_NUM];

	    for(int i = 0; i < d_bf; i++)
	    {
	    	index[i] = (bobhash[i]->run((const char *) &key, 4)) % w_bf;
	    	// FLAG &= ((bf[seq][index[i] >> 6] >> (index[i] & 0x3F)) & 1);
	    	if(((bf[seq][index[i] >> 6] >> (index[i] & 0x3F)) & 1) == 0)
	    		FLAG = 0;

			bf[seq][index[i] >> 6] |= ((uint64)1 << (index[i] & 0x3F));
	    }


	    for(int i = 0; i < d_ibf; i++)
	    {
	    	index[i] = (bobhash[i + d_bf]->run((const char *) &key, 4)) % w_ibf;
	    	if(FLAG == 0)
	    	{
	    		num_flow ++;
	    		ibf[seq][index[i]].flowx ^= key;
	    		ibf[seq][index[i]].flowc ++;
	    	}
	    	ibf[seq][index[i]].packetc += f;
	    }
	}
	void dump(SC * sc, int seq)
	{
		uint key;
	    int count;
	    int index[MAX_HASH_NUM];

	 	while (1)
	   	{
	       	bool FLAG = true;
	       	for(int i = 0; i < w_ibf; i++)
	       	{
	       		if(ibf[seq][i].flowc == 1)
	       		{
	       			key = ibf[seq][i].flowx;
	       			count = ibf[seq][i].packetc;

	       			if(sc == NULL)
	        			unmp[seq][key] = count;
	        		else
	        			unmp[seq][key] = count + sc->Query(key);

	        		unmp_total[key] = 1;

	        		for(int j = 0; j < d_ibf; j++)
	        		{
	    				index[j] = (bobhash[j + d_bf]->run((const char *) &key, 4)) % w_ibf;
	    					
	    				ibf[seq][index[j]].flowc --;
	    				ibf[seq][index[j]].flowx ^= key;
	    				ibf[seq][index[j]].packetc -= count;
	        		}
	        		FLAG = false;
	        	}
	        }
            if (FLAG)
            	break;
	    }

	}

	void rev(unordered_map <uint, int> * unmp_res)
	{
		// printf("num_flow = %d\n", num_flow / d_ibf);
	    
		uint key;
	    int count, count2;

    	for(unordered_map<uint, int>::iterator it = unmp_total.begin(); it != unmp_total.end(); it++)
    	{
    		key = it->first;
    		count = unmp[0][key];
    		count2 = unmp[1][key];

    		if(abs(count - count2) > threshold)
    		{
    			(*unmp_res)[key] = abs(count - count2);
    		}
    	}
	}
};
#endif//_MYREV1_H

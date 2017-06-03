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

using namespace std;

class Myrev1
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
	Myrev1(int wbf, int wibf, int dbf, int dibf, int _threshold)
	{
		w_bf = wbf;
		w_ibf = wibf;
		d_bf = dbf;
		d_ibf = dibf;
		threshold = _threshold;

		bf[0] = new uint64_t[wbf >> 6];
		memset(bf[0], 0, sizeof(uint64_t) * (wbf >> 6));
		ibf[0] = new node[wibf];
		memset(ibf[0], 0, sizeof(node) * wibf);


		bf[1] = new uint64_t[wbf >> 6];
		memset(bf[1], 0, sizeof(uint64_t) * (wbf >> 6));
		ibf[1] = new node[wibf];
		memset(ibf[1], 0, sizeof(node) * wibf);

		unmp[0].clear();
		unmp[1].clear();

		for (int i = 0; i < d_bf + d_ibf; i++)
		{
			bobhash[i] = new BOBHash32(i + 500);
		}
	}
	void Insert(uint ip, int seq)
	{
	    int FLAG = 1;
	    int index[MAX_HASH_NUM * 10];

	    for(int i = 0; i < d_bf; i++)
	    {
	    	index[i] = (bobhash[i]->run((const char *) &ip, 4)) % w_bf;
	    	// FLAG &= ((bf[seq][index[i] >> 6] >> (index[i] & 0x3F)) & 1);
	    	if(((bf[seq][index[i] >> 6] >> (index[i] & 0x3F)) & 1) == 0)
	    		FLAG = 0;

			bf[seq][index[i] >> 6] |= ((uint64)1 << (index[i] & 0x3F));
	    }


	    for(int i = 0; i < d_ibf; i++)
	    {
	    	index[i] = (bobhash[i + d_bf]->run((const char *) &ip, 4)) % w_ibf;
	    	if(FLAG == 0)
	    	{
	    		num_flow ++;
	    		ibf[seq][index[i]].flowx ^= ip;
	    		ibf[seq][index[i]].flowc ++;
	    	}
	    	ibf[seq][index[i]].packetc++;
	    }

	}
	void rev(unordered_map <uint, int> * unmp_res)
	{
		// printf("num_flow = %d\n", num_flow / d_ibf);
	    uint ip;
	    int count, count2;
	    int index[MAX_HASH_NUM * 10];

	    for(int seq = 0; seq < 2; seq ++)
	    {
	 	  	while (1)
	   		{
	        	bool FLAG = true;
	        	for(int i = 0; i < w_ibf; i++)
	        	{
	        		if(ibf[seq][i].flowc == 1)
	        		{
	        			ip = ibf[seq][i].flowx;
	        			count = ibf[seq][i].packetc;

	        			unmp[seq][ip] = count;
	        			unmp_total[ip] = 1;

	        			for(int j = 0; j < d_ibf; j++)
	        			{
	    					index[j] = (bobhash[j + d_bf]->run((const char *) &ip, 4)) % w_ibf;
	    					
	    					ibf[seq][index[j]].flowc --;
	    					ibf[seq][index[j]].flowx ^= ip;
	    					ibf[seq][index[j]].packetc -= count;
	        			}
	        			FLAG = false;
	        		}
	        	}
            	if (FLAG)
            		break;
	        }
	    }
    	for(unordered_map<uint, int>::iterator it = unmp_total.begin(); it != unmp_total.end(); it++)
    	{
    		ip = it->first;
    		count = unmp[0][ip];
    		count2 = unmp[1][ip];

    		if(abs(count - count2) > threshold)
    		{
    			(*unmp_res)[ip] = abs(count - count2);
    		}
    	}
	}
	void rev_diff(unordered_map <uint, int> * unmp_res)
	{
	    int index[MAX_HASH_NUM * 10];
	    int count;
	    uint ip;

		for(int i = 0; i < w_ibf; i++)
		{
	    	// ibf[0][i].flowc = abs(ibf[0][i].flowc - ibf[1][i].flowc);
	    	// ibf[0][i].packetc = abs(ibf[0][i].packetc - ibf[1][i].packetc);

	    	ibf[0][i].flowc = ibf[0][i].flowc - ibf[1][i].flowc;
	    	ibf[0][i].packetc = ibf[0][i].packetc - ibf[1][i].packetc;

	    	ibf[0][i].flowx = ibf[0][i].flowx ^ ibf[1][i].flowx;
		}
		int last_size = 0;
		int now_size = 0;
		int cnt = 0;
		while (1)
	   	{
	       	bool FLAG = true;
	       	for(int i = 0; i < w_ibf; i++)
	       	{
	       		if(abs(ibf[0][i].flowc) == 1)
	       		{
	       			ip = ibf[0][i].flowx;
	       			count = ibf[0][i].packetc;

	       			int count_abs = abs(count);

	       			if(count_abs > threshold)
	       			{
    					(*unmp_res)[ip] = count_abs;
    					// now_size = unmp_res->size();
    					// if(now_size == last_size)
    					// {
    					// 	FLAG = true;
    					// 	break;
    					// }
    					// printf("%d\n", now_size);
    					// last_size =  unmp_res->size();
	       			}
	       			
	       			for(int j = 0; j < d_ibf; j++)
	       			{
	    				index[j] = (bobhash[j + d_bf]->run((const char *) &ip, 4)) % w_ibf;
	    				
	    				ibf[0][index[j]].flowc -= ibf[0][i].flowc;
	    				ibf[0][index[j]].flowx ^= ip;
	    				ibf[0][index[j]].packetc -= count;
	       			}
	       			FLAG = false;
	       		}
	       	}
	       	cnt++;
           	if (FLAG || cnt > MAX_DECODE_ITERATOR)
           		break;
	    }
	}
};
#endif//_MYREV1_H

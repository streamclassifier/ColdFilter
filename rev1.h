#ifndef _rev1_H
#define _rev1_H

#include <algorithm>
#include <cstring>
#include <string.h>
#include "params.h" 
#include "BOBHash32.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class rev1
{
private:
	BOBHash32 * bobhash[MAX_HASH_NUM];
	int w, d, D, W, cnt, param;
	vector <unsigned int> ans;
	struct node 
	{
		unsigned int flowx;
		int flowc, packetc;
	};
	node *p[2][MAX_HASH_NUM];
	bool *bf[2][MAX_HASH_NUM], *v[2][MAX_HASH_NUM];
public:
	rev1(int _w, int _d, int _D, int _W, int _param)
	{
		param = _param;
		ans.clear(); 
		cnt = 0;
		w=_w; W=_w;
		d=_d; D=_d;
		for (int i = 0; i < D; i++)
		{
		    bf[0][i] = new bool[W];
		    bf[1][i] = new bool[W];
		    for (int j = 0; j < W; j++) 
		    	bf[0][i][j] = bf[1][i][j] = false;
		}
		for (int i = 0; i < d; i++)
		{
			p[0][i] = new node[w]; v[0][i] = new bool[w];
			p[1][i] = new node[w]; v[1][i] = new bool[w];
			for (int j = 0; j < w; j++)
			{
			    p[0][i][j].flowx = p[1][i][j].flowx = 0;
			    p[0][i][j].flowc = p[1][i][j].flowc = p[0][i][j].packetc = p[1][i][j].packetc = 0;
			    v[0][i][j] = v[1][i][j] = false;
			}
		}
		for (int i = 0; i < d + D; i++)
		{
			bobhash[i] = new BOBHash32(i + 1000);
		}
	}
	void Insert(const char * str,int wz)
	{
	    int now = 0; unsigned int X = 0; 
	    int B;
	    for (int i = 0;;i++)
	    {
	        if (str[i]=='.' || str[i]==' ') {X=X*256+now; now=0;}
	        if (str[i]==' ') {B=i; break;}
	        if (str[i]>='0' && str[i]<='9') now=now*10+str[i]-'0';
	    }
	    bool FLAG = true; int index;
	    for (int i = 0; i < D; i++)
	    {
	        index = (bobhash[d + i]->run(str, B)) % W;
	        FLAG &= bf[wz][i][index]; 
	        bf[wz][i][index] = true;
	    }
	    for (int i = 0; i < d; i++)
		{
			index = (bobhash[i]->run(str, B)) % w;
			if (!FLAG) 
			{
				p[wz][i][index].flowx ^= X; 
				p[wz][i][index].flowc++;
			}
			p[wz][i][index].packetc++;
		}
	}
	void rev()
	{
	    while (1)
	    {
	        bool FLAG = true;
	        for (int F = 0; F < 2; F++)
	        {
	        	for (int i = 0; i < d; i++)
	        	{
            	    for (int j = 0; j < w; j++)
            	    {
            	        if (p[F][i][j].flowc == 1 && !v[F][i][j])
            	        {
            	            v[F][i][j] = true;
            	            unsigned int a = p[F][i][j].flowx;
            	            int D=a%256; a/=256; int C=a%256; a/=256; int B=a%256; a/=256; int A=a;
            	            string S = to_string(A)+'.'+to_string(B)+'.'+to_string(C)+'.'+to_string(D);
            	            for (int k = 0; k < d; k++)
            	            { 
            	            	if (k != i)
            	            	{
            	            	    int index = (bobhash[k]->run(S.c_str(), S.length())) % w;
            	            	    p[F][k][index].flowc --; 
            	            	    p[F][k][index].flowx ^= p[F][i][j].flowx;
            	            	}
            	            	FLAG = false;
            	        	}
            	        }
            	    }
            	}
            }
            if (FLAG) 
            	break;
	    }
	}
	int Query1()
	{
	    for (int i = 0; i < d; i++)
            for (int j = 0; j < w; j++)
                if (p[0][i][j].flowc == 1)
                {
                        unsigned int a = p[0][i][j].flowx;
                        int D=a%256; a/=256; int C=a%256; a/=256; int B=a%256; a/=256; int A=a;
                        string S=to_string(A)+'.'+to_string(B)+'.'+to_string(C)+'.'+to_string(D);
                       // cout<<S<<' '<<p[0][i][j].flowx<<' '<<bobhash[i]->run(S.c_str(),S.length())%w<<' '<<j<<' '<<S.length()<<endl;
                        int MIN = p[0][i][j].packetc;
                        bool FLAG = false;
                        int MIN2 = p[1][i][j].packetc;
                        for (int k = 0; k < d; k++)
                        {
                            int index = (bobhash[k]->run(S.c_str(),S.length())) % w;
                            MIN = min(MIN, p[0][k][index].packetc);
                            MIN2 = min(MIN2, p[1][k][index].packetc);
                            if (p[1][k][index].flowc == 1 && p[1][k][index].flowx == p[0][i][j].flowx) 
                            	FLAG=true;
                        }
                     //   cout<<MIN<<' '<<MIN2<<endl;
                        if (!FLAG && MIN >= param || FLAG && abs(MIN-MIN2) >= param) 
                        {
                        	++cnt; 
                        	ans.push_back(p[0][i][j].flowx); 
                        }
                }	    
		for (int i=0; i<d; i++)
            for (int j=0; j<w; j++)
                if (p[1][i][j].flowc==1)
                {
                        unsigned int a=p[1][i][j].flowx;
                        int D=a%256; a/=256; int C=a%256; a/=256; int B=a%256; a/=256; int A=a;
                        string S=to_string(A)+'.'+to_string(B)+'.'+to_string(C)+'.'+to_string(D);
                        int MIN=p[1][i][j].packetc;
                        bool FLAG=false;
                        for (int k=0; k<d; k++)
                        {
                            int index=(bobhash[k]->run(S.c_str(),S.length())) % w;
                            MIN=min(MIN,p[1][k][index].packetc);
                            if (p[0][k][index].flowc==1 && p[0][k][index].flowx==p[1][i][j].flowx) 
                            	FLAG=true;
                        }
                        if (!FLAG && MIN>=param) 
                        {
                        	++cnt; 
                        	ans.push_back(p[1][i][j].flowx); 
                        }
                } //else if (p[1][i][j].flowc) cout<<i<<' '<<j<<' '<<p[1][i][j].flowc<<endl;
        return cnt;
    }
    int Query2(const int x)
    {
        return ans[x];
    }
	/*~rev1()
	{
			delete []p;
			delete []bf;
	}*/
};
#endif//_CUSketch_H

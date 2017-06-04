#ifndef _CUSKETCH_H
#define _CUSKETCH_H

#include "params.h"
#include "BOBHash32.h"
#include "SPA.h"
#include <string.h>
#include <stdlib.h>

using namespace std;


class CUsketch: public SPA
{
private:
	int *CU;
	int w, d;
	BOBHash32 ** bobhash;

public:
	CUsketch(double M, int _d);
	~CUsketch();
	virtual void Insert(uint key, int f, int seq);
	int Query(uint key);

};

CUsketch::CUsketch(double M, int _d)
{
	d = _d;
	w = M * 1024 * 1024 * 8.0 / 32;

	CU = new int[w];
	memset(CU, 0, sizeof(int) * w);

	bobhash = new BOBHash32 *[d];
	for(int i = 0; i < d; i++)
		bobhash[i] = new BOBHash32(i + 750);		
}
CUsketch::~CUsketch()
{
	delete CU;
	for(int i = 0; i < d; i++)
		delete bobhash[i];
	delete[] bobhash;
}
void CUsketch::Insert(unsigned int key, int f, int seq)
{
	int index[MAX_HASH_NUM];
	int value[MAX_HASH_NUM];
	int V = 1 << 30;
	for(int i = 0; i < d; i++)
	{
		index[i] = (bobhash[i]->run((const char *)&key, 4)) % w;
		value[i] = CU[index[i]];

		V = V < value[i] ? V : value[i];
	}

	int temp = V + f;
	for(int i = 0; i < d; i++)
	{
		CU[index[i]] = CU[index[i]] > temp ? CU[index[i]] : temp;	
	}
	return;
}
int CUsketch::Query(uint key)
{
	int value[MAX_HASH_NUM];
	int V = 1 << 30;
	for(int i = 0; i < d; i++)
	{
		value[i] = CU[(bobhash[i]->run((const char *)&key, 4)) % w];

		V = V < value[i] ? V : value[i];
	}
	return V;
}

#endif //_CUSKETCH_H
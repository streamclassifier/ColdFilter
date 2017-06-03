#ifndef _SC_H
#define _SC_H

#include "params.h"
#include "BOBHash32.h"
#include "BOBHash64.h"
#include "SPA.h"
#include <string.h>
#include <x86intrin.h>
#include <bmiintrin.h>
#include <stdlib.h>
#include <stdio.h>
using namespace std;

class SC
{
private:
	int bucket_num, counter_num;
	uint ** ID;
	int ** counter;
	int *cur_pos;

	int d1, w1, w_word;
	ull * L1;

	int d2, w2, T2;
	short int * L2;

	SPA * spa;

	BOBHash32 * bobhash1;
	BOBHash32 ** bobhash2;

public:
	//_bucket_num: 16; _counter_num: 16; _d1/_d2: 3; M1/M2: x.xx MB; _T2: 240;
	SC(int _bucket_num, int _counter_num, int _d1, double M1, int _d2, double M2, int _T2);

	~SC();

	//periodical refreshing for continuous top-k;
	void init_array();

	void init_spa(SPA * _spa);

	void Insert(uint key);

	void Insert_SC_SPA(uint kick_ID, int kick_f);

	void refresh();

	int Query(uint key);
};

//_bucket_num: 16; _counter_num: 16; _d1/_d2: 3; M1/M2: x.xx MB; _T2: 240;
SC::SC(int _bucket_num, int _counter_num, int _d1, double M1, int _d2, double M2, int _T2)
{
	//init the buckets
	bucket_num = _bucket_num;
	counter_num = _counter_num;

	ID = new uint * [bucket_num];
	counter = new int * [bucket_num];
	cur_pos = new int[bucket_num];
	memset(cur_pos, 0, sizeof(int) * bucket_num);
		
	for(int i = 0; i < bucket_num; i++)
	{
		ID[i] = new uint[counter_num];
		memset(ID[i], 0, sizeof(uint) * counter_num);
		counter[i] = new int[counter_num];
		memset(counter[i], 0, sizeof(int) * counter_num);
	}


	d1 = _d1;
	d2 = _d2;
	T2 = _T2;

	w1 = (M1 * 1024 * 1024 * 8.0) / 4;//4 bits per counter
	w2 = (M2 * 1024 * 1024 * 8.0) / 16;//16 bits per counter

	w_word = w1 >> 4;//16 counters per machine word

	L1 = new ull[w_word];
	memset(L1, 0, sizeof(ull) * w_word);

	L2 = new short int[w2];
	memset(L2, 0, sizeof(short int) * w2);

	bobhash1 = new BOBHash32(500);

	bobhash2 = new BOBHash32 *[d2];
	for(int i = 0; i < d2; i++)
		bobhash2[i] = new BOBHash32(1000 + i);

	spa = NULL;
}

SC::~SC()
{
	for(int i = 0; i < bucket_num; i++)
	{
		delete []ID[i];
		delete []counter[i];
	}
	delete []ID;
	delete []counter;
	delete []cur_pos;

	delete []L1;
	delete []L2;
 	
 	delete bobhash1;
 	for(int i = 0; i < d2; i++)
 		delete bobhash2[i];
 	delete []bobhash2;

}
//periodical refreshing for continuous top-k;
void SC::init_array()
{
	for(int i = 0; i < w_word; i++)
	{
		ull temp = L1[i];

		temp = (temp & (0xF)) == 0xF ? temp : (temp & 0xFFFFFFFFFFFFFFF0);
		temp = (temp & (0xF0)) == 0xF0 ? temp : (temp & 0xFFFFFFFFFFFFFF0F);
		temp = (temp & (0xF00)) == 0xF00 ? temp : (temp & 0xFFFFFFFFFFFFF0FF);
		temp = (temp & (0xF000)) == 0xF000 ? temp : (temp & 0xFFFFFFFFFFFF0FFF);
			
		temp = (temp & (0xF0000)) == 0xF0000 ? temp : (temp & 0xFFFFFFFFFFF0FFFF);
		temp = (temp & (0xF00000)) == 0xF00000 ? temp : (temp & 0xFFFFFFFFFF0FFFFF);
		temp = (temp & (0xF000000)) == 0xF000000 ? temp : (temp & 0xFFFFFFFFF0FFFFFF);
		temp = (temp & (0xF0000000)) == 0xF0000000 ? temp : (temp & 0xFFFFFFFF0FFFFFFF);
			
		temp = (temp & (0xF00000000)) == 0xF00000000 ? temp : (temp & 0xFFFFFFF0FFFFFFFF);
		temp = (temp & (0xF000000000)) == 0xF000000000 ? temp : (temp & 0xFFFFFF0FFFFFFFFF);
		temp = (temp & (0xF0000000000)) == 0xF0000000000 ? temp : (temp & 0xFFFFF0FFFFFFFFFF);
		temp = (temp & (0xF00000000000)) == 0xF00000000000 ? temp : (temp & 0xFFFF0FFFFFFFFFFF);
				
		temp = (temp & (0xF000000000000)) == 0xF000000000000 ? temp : (temp & 0xFFF0FFFFFFFFFFFF);
		temp = (temp & (0xF0000000000000)) == 0xF0000000000000 ? temp : (temp & 0xFF0FFFFFFFFFFFFF);
		temp = (temp & (0xF00000000000000)) == 0xF00000000000000 ? temp : (temp & 0xF0FFFFFFFFFFFFFF);
		temp = (temp & (0xF000000000000000)) == 0xF000000000000000 ? temp : (temp & 0x0FFFFFFFFFFFFFFF);
			
		L1[i] = temp;
	}

	for(int i = 0; i < w2; i++)
	{
		short int temp = L2[i];
		L2[i] = (temp == T2) ? temp : 0;
	}
}

void SC::init_spa(SPA * _spa)
{
	spa = _spa;
}

void SC::Insert(uint key)
{
	int bucket_id = key % bucket_num;
	// int bucket_id = key & 0x2FF;

	const __m128i item = _mm_set1_epi32((int)key);

	__m128i *keys_p = (__m128i *)ID[bucket_id];

	__m128i a_comp = _mm_cmpeq_epi32(item, keys_p[0]);
	__m128i b_comp = _mm_cmpeq_epi32(item, keys_p[1]);
	__m128i c_comp = _mm_cmpeq_epi32(item, keys_p[2]);
	__m128i d_comp = _mm_cmpeq_epi32(item, keys_p[3]);

	a_comp = _mm_packs_epi32(a_comp, b_comp);
	c_comp = _mm_packs_epi32(c_comp, d_comp);
	a_comp = _mm_packs_epi32(a_comp, c_comp);

	int matched = _mm_movemask_epi8(a_comp);

	if(matched != 0)
	{
		//return 32 if input is zero;
		int matched_index = _tzcnt_u32((uint)matched);

		// if(key != ID[bucket_id][matched_index])
			// printf("fasfdasfasf\n");
		// printf("matched: %u vs. %u : %d\n", key, ID[bucket_id][matched_index], counter[bucket_id][matched_index]);
		counter[bucket_id][matched_index] ++;
		return;
	}

	

	int cur_pos_now = cur_pos[bucket_id];
	if(cur_pos_now != counter_num)
	{
		// printf("%d\n", cur_pos_now);
		ID[bucket_id][cur_pos_now] = key;
		counter[bucket_id][cur_pos_now] ++;
		cur_pos[bucket_id]++;
		return;
	}


	int start_point = 8;

	for(int i = start_point; i < 16; i++)
	{
		Insert_SC_SPA(ID[bucket_id][i], counter[bucket_id][i]);
		ID[bucket_id][i] = 0;
		counter[bucket_id][i] = 0;
	}

	ID[bucket_id][start_point] = key;
	counter[bucket_id][start_point] = 1;
	
	cur_pos[bucket_id] = start_point + 1;

}
void SC::Insert_SC_SPA(uint kick_ID, int kick_f)
{

	int V1 = 1 << 30;

	int value[MAX_HASH_NUM];
	int index[MAX_HASH_NUM];
	int offset[MAX_HASH_NUM];
	
	ull hash_value = (bobhash1->run((const char *)&kick_ID, 4));
	int word_index = hash_value % w_word;
	hash_value >>= 16;


	ull temp = L1[word_index];
	for(int i = 0; i < d1; i++)
	{
		offset[i] = (hash_value & 0xF);
	
		value[i] = (temp >> (offset[i] << 2)) & 0xF;
		V1 = value[i] < V1 ? value[i] : V1;

		hash_value >>= 4;
	}
	// printf("%d\n", V1);

	int temp2 = V1 + kick_f;
	if(temp2 <= 15)
	{
		for(int i = 0; i < d1; i++)
		{
			int temp3 = ((temp >> (offset[i] << 2)) & 0xF);
			if(temp3 < temp2)
			{
				temp += ((uint64)(temp2 - temp3) << (offset[i] << 2));
			}
		}
		L1[word_index] = temp;
		return;
	}

	for(int i = 0; i < d1; i++)
	{
		temp |= ((uint64)0xF << (offset[i] << 2));
	}
	L1[word_index] = temp;


	int Delta1 = 15 - V1;	
	kick_f -= Delta1;







	int V2 = 1 << 30;
	for(int i = 0; i < d2; i++)
	{
		index[i] = (bobhash2[i]->run((const char *)&kick_ID, 4)) % w2;
		
		value[i] = L2[index[i]];
		V2 = value[i] < V2 ? value[i] : V2;
	}

	// printf("%d\n", V2);
	temp2 = V2 + kick_f;		
	if(temp2 <= T2)
	{
		for(int i = 0; i < d2; i++)
		{
			L2[index[i]] = (L2[index[i]] > temp2) ? L2[index[i]] : temp2;
		}
		return;
	}

	for(int i = 0; i < d2; i++)
	{
		L2[index[i]] = T2;
	}

	int Delta2 = T2 - V2;
	kick_f -= Delta2;

	
	spa->Insert(kick_ID, kick_f);
	
}
void SC::refresh()
{
	for(int i = 0; i < bucket_num; i++)
	{
		for(int j = 0; j < counter_num; j++)
		{
			Insert_SC_SPA(ID[i][j], counter[i][j]);
		}
	}
	return;
}
int SC::Query(uint key)
{
	int V1 = 1 << 30;

	int value[MAX_HASH_NUM];
	int index[MAX_HASH_NUM];
	int offset[MAX_HASH_NUM];
	
	ull hash_value = (bobhash1->run((const char *)&key, 4));
	int word_index = hash_value % w_word;
	hash_value >>= 16;


	ull temp = L1[word_index];
	for(int i = 0; i < d1; i++)
	{
		offset[i] = (hash_value & 0xF);
	
		value[i] = (temp >> (offset[i] << 2)) & 0xF;
		V1 = value[i] < V1 ? value[i] : V1;

		hash_value >>= 4;
	}

	if(V1 != 15)
		return V1;

	int V2 = 1 << 30;
	for(int i = 0; i < d2; i++)
	{
		index[i] = (bobhash2[i]->run((const char *)&key, 4)) % w2;
		
		value[i] = L2[index[i]];
		V2 = value[i] < V2 ? value[i] : V2;
	}

	// printf("V1 + V2 = %d\n", V1 + V2);
	return V1 + V2;
}

#endif//_SC_H
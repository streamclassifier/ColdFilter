#ifndef _STDTOPK_H
#define _STDTOPK_H

#include "params.h"
#include <string>
#include <map>

#include <cstdio>
#include <cstring>

#include <algorithm>

using namespace std;

class StdTopK{
public:
		map<string, uint32_t> cntr;
		uint32_t k;
		
		StdTopK(uint32_t _k);
		~StdTopK();
		
		void insert(const char *key);
		uint32_t query(const char *key) const;
		void GetTopK(char **Ans) const;
		void GetFreq(uint32_t limit, char **Ans) const;
		
private:
		StdTopK();
		StdTopK(const StdTopK &);
		StdTopK &operator =(const StdTopK &);
};


StdTopK ::
StdTopK(uint32_t _k): k(_k){
}

StdTopK ::
~StdTopK(){
}

void StdTopK ::
insert(const char *key){
	string key_s(key);
	
	map<string, uint32_t> :: iterator p = cntr.find(key_s);
	if (p != cntr.end()){
		++p -> second;
	} else {
		cntr.insert(pair<string, uint32_t>(key_s, 1));
	}
}

uint32_t StdTopK ::
query(const char *key) const{
	string key_s(key);
	map<string, uint32_t> :: const_iterator p = cntr.find(key_s);
	if (p != cntr.end())
		return p -> second;
	 else
		return 0;
}

static bool cmp(const pair<string, uint32_t> &x, const pair<string, uint32_t> &y){
	return x.second > y.second;
}

void StdTopK ::
GetTopK(char **Ans) const{
	uint32_t s = cntr.size();
	pair<string, uint32_t> *t = new pair<string, uint32_t>[s];
	uint32_t i = 0; map<string, uint32_t> :: const_iterator p;
	for (i = 0, p = cntr.begin(); i < s; ++i, ++p)
		t[i] = *p;
	
	sort(t, t + s, cmp);
	for (i = 0; i < k; ++i){
		uint32_t s = t[i].first.length();
		char *key = new char[s+1];
		memcpy(key, t[i].first.c_str(), s);
		key[s] = 0;
		Ans[i] = key;
	}
	delete []t;
}

void StdTopK ::
GetFreq(uint32_t limit, char **Ans) const{
	uint32_t i = 0;
	map<string, uint32_t> :: const_iterator p;
	for (p = cntr.begin(); p != cntr.end(); ++p)
	 if (p -> second >= limit){
		 uint32_t s = p -> first.length();
		 char *key = new char[s+1];
		 memcpy(key, p -> first.c_str(), s);
		 key[s] = 0;
		 Ans[i++] = key;
	 }
	Ans[i] = 0;
}

#endif//_STDTOPK_H
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <string.h>
#include <ctime>
#include <time.h>
#include <iterator>
#include <math.h>
#include <map>
#include <vector>
#include "rev1.h"
#include "rev2.h"
#include "params.h"


using namespace std;
int param = 1000;

char * filename_FlowTraffic = "../../BloomSketch/1";

char insert[10000000 + 10000000 / 5][50];
map<unsigned int ,int> mp;
unordered_map<string, int> unmp;
unsigned int t[10000005];
int main(int argc, char** argv)
{
    if(argc == 2)
    {
        filename_FlowTraffic = argv[1];
    }

    unmp.clear();
    uint ip_s, ip_d;
    int val=0;

    int package_num = 0;
    FILE *file_FlowTraffic = fopen(filename_FlowTraffic, "r");
    while(fgets(insert[package_num], 50, file_FlowTraffic) != NULL)
    {
        if (package_num < 5000000) 
            unmp[string(insert[package_num])]++; 
        else
            unmp[string(insert[package_num])]--;
        package_num++;

        if(package_num == MAX_INSERT_PACKAGE)
            break;
    }
    fclose(file_FlowTraffic);

    printf("dataset: %s\n", filename_FlowTraffic);
    printf("total stream size = %d\n", package_num);
    printf("distinct item number = %d\n", unmp.size());

    int max_freq = 0;
    unordered_map<string, int>::iterator it = unmp.begin();

    for(int i = 0; i < unmp.size(); i++, it++)
    {
        int temp2 = it->second;
        max_freq = max_freq > temp2 ? max_freq : temp2;
    }
    printf("max_freq = %d\n", max_freq);




    double memory = 10.0;

    FILE * file_coef = fopen("./coef_res.txt", "w");

    double coef_low = 0.7;
    double coef_bf = 0.10;


    int w = memory * 1024 * 1024 * 8.0 / COUNTER_SIZE;

    rev1 *re1;
    rev2 *re2;
    re1 = new rev1(w / LOW_HASH_NUM / 6 * 0.8 , LOW_HASH_NUM, w / LOW_HASH_NUM /2 * 32 * 0.2 , LOW_HASH_NUM * 2, param);
    re2 = new rev2(w / LOW_HASH_NUM / 6 * 0.2 , LOW_HASH_NUM, w / LOW_HASH_NUM /2 * 32 * 0.1 , LOW_HASH_NUM * 2, w / LOW_HASH_NUM / 2 * 2 * 0.7, LOW_HASH_NUM, param);


    for(int i = 0; i < package_num / 2; i++)
    {
        re1->Insert(insert[i], 0);
        re2->Insert(insert[i], 0);
    }
    for (int i = package_num / 2; i < package_num; i++)
    {
        re1->Insert(insert[i], 1);
        re2->Insert(insert[i], 1);
    }

    FILE * file_res = fopen("./res.txt", "w");
    int cnt = 0;
    for(unordered_map<string, int>::iterator it = unmp.begin(); it != unmp.end(); it++)
    {
        if(abs(it->second) >= param)
        {
            cnt++; 
            int now = 0;
            for (int i = 0; ; i++)
            {
                if (it->first[i] == '.' || it->first[i] == ' ') 
                {
                    t[cnt] = t[cnt] * 256 + now; 
                    now = 0;
                }
                if (it->first[i] == ' ') 
                    break;
                if (it->first[i] >= '0' && it->first[i] <= '9') 
                    now = now * 10 + it->first[i] - '0';
            }
            mp[t[cnt]] = 1;
        }
    }
    fclose(file_res);
    re1->rev(); 
    re2->rev();

    int CNT = re1->Query1();
    for (int i = 0; i < CNT; i++)
    {
        unsigned int X = re1->Query2(i);
        if (mp[X]) 
            val++;
    }
    int val2 = 0;
    int CNT2 = re2->Query1();
    for (int i = 0; i < CNT2; i++)
    {
        unsigned int X = re2->Query2(i);
        if (mp[X]) 
            val2++;
    }


    double a = package_num * 1.0;
    double b = unmp.size() * 1.0;
    fprintf(stderr, "re1:\ntotal = %d\n answer = %d\n accepted = %d\n", cnt, CNT, val);
    fprintf(stderr, "\nre2:\ntotal = %d\n answer = %d\n accepted = %d\n", cnt, CNT2, val2);


    return 0;

}

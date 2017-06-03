#include <stdio.h>
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

#include "SC.h"
// #include "SC_simd32.h"

#include "CUsketch.h"
#include "SpaceSaving.h"
#include "StdTopK.h"

#define testcycles 1

// #define CU_TEST
#define SS

using namespace std;

uint insert[MAX_INSERT_PACKAGE + MAX_INSERT_PACKAGE / 500];
uint query[MAX_INSERT_PACKAGE + MAX_INSERT_PACKAGE / 500];

unordered_map<uint, int> unmp;

int main(int argc, char** argv)
{
    char filename_FlowTraffic[500];
    strcpy(filename_FlowTraffic, "./format_data/formatted06.dat");

    if(argc == 2)
    {
        int id = atoi(argv[1]);
        char buffer[10];
        if(id < 10)
        {
            sprintf(buffer, "0%d", id);
        }
        else
        {
            sprintf(buffer, "%d", id);
        }
        strcpy(filename_FlowTraffic, "./format_data/formatted");
        strcat(filename_FlowTraffic, buffer);
        strcat(filename_FlowTraffic, ".dat");
        // printf("%s\n", filename_FlowTraffic);
    }

    int package_num = 0;
    FILE *file_FlowTraffic = fopen(filename_FlowTraffic, "rb");
    
    char *ip[8];
    while(fread(ip, 1, 8, file_FlowTraffic))
    {
        uint key = *((uint *)ip);
        insert[package_num] = key;

        unmp[key]++;
        package_num++;

        if(package_num == MAX_INSERT_PACKAGE)
            break;
    }
    fclose(file_FlowTraffic);

    printf("dataset: %s\n", filename_FlowTraffic);
    printf("total stream size = %d\n", package_num);
    printf("distinct item number = %d\n", unmp.size());

    int max_freq = 0;
    unordered_map<uint, int>::iterator it = unmp.begin();

    for(int i = 0; i < unmp.size(); i++, it++)
    {
        query[i] = it->first;
        int temp2 = it->second;
        max_freq = max_freq > temp2 ? max_freq : temp2;
    }
    printf("max_freq = %d\n", max_freq);








/********************* test for the CU sketch ********************/

#ifdef CU_TEST

    CUsketch * cusketch = NULL;
    CUsketch * cusketch_sc = NULL;
    SC * sc = NULL;

    double memory = 1.0;
    int d = 4;


    cusketch = new CUsketch(memory, d);


    int bucket_num = 1000;
    int counter_num = 16;
    double M_SIMD = bucket_num * counter_num * 8.0 / (1024 * 1024);
    printf("M_SIMD = %lf\n", M_SIMD);

    double M1 = 0.5 * memory - M_SIMD;
    double M2 = 0.45 * memory;
    int T1 = 15;
    int T2 = 240;
    int T = T1 + T2;
    
    cusketch_sc = new CUsketch(memory - M1 - M2, d);
    // SC(int _bucket_num, int _counter_num, int _d1, double M1, int _d2, double M2, int _T2)
    sc = new SC(bucket_num, counter_num, d, M1, d, M2, T2);
    sc->init_spa(cusketch_sc);



    timespec time1, time2;
    long long resns;
    clock_gettime(CLOCK_MONOTONIC, &time1);

    for(int x = 0; x < testcycles; x++)
    {
        cusketch = new CUsketch(memory, d);
        for(int i = 0; i < MAX_INSERT_PACKAGE; i++)
        {
            cusketch->Insert(insert[i], 1);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    double throughput_cu = (double)1000.0 * testcycles * package_num / resns;
    printf("throughput of CU (insert): %.6lf\n", throughput_cu);
    


    clock_gettime(CLOCK_MONOTONIC, &time1);
    
    for(int x = 0; x < testcycles; x++)
    {
        cusketch_sc = new CUsketch(memory - M1 - M2, d);
        sc = new SC(bucket_num, counter_num, d, M1, d, M2, T2);
        sc->init_spa(cusketch_sc);
        for(int i = 0; i < MAX_INSERT_PACKAGE; i++)
        {
            sc->Insert(insert[i]);
        }
        sc->refresh();
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    double throughput_cu_sc = (double)1000.0 * testcycles * package_num / resns;
    printf("throughput of CU_SC (insert): %.6lf\n", throughput_cu_sc);
    






    int flow_num = unmp.size();
    int temp = 0;
    int sum = 0;

    clock_gettime(CLOCK_MONOTONIC, &time1);

    for(int x = 0; x < testcycles; x++)
    {
        for(int i = 0; i < flow_num; i++)
        {
            temp += cusketch->Query(query[i]);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    double throughput_cu_q = (double)1000.0 * testcycles * flow_num / resns;
    printf("throughput of CU (query): %.6lf\n", throughput_cu_q);
    
    sum += temp;



    clock_gettime(CLOCK_MONOTONIC, &time1);
    
    for(int x = 0; x < testcycles; x++)
    {
        for(int i = 0; i < flow_num; i++)
        {
            temp = sc->Query(query[i]);
            if(temp == T)
                temp += cusketch_sc->Query(query[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    double throughput_cu_sc_q = (double)1000.0 * testcycles * flow_num / resns;
    printf("throughput of CU_SC (query): %.6lf\n", throughput_cu_sc_q);
    
    sum += temp;

    if(sum == (1 << 30))
        return 0;






    double AE1 = 0.0, AE2 = 0.0;
    int i = 0;

    FILE * res = fopen("result.txt", "w");
    for(unordered_map<uint, int>::iterator it = unmp.begin(); it != unmp.end();i++, it++)
    {
        uint key = it->first;
        int val = it->second;

        int val1 = cusketch->Query(key);
        AE1 += val1 - val;


        int val2 = sc->Query(key);
        if(val2 == T)
            val2 += cusketch_sc->Query(key);
        
        AE2 += val2 - val;

        // if(val2 < val)
            // printf("fasfdasdfa\n");
        fprintf(res, "%d\t%d\t%d\n", val, val1, val2);
    }

    printf("AE1 = %lf\n", AE1 / flow_num);
    printf("AE2 = %lf\n", AE2 / flow_num);

#endif//CU_TEST

/**********************************************************************/



/********************* test for the SpaceSaving ********************/


#ifdef SS

    const int K = 256;
    const int SS_HEAPSIZE = 256;

    SpaceSaving * ss = NULL;
    ss = new SpaceSaving(K, SS_HEAPSIZE);
    StdTopK * stk = NULL;
    stk = new StdTopK(K);

    char trans[100];

    for(int i = 0; i < package_num; i++)
    {
        sprintf(trans, "%u\0", insert[i]);
        // for(int i = 0; i < 100; i++)
        // {
        //     printf("%c\n", trans[i]);
        // }
        // printf("%d\n", strlen(trans));
        // printf("%s\n", trans);
        ss->insert(trans);
        stk->insert(trans);
    }

    char **ssAns, **stkAns;

    ssAns = new char *[K+1];
    stkAns = new char *[K+1];


    ss->GetTopK(ssAns);
    stk->GetTopK(stkAns);
    
    uint sscnt, stkcnt;
    sscnt = stkcnt = K;




    uint cnt = 0;
    for (uint i = 0; i < sscnt; ++i)
     if (ssAns[i]){
        for (uint j = 0; j < stkcnt; ++j)
         if (stkAns[j] && strcmp(ssAns[i], stkAns[j]) == 0){
            ++cnt;
            break;
         }
     }
    printf("%d %d\n", cnt, sscnt);
    double ss_rate_pr = (double)cnt / sscnt;
    



    cnt = 0;
    for (uint i = 0; i < stkcnt; ++i)
     if (stkAns[i]){
         for (uint j = 0; j < sscnt; ++j)
          if (ssAns[j] && strcmp(ssAns[j], stkAns[i]) == 0){
              ++cnt;
              break;
          }
     }
    double ss_rate_rr = (double)cnt / stkcnt;



    FILE *outf = stdout;
    fprintf(outf, "%d,%.10lf,%.10lf\n", K, ss_rate_pr, ss_rate_rr);
    fclose(outf);
    
#endif//SS

    return 0;

}

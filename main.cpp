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
// #include "SpaceSaving.h"
// #include "StdTopK.h"
#include "SS1.h"
#include "Myrev1.h"

#define testcycles 1

#define CU_TEST
// #define SS
// #define HC

using namespace std;

uint insert[MAX_INSERT_PACKAGE + MAX_INSERT_PACKAGE / 500];
uint query[MAX_INSERT_PACKAGE + MAX_INSERT_PACKAGE / 500];

unordered_map<uint, int> unmp;
unordered_map<uint ,int> unmp_myres[2];
unordered_map<uint ,int> unmp_standres;

struct node {uint x; int y;} t[MAX_INSERT_PACKAGE / 50];

node t1[MAX_INSERT_PACKAGE / 50];

node t2[MAX_INSERT_PACKAGE / 50];

int CMP(node i,node j) {return i.y>j.y;}


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




/********************* test for the CU sketch ********************/

#ifdef CU_TEST

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




    CUsketch * cusketch = NULL;
    CUsketch * cusketch_sc = NULL;
    SC * sc = NULL;

    double memory = 2.0;
    int d = 4;


    cusketch = new CUsketch(memory, d);


    int bucket_num = 5000;
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
            cusketch->Insert(insert[i], 1, 0);
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
            sc->Insert(insert[i], 0);
        }
        sc->refresh(0);
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

        int correct_cnt1 = cusketch->Query(key);
        AE1 += correct_cnt1 - val;


        int correct_cnt2 = sc->Query(key);
        if(correct_cnt2 == T)
            correct_cnt2 += cusketch_sc->Query(key);
        
        AE2 += correct_cnt2 - val;

        // if(correct_cnt2 < val)
            // printf("fasfdasdfa\n");
        fprintf(res, "%d\t%d\t%d\n", val, correct_cnt1, correct_cnt2);
    }

    printf("AE1 = %lf\n", AE1 / flow_num);
    printf("AE2 = %lf\n", AE2 / flow_num);

#endif//CU_TEST

/**********************************************************************/



/********************* test for the SpaceSaving ********************/


#ifdef SS

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





    // const int K = 256;
    // const int SS_HEAPSIZE = 512;

    // SpaceSaving * ss = NULL;
    // ss = new SpaceSaving(K, SS_HEAPSIZE);
    
    
    // SpaceSaving * ss_sc = NULL;
    // ss_sc = new SpaceSaving(K, SS_HEAPSIZE);
    

    // StdTopK * stk = NULL;
    // stk = new StdTopK(K);



    // char trans[100];

    // for(int i = 0; i < package_num; i++)
    // {
    //     sprintf(trans, "%u\0", insert[i]);
    //     // for(int i = 0; i < 100; i++)
    //     // {
    //     //     printf("%c\n", trans[i]);
    //     // }
    //     // printf("%d\n", strlen(trans));
    //     // printf("%s\n", trans);
    //     ss->insert(trans);
    //     stk->insert(trans);
    // }

    // char **ssAns, **stkAns;

    // ssAns = new char *[K+1];
    // stkAns = new char *[K+1];


    // ss->GetTopK(ssAns);
    // stk->GetTopK(stkAns);
    
    // uint sscnt, stkcnt;
    // sscnt = stkcnt = K;




    // uint cnt = 0;
    // for (uint i = 0; i < sscnt; ++i)
    //  if (ssAns[i]){
    //     for (uint j = 0; j < stkcnt; ++j)
    //      if (stkAns[j] && strcmp(ssAns[i], stkAns[j]) == 0){
    //         ++cnt;
    //         break;
    //      }
    //  }
    // printf("%d %d\n", cnt, sscnt);
    // double ss_rate_pr = (double)cnt / sscnt;
    



    // cnt = 0;
    // for (uint i = 0; i < stkcnt; ++i)
    //  if (stkAns[i]){
    //      for (uint j = 0; j < sscnt; ++j)
    //       if (ssAns[j] && strcmp(ssAns[j], stkAns[i]) == 0){
    //           ++cnt;
    //           break;
    //       }
    //  }
    // double ss_rate_rr = (double)cnt / stkcnt;



    // FILE *outf = stdout;
    // fprintf(outf, "%d,%.10lf,%.10lf\n", K, ss_rate_pr, ss_rate_rr);
    // fclose(outf);
  




    double memory = 0.05;
    SC * sc = NULL;

    int d = 4;

    int bucket_num = 50;
    int counter_num = 16;
    double M_SIMD = bucket_num * counter_num * 8.0 / (1024 * 1024);
    printf("M_SIMD = %lf\n", M_SIMD);

    double M1 = 0.5 * memory - M_SIMD;
    double M2 = 0.5 * memory;
    int T1 = 15;
    int T2 = 2048 - 15;
    int T = T1 + T2;


    // int counter_size = 10;
    // int w = memory * 1024 * 1024 * 8.0 / counter_size;
    const int K = 16;
    const int H1 = 2 * K;
    const int H2 = 1 * K;


    SS1 *ss;
    ss = new SS1(K, H1);
    SS1 *ss_sc;
    ss_sc = new SS1(K, H2);

    sc = new SC(bucket_num, counter_num, d, M1, d, M2, T2);
    sc->init_spa(ss_sc);

    for(int i = 0; i < package_num; i++)
    {
        ss->Insert(insert[i], 1, 0);
        sc->Insert(insert[i], 0);
    }
    sc->refresh(0);


    int cnt = 0;
    for(unordered_map<uint, int>::iterator it = unmp.begin(); it != unmp.end(); it++)
    {
        t[++cnt].x = it->first; 
        t[cnt].y = it->second;
    }
    sort(t + 1, t + cnt + 1, CMP);

    printf("Top-(k+1) = %d\n", t[K + 1].y);


    cnt = 0;
    uint * ID_ss = ss->Get_ID();
    int * count_ss = ss->Get_count();
    for(int i = 0; i < H1; i++)
    {
        t1[++cnt].x = ID_ss[i]; 
        t1[cnt].y = count_ss[i];
    }
    sort(t1 + 1, t1 + cnt + 1, CMP);


    cnt = 0;
    uint * ID_ss_sc = ss_sc->Get_ID();
    int * count_ss_sc = ss_sc->Get_count();
    for(int i = 0; i < H2; i++)
    {
        t2[++cnt].x = ID_ss_sc[i]; 
        t2[cnt].y = count_ss_sc[i];
    }
    sort(t2 + 1, t2 + cnt + 1, CMP);






    int correct_cnt1 = 0, correct_cnt2 = 0;
    int stand_val1 = 0, stand_val2 = 0;

    double RE_ss = 0;
    double RE_ss_sc = 0;


    for (int i = 1; i <= K; i++)
    {
        uint X = t1[i].x;
        stand_val1 = unmp[X];

        for (int j = 1; j <= K; j++) 
        {
            if (X == t[j].x)
            {
                correct_cnt1++;
                RE_ss += fabs(t1[i].y - stand_val1) * 1.0 / stand_val1;
            }
        }
    }


    for(int i = 1; i <= K; i++)
    {
        uint X = t2[i].x;
        stand_val2 = unmp[X];
     
        for (int j = 1; j <= K; j++)
        {
            if (X == t[j].x)
            {
                correct_cnt2++;
                RE_ss_sc += fabs(t2[i].y + T - stand_val2) * 1.0 / stand_val2;
            }
        }
    }


    double a = package_num * 1.0;
    double b = unmp.size() * 1.0;
    // fprintf(stderr, "SS1 precision rate %d/%d = %lf, recall rate %d/%d = %lf, with RE = %lf\n", correct_cnt1, H1, correct_cnt1 * 1.0 / H1, correct_cnt1, K, correct_cnt1 * 1.0 / K, RE_ss);
    // fprintf(stderr, "SS2 precision rate %d/%d = %lf, recall rate %d/%d = %lf, with RE = %lf\n", correct_cnt2, H2, correct_cnt2 * 1.0 / H2, correct_cnt2, K, correct_cnt2 * 1.0 / K, RE_ss_sc);

    fprintf(stderr, "SS1 accepted %d/%d = %lf, with ARE = %lf\n", correct_cnt1, K, correct_cnt1 * 1.0 / K, RE_ss * 1.0 / K);
    fprintf(stderr, "SS2 accepted %d/%d = %lf, with ARE = %lf\n", correct_cnt2, K, correct_cnt2 * 1.0 / K, RE_ss_sc * 1.0 / K);
    


#endif//SS

/*******************************************************************/




/********************* test for the FlowRadar ********************/

#ifdef HC

    FILE *file_FlowTraffic = fopen(filename_FlowTraffic, "rb");

    char *ip[8];


    int package_num = 0;
    while(fread(ip, 1, 8, file_FlowTraffic))
    {
        uint key = *((uint *)ip);
        insert[package_num] = key;

        unmp[key]++;
        package_num++;

        if(package_num == MAX_INSERT_PACKAGE / 2)
            break;
    }

    int M = unmp.size();

    while(fread(ip, 1, 8, file_FlowTraffic))
    {
        uint key = *((uint *)ip);
        insert[package_num] = key;

        unmp[key]--;
        package_num++;

        if(package_num == MAX_INSERT_PACKAGE)
            break;
    }
    fclose(file_FlowTraffic);







    double memory = 1.0;
    int threshold = 1000;




    double a = 0.10, b = 0.90;
    double bf_m = memory * a;
    double ibf_m = memory * b;
    int d_bf = 4;
    int d_ibf = 4;

    // printf("dataset1 flow number = %d\n\n", M);

    Myrev1 *re = NULL;
    re = new Myrev1(bf_m, ibf_m, d_bf, d_ibf, threshold);






    a = 0.09, b = 0.81;
    double c = 1 - a - b;

    bf_m = memory * a;
    ibf_m = memory * b;
    double sc_m = memory * c;

    d_bf = 4;
    d_ibf = 4;

    SC * sc = NULL;

    int d = 4;

    int bucket_num = 100;
    int counter_num = 16;
    double M_SIMD = bucket_num * counter_num * 8.0 / (1024 * 1024);
    // printf("M_SIMD = %lf\n", M_SIMD);

    double M1 = 0.5 * sc_m - M_SIMD;
    double M2 = 0.5 * sc_m;
    int T1 = 15;
    int T2 = 140;
    int T = T1 + T2;

    Myrev1 *re_sc;
    re_sc = new Myrev1(bf_m, ibf_m, d_bf, d_ibf, threshold);

    sc = new SC(bucket_num, counter_num, d, M1, d, M2, T2);
    sc->init_spa(re_sc);









    for(int i = 0; i < MAX_INSERT_PACKAGE / 2; i++)
    {
        re->Insert(insert[i], 1, 0);
        sc->Insert(insert[i], 0);
    }
    sc->refresh(0);

    re->dump(NULL, 0);
    re_sc->dump(sc, 0);

    sc->init_array_all();


    printf("num_flow1 = %d\n", re->num_flow);
    printf("num_flow2 = %d\n", re_sc->num_flow);


    for (int i = MAX_INSERT_PACKAGE / 2; i < package_num; i++)
    {
        //ID, f, seq
        re->Insert(insert[i], 1, 1);
        //ID, seq
        sc->Insert(insert[i], 1);
    }
    sc->refresh(1);

    re->dump(NULL, 1);
    re_sc->dump(sc, 1);

    sc->init_array_all();

    // printf("dataset1 flow number (cu) = %d\n\n", re_sc->unmp_flow.size());



    unmp_myres[0].clear();
    re->rev(&(unmp_myres[0]));

    unmp_myres[1].clear();
    re_sc->rev(&(unmp_myres[1]));




    int stand_cnt = 0;
    int my_cnt1 = 0, my_cnt2 = 0 ;
    int my_cor_cnt1 = 0, my_cor_cnt2 = 0;

    my_cnt1 = unmp_myres[0].size();
    printf("%d\n", my_cnt1);
    my_cnt2 = unmp_myres[1].size();
    printf("%d\n", my_cnt2);



    int count;
    char temp[100];
    uint key;
    int total_change = 0;
    for(unordered_map<uint, int>::iterator it = unmp.begin(); it != unmp.end(); it++)
    {
        key = it->first;
        total_change += abs(it->second);

        if(abs(it->second) > threshold)
        {
            stand_cnt++;

            unmp_standres[key] = abs(it->second);
            
            if(unmp_myres[0].count(key) > 0)
            {
                my_cor_cnt1 ++;
            }
            if(unmp_myres[1].count(key) > 0)
            {
                my_cor_cnt2 ++;
            }
        }
    }
    printf("total_change = %d\n", total_change);

    printf("rev1: precision rate = %lf, recall rate = %lf\n", my_cor_cnt1 * 1.0 / my_cnt1, my_cor_cnt1 * 1.0 / stand_cnt);
    printf("rev2: precision rate = %lf, recall rate = %lf\n", my_cor_cnt2 * 1.0 / my_cnt2, my_cor_cnt2 * 1.0 / stand_cnt);



#endif//HC


/*****************************************************************/



    return 0;

}

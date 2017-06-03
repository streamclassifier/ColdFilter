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
#include "Myrev1.h"
#include "Myrev2.h"
#include "params.h"

using namespace std;

char * filename_FlowTraffic = "./format_data/formatted01.dat";
char * filename_FlowTraffic2 = "./format_data/formatted02.dat";


uint insert[MAX_INSERT_PACKAGE * 2 + 100];
unordered_map<uint ,int> unmp_myres[2];
unordered_map<uint ,int> unmp_standres;

unordered_map<uint, int> unmp;


int main(int argc, char** argv)
{
    if(argc == 2)
    {
        filename_FlowTraffic = argv[1];
    }

	unmp.clear();

    FILE *file_FlowTraffic = fopen(filename_FlowTraffic, "rb");
    FILE *file_FlowTraffic2 = fopen(filename_FlowTraffic2, "rb");

    char *ip[8];


    int package_num = 0;
    while(fread(ip, 1, 8, file_FlowTraffic))
    {
        uint key = *((uint *)ip);
        insert[package_num] = key;

        unmp[key]++;
        package_num++;

        if(package_num == MAX_INSERT_PACKAGE)
            break;
    }
    // fclose(file_FlowTraffic);

    int M = unmp.size();

    while(fread(ip, 1, 8, file_FlowTraffic))
    {
        uint key = *((uint *)ip);
        insert[package_num] = key;

        unmp[key]--;
        package_num++;

        if(package_num == MAX_INSERT_PACKAGE * 2)
            break;
    }
    fclose(file_FlowTraffic);


    // printf("dataset: %s and %s\n", filename_FlowTraffic, filename_FlowTraffic2);
    // printf("total stream size = %d\n", package_num);
    // printf("distinct item number = %d\n", unmp.size());

    // int max_freq = 0;
    // unordered_map<uint, int>::iterator it = unmp.begin();

    // for(int i = 0; i < unmp.size(); i++, it++)
    // {
    //     int temp2 = abs(it->second);
    //     max_freq = max_freq > temp2 ? max_freq : temp2;
    // }
    // printf("max_changed_freq = %d\n", max_freq);




    double memory = 1.0;
    int threshold = 500;


    double a = 0.10, b = 0.90;

    double bf_m = memory * a;
    double ibf_m = memory * b;

    int w_bf = bf_m * 1024 * 1024 * 8.0 / 1;
    // int d_bf = w_bf * 1.0 / M * log(2) / log(2.71828);
    int d_bf = 4;

    int w_ibf = ibf_m * 1024 * 1024 / 16;
    int d_ibf = 3;

    printf("rev1\n");
    printf("d_bf = %d\n", d_bf);
    printf("w_ibf = %d, dataset1 flow number = %d\n\n", w_ibf, M);

    Myrev1 *re1;
    re1 = new Myrev1(w_bf, w_ibf, d_bf, d_ibf, threshold);




    a = 0.02, b = 0.88;
    double c = 0.1;

    bf_m = memory * a;
    ibf_m = memory * b;
    double cu_m = memory * c;

    w_bf = bf_m * 1024 * 1024 * 8.0 / 1;
    // d_bf = w_bf * 1.0 / 43065 * log(2) / log(2.71828);
    d_bf = 4;

    printf("rev2\n");
    printf("d_bf = %d\n", d_bf);


    w_ibf = ibf_m * 1024 * 1024 / 16;
    d_ibf = 3;

    int d_cu = 3;
    int count_size_cu = 4;
    int w_cu = cu_m * 1024 * 1024 * 8.0 / count_size_cu;


	Myrev2 *re2;
	re2 = new Myrev2(w_bf, w_ibf, d_bf, d_ibf, w_cu, d_cu, count_size_cu, threshold);








    for(int i = 0; i < MAX_INSERT_PACKAGE; i++)
    {
        re1->Insert(insert[i], 0);
        re2->Insert(insert[i], 0);
    }
    for (int i = MAX_INSERT_PACKAGE; i < package_num; i++)
    {
        re1->Insert(insert[i], 1);
        re2->Insert(insert[i], 1);
    }



    printf("w_ibf = %d, dataset1 flow number (cu) = %d\n\n", w_ibf, re2->unmp_flow.size());

    // printf("bloom num_flow = %d, stand num_flow = %d \n", num_flow / d_ibf, unmp_flow.size());


    unmp_myres[0].clear();
    re1->rev(&(unmp_myres[0]));
    // re1->rev_diff(&(unmp_myres[0]));

    unmp_myres[1].clear();
    re2->rev(&(unmp_myres[1]));
    // re2->rev_diff(&(unmp_myres[1]));




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
    for(unordered_map<uint, int>::iterator it = unmp.begin(); it != unmp.end(); it++)
    {
        key = it->first;

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

    printf("rev1: precision rate = %lf, recall rate = %lf\n", my_cor_cnt1 * 1.0 / my_cnt1, my_cor_cnt1 * 1.0 / stand_cnt);
    printf("rev2: precision rate = %lf, recall rate = %lf\n", my_cor_cnt2 * 1.0 / my_cnt2, my_cor_cnt2 * 1.0 / stand_cnt);


    return 0;

}

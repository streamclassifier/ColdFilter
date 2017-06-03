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
#include <vector>
#include "params.h"
#include "classify1.h"


using namespace std;

char * filename_FlowTraffic = "../../BloomSketch/2";

char insert[10000000 + 10000000 / 5][200];

unordered_map<string, int> unmp;


int main(int argc, char** argv)
{
    if(argc == 2)
    {
        filename_FlowTraffic = argv[1];
    }

	unmp.clear();
	uint ip_s, ip_d;
	int val;

    int package_num = 0;
    FILE *file_FlowTraffic = fopen(filename_FlowTraffic, "r");
    while(fgets(insert[package_num], 200, file_FlowTraffic) != NULL)
    {
        unmp[string(insert[package_num])]++;
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




    double memory = 1.0;
    double coef = 0.9;

    int lcs = 4;
    int dl = 4;
    int wl = memory * coef * 1024 * 1024 * 8.0 / lcs;
    
    // int hcs = 1;
    // int dh = 30;

    int hcs = 2;
    int dh = 10;

    int wh = memory * (1 - coef) * 1024 * 1024 * 8.0 / hcs;

    // printf("w_l = %d, w_h = %d\n", wl, wh);
    classify1 * cls = new classify1(wl, wh, dl, dh, lcs, hcs);
    

    // int lcs2 = 4;
    // int dl2 = 4;
    // int wl2 = memory * coef * 1024 * 1024 * 8.0 / lcs;
    
    // int hcs2 = 2;
    // int dh2 = 10;
    // int wh2 = memory * (1 - coef) * 1024 * 1024 * 8.0 / hcs;
 
    // classify1 * cls2 = new classify1(wl2, wh2, dl2, dh2, lcs2, hcs2);


    
    for(int i = 0; i < package_num; i++)
    {
        cls->Insert(insert[i]);
    }

    int N = cls->N;
    int M = wh;
    int K = (M * 1.0 / N) * log(2) / log(2.71828);

    printf("N = %d, M = %d, K = %d\n", N, M, K);



    char temp[500];

    int threhold = ((1 << lcs) - 1) * dl + ((1 << hcs) - 1) * dh;
    int stand_ele = 0;
    int my_ele = 0;
    int cor_ele = 0;

    for(unordered_map<string, int>::iterator it = unmp.begin(); it != unmp.end(); it++)
    {
    	strcpy(temp, (it->first).c_str());
    	val = it->second;
		
        bool isele = cls->Query(temp);
        if(isele)
        {
            my_ele ++;
        }

        if(val > threhold)
        {
            stand_ele ++;
            if(isele)
            {
                cor_ele++;
            }
        }
    }

    double a = package_num * 1.0;
    double b = unmp.size() * 1.0;

    printf("man_insert %lf\n", cls->man_insert * 1.0 / a);
    printf("man_query %lf\n", cls->man_query * 1.0 / b);

    printf("precision rate = %lf\n", cor_ele * 1.0 / stand_ele);
    printf("recall rate = %lf\n", cor_ele * 1.0 / my_ele);


    return 0;
}

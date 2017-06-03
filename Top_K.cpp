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

#include "SS1.h"
#include "SS2.h"
#include "SS3.h"


using namespace std;
int param=256;


uint insert[MAX_INSERT_PACKAGE + MAX_INSERT_PACKAGE / 500];

unordered_map<uint, int> unmp;
struct node {uint x; int y;} t[MAX_INSERT_PACKAGE + MAX_INSERT_PACKAGE / 500];
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


    unmp.clear();
    uint ip_s, ip_d;
    int val;

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
        int temp2 = it->second;
        max_freq = max_freq > temp2 ? max_freq : temp2;
    }
    printf("max_freq = %d\n", max_freq);




    double memory = 1.0;


    int counter_size = 10;
    int w = memory * 1024 * 1024 * 8.0 / counter_size;

    SS1 *ss1;
    ss1 = new SS1(param);
    SS2 *ss2;
    ss2 = new SS2(param, w, 4, counter_size);


    for(int i = 0; i < package_num; i++)
    {
        ss1->Insert(insert[i]);
        ss2->Insert(insert[i]);
    }

    int cnt = 0;
    for(unordered_map<uint, int>::iterator it = unmp.begin(); it != unmp.end(); it++)
    {
        t[++cnt].x = it->first; 
        t[cnt].y = it->second;
    }
    sort(t + 1, t + cnt + 1, CMP);

    int val1 = 0, val2 = 0, val3 = 0;
    int stand_val1 = 0, stand_val2 = 0, stand_val3 = 0;
    
    int * count_ss1 = ss1->Get_count();
    int * count_ss2 = ss2->Get_count();


    double RE_ss1 = 0;
    double RE_ss2 = 0;


    for (int i = 0; i < param; i++)
    {
        uint X = ss1->Query(i);
        stand_val1 = unmp[X];

        for (int j = 1; j <= param; j++) 
        {
            if (X == t[j].x)
            {
                val1++;
            }
        }
        RE_ss1 += fabs(count_ss1[i] - stand_val1) * 1.0 / stand_val1;
        // printf("%d vs %d\n", count_ss1[i], stand_val1);


        X = ss2->Query(i);
        stand_val2 = unmp[X];
     
        for (int j = 1; j <= param; j++)
        {
            if (X == t[j].x)
            {
                val2++;
            }
        }
        RE_ss2 += fabs(count_ss2[i] - stand_val2) * 1.0 / stand_val2;
        // printf("%d vs %d\n\n", count_ss2[i], stand_val2);
        
    }


    double a = package_num * 1.0;
    double b = unmp.size() * 1.0;
    fprintf(stderr, "SS1 accepted %d/%d = %lf, with RE = %lf\n",val1, param, val1 * 1.0 / param, RE_ss1);
    fprintf(stderr, "SS2 accepted %d/%d = %lf, with RE = %lf\n",val2, param, val2 * 1.0 / param, RE_ss2);


    return 0;

}

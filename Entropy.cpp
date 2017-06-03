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
#include "params.h"

#include "ori_sieving.h"
#include "sketch_sieving.h"


using namespace std;

char * filename_FlowTraffic = "../../BloomSketch/1";

char insert[10][50];

unsigned int ip_set[MAX_INSERT_PACKAGE + MAX_INSERT_PACKAGE / 10];

unordered_map<unsigned int, int> unmp;
void parse(const char *str, unsigned int * ip, int *len)
{
    unsigned int now = 0; 
    unsigned int X = 0; 
    int B;
    for (int i = 0;;i++)
    {
        if (str[i]=='.' || str[i]==' ') 
        {
            X = X * 256 + now; 
            now = 0;
        }
        if (str[i]==' ') 
        {
            B = i;
            break;
        }
        if (str[i] >= '0' && str[i] <= '9') 
            now = now * 10 + str[i] - '0';
    }
    * ip = X;
    * len = B;
}
    
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
    while(fgets(insert[0], 50, file_FlowTraffic) != NULL)
    {
        unsigned int ip;
        int len;
        parse(insert[0], &ip, &len);

        unmp[ip]++;
        ip_set[package_num] = ip;
        package_num++;

        if(package_num == MAX_INSERT_PACKAGE)
            break;
    }
    fclose(file_FlowTraffic);



    printf("dataset: %s\n", filename_FlowTraffic);
    printf("total stream size = %d\n", package_num);
    printf("distinct item number = %d\n", unmp.size());

    int max_freq = 0;
    unordered_map<unsigned int, int>::iterator it = unmp.begin();

    double stand_entropy = 0.0;
    for(int i = 0; i < unmp.size(); i++, it++)
    {
        int temp2 = it->second;
        max_freq = max_freq > temp2 ? max_freq : temp2;
        stand_entropy += temp2 * log(temp2) / log(2.0);
    }
    printf("max_freq = %d\n", max_freq);






    double sample_rate = 0.001;
    int g = 50;

    ori_sieving *entropy1;
    entropy1 = new ori_sieving(sample_rate, g);
    sketch_sieving *entropy2;
    entropy2 = new sketch_sieving(sample_rate, g);


    for(int i = 0; i < package_num; i++)
    {
        entropy1->Insert(ip_set[i]);
        entropy2->Insert(ip_set[i]);
    }

    double ori_sieving_entropy1 = entropy1->Query();
    double ori_sieving_entropy2 = entropy2->Query();


    printf("stand_entropy = %lf\n", stand_entropy);
    printf("ori_sieving_entropy1 = %lf\n", ori_sieving_entropy1);
    printf("relative error1 = %lf\n", abs(stand_entropy - ori_sieving_entropy1) / stand_entropy);
    
    printf("ori_sieving_entropy2 = %lf\n", ori_sieving_entropy2);
    printf("relative error2 = %lf\n", abs(stand_entropy - ori_sieving_entropy2) / stand_entropy);
    
    return 0;

}

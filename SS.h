#ifndef _SS_H
#define _SS_H

#include <algorithm>
#include <cstring>
#include <string.h>
#include <iostream>

class SS
{	
public:	
	int *count;
	char **items;
	SS(int item_num)
	{
		count = new int[item_num];
		memset(count, 0, sizeof(int) * item_num);

		items = new char *[item_num];
		for(int i = 0; i < item_num; i++)
		{
			items[i] = new char[100];
			items[i][0] = '\0';
		}
	}
	
};

#endif _SS_H
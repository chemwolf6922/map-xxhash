#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "map.h"


int main(int argc, char const *argv[])
{
    int value = 1;
    map_handle_t map = map_create();

    for(int i=0;i<1000000;i++)
    {
        int key = i;
        // int key = random();
        map_add(map,&key,sizeof(int),&value);
    }
    printf("Conflict ratio:%f\n",map_get_conflict_ratio(map));
    printf("Avg ops:%f\n",map_get_average_ops(map));
    printf("Max ops:%ld\n",map_get_max_ops(map));
    for(int i=0;i<1000000;i++)
    {
        map_remove(map,&i,sizeof(int));
    }
    map_delete(map,NULL,NULL);

    return 0;    
}

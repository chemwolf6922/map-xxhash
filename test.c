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
    for(int i=0;i<1000000;i++)
    {
        int key = i;
        map_add(map,&key,sizeof(int),&value);
    }
    printf("Conflict ratio:%f\n",map_get_conflict_ratio(map));
    printf("Avg ops:%f\n",map_get_average_ops(map));
    printf("Max ops:%ld\n",map_get_max_ops(map));
    for(int i=0;i<1000000;i++)
    {
        map_remove(map,&i,sizeof(int));
    }
    for(int i=0;i<10;i++)
    {
        int key = i;
        map_add(map,&key,sizeof(int),&value);
    }
    size_t keys_len = 0;
    map_key_t* keys = map_keys(map,&keys_len);
    for(int i=0;i<10;i++)
    {
        printf("key: %d\n",*(int*)(keys[i].key));
    }
    free(keys);
    map_delete(map,NULL,NULL);

    return 0;    
}

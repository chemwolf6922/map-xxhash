#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "map.h"


int main(int argc, char const *argv[])
{
    // init random keys
    int random_nums[1000000] = {0};
    for(int i=0;i<1000000;i++)
    {
        random_nums[i] = random();
    }
    // dummy data
    int value = 1;
    // create map
    map_handle_t map = map_create();
    // add entries
    for(int i=0;i<1000000;i++)
    {
        map_add(map,&(random_nums[i]),sizeof(int),&value);
    }
    // repeat add
    for(int i=0;i<1000000;i++)
    {
        map_add(map,&(random_nums[i]),sizeof(int),&value);
    }
    // metrics
    printf("Conflict ratio:%f\n",map_get_conflict_ratio(map));
    printf("Avg ops:%f\n",map_get_average_ops(map));
    printf("Max ops:%ld\n",map_get_max_ops(map));
    // remove entries
    for(int i=0;i<1000000;i++)
    {
        map_remove(map,&(random_nums[i]),sizeof(int));
    }
    // test clear
    for(int i=0;i<1000000;i++)
    {
        map_add(map,&(random_nums[i]),sizeof(int),&value);
    }
    map_clear(map,NULL,NULL);
    for(int i=0;i<1000000;i++)
    {
        map_add(map,&(random_nums[i]),sizeof(int),&value);
    }
    map_clear(map,NULL,NULL);
    // keys() iterator demo
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
    // forEach iterator demo
    map_entry_t entry;
    map_forEach(map,entry)
    {
        printf("key: %d, value: %d\n",*(int*)(entry.key.key),*(int*)(entry.value));
    }
    // delete map
    map_delete(map,NULL,NULL);

    return 0;    
}

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "map.h"

static int random_nums[1000000] = {0};

#define TIME_SECTION(label, code) do { \
    struct timespec _ts_start, _ts_end; \
    clock_gettime(CLOCK_MONOTONIC, &_ts_start); \
    code \
    clock_gettime(CLOCK_MONOTONIC, &_ts_end); \
    long _elapsed_us = (_ts_end.tv_sec - _ts_start.tv_sec) * 1000000L \
                     + (_ts_end.tv_nsec - _ts_start.tv_nsec) / 1000L; \
    printf("[%s] %ld us\n", label, _elapsed_us); \
} while(0)


int main(int argc, char const *argv[])
{
    (void)argc;
    (void)argv;
    // init random keys
    for(int i=0;i<1000000;i++)
    {
        random_nums[i] = random();
    }
    // dummy data
    int value = 1;
    // create map
    map_handle_t map = map_create();
    // add entries
    TIME_SECTION("add entries", {
        for(int i=0;i<1000000;i++)
        {
            map_add(map,&(random_nums[i]),sizeof(int),&value);
        }
    });
    // repeat add
    TIME_SECTION("repeat add", {
        for(int i=0;i<1000000;i++)
        {
            map_add(map,&(random_nums[i]),sizeof(int),&value);
        }
    });
    // metrics
    printf("Conflict ratio:%f\n",map_get_conflict_ratio(map));
    printf("Avg ops:%f\n",map_get_average_ops(map));
    printf("Max ops:%ld\n",map_get_max_ops(map));
    // find entries
    TIME_SECTION("find entries", {
        for(int i=0;i<1000000;i++)
        {
            map_get(map,&(random_nums[i]),sizeof(int));
        }
    });
    // remove entries
    TIME_SECTION("remove entries", {
        for(int i=0;i<1000000;i++)
        {
            map_remove(map,&(random_nums[i]),sizeof(int));
        }
    });
    // delete map
    map_delete(map,NULL,NULL);

    return 0;
}

#ifndef __MAP_H
#define __MAP_H

#include <stddef.h>
#include <stdbool.h>

typedef void* map_handle_t;

// only used for return values
typedef struct
{
    void* key;
    size_t len;
}map_key_t;
typedef struct
{
    map_key_t key;
    void* value;
}map_entry_t;

map_handle_t map_create(void);
int map_delete(map_handle_t handle,void(*free_value)(void* value,void* ctx),void* ctx);
// Return NULL if failed.
// Return value if key is new.
// Return the replaced value if key exists.
void* map_add(map_handle_t handle,void* key,size_t key_len,void* value);
void* map_remove(map_handle_t handle,void* key,size_t key_len);
void* map_get(map_handle_t handle,void* key,size_t key_len);
bool map_has(map_handle_t handle,void* key,size_t key_len);
size_t map_get_length(map_handle_t handle);
// Keys are only references. They will be invalid as soon as the map is changed.
// !! return value needs to be freed after use
map_key_t* map_keys(map_handle_t handle,size_t* len);
// !! return value needs to be freed after use
void** map_values(map_handle_t handle,size_t* len);
// !! return value needs to be freed after use
map_entry_t* map_entries(map_handle_t handle,size_t* len);

/* for diagnose */
float map_get_conflict_ratio(map_handle_t* handle);
float map_get_average_ops(map_handle_t* handle);
size_t map_get_max_ops(map_handle_t* handle);

#endif
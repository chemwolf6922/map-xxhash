#pragma once

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* map_handle_t;

typedef struct
{
    const void* key;
    size_t len;
}map_key_t;
typedef struct
{
    map_key_t key;
    void* value;
}map_entry_t;

/**
 * @brief Create a new map.
 * 
 * @return map_handle_t 
 */
map_handle_t map_create(void);

/**
 * @brief Delete the map. NOT for removing one item in the map.
 * 
 * @param handle The map handle to be deleted.
 * @param free_value The function to free the values in the map. Can be NULL if not used.
 * @param ctx The context to be passed to free_value. Can be NULL if not used.
 * @return int 0 if success, -1 if failed.
 */
int map_delete(map_handle_t handle,void(*free_value)(void* value,void* ctx),void* ctx);

/**
 * @brief Add a key-value pair to the map. If the key already exists, replace the value and return the old value.
 * 
 * @param handle The map handle.
 * @param key The key to be added. The key will be copied to the map.
 * @param key_len The length of the key memory block.
 * @param value The value to be added. Owned by the caller. NULL is not allowed.
 * @return void* NULL if failed, the value if key is new, the replaced value if key exists.
 */
void* map_add(map_handle_t handle, const void* key, size_t key_len, void* value);

/**
 * @brief Remove a key-value pair from the map. Return the removed value. If there is no such entry, return NULL.
 * 
 * @param handle The map handle.
 * @param key The key to be removed.
 * @param key_len The length of the key memory block.
 * @return void* NULL if there is no such entry, the removed value otherwise.
 */
void* map_remove(map_handle_t handle, const void* key, size_t key_len);
/*
    Clear the map.
    Use free_value and ctx to free the values themselves.
    Those two can be NULL if not used.
*/

/**
 * @brief Clear the map's entries.
 * 
 * @param handle The map handle.
 * @param free_value The function to free the values in the map. Can be NULL if not used.
 * @param ctx The context to be passed to free_value. Can be NULL if not used.
 * @return int 0 if success, -1 if failed.
 */
int map_clear(map_handle_t handle,void(*free_value)(void* value,void* ctx),void* ctx);

/* Return NULL if the entry does not exist */

/**
 * @brief Get the value of the entry with the given key. Return NULL if there is no such entry.
 * 
 * @param handle The map handle.
 * @param key The key to be searched.
 * @param key_len The length of the key memory block.
 * @return void* NULL if there is no such entry, the value otherwise.
 */
void* map_get(map_handle_t handle, const void* key, size_t key_len);

/**
 * @brief Check if the map has an entry with the given key.
 * 
 * @param handle The map handle.
 * @param key The key to be searched.
 * @param key_len The length of the key memory block.
 * @return true
 * @return false 
 */
bool map_has(map_handle_t handle, const void* key, size_t key_len);

/**
 * @brief Get the number of entries in the map.
 * 
 * @param handle The map handle.
 * @return size_t 
 */
size_t map_get_length(map_handle_t handle);

/**
 * @brief Return the map keys as a map_key_t* array.
 * @warning DO NOT modify the keys! For they are references to the map keys owned by the map.
 * 
 * @param handle The map handle.
 * @param len The pointer to the variable to store the array length.
 * @return map_key_t* The array of keys. Needs to be freed by the caller. NULL if failed.
 */
map_key_t* map_keys(map_handle_t handle,size_t* len);

/**
 * @brief Return the map values as a void* array.
 * 
 * @param handle The map handle.
 * @param len The pointer to the variable to store the array length.
 * @return void** The array of values. Needs to be freed by the caller. NULL if failed.
 */
void** map_values(map_handle_t handle, size_t* len);

/**
 * @brief Return the map entries as a map_entry_t* array.
 * @warning DO NOT modify the keys! For they are references to the map keys owned by the map.
 * 
 * @param handle The map handle.
 * @param len The pointer to the variable to store the array length.
 * @return map_entry_t* The array of entries. Needs to be freed by the caller. NULL if failed.
 */
map_entry_t* map_entries(map_handle_t handle,size_t* len);

/**
 * @brief Initialize the map forEach loop.
 * @warning DO NOT use this directly. Use map_forEach instead.
 * 
 * @param handle The map handle.
 * @param entry The map_entry_t variable to store the current entry. Needs to be provided by the caller.
 * @return int 0 if success, -1 if failed.
 */
int map_forEach_start(map_handle_t handle,map_entry_t* entry);

/**
 * @brief Progress the map forEach loop to the next entry.
 * @warning DO NOT use this directly. Use map_forEach instead.
 * 
 * @param handle The map handle.
 * @param entry The map_entry_t variable to store the current entry. Needs to be provided by the caller.
 * @return int 0 if there is a next entry, -1 if the loop is finished or failed.
 */
int map_forEach_next(map_handle_t handle,map_entry_t* entry);

/**
 * @brief Iterate through the map entries.
 * @warning DO NOT modify the map during this operation!
 * @warning The order of entries is undefined.
 * 
 * @param handle The map handle.
 * @param entry The map_entry_t variable to store the current entry. Needs to be provided by the caller.
 */
#define map_forEach(handle,entry) \
for(int i_f921f793=map_forEach_start(handle,&entry);\
        i_f921f793==0;\
        i_f921f793=map_forEach_next(handle,&entry))

#ifdef MAP_ENABLE_DIAGNOSTIC

/* for diagnose */
float map_get_conflict_ratio(map_handle_t handle);
float map_get_average_ops(map_handle_t handle);
size_t map_get_max_ops(map_handle_t handle);

#endif

#ifdef __cplusplus
}
#endif

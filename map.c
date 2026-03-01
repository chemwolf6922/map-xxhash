#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "map.h"

#ifndef USE_SIMPLE_HASH
#include "xxHash/xxhash.h"
#endif

#define MIN_HASH_TABLE_SIZE (1<<4)

typedef struct list_head_s list_head_t;
struct list_head_s
{
    list_head_t* next;
};

typedef struct hash_node_s
{
    /** This MUST be at the begining. */
    list_head_t list_head;
    void* value;
    uint32_t full_hash;
    size_t key_len;
    uint8_t key[0];
} hash_node_t;

typedef struct
{
    list_head_t* hash_table;
    uint32_t hash_mask;
    size_t table_len;
    size_t item_len;
    size_t decrease_th;
    /** For iterator */
    size_t iterator_cnt;
    hash_node_t* iterator_node;
} map_t;

/* pre defines */
static inline uint32_t get_hash(const uint8_t* data,size_t len)
{
#ifndef USE_SIMPLE_HASH
    return (uint32_t)XXH3_64bits(data,len);
#else
    uint32_t h = 0;
    for(size_t i = 0; i < len; i++)
        h = h * 263 + data[i];
    return h;
#endif
}
static inline hash_node_t* map_locate(
    map_t* map, const void* key, size_t key_len, list_head_t** p_prev);
static inline void try_increase_hash_table(map_t* map);
static inline void try_decrease_hash_table(map_t* map);

/* public methods */

map_handle_t map_create(void)
{
    map_t* map = malloc(sizeof(map_t));
    if(!map)
        goto error;
    memset(map,0,sizeof(map_t));
    map->item_len = 0;
    /**
     * decrase threshold = table_len/4
     * decrase threshold is 0 if it < min_hash_table_size/2
     */
    map->decrease_th = 0;
    map->table_len = MIN_HASH_TABLE_SIZE;
    map->hash_mask = (uint32_t)(map->table_len) - 1;
    map->hash_table = malloc(sizeof(list_head_t)*(map->table_len));
    if(!map->hash_table)
        goto error;
    memset(map->hash_table,0,sizeof(list_head_t)*(map->table_len));

    return (map_handle_t)map;
error:
    if(map)
    {
        if(map->hash_table != NULL)
            free(map->hash_table);
        free(map);
    }
    return NULL;
}

int map_clear(map_handle_t handle,void(*free_value)(void* value,void* ctx),void* ctx)
{
    if(!handle)
        return -1;
    map_t* map = (map_t*)handle;
    /* free values */
    for(size_t i=0;i<map->table_len;i++)
    {
        list_head_t* entry = &(map->hash_table[i]);
        hash_node_t* node = (hash_node_t*)(entry->next);
        while(node)
        {
            hash_node_t* next = (hash_node_t*)(node->list_head.next);
            if(free_value)
            {
                free_value(node->value,ctx);
            }
            // key & node use one block of memory
            free(node);
            node = next;
        }
    }
    map->item_len = 0;
    /* Shrink hash table */
    list_head_t* new_table = realloc(map->hash_table,sizeof(list_head_t)*MIN_HASH_TABLE_SIZE);
    if (!new_table)
    {
        /** Shrink failed (How?). Wipe the old table */
        memset(map->hash_table, 0, sizeof(list_head_t) * (map->table_len));
        return 0;
    }
    memset(new_table,0,sizeof(list_head_t)*MIN_HASH_TABLE_SIZE);
    map->hash_table = new_table;
    map->decrease_th = 0;
    map->table_len = MIN_HASH_TABLE_SIZE;
    map->hash_mask = (uint32_t)(map->table_len) - 1;
    return 0;
}

int map_delete(map_handle_t handle,void(*free_value)(void* value,void* ctx),void* ctx)
{
    int result = map_clear(handle,free_value,ctx);
    if(result != 0)
        return result;
    map_t* map = (map_t*)handle;
    free(map->hash_table);
    free(map);
    return 0;
}

void* map_add(map_handle_t handle, const void* key, size_t key_len, void* value)
{
    if(!handle || !key || key_len == 0 || !value)
        return NULL;
    map_t* map = (map_t*)handle;

    /** check for repeat key */
    uint32_t full_hash = get_hash(key,key_len);
    uint32_t hash = full_hash & map->hash_mask;
    list_head_t* entry = &(map->hash_table[hash]);
    hash_node_t* old_node = (hash_node_t*)(entry->next);
    while(old_node)
    {
        if(old_node->full_hash == full_hash && old_node->key_len == key_len)
        {
            if(memcmp(old_node->key,key,key_len)==0)
            {
                void* old_value = old_node->value;
                old_node->value = value;
                return old_value;
            }
        }
        old_node = (hash_node_t*)old_node->list_head.next;
    }

    /** key & node use one block of memory */
    hash_node_t* new_node = malloc(sizeof(hash_node_t)+key_len);
    if(!new_node)
        return NULL;

    /** redundant operation */
    // new_node->next = NULL;
    new_node->key_len = key_len;
    memcpy(new_node->key,key,key_len);
    new_node->value = value;
    new_node->full_hash = full_hash;
    new_node->list_head.next = entry->next;
    entry->next = (list_head_t*)new_node;
    map->item_len ++;

    try_increase_hash_table(map);

    return value;
}

void* map_remove(map_handle_t handle, const void* key, size_t key_len)
{
    if(!handle || !key || key_len == 0)
        return NULL;
    map_t* map = (map_t*)handle;
    list_head_t* prev_head = NULL;
    hash_node_t* node = map_locate(map,key,key_len,&prev_head);
    if(!node)
        return NULL;
    void* value = node->value;
    /** Checking of prev_head != NULL is redundant. */
    prev_head->next = node->list_head.next;
    free(node);
    map->item_len--;

    try_decrease_hash_table(map);

    return value;
}

void* map_get(map_handle_t handle, const void* key, size_t key_len)
{
    if(!handle || !key || key_len == 0)
        return NULL;
    map_t* map = (map_t*)handle;
    hash_node_t* node = map_locate(map,key,key_len,NULL);
    if(node)
        return node->value;
    return NULL;
}

bool map_has(map_handle_t handle, const void* key, size_t key_len)
{
    if(!handle || !key || key_len == 0)
        return false;
    map_t* map = (map_t*)handle;
    return map_locate(map,key,key_len,NULL) != NULL;
}

size_t map_get_length(map_handle_t handle)
{
    /** Trust handle != NULL to avoid return -1 (SIZE_MAX) */
    map_t* map = (map_t*)handle;
    return map->item_len;
}

map_key_t* map_keys(map_handle_t handle,size_t* len)
{
    if(!handle)
    {
        *len = 0;
        return NULL;
    }
    map_t* map = (map_t*)handle;
    if (map->item_len == 0)
    {
        *len = 0;
        return NULL;
    }
    map_key_t* keys = malloc(sizeof(map_key_t) * (map->item_len));
    if(!keys)
    {
        *len = 0;
        return NULL;
    }
    size_t i = 0;
    for(size_t j=0; j<map->table_len; j++)
    {
        hash_node_t* node = (hash_node_t*)(map->hash_table[j].next);
        while(node)
        {
            keys[i].key = node->key;
            keys[i].len = node->key_len;
            i++;
            node = (hash_node_t*)(node->list_head.next);
        }
    }
    *len = map->item_len;
    return keys;
}

void** map_values(map_handle_t handle,size_t* len)
{
    if(!handle)
    {
        *len = 0;
        return NULL;
    }
    map_t* map = (map_t*)handle;
    if (map->item_len == 0)
    {
        *len = 0;
        return NULL;
    }
    void** values = malloc(sizeof(void*) * (map->item_len));
    if(!values)
    {
        *len = 0;
        return NULL;
    }
    size_t i = 0;
    for(size_t j=0;j<map->table_len;j++)
    {
        hash_node_t* node = (hash_node_t*)(map->hash_table[j].next);
        while(node != NULL)
        {
            values[i] = node->value;
            i++;
            node = (hash_node_t*)(node->list_head.next);
        }
    }
    *len = map->item_len;
    return values;
}

map_entry_t* map_entries(map_handle_t handle,size_t* len)
{
    if(!handle)
    {
        *len = 0;
        return NULL;
    }
    map_t* map = (map_t*)handle;
    if (map->item_len == 0)
    {
        *len = 0;
        return NULL;
    }
    map_entry_t* entries = malloc(sizeof(map_entry_t) * (map->item_len));
    if(!entries)
    {
        *len = 0;
        return NULL;
    }
    size_t i = 0;
    for(size_t j=0;j<map->table_len;j++)
    {
        hash_node_t* node = (hash_node_t*)(map->hash_table[j].next);
        while(node != NULL)
        {
            entries[i].key.key = node->key;
            entries[i].key.len = node->key_len;
            entries[i].value = node->value;
            i++;
            node = (hash_node_t*)(node->list_head.next);
        }
    }
    *len = map->item_len;
    return entries;
}

int map_forEach_start(map_handle_t handle,map_entry_t* entry)
{
    if(!handle)
    {
        return -1;
    }
    map_t* map = (map_t*)handle;
    map->iterator_cnt = 0;
    map->iterator_node = (hash_node_t*)((map->hash_table[0]).next);
    return map_forEach_next(handle,entry);
}

int map_forEach_next(map_handle_t handle,map_entry_t* entry)
{
    map_t* map = (map_t*)handle;
    /** get next valid node */
    while(map->iterator_node == NULL)
    {
        map->iterator_cnt++;
        if(map->iterator_cnt >= map->table_len)
            return -1;
        map->iterator_node = (hash_node_t*)(map->hash_table[map->iterator_cnt].next);
    }
    /** set entry */
    entry->key.key = map->iterator_node->key;
    entry->key.len = map->iterator_node->key_len;
    entry->value = map->iterator_node->value;
    /** progress to next */
    map->iterator_node = (hash_node_t*)(map->iterator_node->list_head.next);
    return 0;
}

/* private methods */

static inline hash_node_t* map_locate(
    map_t* map, const void* key, size_t key_len, list_head_t** p_prev)
{
    uint32_t full_hash = get_hash(key,key_len);
    uint32_t hash = full_hash & map->hash_mask;
    list_head_t* prev = &(map->hash_table[hash]);
    hash_node_t* node = (hash_node_t*)(prev->next);
    while(node)
    {
        if(full_hash == node->full_hash && key_len == node->key_len)
        {
            if(memcmp(key,node->key,key_len)==0)
                break;
        }
        prev = (list_head_t*)node;
        node = (hash_node_t*)(prev->next);
    }
    /** This does not harm if node is NULL. */
    if (p_prev)
        *p_prev = prev;
    return node;
}

static inline void try_increase_hash_table(map_t* map)
{
    if(map->item_len <= map->table_len)
        return;
    size_t old_len = map->table_len;
    if (old_len > UINT32_MAX)
    {
        /** The map won't be indexable by the 32bit hash after this. */
        return;
    }
    void* new_table = realloc(map->hash_table,sizeof(list_head_t) * old_len * 2);
    if(!new_table)
        return;
    map->hash_table = new_table;
    memset((map->hash_table)+old_len, 0, sizeof(list_head_t)*old_len);
    map->table_len = old_len * 2;
    map->hash_mask = (uint32_t)(map->table_len)-1;
    map->decrease_th = map->table_len/4;
    /** this is redundant */
    // if(map->decrease_th < MIN_HASH_TABLE_SIZE/2)
    //     map->decrease_th = 0;
    /** adjust nodes */
    uint32_t select_mask = (uint32_t)old_len;
    for(size_t i=0;i<old_len;i++)
    {
        list_head_t* prev = &(map->hash_table[i]);
        list_head_t* new_prev = prev + old_len;
        hash_node_t* node = (hash_node_t*)prev->next;
        while(node)
        {
            if(node->full_hash & select_mask)
            {
                /** move node. prev stays the same */
                prev->next = node->list_head.next;
                node->list_head.next = new_prev->next;
                new_prev->next = (list_head_t*)node;
            }
            else
            {
                /** advance prev */
                prev = (list_head_t*)node;
            }
            node = (hash_node_t*)prev->next;
        }
    }
}

static inline void try_decrease_hash_table(map_t* map)
{
    if(map->item_len >= map->decrease_th)
        return;
    size_t old_len = map->table_len;
    size_t new_len = old_len/2;

    for(size_t i=new_len;i<old_len;i++)
    {
        list_head_t* entry = &(map->hash_table[i]);
        if(entry->next)
        {
            hash_node_t* old_head = (hash_node_t*)(entry->next);
            /** find the old tail */
            hash_node_t* old_tail = old_head;
            while(old_tail->list_head.next)
                old_tail = (hash_node_t*)(old_tail->list_head.next);
            /** connect old chain to the head of the new chain. */
            list_head_t* new_entry = &(map->hash_table[i-new_len]);
            old_tail->list_head.next = new_entry->next;
            new_entry->next = (list_head_t*)old_head;
        }
    }

    list_head_t* new_table = realloc(map->hash_table,sizeof(list_head_t) * new_len);
    if (new_table)
    {
        /** If the shrink failed. The hash_table just takes a larger size than it appears. */
        map->hash_table = new_table;
    }
    map->table_len = new_len;
    map->hash_mask = (uint32_t)(map->table_len)-1;
    map->decrease_th = map->table_len/4;
    if(map->decrease_th < MIN_HASH_TABLE_SIZE/2)
        map->decrease_th = 0;
}

#ifdef MAP_ENABLE_DIAGNOSTIC

/* diagnostic */

float map_get_conflict_ratio(map_handle_t handle)
{
    map_t* map = (map_t*)handle;
    size_t conflict_cnt = 0;
    for(size_t i=0;i<map->table_len;i++)
    {
        hash_node_t* node = (hash_node_t*)(map->hash_table[i].next);
        size_t entry_len = 0;
        while(node != NULL)
        {
            entry_len ++;
            node = (hash_node_t*)(node->list_head.next);
        }
        if(entry_len > 1)
            conflict_cnt += entry_len;
    }
    return ((float)conflict_cnt)/((float)map->item_len);
}

float map_get_average_ops(map_handle_t handle)
{
    map_t* map = (map_t*)handle;
    size_t working_entry_cnt = 0;
    for(size_t i=0;i<map->table_len;i++)
    {
        if(map->hash_table[i].next != NULL)
            working_entry_cnt++;
    }
    return ((float)map->item_len)/((float)working_entry_cnt);
}

size_t map_get_max_ops(map_handle_t handle)
{
    map_t* map = (map_t*)handle;
    size_t max_entry_len = 0;
    for(size_t i=0;i<map->table_len;i++)
    {
        hash_node_t* node = (hash_node_t*)(map->hash_table[i].next);
        size_t entry_len = 0;
        while(node != NULL)
        {
            entry_len ++;
            node = (hash_node_t*)(node->list_head.next);
        }
        if(entry_len > max_entry_len)
            max_entry_len = entry_len;
    }
    return max_entry_len;
}

#endif

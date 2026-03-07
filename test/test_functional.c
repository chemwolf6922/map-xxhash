#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "map.h"

static int test_count = 0;
static int fail_count = 0;

#define ASSERT(cond, fmt, ...) do { \
    test_count++; \
    if(!(cond)) { \
        fail_count++; \
        printf("  FAIL [%s:%d]: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
    } \
} while(0)

#define RUN_TEST(func) do { \
    int _prev_fails = fail_count; \
    printf("Running %s...\n", #func); \
    func(); \
    if(fail_count == _prev_fails) printf("  PASS\n"); \
} while(0)

/* ------------------------------------------------------------------ */
/*  test_create_delete                                                 */
/* ------------------------------------------------------------------ */
static void test_create_delete(void)
{
    map_handle_t map = map_create();
    ASSERT(map != NULL, "map_create returned NULL");
    ASSERT(map_get_length(map) == 0, "new map length=%zu, expected 0", map_get_length(map));
    int ret = map_delete(map, NULL, NULL);
    ASSERT(ret == 0, "map_delete returned %d, expected 0", ret);
}

/* ------------------------------------------------------------------ */
/*  test_add_get_basic                                                 */
/* ------------------------------------------------------------------ */
static void test_add_get_basic(void)
{
    map_handle_t map = map_create();
    int values[] = {10, 20, 30, 40, 50};

    for(int i = 0; i < 5; i++)
    {
        int key = i;
        void* ret = map_add(map, &key, sizeof(int), &values[i]);
        ASSERT(ret == &values[i], "map_add new key=%d should return the value pointer", i);
    }
    ASSERT(map_get_length(map) == 5, "length=%zu after adding 5, expected 5", map_get_length(map));

    for(int i = 0; i < 5; i++)
    {
        int key = i;
        int* v = (int*)map_get(map, &key, sizeof(int));
        ASSERT(v != NULL, "map_get key=%d returned NULL", i);
        ASSERT(v == &values[i], "map_get key=%d value pointer mismatch", i);
        ASSERT(*v == values[i], "map_get key=%d value=%d, expected %d", i, v ? *v : -1, values[i]);
    }

    int missing = 99;
    ASSERT(map_get(map, &missing, sizeof(int)) == NULL, "map_get non-existing key should return NULL");

    map_delete(map, NULL, NULL);
}

/* ------------------------------------------------------------------ */
/*  test_add_replace                                                   */
/* ------------------------------------------------------------------ */
static void test_add_replace(void)
{
    map_handle_t map = map_create();
    int v1 = 100, v2 = 200;
    int key = 42;

    void* ret1 = map_add(map, &key, sizeof(int), &v1);
    ASSERT(ret1 == &v1, "first add should return value pointer");
    ASSERT(map_get_length(map) == 1, "length should be 1 after first add");

    void* ret2 = map_add(map, &key, sizeof(int), &v2);
    ASSERT(ret2 == &v1, "replace add should return old value pointer, got %p expected %p", ret2, (void*)&v1);
    ASSERT(map_get_length(map) == 1, "length should still be 1 after replace");

    int* v = (int*)map_get(map, &key, sizeof(int));
    ASSERT(v == &v2, "map_get after replace should return new value pointer");
    ASSERT(*v == 200, "map_get after replace value=%d, expected 200", *v);

    map_delete(map, NULL, NULL);
}

/* ------------------------------------------------------------------ */
/*  test_has                                                           */
/* ------------------------------------------------------------------ */
static void test_has(void)
{
    map_handle_t map = map_create();
    int key = 7, value = 1;
    int missing = 99;

    ASSERT(map_has(map, &key, sizeof(int)) == false, "map_has on empty map should be false");

    map_add(map, &key, sizeof(int), &value);
    ASSERT(map_has(map, &key, sizeof(int)) == true, "map_has existing key should be true");
    ASSERT(map_has(map, &missing, sizeof(int)) == false, "map_has non-existing key should be false");

    map_remove(map, &key, sizeof(int));
    ASSERT(map_has(map, &key, sizeof(int)) == false, "map_has after remove should be false");

    map_delete(map, NULL, NULL);
}

/* ------------------------------------------------------------------ */
/*  test_remove                                                        */
/* ------------------------------------------------------------------ */
static void test_remove(void)
{
    map_handle_t map = map_create();
    int values[] = {10, 20, 30};

    for(int i = 0; i < 3; i++)
    {
        int key = i;
        map_add(map, &key, sizeof(int), &values[i]);
    }
    ASSERT(map_get_length(map) == 3, "length should be 3");

    int key = 1;
    void* removed = map_remove(map, &key, sizeof(int));
    ASSERT(removed == &values[1], "map_remove should return the removed value pointer");
    ASSERT(map_get_length(map) == 2, "length should be 2 after removing one");
    ASSERT(map_get(map, &key, sizeof(int)) == NULL, "map_get removed key should return NULL");
    ASSERT(map_has(map, &key, sizeof(int)) == false, "map_has removed key should be false");

    int missing = 99;
    ASSERT(map_remove(map, &missing, sizeof(int)) == NULL, "map_remove non-existing should return NULL");
    ASSERT(map_get_length(map) == 2, "length unchanged after removing non-existing");

    ASSERT(map_remove(map, &key, sizeof(int)) == NULL, "map_remove already-removed should return NULL");

    int key0 = 0, key2 = 2;
    ASSERT(map_get(map, &key0, sizeof(int)) == &values[0], "key 0 still present");
    ASSERT(map_get(map, &key2, sizeof(int)) == &values[2], "key 2 still present");

    map_delete(map, NULL, NULL);
}

/* ------------------------------------------------------------------ */
/*  test_clear                                                         */
/* ------------------------------------------------------------------ */
static void test_clear(void)
{
    map_handle_t map = map_create();
    int value = 1;

    for(int i = 0; i < 20; i++)
    {
        int key = i;
        map_add(map, &key, sizeof(int), &value);
    }
    ASSERT(map_get_length(map) == 20, "length should be 20 before clear");

    int ret = map_clear(map, NULL, NULL);
    ASSERT(ret == 0, "map_clear returned %d, expected 0", ret);
    ASSERT(map_get_length(map) == 0, "length should be 0 after clear");

    for(int i = 0; i < 20; i++)
    {
        int key = i;
        ASSERT(map_get(map, &key, sizeof(int)) == NULL, "key %d should be NULL after clear", i);
    }

    int key = 100;
    map_add(map, &key, sizeof(int), &value);
    ASSERT(map_get_length(map) == 1, "length should be 1 after re-add");
    ASSERT(map_get(map, &key, sizeof(int)) == &value, "re-added key should be retrievable");

    map_delete(map, NULL, NULL);
}

/* ------------------------------------------------------------------ */
/*  test_clear_with_callback                                           */
/* ------------------------------------------------------------------ */
static int free_call_count;
static void free_counter(void* value, void* ctx)
{
    (void)value;
    (void)ctx;
    free_call_count++;
}

static void test_clear_with_callback(void)
{
    map_handle_t map = map_create();
    int value = 1;

    for(int i = 0; i < 5; i++)
    {
        int key = i;
        map_add(map, &key, sizeof(int), &value);
    }

    free_call_count = 0;
    map_clear(map, free_counter, NULL);
    ASSERT(free_call_count == 5, "free callback called %d times, expected 5", free_call_count);
    ASSERT(map_get_length(map) == 0, "length should be 0 after clear");

    map_delete(map, NULL, NULL);
}

/* ------------------------------------------------------------------ */
/*  test_delete_with_callback                                          */
/* ------------------------------------------------------------------ */
static void test_delete_with_callback(void)
{
    map_handle_t map = map_create();
    int value = 1;

    for(int i = 0; i < 8; i++)
    {
        int key = i;
        map_add(map, &key, sizeof(int), &value);
    }

    free_call_count = 0;
    int ret = map_delete(map, free_counter, NULL);
    ASSERT(ret == 0, "map_delete returned %d, expected 0", ret);
    ASSERT(free_call_count == 8, "free callback called %d times on delete, expected 8", free_call_count);
}

/* ------------------------------------------------------------------ */
/*  test_string_keys                                                   */
/* ------------------------------------------------------------------ */
static void test_string_keys(void)
{
    map_handle_t map = map_create();
    int va = 1, vb = 2, vc = 3;

    const char* ka = "hello";
    const char* kb = "world";
    const char* kc = "hello world";

    map_add(map, ka, strlen(ka), &va);
    map_add(map, kb, strlen(kb), &vb);
    map_add(map, kc, strlen(kc), &vc);
    ASSERT(map_get_length(map) == 3, "length should be 3 with string keys");

    ASSERT(*(int*)map_get(map, ka, strlen(ka)) == 1, "value for 'hello' should be 1");
    ASSERT(*(int*)map_get(map, kb, strlen(kb)) == 2, "value for 'world' should be 2");
    ASSERT(*(int*)map_get(map, kc, strlen(kc)) == 3, "value for 'hello world' should be 3");

    const char* partial = "hell";
    ASSERT(map_get(map, partial, strlen(partial)) == NULL, "partial key 'hell' should return NULL");

    map_delete(map, NULL, NULL);
}

/* ------------------------------------------------------------------ */
/*  test_keys_array                                                    */
/* ------------------------------------------------------------------ */
static void test_keys_array(void)
{
    map_handle_t map = map_create();
    int value = 1;

    for(int i = 0; i < 10; i++)
    {
        int key = i;
        map_add(map, &key, sizeof(int), &value);
    }

    size_t len = 0;
    map_key_t* keys = map_keys(map, &len);
    ASSERT(keys != NULL, "map_keys returned NULL");
    ASSERT(len == 10, "map_keys length=%zu, expected 10", len);

    int found[10] = {0};
    for(size_t i = 0; i < len; i++)
    {
        ASSERT(keys[i].len == sizeof(int), "key[%zu] len=%zu, expected %zu", i, keys[i].len, sizeof(int));
        int k = *(int*)(keys[i].key);
        ASSERT(k >= 0 && k < 10, "key value %d out of range", k);
        if(k >= 0 && k < 10) found[k] = 1;
    }
    for(int i = 0; i < 10; i++)
    {
        ASSERT(found[i] == 1, "key %d not found in map_keys result", i);
    }

    free(keys);

    map_clear(map, NULL, NULL);
    keys = map_keys(map, &len);
    ASSERT(len == 0, "map_keys on empty map length=%zu, expected 0", len);
    free(keys);

    map_delete(map, NULL, NULL);
}

/* ------------------------------------------------------------------ */
/*  test_values_array                                                  */
/* ------------------------------------------------------------------ */
static void test_values_array(void)
{
    map_handle_t map = map_create();
    int values[5] = {100, 200, 300, 400, 500};

    for(int i = 0; i < 5; i++)
    {
        int key = i;
        map_add(map, &key, sizeof(int), &values[i]);
    }

    size_t len = 0;
    void** vals = map_values(map, &len);
    ASSERT(vals != NULL, "map_values returned NULL");
    ASSERT(len == 5, "map_values length=%zu, expected 5", len);

    int found[5] = {0};
    for(size_t i = 0; i < len; i++)
    {
        int v = *(int*)(vals[i]);
        for(int j = 0; j < 5; j++)
        {
            if(v == values[j]) { found[j] = 1; break; }
        }
    }
    for(int i = 0; i < 5; i++)
    {
        ASSERT(found[i] == 1, "value %d not found in map_values result", values[i]);
    }

    free(vals);
    map_delete(map, NULL, NULL);
}

/* ------------------------------------------------------------------ */
/*  test_entries_array                                                 */
/* ------------------------------------------------------------------ */
static void test_entries_array(void)
{
    map_handle_t map = map_create();
    int values[5] = {10, 20, 30, 40, 50};

    for(int i = 0; i < 5; i++)
    {
        int key = i;
        map_add(map, &key, sizeof(int), &values[i]);
    }

    size_t len = 0;
    map_entry_t* entries = map_entries(map, &len);
    ASSERT(entries != NULL, "map_entries returned NULL");
    ASSERT(len == 5, "map_entries length=%zu, expected 5", len);

    for(size_t i = 0; i < len; i++)
    {
        int k = *(int*)(entries[i].key.key);
        int v = *(int*)(entries[i].value);
        ASSERT(k >= 0 && k < 5, "entry key %d out of range", k);
        ASSERT(v == values[k], "entry key=%d value=%d, expected %d", k, v, values[k]);
        ASSERT(entries[i].key.len == sizeof(int), "entry key len=%zu, expected %zu", entries[i].key.len, sizeof(int));
    }

    free(entries);
    map_delete(map, NULL, NULL);
}

/* ------------------------------------------------------------------ */
/*  test_forEach                                                       */
/* ------------------------------------------------------------------ */
static void test_forEach(void)
{
    map_handle_t map = map_create();
    int values[10];
    for(int i = 0; i < 10; i++)
    {
        values[i] = i * 10;
        int key = i;
        map_add(map, &key, sizeof(int), &values[i]);
    }

    int visited[10] = {0};
    int count = 0;
    map_entry_t entry;
    map_forEach(map, entry)
    {
        int k = *(int*)(entry.key.key);
        int v = *(int*)(entry.value);
        ASSERT(k >= 0 && k < 10, "forEach key %d out of range", k);
        ASSERT(v == k * 10, "forEach key=%d value=%d, expected %d", k, v, k * 10);
        if(k >= 0 && k < 10) visited[k] = 1;
        count++;
    }
    ASSERT(count == 10, "forEach visited %d entries, expected 10", count);
    for(int i = 0; i < 10; i++)
    {
        ASSERT(visited[i] == 1, "forEach did not visit key %d", i);
    }

    map_clear(map, NULL, NULL);
    count = 0;
    map_forEach(map, entry)
    {
        count++;
    }
    ASSERT(count == 0, "forEach on empty map iterated %d times, expected 0", count);

    map_delete(map, NULL, NULL);
}

/* ------------------------------------------------------------------ */
/*  test_empty_map_operations                                          */
/* ------------------------------------------------------------------ */
static void test_empty_map_operations(void)
{
    map_handle_t map = map_create();
    int key = 0;

    ASSERT(map_get_length(map) == 0, "empty map length should be 0");
    ASSERT(map_get(map, &key, sizeof(int)) == NULL, "get on empty map should be NULL");
    ASSERT(map_has(map, &key, sizeof(int)) == false, "has on empty map should be false");
    ASSERT(map_remove(map, &key, sizeof(int)) == NULL, "remove on empty map should be NULL");

    ASSERT(map_clear(map, NULL, NULL) == 0, "clear on empty map should return 0");

    size_t len = 99;
    map_key_t* keys = map_keys(map, &len);
    ASSERT(len == 0, "keys on empty map length=%zu, expected 0", len);
    free(keys);

    void** vals = map_values(map, &len);
    ASSERT(len == 0, "values on empty map length=%zu, expected 0", len);
    free(vals);

    map_entry_t* entries = map_entries(map, &len);
    ASSERT(len == 0, "entries on empty map length=%zu, expected 0", len);
    free(entries);

    map_delete(map, NULL, NULL);
}

/* ------------------------------------------------------------------ */
/*  test_large_key                                                     */
/* ------------------------------------------------------------------ */
static void test_large_key(void)
{
    map_handle_t map = map_create();
    int value = 42;

    char big_key[4096];
    memset(big_key, 'A', sizeof(big_key));

    map_add(map, big_key, sizeof(big_key), &value);
    ASSERT(map_get_length(map) == 1, "length should be 1");
    ASSERT(*(int*)map_get(map, big_key, sizeof(big_key)) == 42, "get with large key should work");

    big_key[4095] = 'B';
    ASSERT(map_get(map, big_key, sizeof(big_key)) == NULL, "different large key should return NULL");

    map_delete(map, NULL, NULL);
}

/* ------------------------------------------------------------------ */
/*  test_many_entries                                                  */
/* ------------------------------------------------------------------ */
static void test_many_entries(void)
{
    map_handle_t map = map_create();
    int value = 1;
    int n = 10000;

    for(int i = 0; i < n; i++)
    {
        map_add(map, &i, sizeof(int), &value);
    }
    ASSERT(map_get_length(map) == (size_t)n, "length=%zu after %d adds, expected %d",
           map_get_length(map), n, n);

    for(int i = 0; i < n; i++)
    {
        ASSERT(map_get(map, &i, sizeof(int)) != NULL, "key %d missing after bulk add", i);
    }

    for(int i = 0; i < n / 2; i++)
    {
        map_remove(map, &i, sizeof(int));
    }
    ASSERT(map_get_length(map) == (size_t)(n / 2), "length=%zu after removing half, expected %d",
           map_get_length(map), n / 2);

    for(int i = 0; i < n / 2; i++)
    {
        ASSERT(map_get(map, &i, sizeof(int)) == NULL, "removed key %d should be NULL", i);
    }
    for(int i = n / 2; i < n; i++)
    {
        ASSERT(map_get(map, &i, sizeof(int)) != NULL, "remaining key %d should exist", i);
    }

    map_delete(map, NULL, NULL);
}

/* ------------------------------------------------------------------ */
/*  main                                                               */
/* ------------------------------------------------------------------ */
int main(int argc, char const *argv[])
{
    (void)argc;
    (void)argv;

    RUN_TEST(test_create_delete);
    RUN_TEST(test_add_get_basic);
    RUN_TEST(test_add_replace);
    RUN_TEST(test_has);
    RUN_TEST(test_remove);
    RUN_TEST(test_clear);
    RUN_TEST(test_clear_with_callback);
    RUN_TEST(test_delete_with_callback);
    RUN_TEST(test_string_keys);
    RUN_TEST(test_keys_array);
    RUN_TEST(test_values_array);
    RUN_TEST(test_entries_array);
    RUN_TEST(test_forEach);
    RUN_TEST(test_empty_map_operations);
    RUN_TEST(test_large_key);
    RUN_TEST(test_many_entries);

    printf("\n%d/%d assertions passed.\n", test_count - fail_count, test_count);
    return fail_count > 0 ? 1 : 0;
}

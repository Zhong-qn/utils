/**
 * @file hash.c
 * @author Zhong Qiaoning (691365572@qq.com)
 * @brief 
 * @version 0.1
 * @date 2022-07-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <string.h>
#include "ut_hash.h"

#define INITIAL_MAX 15 /* 2^n - 1 */

#define HASH_DEFAULT_MULTIPLER 33

typedef struct hash_entry {
    int32_t hash;               /* 哈希值 */
    char key[UT_LEN_32];           /* 键 */
    void* value;                /* 值 */
    struct hash_entry* next;    /* 哈希链表下一个 */
} hash_entry_t;


struct ut_hash_t {
    hash_entry_t **array;       /* 哈希bucket */
    uint32_t count;             /* 当前哈希表内的数据 */
    uint32_t max;               /* 哈希表最大存储数据 */
    ut_hash_func ut_hash_func;        /* 哈希函数 */
    hash_entry_t *free;         /* 避免频繁free使用 */
};

/**
 * @brief 默认的哈希函数
 * 
 * @param char_key 
 * @return uint32_t 
 */
static uint32_t hashfunc_default(const char* char_key)
{
    const char*     key = (const char *)char_key;
    const char*     cur_pos = NULL;
    uint32_t        hash = 0;

    for (cur_pos = key; *cur_pos; cur_pos++) {
        hash = hash * HASH_DEFAULT_MULTIPLER + *cur_pos;
    }
    return hash;
}

/**
 * @brief 分配哈希bucket
 * 
 * @param hash_table 哈希表描述结构体
 * @param max 哈希表最大值
 * @return hash_entry_t** 
 */
static hash_entry_t** __alloc_array(ut_hash_t *hash_table, uint32_t max)
{
    return ut_zero_alloc(sizeof(*hash_table->array) * (max + 1));
}

/**
 * @brief 进行哈希表的扩容
 * 
 * @param hash_table 哈希表描述结构体
 */
static void __expand_array(ut_hash_t *hash_table)
{
    hash_entry_t**   new_array = NULL;
    uint32_t            new_max;

    new_max = hash_table->max * 2 + 1;
    new_array = __alloc_array(hash_table, new_max);

    for (uint32_t i = 0; i <= hash_table->max; i++) {
        hash_entry_t *entry = hash_table->array[i];
        while (entry) {
            hash_entry_t *next = entry->next;
            uint32_t index = entry->hash & new_max;
            entry->next = new_array[index];
            new_array[index] = entry;
            entry = next;
        }
    }
    free(hash_table->array);
    hash_table->array = new_array;
    hash_table->max = new_max;
}


ut_errno_t ut_hash_create(ut_hash_t **out, uint32_t size, ut_hash_func ut_hash_func)
{
    uint32_t    max_size = 0;
    ut_hash_t*  hash_table = NULL;
    ut_errno_t  retval = UT_ERRNO_OK;

    CHECK_PTR_RET(out, retval, UT_ERRNO_NULLPTR);
    CHECK_VAL_EQ(size < 0, UT_TRUE, retval = UT_ERRNO_INVALID, TAG_OUT);

    if (size == 0) {
        max_size = INITIAL_MAX;
    } else {
        // 0.75 load factor.
        size = size * 4 / 3;
        max_size = 1;
        while (max_size < size) {
            max_size <<= 1;
        }
        max_size -= 1;
    }

    if (ut_hash_func == NULL)
        ut_hash_func = hashfunc_default;

    hash_table = ut_zero_alloc(sizeof(ut_hash_t));
    *out = hash_table;
    hash_table->count = 0;
    hash_table->max = max_size;
    hash_table->ut_hash_func = ut_hash_func;
    hash_table->array = __alloc_array(hash_table, hash_table->max);
    hash_table->free = NULL;

TAG_OUT:
    return retval;
}


ut_errno_t ut_hash_destroy(ut_hash_t *hash_table)
{
    ut_errno_t          retval = UT_ERRNO_OK;
    hash_entry_t*    entry = NULL;

    CHECK_PTR_RET(hash_table, retval, UT_ERRNO_NULLPTR);

    for (uint32_t i = 0; i <= hash_table->max; i++) {
        entry = hash_table->array[i];
        while (entry) {
            hash_entry_t *next = entry->next;
            free(entry);
            entry = next;
        }
    }

    entry = hash_table->free;
    while(entry) {
        hash_entry_t *next = entry->next;
        free(entry);
        entry = next;
    }

    free(hash_table->array);
    free(hash_table);

TAG_OUT:
    return retval;
}

static hash_entry_t **find_entry(ut_hash_t *hash_table, const char *key, const void* value)
{
    hash_entry_t**   retval = NULL;
    hash_entry_t*    hash_entry = NULL;
    uint32_t            hash = 0;
    ut_bool_t           found = UT_FALSE;

    CHECK_PTR(hash_table);

    hash = hash_table->ut_hash_func(key);
    retval = &hash_table->array[hash & hash_table->max];

    for (hash_entry = *retval; hash_entry; retval = &hash_entry->next, hash_entry = *retval) {
        if (hash_entry->hash == hash && strcmp(hash_entry->key, key) == 0) {
            found = UT_TRUE;
            break;
        }
    }
    CHECK_VAL_NEQ(!value && !found, UT_FALSE, retval = NULL, TAG_OUT);
    CHECK_VAL_NEQ(hash_entry || !value, UT_FALSE, NULL, TAG_OUT);

    if ((hash_entry = hash_table->free) != NULL) {
        hash_table->free = hash_entry->next;
    } else {
        hash_entry = ut_zero_alloc(sizeof(*hash_entry));
    }
    hash_entry->next = NULL;
    hash_entry->hash = hash;
    strncpy(hash_entry->key, key, sizeof(hash_entry->key) - 2);
    *retval = hash_entry;
    hash_table->count++;

TAG_OUT:
    return retval;
}


void* ut_hash_push(ut_hash_t *hash_table, const char *key, const void* value)
{
    void* old_value = NULL;
    hash_entry_t **hash_entry_addr;

    if (key == NULL) {
        return NULL;
    }

    hash_entry_addr = find_entry(hash_table, key, value);
    if (hash_entry_addr == NULL) {
        /* 没有存储过却要删除key */
        return old_value;
    } else if (*hash_entry_addr) {
        if (!value) {
            /* delete entry */
            hash_entry_t *old = *hash_entry_addr;
            *hash_entry_addr = (*hash_entry_addr)->next;
            old->next = hash_table->free;
            old_value = old->value;
            old->value = NULL;
            //free指针指向被移除的节点，该节点将提供给下一个写入节点使用，避免频繁malloc
            hash_table->free = old;
            --hash_table->count;
        } else {
            /* replace entry */
            old_value = (*hash_entry_addr)->value;
            (*hash_entry_addr)->value = (void*)value;
            /* check that the collision rate isn't too high */
            if (hash_table->count > hash_table->max) {
                __expand_array(hash_table);
            }
        }
    }
    return (void*)old_value;
}


void* ut_hash_pop(ut_hash_t* hash_table, const char* key)
{
    return ut_hash_push(hash_table, key, NULL);
}


void* ut_hash_peek(ut_hash_t *hash_table, const char *key)
{
    hash_entry_t**   hash_entry_addr = NULL;

    hash_entry_addr = find_entry(hash_table, key, NULL);
    if (hash_entry_addr)
        return (void*)((*hash_entry_addr)->value);
    else
        return NULL;
}


int32_t ut_hash_count(ut_hash_t *hash_table)
{
    return hash_table->count;
}

void ut_hash_foreach(ut_hash_t *hash_table, ut_hash_cb callback, void* context)
{
    uint32_t    i = 0;

    for (i = 0; i <= hash_table->max; i++) {
        hash_entry_t* entry = hash_table->array[i];
        while (entry) {
            hash_entry_t *next = entry->next;
            if (!callback(entry->key, entry->value, context)) {
                return;
            }
            entry = next;
        }
    }
}


#include <stdlib.h>
#include "jansson/hashtable.h"

typedef struct hashtable_list list_t;
typedef struct hashtable_pair pair_t;
typedef struct hashtable_bucket bucket_t;

#define container_of(ptr_, type_, member_)                      \
    ((type_ *)((char *)ptr_ - (size_t)&((type_ *)0)->member_))

#define list_to_pair(list_)  container_of(list_, pair_t, list)

static inline void list_init(list_t *list)
{
    list->next = list;
    list->prev = list;
}

static inline void list_insert(list_t *list, list_t *node)
{
    node->next = list;
    node->prev = list->prev;
    list->prev->next = node;
    list->prev = node;
}

static inline void list_remove(list_t *list)
{
    list->prev->next = list->next;
    list->next->prev = list->prev;
}

static inline int bucket_is_empty(hashtable_t *hashtable, bucket_t *bucket)
{
    return bucket->first == &hashtable->list && bucket->first == bucket->last;
}

static void insert_to_bucket(hashtable_t *hashtable, bucket_t *bucket,
                             list_t *list)
{
    if(bucket_is_empty(hashtable, bucket))
    {
        list_insert(&hashtable->list, list);
        bucket->first = bucket->last = list;
    }
    else
    {
        list_insert(bucket->first, list);
        bucket->first = list;
    }
}

static unsigned int primes[] = {
    5, 13, 23, 53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593,
    49157, 98317, 196613, 393241, 786433, 1572869, 3145739, 6291469,
    12582917, 25165843, 50331653, 100663319, 201326611, 402653189,
    805306457, 1610612741
};
static const unsigned int num_primes = sizeof(primes) / sizeof(unsigned int);

static inline unsigned int num_buckets(hashtable_t *hashtable)
{
    return primes[hashtable->num_buckets];
}


static pair_t *hashtable_find_pair(hashtable_t *hashtable, bucket_t *bucket,
                                   const void *key, unsigned int hash)
{
    list_t *list;
    pair_t *pair;

    if(bucket_is_empty(hashtable, bucket))
        return NULL;

    list = bucket->first;
    while(1)
    {
        pair = list_to_pair(list);
        if(pair->hash == hash && hashtable->cmp_keys(pair->key, key))
            return pair;

        if(list == bucket->last)
            break;

        list = list->next;
    }

    return NULL;
}

/* returns 0 on success, -1 if key was not found */
static int hashtable_do_del(hashtable_t *hashtable,
                            const void *key, unsigned int hash)
{
    pair_t *pair;
    bucket_t *bucket;
    unsigned int index;

    index = hash % num_buckets(hashtable);
    bucket = &hashtable->buckets[index];

    pair = hashtable_find_pair(hashtable, bucket, key, hash);
    if(!pair)
        return -1;

    if(&pair->list == bucket->first && &pair->list == bucket->last)
        bucket->first = bucket->last = &hashtable->list;

    else if(&pair->list == bucket->first)
        bucket->first = pair->list.next;

    else if(&pair->list == bucket->last)
        bucket->last = pair->list.prev;

    list_remove(&pair->list);

    if(hashtable->free_key)
        hashtable->free_key(pair->key);
    if(hashtable->free_value)
        hashtable->free_value(pair->value);

    free(pair);
    hashtable->size--;

    return 0;
}

static void hashtable_do_clear(hashtable_t *hashtable)
{
    list_t *list, *next;
    pair_t *pair;

    for(list = hashtable->list.next; list != &hashtable->list; list = next)
    {
        next = list->next;
        pair = list_to_pair(list);
        if(hashtable->free_key)
            hashtable->free_key(pair->key);
        if(hashtable->free_value)
            hashtable->free_value(pair->value);
        free(pair);
    }
}

static int hashtable_do_rehash(hashtable_t *hashtable)
{
    list_t *list, *next;
    pair_t *pair;
    unsigned int i, index, new_size;

    free(hashtable->buckets);

    hashtable->num_buckets++;
    new_size = num_buckets(hashtable);

    hashtable->buckets = malloc(new_size * sizeof(bucket_t));
    if(!hashtable->buckets)
        return -1;

    for(i = 0; i < num_buckets(hashtable); i++)
    {
        hashtable->buckets[i].first = hashtable->buckets[i].last =
            &hashtable->list;
    }

    list = hashtable->list.next;
    list_init(&hashtable->list);

    for(; list != &hashtable->list; list = next) {
        next = list->next;
        pair = list_to_pair(list);
        index = pair->hash % new_size;
        insert_to_bucket(hashtable, &hashtable->buckets[index], &pair->list);
    }

    return 0;
}


hashtable_t *hashtable_create(key_hash_fn hash_key, key_cmp_fn cmp_keys,
                              free_fn free_key, free_fn free_value)
{
    hashtable_t *hashtable = malloc(sizeof(hashtable_t));
    if(!hashtable)
        return NULL;

    if(hashtable_init(hashtable, hash_key, cmp_keys, free_key, free_value))
    {
        free(hashtable);
        return NULL;
    }

    return hashtable;
}

void hashtable_destroy(hashtable_t *hashtable)
{
    hashtable_close(hashtable);
    free(hashtable);
}

int hashtable_init(hashtable_t *hashtable,
                   key_hash_fn hash_key, key_cmp_fn cmp_keys,
                   free_fn free_key, free_fn free_value)
{
    unsigned int i;

    hashtable->size = 0;
    hashtable->num_buckets = 0;  /* index to primes[] */
    hashtable->buckets = malloc(num_buckets(hashtable) * sizeof(bucket_t));
    if(!hashtable->buckets)
        return -1;

    list_init(&hashtable->list);

    hashtable->hash_key = hash_key;
    hashtable->cmp_keys = cmp_keys;
    hashtable->free_key = free_key;
    hashtable->free_value = free_value;

    for(i = 0; i < num_buckets(hashtable); i++)
    {
        hashtable->buckets[i].first = hashtable->buckets[i].last =
            &hashtable->list;
    }

    return 0;
}

void hashtable_close(hashtable_t *hashtable)
{
    hashtable_do_clear(hashtable);
    free(hashtable->buckets);
}

int hashtable_set(hashtable_t *hashtable, void *key, void *value)
{
    pair_t *pair;
    bucket_t *bucket;
    unsigned int hash, index;

    /* rehash if the load ratio exceeds 1 */
    if(hashtable->size >= num_buckets(hashtable))
        if(hashtable_do_rehash(hashtable))
            return -1;

    hash = hashtable->hash_key(key);
    index = hash % num_buckets(hashtable);
    bucket = &hashtable->buckets[index];
    pair = hashtable_find_pair(hashtable, bucket, key, hash);

    if(pair)
    {
        if(hashtable->free_key)
            hashtable->free_key(key);
        if(hashtable->free_value)
            hashtable->free_value(pair->value);
        pair->value = value;
    }
    else
    {
        pair = malloc(sizeof(pair_t));
        if(!pair)
            return -1;

        pair->key = key;
        pair->value = value;
        pair->hash = hash;
        list_init(&pair->list);

        insert_to_bucket(hashtable, bucket, &pair->list);

        hashtable->size++;
    }
    return 0;
}

void *hashtable_get(hashtable_t *hashtable, const void *key)
{
    pair_t *pair;
    unsigned int hash;
    bucket_t *bucket;

    hash = hashtable->hash_key(key);
    bucket = &hashtable->buckets[hash % num_buckets(hashtable)];

    pair = hashtable_find_pair(hashtable, bucket, key, hash);
    if(!pair)
        return NULL;

    return pair->value;
}

int hashtable_del(hashtable_t *hashtable, const void *key)
{
    unsigned int hash = hashtable->hash_key(key);
    return hashtable_do_del(hashtable, key, hash);
}

void hashtable_clear(hashtable_t *hashtable)
{
    unsigned int i;

    hashtable_do_clear(hashtable);

    for(i = 0; i < num_buckets(hashtable); i++)
    {
        hashtable->buckets[i].first = hashtable->buckets[i].last =
            &hashtable->list;
    }

    list_init(&hashtable->list);
    hashtable->size = 0;
}

void *hashtable_iter(hashtable_t *hashtable)
{
    return hashtable_iter_next(hashtable, &hashtable->list);
}

void *hashtable_iter_at(hashtable_t *hashtable, const void *key)
{
    pair_t *pair;
    unsigned int hash;
    bucket_t *bucket;

    hash = hashtable->hash_key(key);
    bucket = &hashtable->buckets[hash % num_buckets(hashtable)];

    pair = hashtable_find_pair(hashtable, bucket, key, hash);
    if(!pair)
        return NULL;

    return &pair->list;
}

void *hashtable_iter_next(hashtable_t *hashtable, void *iter)
{
    list_t *list = (list_t *)iter;
    if(list->next == &hashtable->list)
        return NULL;
    return list->next;
}

void *hashtable_iter_key(void *iter)
{
    pair_t *pair = list_to_pair((list_t *)iter);
    return pair->key;
}

void *hashtable_iter_value(void *iter)
{
    pair_t *pair = list_to_pair((list_t *)iter);
    return pair->value;
}

void hashtable_iter_set(hashtable_t *hashtable, void *iter, void *value)
{
    pair_t *pair = list_to_pair((list_t *)iter);

    if(hashtable->free_value)
        hashtable->free_value(pair->value);

    pair->value = value;
}

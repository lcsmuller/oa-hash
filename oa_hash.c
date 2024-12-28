#include <string.h>

#include "oa_hash.h"

void
oa_hash_init(struct oa_hash *ht,
             struct oa_hash_entry *buckets,
             const size_t capacity)
{
    ht->buckets = buckets;
    ht->length = 0;
    ht->capacity = capacity;
}

void
oa_hash_cleanup(struct oa_hash *ht)
{
    if (!ht) return;

    ht->length = 0;
    ht->capacity = 0;
    ht->buckets = NULL;
}

static size_t
_oa_hash_genhash(const char *const key, const size_t capacity)
{
    const unsigned char *str = (const unsigned char *)key;
    unsigned long hash = 5381; /* DJB2 initial value */
    int c;

    if (!key || !capacity) return 0;

    while ((c = *str++)) {
        /* hash * 33 = (hash * 32) + hash */
        hash = ((hash & 0x7fffffff) << 5) + hash + c;
    }
    return hash % capacity;
}

struct oa_hash_entry *
oa_hash_get_entry(struct oa_hash *ht, const char *key, const size_t key_len)
{
    const size_t slot = _oa_hash_genhash(key, ht->capacity),
                 capacity = ht->capacity - slot;
    struct oa_hash_entry *entry = &ht->buckets[slot];
    size_t i;

    if (key_len == 0) return NULL;

    for (i = 0; i < capacity; ++i) {
        if (key_len == entry->key.length
            && 0 == strncmp(entry->key.buf, key, entry->key.length))
        {
            return entry;
        }
        ++entry;
    }
    return NULL;
}

static void
_oa_hash_pair(struct oa_hash_entry *entry,
              const char *key,
              const size_t key_len,
              void *value)
{
    entry->key.buf = key;
    entry->key.length = key_len;
    entry->value = value;
}

struct oa_hash_entry *
oa_hash_set_entry(struct oa_hash *ht,
                  const char *key,
                  const size_t key_len,
                  void *value)
{
    const size_t slot = _oa_hash_genhash(key, ht->capacity),
                 capacity = ht->capacity - slot;
    struct oa_hash_entry *entry = &ht->buckets[slot];
    size_t i;

    if (key_len == 0) return NULL;

    if (!entry->key.length) {
        _oa_hash_pair(entry, key, key_len, value);
        return entry;
    }

    for (i = 1; i < capacity; ++i) {
        ++entry;
        if (!entry->key.length
            || (entry->key.length == key_len
                && 0 == strncmp(entry->key.buf, key, entry->key.length)))
        {
            _oa_hash_pair(entry, key, key_len, value);
            return entry;
        }
    }
    return NULL;
}

int
oa_hash_remove(struct oa_hash *ht, const char *key, const size_t key_len)
{
    const size_t slot = _oa_hash_genhash(key, ht->capacity),
                 capacity = ht->length - slot;
    struct oa_hash_entry *entry = &ht->buckets[slot];
    size_t i;

    if (key_len == 0) return 0;

    for (i = 0; i < capacity; ++i) {
        if (entry->key.length == key_len
            && 0 == strncmp(entry->key.buf, key, entry->key.length))
        {
            memset(entry, 0, sizeof *entry);
            return 1;
        }
        ++entry;
    }
    return 0;
}

int
oa_hash_rehash(struct oa_hash *ht,
               struct oa_hash_entry *new_buckets,
               const size_t new_capacity)
{
    struct oa_hash_entry *old_buckets = ht->buckets;
    const size_t old_capacity = ht->capacity;
    size_t i;

    if (new_capacity <= old_capacity) return 0;

    ht->buckets = new_buckets;
    ht->capacity = new_capacity;
    ht->length = 0;

    for (i = 0; i < old_capacity; ++i) {
        if (!old_buckets[i].key.length) continue;

        oa_hash_set_entry(ht, old_buckets[i].key.buf,
                          old_buckets[i].key.length, old_buckets[i].value);
    }
    return 1;
}

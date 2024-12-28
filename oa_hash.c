#include <string.h>
#include <stdint.h>

#include "oa_hash.h"

void
oa_hash_init(struct oa_hash *ht,
             struct oa_hash_entry *buckets,
             const size_t capacity)
{
    memset(buckets, 0, sizeof(struct oa_hash_entry) * capacity);
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
_oa_hash_genhash(const char *const key, size_t key_len, const size_t capacity)
{
    const unsigned char *str = (const unsigned char *)key;
    unsigned long hash = 5381; /* DJB2 initial value */

    if (!key || !capacity) return 0;

    while (key_len--) {
        hash = ((hash & 0x7fffffff) << 5) + hash + *str++;
    }
    return hash % capacity;
}

struct oa_hash_entry *
oa_hash_get_entry(struct oa_hash *ht, const char *key, const size_t key_len)
{
    const size_t start_slot = _oa_hash_genhash(key, key_len, ht->capacity);
    size_t slot = start_slot;

    if (!key_len || !ht->capacity) return NULL;

    do {
        struct oa_hash_entry *entry = &ht->buckets[slot];

        if (entry->state == OA_HASH_ENTRY_EMPTY) {
            return NULL;
        }

        if (entry->state == OA_HASH_ENTRY_OCCUPIED
            && key_len == entry->key.length
            && 0 == memcmp(entry->key.buf, key, key_len))
        {
            return entry;
        }

        slot = (slot + 1) % ht->capacity;
    } while (slot != start_slot);

    return NULL;
}

struct oa_hash_entry *
oa_hash_set_entry(struct oa_hash *ht,
                  const char *key,
                  const size_t key_len,
                  void *value)
{
    const size_t start_slot = _oa_hash_genhash(key, key_len, ht->capacity);
    size_t slot = start_slot;
    size_t first_deleted = SIZE_MAX;

    if (!key_len || !ht->capacity) return NULL;

    do {
        struct oa_hash_entry *entry = &ht->buckets[slot];

        if (entry->state != OA_HASH_ENTRY_OCCUPIED) {
            if (first_deleted == SIZE_MAX
                && entry->state == OA_HASH_ENTRY_DELETED)
            {
                first_deleted = slot;
            }
            if (entry->state == OA_HASH_ENTRY_EMPTY) {
                slot = (first_deleted != SIZE_MAX) ? first_deleted : slot;
                entry = &ht->buckets[slot];
                entry->key.buf = (char *)key;
                entry->key.length = key_len;
                entry->value = value;
                entry->state = OA_HASH_ENTRY_OCCUPIED;
                ht->length++;
                return entry;
            }
        }

        if (entry->state == OA_HASH_ENTRY_OCCUPIED
            && key_len == entry->key.length
            && 0 == memcmp(entry->key.buf, key, key_len))
        {
            entry->value = value;
            return entry;
        }

        slot = (slot + 1) % ht->capacity;
    } while (slot != start_slot);

    return NULL;
}

int
oa_hash_remove(struct oa_hash *ht, const char *key, const size_t key_len)
{
    const size_t start_slot = _oa_hash_genhash(key, key_len, ht->capacity);
    size_t slot = start_slot;

    if (!key_len || !ht->capacity) return 0;

    do {
        struct oa_hash_entry *entry = &ht->buckets[slot];

        if (entry->state == OA_HASH_ENTRY_EMPTY) {
            return 0;
        }

        if (entry->state == OA_HASH_ENTRY_OCCUPIED
            && key_len == entry->key.length
            && 0 == memcmp(entry->key.buf, key, key_len))
        {
            entry->state = OA_HASH_ENTRY_DELETED;
            ht->length--;
            return 1;
        }

        slot = (slot + 1) % ht->capacity;
    } while (slot != start_slot);

    return 0;
}

int
oa_hash_rehash(struct oa_hash *ht,
               struct oa_hash_entry *new_buckets,
               const size_t new_capacity)
{
    struct oa_hash_entry *old_buckets = ht->buckets;
    const size_t old_capacity = ht->capacity;
    const size_t old_length = ht->length;
    size_t i;

    if (!new_buckets || new_capacity <= old_capacity) return 0;

    memset(new_buckets, 0, sizeof(struct oa_hash_entry) * new_capacity);

    /* temporarily switch to new buckets */
    ht->buckets = new_buckets;
    ht->capacity = new_capacity;
    ht->length = 0;

    for (i = 0; i < old_capacity; ++i) {
        if (old_buckets[i].state == OA_HASH_ENTRY_OCCUPIED
            && !oa_hash_set_entry(ht, old_buckets[i].key.buf,
                                  old_buckets[i].key.length,
                                  old_buckets[i].value))
        {
            /* restore original state on failure */
            ht->buckets = old_buckets;
            ht->capacity = old_capacity;
            ht->length = old_length;
            return 0;
        }
    }
    return 1;
}

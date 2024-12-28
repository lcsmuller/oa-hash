#ifndef OA_HASH_H
#define OA_HASH_H

#include <stddef.h>

/** @brief Hash table entry state */
enum oa_hash_entry_state {
    OA_HASH_ENTRY_EMPTY = 0, /**< empty entry */
    OA_HASH_ENTRY_OCCUPIED, /**< occupied entry */
    OA_HASH_ENTRY_DELETED /**< deleted entry */
};

/** @brief Entry holding key-value pair in hash table */
struct oa_hash_entry {
    enum oa_hash_entry_state state; /**< entry state */
    struct {
        const char *buf; /**< key buffer */
        size_t length; /**< key length */
    } key;
    void *value; /**< value pointer */
};

/** @brief Open addressing hash table */
struct oa_hash {
    size_t length; /**< amount of entries */
    size_t capacity; /**< total buckets capacity */
    struct oa_hash_entry *buckets; /**< entries array */
};

/**
 * @brief Initialize hash table with given buckets array
 *
 * @param ht the hash table to be initialized
 * @param buckets pre-allocated array of entries
 * @param capacity amount of buckets
 */
void oa_hash_init(struct oa_hash *ht,
                  struct oa_hash_entry *buckets,
                  const size_t capacity);

/**
 * @brief Clean up hash table entries and struct
 *
 * @param ht the hash table to be cleaned
 */
void oa_hash_cleanup(struct oa_hash *ht);

/**
 * @brief Retrieve entry by key
 *
 * @param ht the hash table
 * @param key the key to search for
 * @param key_len the key length
 * @return entry if found, NULL otherwise
 */
struct oa_hash_entry *oa_hash_get_entry(struct oa_hash *ht,
                                        const char *key,
                                        const size_t key_len);

/**
 * @brief Retrieve value by key (wrapper around oa_hash_get_entry)
 *
 * @param ht the hash table
 * @param key the key to search for
 * @param key_len the key length
 * @return value if found, NULL otherwise
 */
void *oa_hash_get(struct oa_hash *ht, const char *key, const size_t key_len);

/**
 * @brief Insert or update entry
 *
 * @param ht the hash table
 * @param key the key to insert/update
 * @param key_len the key length
 * @param value the value to be assigned
 * @return entry if successful, or NULL if no space left, in which case
 *      oa_hash_rehash() should be called
 */
struct oa_hash_entry *oa_hash_set_entry(struct oa_hash *ht,
                                        const char *key,
                                        const size_t key_len,
                                        void *value);

/**
 * @brief Insert or update entry (wrapper around oa_hash_set_entry)
 *
 * @param ht the hash table
 * @param key the key to insert/update
 * @param key_len the key length
 * @param value the value to be assigned
 * @return value if successful, or NULL if no space left, in which case
 *     oa_hash_rehash() should be called
 */
void *oa_hash_set(struct oa_hash *ht,
                  const char *key,
                  const size_t key_len,
                  void *value);

/**
 * @brief Remove entry by key
 *
 * @param ht the hash table
 * @param key the key to be removed
 * @param key_len the key length
 * @return 1 if found and removed, 0 otherwise
 */
int oa_hash_remove(struct oa_hash *ht, const char *key, const size_t key_len);

/**
 * @brief Rehash table to new buckets array
 *
 * @param ht the hash table
 * @param new_buckets the new buckets array
 * @param new_capacity the new buckets capacity
 * @return 1 if successful, 0 if new_capacity <= current capacity
 */
int oa_hash_rehash(struct oa_hash *ht,
                   struct oa_hash_entry *new_buckets,
                   const size_t new_capacity);

#endif /* OA_HASH_H */

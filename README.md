# oa_hash

A lightweight single-header open-addressing hashtable implementation in C that gives complete memory allocation control to the user.

## About

This library implements a hash table using open addressing for collision resolution, where you provide the pre-allocated buckets array. This approach gives you full control over memory management while maintaining a simple and efficient API.

## Key Features

- Zero internal memory allocations - you control all memory
- Open addressing collision resolution
- String keys with explicit lengths
- Generic void* values
- Simple and minimal API
- Single header library
- C89 compatible
- MIT licensed

## Examples

### Stack Allocation

```c
struct oa_hash ht;
struct oa_hash_entry buckets[64] = {0}; // pre-allocated buckets
int value = 42;

// Initialize with pre-allocated buckets
oa_hash_init(&ht, buckets, 64);

// Store and retrieve values
oa_hash_set(&ht, "key", 3, &value);
int *got = oa_hash_get(&ht, "key", 3);

printf("Value: %d\n", *got); // 42

// Cleanup (doesn't free memory - that's up to you)
oa_hash_cleanup(&ht);
```

### Dynamic Resizing

```c
struct oa_hash ht;
struct oa_hash_entry *buckets;
size_t capacity = 64;
int value = 42;

// Initial allocation
buckets = malloc(capacity * sizeof(*buckets));
oa_hash_init(&ht, buckets, capacity);

// Add some values
oa_hash_set(&ht, "key", 3, &value);

// Time to grow the table
struct oa_hash_entry *new_buckets = malloc(capacity * 2 * sizeof(*buckets));
struct oa_hash_entry *old_buckets = oa_hash_rehash(&ht, new_buckets, capacity * 2);
if (old_buckets) { // Rehash succeeded, free old buckets
    assert(old_buckets == buckets); // old_buckets will always be the same as the original buckets
    free(old_buckets);
    buckets = new_buckets;
} else { // Rehash failed, free new buckets
    free(new_buckets);
}

// Cleanup
oa_hash_cleanup(&ht);
free(buckets);
```

### Embedded Hash Table

The library provides a flexible way to embed hash table fields directly into your own structures using the `OA_HASH_ATTRS` macro. This is useful for:

1. Embedding hash tables inside larger structures
2. Creating read-only views of existing hash tables
3. Custom memory layouts while maintaining API compatibility

#### Example: Read-Only Hash Table View

```c
/* Define a read-only view of a hash table */
struct oa_hash_view {
    OA_HASH_ATTRS(const); /* const qualifier for read-only access */
};

void print_hash_stats(const struct oa_hash *ht) {
    /* Cast to read-only view for safer access */
    const struct oa_hash_view *view = (const struct oa_hash_view*)ht;

    printf("Hash table stats:\n");
    printf("  Entries: %zu\n", view->length);
    printf("  Capacity: %zu\n", view->capacity);
    printf("  Load factor: %.2f\n", (double)view->length / view->capacity);
}
```

#### Example: Custom Structure with Embedded Hash Table

```c
/* Structure with embedded hash table */
struct my_dictionary {
    OA_HASH_ATTRS(mut); /* Mutable hash table fields, must be first attribute */
    char *name;
    void (*on_insert)(const char *key);
};

/* Initialize dictionary with embedded hash table */
void dictionary_init(struct my_dictionary *dict,
                    const char *name,
                    struct oa_hash_entry buckets[],
                    size_t capacity)
{
    dict->name = strdup(name);
    dict->buckets = buckets;
    dict->capacity = capacity;
    dict->length = 0;
    dict->on_insert = NULL;

    memset(buckets, 0, sizeof(struct oa_hash_entry) * capacity);
}

/* Use regular oa_hash functions by casting */
void dictionary_insert(struct my_dictionary *dict,
                      const char *key,
                      size_t key_len,
                      void *value)
{
    struct oa_hash *ht = (struct oa_hash*)dict;
    oa_hash_set(ht, key, key_len, value);

    if (dict->on_insert) {
        dict->on_insert(key);
    }
}
```

The macro allows for seamless casting between compatible structures while maintaining type safety with the specified qualifiers.

## Memory Management

Unlike most hashtable implementations, this library:
- Never allocates memory internally
- Requires you to provide pre-allocated bucket arrays
- Lets you choose allocation strategy (stack, heap, arena, etc)
- Makes memory usage explicit and predictable

When using dynamic allocation:
- Free old buckets after successful rehash
- Free new buckets if rehash fails

## Build

oa_hash is single-header-only library, so it includes additional macros for more complex uses cases. `#define OA_HASH_STATIC` hides all oa_hash API symbols by making them static. Also, if you want to include `oa_hash.h` from multiple C files, to avoid duplication of symbols you may define `OA_HASH_HEADER` macro.

```c
/* In every .c file that uses oa_hash include only declarations: */
#define OA_HASH_HEADER
#include "oa_hash.h"

/* Additionally, create one oa_hash.c file for oa_hash implementation: */
#include "oa_hash.h"
```

## License

[MIT License](LICENSE) - see LICENSE file for details

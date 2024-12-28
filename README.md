# oa_hash

A lightweight open-addressing hashtable implementation in C that gives complete memory allocation control to the user.

## About

This library implements a hash table using open addressing for collision resolution, where you provide the pre-allocated buckets array. This approach gives you full control over memory management while maintaining a simple and efficient API.

## Key Features

- Zero internal memory allocations - you control all memory
- Open addressing collision resolution
- String keys with explicit lengths
- Generic void* values
- Simple and minimal API
- Single header/source pair
- C89 compatible
- MIT licensed

## Usage

```c
#include "oa_hash.h"

int main(void) {
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
}
```

## Memory Management

Unlike most hashtable implementations, this library:
- Never allocates memory internally
- Requires you to provide pre-allocated bucket arrays
- Lets you choose allocation strategy (stack, heap, arena, etc)
- Makes memory usage explicit and predictable

## Build

```bash
make        # Build library
make clean  # Clean build artifacts
```

## License

[MIT License](LICENSE) - see LICENSE file for details

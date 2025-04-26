#include "greatest.h"
#include "oa_hash.h"

#define BUCKETS_SIZE 64

static struct oa_hash ht;
static struct oa_hash_entry buckets[BUCKETS_SIZE];

static void
setup(void *data)
{
    (void)data;
    memset(buckets, 0, sizeof(buckets));
    oa_hash_init(&ht, buckets, BUCKETS_SIZE);
}

static void
teardown(void *data)
{
    (void)data;
    oa_hash_cleanup(&ht);
}

TEST
test_init(void)
{
    ASSERT_EQ(0, ht.length);
    ASSERT_EQ(BUCKETS_SIZE, ht.capacity);
    ASSERT_EQ(buckets, ht.buckets);
    PASS();
}

TEST
test_set_get(void)
{
    int value = 42;
    const struct oa_hash_entry *entry;

    entry = oa_hash_set_entry(&ht, "key", 3, &value);
    ASSERT(entry != NULL);
    ASSERT_EQ(&value, entry->value);

    entry = oa_hash_get_entry(&ht, "key", 3);
    ASSERT(entry != NULL);
    ASSERT_EQ(&value, entry->value);

    PASS();
}

TEST
test_collision(void)
{
    int val1 = 1, val2 = 2;
    const struct oa_hash_entry *entry;

    /* Insert first value */
    entry = oa_hash_set_entry(&ht, "test", 4, &val1);
    ASSERT(entry != NULL);

    /* Insert second value that may collide */
    entry = oa_hash_set_entry(&ht, "test2", 5, &val2);
    ASSERT(entry != NULL);

    /* Verify both values can be retrieved */
    entry = oa_hash_get_entry(&ht, "test", 4);
    ASSERT(entry != NULL);
    ASSERT_EQ(&val1, entry->value);

    entry = oa_hash_get_entry(&ht, "test2", 5);
    ASSERT(entry != NULL);
    ASSERT_EQ(&val2, entry->value);

    PASS();
}

TEST
test_remove(void)
{
    int value = 42;
    const struct oa_hash_entry *entry;

    /* Insert and verify */
    entry = oa_hash_set_entry(&ht, "key", 3, &value);
    ASSERT(entry != NULL);

    /* Remove and verify */
    ASSERT_EQ(1, oa_hash_remove(&ht, "key", 3));

    /* Verify it's gone */
    entry = oa_hash_get_entry(&ht, "key", 3);
    ASSERT(entry == NULL);

    PASS();
}

TEST
test_rehash(void)
{
    int value = 42;
    struct oa_hash_entry new_buckets[BUCKETS_SIZE * 2] = { 0 };
    const struct oa_hash_entry *entry;

    /* Insert initial value */
    entry = oa_hash_set_entry(&ht, "key", 3, &value);
    ASSERT(entry != NULL);

    /* Rehash to larger table */
    ASSERT_EQ(buckets, oa_hash_rehash(&ht, new_buckets, BUCKETS_SIZE * 2));

    /* Verify value still accessible */
    entry = oa_hash_get_entry(&ht, "key", 3);
    ASSERT(entry != NULL);
    ASSERT_EQ(&value, entry->value);

    PASS();
}

TEST
test_edge_cases(void)
{
    /* Empty key */
    ASSERT(NULL == oa_hash_set_entry(&ht, "key", 0, NULL));
    ASSERT(NULL == oa_hash_get_entry(&ht, "key", 0));
    ASSERT_EQ(0, oa_hash_remove(&ht, "key", 0));

    /* NULL key */
    ASSERT(NULL == oa_hash_get_entry(&ht, NULL, 5));

    PASS();
}

TEST
test_linear_probing_wraparound(void)
{
    int val1 = 1, val2 = 2;
    const struct oa_hash_entry *entry;

    // Force hash collision by using same hash value
    entry = oa_hash_set_entry(&ht, "cw", 2, &val1);
    ASSERT(entry != NULL);
    ASSERT_EQ(&val1, entry->value);

    // Should wrap around and find next empty slot
    entry = oa_hash_set_entry(&ht, "wc", 2, &val2);
    ASSERT(entry != NULL);
    ASSERT_EQ(&val2, entry->value);

    // Verify both values are still accessible
    entry = oa_hash_get_entry(&ht, "cw", 2);
    ASSERT(entry != NULL);
    ASSERT_EQ(&val1, entry->value);

    entry = oa_hash_get_entry(&ht, "wc", 2);
    ASSERT(entry != NULL);
    ASSERT_EQ(&val2, entry->value);

    PASS();
}

TEST
test_key_length_handling(void)
{
    int val1 = 1, val2 = 2;
    const struct oa_hash_entry *entry;

    // Insert key with embedded null bytes
    entry = oa_hash_set_entry(&ht, "u\0a", 3, &val1);
    ASSERT(entry != NULL);

    // Different key with same prefix should not match
    entry = oa_hash_set_entry(&ht, "u\0b", 3, &val2);
    ASSERT(entry != NULL);

    // Verify correct value is returned
    entry = oa_hash_get_entry(&ht, "u\0a", 3);
    ASSERT(entry != NULL);
    ASSERT_EQ(&val1, entry->value);

    entry = oa_hash_get_entry(&ht, "u\0b", 3);
    ASSERT(entry != NULL);
    ASSERT_EQ(&val2, entry->value);

    PASS();
}

TEST
test_lookup_stops_at_empty(void)
{
    int val = 42;
    const struct oa_hash_entry *entry;
    size_t i;

    // Fill first few slots
    for (i = 0; i < 3; i++) {
        char key[2] = { 'a' + i, '\0' };
        ASSERT(oa_hash_set_entry(&ht, key, 1, &val) != NULL);
    }

    // Lookup of non-existent key should stop at first empty slot
    entry = oa_hash_get_entry(&ht, "test", 4);
    ASSERT(entry == NULL);

    PASS();
}

TEST
test_deletion_with_gravestones(void)
{
    const struct oa_hash_entry *entry;
    int val1 = 1, val2 = 2;

    // Insert two entries that may collide
    ASSERT(oa_hash_set_entry(&ht, "test1", 5, &val1) != NULL);
    ASSERT(oa_hash_set_entry(&ht, "test2", 5, &val2) != NULL);

    // Remove first entry
    ASSERT(oa_hash_remove(&ht, "test1", 5));

    // Second entry should still be accessible
    entry = oa_hash_get_entry(&ht, "test2", 5);
    ASSERT(entry != NULL);
    ASSERT_EQ(&val2, entry->value);

    // Verify first entry slot is marked as deleted
    entry = oa_hash_get_entry(&ht, "test1", 5);
    ASSERT(entry == NULL);

    PASS();
}

SUITE(hash_suite)
{
    SET_SETUP(setup, NULL);
    SET_TEARDOWN(teardown, NULL);

    RUN_TEST(test_init);
    RUN_TEST(test_set_get);
    RUN_TEST(test_collision);
    RUN_TEST(test_remove);
    RUN_TEST(test_rehash);
    RUN_TEST(test_edge_cases);
    RUN_TEST(test_linear_probing_wraparound);
    RUN_TEST(test_key_length_handling);
    RUN_TEST(test_lookup_stops_at_empty);
    RUN_TEST(test_deletion_with_gravestones);
}

GREATEST_MAIN_DEFS();

int
main(int argc, char *argv[])
{
    GREATEST_MAIN_BEGIN();
    RUN_SUITE(hash_suite);
    GREATEST_MAIN_END();
}

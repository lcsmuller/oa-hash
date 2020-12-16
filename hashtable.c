/*
 * Copyright (c) 2020 Lucas Müller
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "hashtable.h"

hashtable_t*
hashtable_init()
{
	hashtable_t *new_hashtable = calloc(1, sizeof *new_hashtable);
	assert(NULL != new_hashtable);

	return new_hashtable;
}

void
hashtable_destroy(hashtable_t *hashtable)
{
	for (size_t i=0; i < hashtable->num_bucket; ++i){
        if (NULL == hashtable->bucket[i])
            continue;

        hashtable_entry_t *entry = hashtable->bucket[i];
        hashtable_entry_t *entry_prev;
        while (NULL != entry){
            entry_prev = entry;
            entry = entry->next;

            free(entry_prev);
            entry_prev = NULL;
        }
	}
	free(hashtable->bucket);
	hashtable->bucket = NULL;
	
	free(hashtable);
	hashtable = NULL;
}

static size_t
_hashtable_genhash(const char *key, const size_t num_bucket)
{
	size_t slot = 0;
	size_t key_len = strlen(key);

	//@todo learn different implementations and improvements
	for (size_t i=0; i < key_len; ++i){
        slot = slot * 37 + key[i];
	}

	slot %= num_bucket;

	return slot;
}

static hashtable_entry_t*
_hashtable_pair(const char *key, const void *value)
{
	hashtable_entry_t *new_entry = calloc(1, sizeof *new_entry);
	assert(NULL != new_entry);

	new_entry->key = (char*)key;
	new_entry->value = (void*)value;

	return new_entry;
}

void
hashtable_build(hashtable_t *hashtable, const size_t num_index)
{
	hashtable->num_bucket = num_index;

	hashtable->bucket = calloc(1, hashtable->num_bucket * sizeof *hashtable->bucket);
	assert(NULL != hashtable->bucket);
}

hashtable_entry_t*
_hashtable_get_entry(hashtable_t *hashtable, const char *key)
{
	if (0 == hashtable->num_bucket) return NULL;

	size_t slot = _hashtable_genhash(key, hashtable->num_bucket);

	hashtable_entry_t *entry = hashtable->bucket[slot];
	while (NULL != entry){ //try to find key and return it
        if (0 == strcmp(entry->key, key)){
            return entry;
        }
        entry = entry->next;
	}

	return NULL;
}

void*
hashtable_get(hashtable_t *hashtable, const char *key)
{
	hashtable_entry_t *entry = _hashtable_get_entry(hashtable, key);
	return (NULL != entry) ? entry->value : NULL;
}

void*
hashtable_set(hashtable_t *hashtable, const char *key, const void *value)
{
	size_t slot = _hashtable_genhash(key, hashtable->num_bucket);

	hashtable_entry_t *entry = hashtable->bucket[slot];
	if (NULL == entry){
        hashtable->bucket[slot] = _hashtable_pair(key, value);
        return hashtable->bucket[slot]->value;
	}

	hashtable_entry_t *entry_prev;
	while (NULL != entry){
        if (0 == strcmp(entry->key, key)){
            return entry->value;
        }
        entry_prev = entry;
        entry = entry->next;
	}

	entry_prev->next = _hashtable_pair(key, value);

	return (void*)value;
}

void
hashtable_remove(hashtable_t *hashtable, const char *key)
{
	if (0 == hashtable->num_bucket) return;

	size_t slot = _hashtable_genhash(key, hashtable->num_bucket);

	hashtable_entry_t *entry = hashtable->bucket[slot];
	hashtable_entry_t *entry_prev = NULL;
	while (NULL != entry){
        if (0 == strcmp(entry->key, key)){
            if (NULL != entry_prev){
                entry_prev->next = entry->next; 
            } else {
                hashtable->bucket[slot] = entry->next; 
            }

            entry->key = NULL;

            free(entry);
            entry = NULL;
            return;
        }
        entry_prev = entry;
        entry = entry->next;
	}
}

dictionary_t*
dictionary_init()
{
	dictionary_t *new_dictionary = calloc(1, sizeof *new_dictionary);
	assert(NULL != new_dictionary);

	return new_dictionary;
}

/* destroys keys and values aswell */
void
dictionary_destroy(dictionary_t *dictionary)
{
	for (size_t i=0; i < dictionary->num_bucket; ++i){
        if (NULL == dictionary->bucket[i])
            continue;

        dictionary_entry_t *entry = dictionary->bucket[i];
        dictionary_entry_t *entry_prev;
        while (NULL != entry){
            entry_prev = entry;
            entry = entry->next;

            free(entry_prev->key);
            entry_prev->key = NULL;

            //free value if its tagged for freeing
            if (entry_prev->free_cb && NULL != entry_prev->value){
                (*entry_prev->free_cb)(entry_prev->value);
            }

            free(entry_prev);
            entry_prev = NULL;
        }
	}
	free(dictionary->bucket);
	dictionary->bucket = NULL;
	
	free(dictionary);
	dictionary = NULL;
}

static dictionary_entry_t*
_dictionary_pair(const char *key, const void *value, void (*free_cb)(void*))
{
	dictionary_entry_t *new_entry = calloc(1, sizeof *new_entry);
	assert(NULL != new_entry);

	char *set_key = strndup(key, strlen(key));
	assert(NULL != set_key);

	new_entry->key = set_key;
	new_entry->value = (void*)value;
	new_entry->free_cb = free_cb;

	return new_entry;
}

/* unlike hashtable_set, if a value is already set it will free it first and then assign a new one */
void*
dictionary_set(dictionary_t *dictionary, const char *key, const void *value, void (*free_cb)(void*))
{
	size_t slot = _hashtable_genhash(key, dictionary->num_bucket);

	dictionary_entry_t *entry = dictionary->bucket[slot];
	if (NULL == entry){
        dictionary->bucket[slot] = _dictionary_pair(key, value, free_cb);
        ++dictionary->len;

        return dictionary->bucket[slot]->value;
	}

	dictionary_entry_t *entry_prev;
	while (NULL != entry){
        if (0 == strcmp(entry->key, key)){
            if (entry->free_cb && NULL != entry->value){
                (*entry->free_cb)(entry->value);
            }

            entry->value = (void*)value;
            entry->free_cb = free_cb;

            return entry->value;
        }
        entry_prev = entry;
        entry = entry->next;
	}

	entry_prev->next = _dictionary_pair(key, value, free_cb);
	++dictionary->len;

	return (void*)value;
}

void
dictionary_remove(dictionary_t *dictionary, const char *key)
{
	if (0 == dictionary->num_bucket) return;

	size_t slot = _hashtable_genhash(key, dictionary->num_bucket);

	dictionary_entry_t *entry = dictionary->bucket[slot];
	dictionary_entry_t *entry_prev = NULL;
	while (NULL != entry){
        if (0 == strcmp(entry->key, key)){
            if (NULL != entry_prev){
                entry_prev->next = entry->next; 
            } else {
                dictionary->bucket[slot] = entry->next; 
            }

            free(entry->key);
            entry->key = NULL;

            //free value if its tagged for freeing
            if (entry->free_cb && NULL != entry->value){
                (*entry->free_cb)(entry->value);
            }

            free(entry);
            entry = NULL;

            --dictionary->len;

            return;
        }
        entry_prev = entry;
        entry = entry->next;
	}
}

void*
dictionary_replace(dictionary_t *dictionary, const char *key, void *new_value)
{
	/* this works, because dictionary and hashtable structs are aligned */
	dictionary_entry_t *entry = (dictionary_entry_t*)_hashtable_get_entry((hashtable_t*)dictionary, key);

	if (entry->free_cb && NULL != entry->value){
        (*entry->free_cb)(entry->value);
	}
	entry->value = new_value;

	return entry->value;
}

/* assume value is string and return its value converted to a long long */
long long
dictionary_get_strtoll(dictionary_t *dictionary, const char *key)
{
	char *str = dictionary_get(dictionary, key);  
	if (NULL == str) return 0;

	return strtoll(str, NULL, 10);
}

/* assume value is string and return its value converted to a double */
double
dictionary_get_strtod(dictionary_t *dictionary, const char *key)
{
	char *str = dictionary_get(dictionary, key);  
	if (NULL == str) return 0.0;

	return strtod(str, NULL);
}

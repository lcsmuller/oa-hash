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

#ifndef STREQ
#define STREQ(src, dest) (0 == strcmp(src, dest))
#endif

hashtable_st*
hashtable_init()
{
  hashtable_st *new_hashtable = calloc(1, sizeof *new_hashtable);
  assert(NULL != new_hashtable);

  return new_hashtable;
}

void
hashtable_destroy(hashtable_st *hashtable)
{
  for (size_t i=0; i < hashtable->num_bucket; ++i){
    if (NULL == hashtable->bucket[i])
      continue;

    hashtable_entry_st *entry = hashtable->bucket[i];
    hashtable_entry_st *entry_prev;
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
_hashtable_genhash(const char *kKey, const size_t kNum_bucket)
{
  size_t slot = 0;
  size_t key_len = strlen(kKey);

  //@todo learn different implementations and improvements
  for (size_t i=0; i < key_len; ++i){
    slot = slot * 37 + kKey[i];
  }

  slot %= kNum_bucket;

  return slot;
}

static hashtable_entry_st*
_hashtable_pair(const char *kKey, const void *kValue)
{
  hashtable_entry_st *new_entry = calloc(1, sizeof *new_entry);
  assert(NULL != new_entry);

  new_entry->key = (char*)kKey;
  new_entry->value = (void*)kValue;

  return new_entry;
}

void
hashtable_build(hashtable_st *hashtable, const size_t kNum_index)
{
  hashtable->num_bucket = kNum_index;

  hashtable->bucket = calloc(1, hashtable->num_bucket * sizeof *hashtable->bucket);
  assert(NULL != hashtable->bucket);
}

hashtable_entry_st*
hashtable_get_entry(hashtable_st *hashtable, const char *kKey)
{
  if (0 == hashtable->num_bucket) return NULL;

  size_t slot = _hashtable_genhash(kKey, hashtable->num_bucket);

  hashtable_entry_st *entry = hashtable->bucket[slot];
  while (NULL != entry){ //try to find key and return it
    if (STREQ(entry->key, kKey)){
      return entry;
    }
    entry = entry->next;
  }

  return NULL;
}

void*
hashtable_get(hashtable_st *hashtable, const char *kKey)
{
  hashtable_entry_st *entry = hashtable_get_entry(hashtable, kKey);
  return (NULL != entry) ? entry->value : NULL;
}

void*
hashtable_set(hashtable_st *hashtable, const char *kKey, const void *kValue)
{
  size_t slot = _hashtable_genhash(kKey, hashtable->num_bucket);

  hashtable_entry_st *entry = hashtable->bucket[slot];
  if (NULL == entry){
    hashtable->bucket[slot] = _hashtable_pair(kKey, kValue);
    return hashtable->bucket[slot]->value;
  }

  hashtable_entry_st *entry_prev;
  while (NULL != entry){
    if (STREQ(entry->key, kKey)){
      return entry->value;
    }
    entry_prev = entry;
    entry = entry->next;
  }

  entry_prev->next = _hashtable_pair(kKey, kValue);

  return (void*)kValue;
}

void
hashtable_remove(hashtable_st *hashtable, const char *kKey)
{
  if (0 == hashtable->num_bucket) return;

  size_t slot = _hashtable_genhash(kKey, hashtable->num_bucket);

  hashtable_entry_st *entry = hashtable->bucket[slot];
  hashtable_entry_st *entry_prev = NULL;
  while (NULL != entry){
    if (STREQ(entry->key, kKey)){
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

dictionary_st*
dictionary_init()
{
  dictionary_st *new_dictionary = calloc(1, sizeof *new_dictionary);
  assert(NULL != new_dictionary);

  return new_dictionary;
}

/* destroys keys and values aswell */
void
dictionary_destroy(dictionary_st *dictionary)
{
  for (size_t i=0; i < dictionary->num_bucket; ++i){
    if (NULL == dictionary->bucket[i])
      continue;

    dictionary_entry_st *entry = dictionary->bucket[i];
    dictionary_entry_st *entry_prev;
    while (NULL != entry){
      entry_prev = entry;
      entry = entry->next;

      free(entry_prev->key);
      entry_prev->key = NULL;
      
      //free value if its tagged for freeing
      if (entry_prev->to_free){
        void **p_ptr = entry_prev->value;
        if (NULL != p_ptr){
          free(*p_ptr);
          *p_ptr = NULL;
        }
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

static dictionary_entry_st*
_dictionary_pair(const char *kKey, const void *kValue, _Bool to_free)
{
  dictionary_entry_st *new_entry = calloc(1, sizeof *new_entry);
  assert(NULL != new_entry);

  char *set_key = strndup(kKey, strlen(kKey));
  assert(NULL != set_key);

  new_entry->key = set_key;
  new_entry->value = (void*)kValue;
  new_entry->to_free = to_free;

  return new_entry;
}

/* unlike hashtable_set, if a value is already set it will free it first and then assign a new one */
void*
dictionary_set(dictionary_st *dictionary, const char *kKey, const void *kValue, _Bool to_free)
{
  size_t slot = _hashtable_genhash(kKey, dictionary->num_bucket);

  dictionary_entry_st *entry = dictionary->bucket[slot];
  if (NULL == entry){
    dictionary->bucket[slot] = _dictionary_pair(kKey, kValue, to_free);
    ++dictionary->len;

    return dictionary->bucket[slot]->value;
  }

  dictionary_entry_st *entry_prev;
  while (NULL != entry){
    if (STREQ(entry->key, kKey)){
      if (entry_prev->to_free){
        void **p_ptr = entry_prev->value;
        if (NULL != p_ptr){
          free(*p_ptr);
          *p_ptr = NULL;
        }
      }
      entry->value = (void*)kValue;
      entry->to_free = to_free;
      return entry->value;
    }
    entry_prev = entry;
    entry = entry->next;
  }

  entry_prev->next = _dictionary_pair(kKey, kValue, to_free);
  ++dictionary->len;

  return (void*)kValue;
}

void
dictionary_remove(dictionary_st *dictionary, const char *kKey)
{
  if (0 == dictionary->num_bucket) return;

  size_t slot = _hashtable_genhash(kKey, dictionary->num_bucket);

  dictionary_entry_st *entry = dictionary->bucket[slot];
  dictionary_entry_st *entry_prev = NULL;
  while (NULL != entry){
    if (STREQ(entry->key, kKey)){
      if (NULL != entry_prev){
        entry_prev->next = entry->next; 
      } else {
        dictionary->bucket[slot] = entry->next; 
      }

      free(entry->key);
      entry->key = NULL;
      
      //free value if its tagged for freeing
      if (entry_prev->to_free){
        void **p_ptr = entry_prev->value;
        if (NULL != p_ptr){
          free(*p_ptr);
          *p_ptr = NULL;
        }
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

/* this should only be called if dictionary_entry value type is
    a pointer to a string char ** */
char*
dictionary_new_string(dictionary_st *dictionary, const char *kKey, char *src)
{
  char **p_dest = dictionary_get(dictionary, kKey);

  if (NULL == p_dest) return NULL; 

  free(*p_dest);
  *p_dest = src;

  return *p_dest;
}

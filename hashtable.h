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

#ifndef HASHTABLE_H_
#define HASHTABLE_H_

typedef struct hashtable_entry_s {
  char *key; //this entry key tag
  void *value; //this entry value
  struct hashtable_entry_s *next; //next entry pointer for when keys don't match
} hashtable_entry_st;

typedef struct hashtable_s {
  hashtable_entry_st **bucket;
  size_t num_bucket;
} hashtable_st;

hashtable_st* hashtable_init();
void hashtable_destroy(hashtable_st *hashtable);
void hashtable_build(hashtable_st *hashtable, const size_t kNum_index);
hashtable_entry_st *hashtable_get_entry(hashtable_st *hashtable, const char *kKey);
void *hashtable_get(hashtable_st *hashtable, const char *kKey);
void *hashtable_set(hashtable_st *hashtable, const char *kKey, const void *kValue);
void hashtable_remove(hashtable_st *hashtable, const char *kKey);

typedef struct dictionary_entry_s {
  char *key; //this entry key tag
  void *value; //this entry value
  struct dictionary_entry_s *next; //next entry pointer for when keys don't match
  _Bool to_free; //set to true to be freed on dictionary_destroy()
} dictionary_entry_st;

/* basically a hashtable with some extra functionalities
    it will allocate the key and free it up for you, also
    allows to pass a value that may be tagged for being freed */
typedef struct dictionary_s {
  dictionary_entry_st **bucket;
  size_t num_bucket;
  size_t len;
} dictionary_st;

dictionary_st* dictionary_init();
void dictionary_destroy(dictionary_st *dictionary);

#define dictionary_build(dict, num_index) hashtable_build((hashtable_st*)dict, num_index)
#define dictionary_get(dict, key) hashtable_get((hashtable_st*)dict, key)
void *dictionary_set(dictionary_st *dictionary, const char *kKey, const void *kValue, _Bool to_free);
void dictionary_remove(dictionary_st *dictionary, const char *kKey);
char *dictionary_new_string(dictionary_st *dictionary, const char *kKey, char *src);


#endif

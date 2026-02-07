#ifndef HTB_H
#define HTB_H

#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HT_MAX_KEY_SIZE 1024

typedef struct {
    char *key;
    char *value;
} ht_item;

typedef struct {
    int count;
    int capacity;
    int base_size;
    ht_item **items;
} ht_table;

ht_table *ht_new();
void ht_table_delete(ht_table *ht);
bool ht_insert(ht_table *ht, const char *key, const char *value);
char *ht_get(ht_table *ht, const char *key);
bool ht_set(ht_table *ht, const char *key, const char *value);
void ht_delete(ht_table *ht, const char *key);

#ifndef HASH_TABLE_IMPLEMENTATION

static int ht__is_prime(const int x) {
    if (x < 2) {
        return -1;
    }
    if (x < 4) {
        return 1;
    }
    if ((x % 2) == 0) {
        return 0;
    }
    for (int i = 3; i <= floor(sqrt((double)x)); i += 2) {
        if ((x % i) == 0) {
            return 0;
        }
    }
    return 1;
}

static int ht__next_prime(int x) {
    while (ht__is_prime(x) != 1) {
        x++;
    }
    return x;
}

static ht_item HT_DELETED_ITEM = {NULL, NULL};
#define HT_INITIAL_BASE_SIZE 50
#define HT_PRIME_1 1171
#define HT_PRIME_2 3637

static ht_table *ht_new_sized(const int base_size) {
    ht_table *ht = (ht_table *)malloc(sizeof(ht_table));
    ht->base_size = base_size;
    ht->capacity = ht__next_prime(base_size);
    ht->count = 0;
    ht->items = (ht_item **)calloc((size_t)ht->capacity, sizeof(ht_item *));
    if (ht->items == NULL) {
        fprintf(stderr, "ERROR: Can't calloc ht_table: %s\n", strerror(errno));
        return NULL;
    }
    return ht;
}

ht_table *ht_new() { return ht_new_sized(HT_INITIAL_BASE_SIZE); }

static void ht_resize(ht_table *ht, const int base_size) {
    if (base_size < HT_INITIAL_BASE_SIZE) return;

    ht_table *temp_table = ht_new_sized(base_size);
    for (int i = 0; i < ht->capacity; ++i) {
        ht_item *item = ht->items[i];
        if (item != NULL && item != &HT_DELETED_ITEM) {
            ht_insert(temp_table, item->key, item->value);
        }
    }

    ht->base_size = temp_table->base_size;
    ht->count = temp_table->count;
    const int tmp_size = ht->capacity;
    ht->capacity = temp_table->capacity;
    temp_table->capacity = tmp_size;

    ht_item **tmp_items = ht->items;
    ht->items = temp_table->items;
    temp_table->items = tmp_items;

    ht_table_delete(temp_table);
}

static void ht_resize_up(ht_table *ht) {
    const int new_size = ht->base_size * 2;
    ht_resize(ht, new_size);
}

static void ht_resize_down(ht_table *ht) {
    const int new_size = ht->base_size / 2;
    ht_resize(ht, new_size);
}

static int ht_load(ht_table *ht) { return ht->count * 100 / ht->capacity; }

static ht_item *ht_new_item(const char *k, const char *v) {
    ht_item *new_item = (ht_item *)malloc(sizeof(ht_item));
    if (new_item == NULL) {
        fprintf(stderr, "ERROR: Cannot allocate item for key: %s\n", k);
        return NULL;
    }
    new_item->key = strdup(k);
    new_item->value = strdup(v);
    return new_item;
}

static void ht_delete_item(ht_item *item) {
    if (item == NULL || item == &HT_DELETED_ITEM) return;
    free(item->key);
    free(item->value);
    free(item);
}

void ht_table_delete(ht_table *ht) {
    for (int i = 0; i < ht->count; ++i) {
        ht_delete_item(ht->items[i]);
    }
    free(ht->items);
    free(ht);
}

static int ht_hash(const char *s, const int num_buckets, const int a) {
    int n = strnlen(s, HT_MAX_KEY_SIZE);
    long hash = 0;
    for (int i = 0; i < n; i++) {
        hash += (long)pow(a, n - (i + 1)) * s[i];
        hash = hash % num_buckets;
    }

    return (int)hash;
}

static int ht_get_hash(const char *s, const int num_buckets,
                       const int attempts) {
    const int hash_a = ht_hash(s, num_buckets, HT_PRIME_1);
    const int hash_b = ht_hash(s, num_buckets, HT_PRIME_2);

    return (hash_a + (attempts * (hash_b + 1))) % num_buckets;
}

bool ht_insert(ht_table *ht, const char *key, const char *value) {
    if (ht_load(ht) > 70) {
        ht_resize_up(ht);
    }

    ht_item *new_item = ht_new_item(key, value);
    int attempts = 0;
    int index = ht_get_hash(new_item->key, ht->capacity, attempts);
    ht_item *curr_item = ht->items[index];

    while (curr_item != NULL && curr_item != &HT_DELETED_ITEM) {
        if (strcmp(curr_item->key, key) == 0) {
            fprintf(stderr, "ERROR: Key %s already exists\n", key);
            return false;
        }

        ++attempts;
        index = ht_get_hash(new_item->key, ht->capacity, attempts);
        curr_item = ht->items[index];
    }

    ht->items[index] = new_item;
    ++ht->count;
    return true;
}

char *ht_get(ht_table *ht, const char *key) {
    int attempts = 0;
    int index = ht_get_hash(key, ht->capacity, attempts);
    ht_item *item = ht->items[index];
    while (item != NULL) {
        if (item != &HT_DELETED_ITEM && strcmp(item->key, key) == 0) {
            return item->value;
        }
        ++attempts;
        index = ht_get_hash(key, ht->capacity, attempts);
        item = ht->items[index];
    }
    return NULL;
}

bool ht_set(ht_table *ht, const char *key, const char *value) {
    int attempts = 0;
    int index = ht_get_hash(key, ht->capacity, attempts);
    ht_item *item = ht->items[index];
    while (item != NULL) {
        if (item != &HT_DELETED_ITEM && strcmp(item->key, key) == 0) {
            free(ht->items[index]->value);
            ht->items[index]->value = strdup(value);
            return true;
        }
        ++attempts;
        index = ht_get_hash(key, ht->capacity, attempts);
        item = ht->items[index];
    }
    return false;
}

void ht_delete(ht_table *ht, const char *key) {
    if (ht_load(ht) < 10) {
        ht_resize_down(ht);
    }
    int attempts = 0;
    int index = ht_get_hash(key, ht->capacity, attempts);
    ht_item *item = ht->items[index];
    while (item != NULL) {
        if (item != &HT_DELETED_ITEM && strcmp(item->key, key) == 0) {
            ht_delete_item(item);
            ht->items[index] = &HT_DELETED_ITEM;
        }
        ++attempts;
        index = ht_get_hash(key, ht->capacity, attempts);
        item = ht->items[index];
    }
    --ht->count;
}

#endif

#endif /* ifndef HTB_H */

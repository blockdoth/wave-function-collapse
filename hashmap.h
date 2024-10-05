#include <stddef.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

typedef struct Entry {
    char* key;              // key is NULL if this slot is empty
    void* value;
    struct Entry* next;
} Entry;

typedef struct HashMap{
    Entry** entries;                    // hash slots
    size_t num_buckets;                 // size of _entries array
    size_t size;                        // number of items in hash table
    unsigned int (*hash)(char *key);    // hash function
} HashMap;


HashMap *create_hashmap(size_t key_space);
void delete_hashmap(HashMap *hm);

Entry *newEntry(void);
void insert_data(HashMap *hm, char *key, void *data);
void remove_data(HashMap *hm, char *key);
void *get_data(HashMap *hm, char *key);

unsigned int hash(char *key);
void set_hash_function(HashMap *hm, unsigned int (*hash_function)(char *key));

size_t memSize(HashMap *hm);

#ifndef HASHMAP_IMPL
#define HASHMAP_IMPL
HashMap *create_hashmap(size_t key_space){
    if(key_space < 1){
        return NULL;
    }

    HashMap *hm = calloc(1,sizeof(HashMap));
    if (hm == NULL){
        return NULL;
    }

    hm->entries = calloc(key_space,sizeof(Entry*));
    if (hm->entries == NULL){
        free(hm);
        return NULL;
    }
    hm->num_buckets = key_space;
    hm->size = 0;
    set_hash_function(hm, hash);
    for(size_t i = 0; i < key_space; i++){
        hm->entries[i] = newEntry();
        if (hm->entries[i] == NULL){
            free(hm);
            return NULL;
        }
    }
    return hm;
}

Entry *newEntry(void){
    Entry *new_entry = calloc(1,sizeof(Entry));
    if (new_entry == NULL){
        return NULL;
    }
    new_entry->key = NULL;
    new_entry->value = NULL;
    new_entry->next = NULL;
    return new_entry;
}

void delete_hashmap(HashMap *hm) {
    if(hm == NULL){
        return;
    }
    for (size_t i = 0; i < hm->num_buckets; i++) {
        Entry *entry = hm->entries[i];
        if (entry->key != NULL) {
            // Free the memory for all the entries in the list
            while(entry != NULL){
                Entry *next_entry = entry->next;
                free(entry->key);
                free(entry);
                entry = next_entry;
            }
        }
        free(entry);
    }
    free(hm->entries);
    free(hm);
}

void insert_data(HashMap *hm, char *key, void *data) {
    if(hm == NULL || key == NULL){
        return;
    }
    Entry *entry = hm->entries[hm->hash(key) % hm->num_buckets];

    char* key_copy = calloc(sizeof(char), (strlen(key) + 1));
    if(key_copy == NULL){
        return;
    }
    strcpy(key_copy, key);

    if(entry->key == NULL){
        //check if the list is empty
        entry->key = key_copy;
        entry->value = data;
        hm->size++;
        return;
    }
    //check if key already exists in the list
    while(entry->next != NULL ){
        if(strcmp(entry->key,key) == 0) {

            free(entry->value);
            entry->value = data;
            free(key_copy);
            return;
        }
        entry = entry->next;
    }
    //create new entry
    Entry *new_entry = newEntry();
    if(new_entry == NULL){
        free(key_copy);
        return;
    }
    new_entry->key = key_copy;
    new_entry->value = data;
    entry->next = new_entry;
    hm->size++;
}

void remove_data(HashMap *hm, char *key) {
    if(hm == NULL || key == NULL){
        return;
    }
    unsigned int hash_key = hm->hash(key) % hm->num_buckets;
    Entry *entry = hm->entries[hash_key];

    if (entry->key == NULL) {
        return;
    }

    Entry *prev_entry = NULL;
    while (entry->next != NULL && strcmp(entry->key, key) != 0) {
        prev_entry = entry;
        entry = entry->next;
    }
    //Found correct entry
    if (prev_entry == NULL) {
        if(entry->next == NULL){
            //Only element in list
            free(entry->key);
            entry->value = NULL;
            entry->key = NULL;
            hm->size--;
            return;
        }else{
            //First element in the list
            hm->entries[hash_key] = entry->next;
            entry->next = NULL;
        }
    } else {
        // Entry in the middle or end of the list
        prev_entry->next = entry->next;
    }
    //free(entry->value);
    free(entry->key);
    free(entry);
    hm->size--;
}

void *get_data(HashMap *hm, char *key){
    if(hm == NULL || key == NULL){
        return NULL;
    }
    unsigned int hash_key = hm->hash(key) % hm->num_buckets;
    Entry *entry = hm->entries[hash_key];
    if(entry->key == NULL){
        return NULL;
    }
    while(entry->key != NULL){
        if(strcmp(entry->key,key) == 0){
            return entry->value;
        }
        entry = entry->next;
        if (entry == NULL) {
            return NULL;
        }
    }
    return NULL;
}

unsigned int hash(char *key){
    unsigned int hash = 0;
    while (*key != '\0') {
        hash += *key;
        key++;
    }
    return hash;
}


void set_hash_function(HashMap *hm, unsigned int (*hash_function)(char *key)){
    if(hm == NULL || hash_function == NULL){
        return;
    }
    hm->hash = hash_function;
    if(hm->size == 0){
        return;
    }

    HashMap *new_hm = create_hashmap(hm->num_buckets);
    new_hm->hash = hm->hash;
    for(size_t i = 0; i < hm->num_buckets; i++){
        Entry *entry = hm->entries[i];
        if(entry->key != NULL){
            while(entry != NULL){
                insert_data(new_hm,entry->key,entry->value);
                entry = entry->next;
            }
        }
    }
    Entry** old_entries = hm->entries;
    hm->entries = new_hm->entries;
    new_hm->entries = old_entries;
    delete_hashmap(new_hm);
}


size_t memSize(HashMap *hm) {
    size_t mem_size = sizeof(HashMap);
    for (size_t i = 0; i < hm->num_buckets; i++) {
        mem_size += sizeof(Entry);
    }
    return mem_size;
}
#endif

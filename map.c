#ifndef __MAP
#define __MAP

#include <stdlib.h>
#include <string.h>

typedef struct map {
	int size;
	void* index[1024];
} map;

int hash(char* key) {
	int i, res = 0, len = strlen(key);
	for (i = 0; i < len; i++) {
		res = (((res * 26) % 1024) + key[i]) % 1024;
	}
	return res;
}

map* create_new_map() {
	map* m = malloc(sizeof(map));
	int i;
	for (i = 0; i < m -> size; i++) {
		m -> index[i] = NULL;
	}
	return m;
}

void put_in_map(map* m, char* key, void* value) {
	int key_hash = hash(key);
	m -> index[key_hash] = value;
}

void* get_from_map(map* m, char* key) {
	int key_hash = hash(key);
	return m -> index[key_hash];
}

void* remove_from_map(map* m, char* key) {
	int key_hash = hash(key);
	void* value = m -> index[key_hash];
	m -> index[key_hash] = NULL;
	return value;
}

#endif
#ifndef __KIVI
#define __KIVI

#include "constants.h"
#include "buffer.h"
#include "map.c"
#include "el.h"

typedef struct kivi_client {
	int fd;
	buffer* in_buf;
	buffer* out_buf;
} kivi_client;

typedef struct kivi_server {
	int fd;
	int max_clients;
	kivi_client** clients;
	event_loop el;
	
	map* db;
} kivi_server;

extern kivi_server server;

#endif

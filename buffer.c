#include "buffer.h"
#include "logging.c"

#include <stdio.h>
#include <stdlib.h>

buffer* create_new_buffer(int size) {
	buffer* b = malloc(sizeof(buffer));
	b -> size = size;
	b -> start_pos = 0;
	b -> end_pos = 0;
	b -> buf = malloc(size * sizeof(char));
	b -> buf[0] = '\0';
	return b;
}

void destroy_buffer(buffer *b) {
	free(b -> buf);
	free(b);
}

int buflen(buffer* b) {
	return b -> end_pos - b -> start_pos;
}

void append_to_buffer(buffer* b, char* s) {
	char tmp[100];
	sprintf(tmp, "%d%c\0", strlen(s), SEPARATOR);
	int len = strlen(s), res_len = strlen(tmp);

	memcpy(b -> buf +  b -> end_pos, tmp, res_len * sizeof(char));
	memcpy(b -> buf + b -> end_pos + res_len, s, len * sizeof(char));
	b -> end_pos += res_len + len;
	b -> buf[b -> end_pos] = '\0';
}

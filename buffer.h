#ifndef __KIVI_BUFFER
#define __KIVI_BUFFER

typedef struct buffer {
	int size;
	int start_pos;
	int end_pos;
	char* buf;
} buffer;

buffer* create_new_buffer(int size);
void destroy_buffer(buffer *b);
int buflen(buffer* b);
void append_to_buffer(buffer* b, char* s);

#endif
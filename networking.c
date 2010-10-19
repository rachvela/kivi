#include "kivi.h"
#include "constants.h"
#include "logging.c"
#include "buffer.c"
#include "el.h"
#include "map.c"

int set_non_blocking(int fd) {
	int opts = fcntl(fd, F_GETFL);
	if (opts < 0) {
		return FAILED;
	}
	
	opts |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, opts) < 0) {
		return FAILED;
	}
	return OK;
}

kivi_client* create_new_client(int fd) {
	kivi_client* client = malloc(sizeof(kivi_client));
	client -> fd = fd;
	client -> in_buf = create_new_buffer(INITIAL_BUFFER_SIZE);
	client -> out_buf = create_new_buffer(INITIAL_BUFFER_SIZE);
	return client;
}

void destroy_client(kivi_client* client) {
	destroy_buffer(client -> in_buf);
	destroy_buffer(client -> out_buf);
	free(client);
}

void remove_client(int fd) {
	remove_from_el(&server.el, fd);
	destroy_client(server.clients[fd]);
	server.clients[fd] = NULL;
}

int socket_reader(event_loop* el, int fd, kivi_client* client) {
	buffer* in_buf = client -> in_buf;
	buffer* out_buf = client -> out_buf;
	int sz = recv(fd, &(in_buf -> buf[in_buf -> end_pos]), (in_buf -> size - in_buf -> end_pos) * sizeof(char), 0);
	if (sz < 0) {
		if (errno == EAGAIN) {
			return;
		}
		log_warn("couldn't read from socket %d", fd);
		return FAILED;
	} else if (sz == 0) {
		remove_client(fd);
		close(fd);
		return OK;
	} else {
		in_buf -> buf[in_buf -> end_pos + sz] = '\0';
		in_buf -> end_pos += sz;
		return OK;
	}
}

int socket_writer(event_loop* el, int fd, kivi_client* client) {
	buffer* out_buf = client -> out_buf;
	if (buflen(out_buf)) {
		int sz = send(fd, &(out_buf -> buf[out_buf -> start_pos]), buflen(out_buf), 0);
		if (sz == -1) {
			log_warn("couldn't send message to %d", fd);
			return FAILED;
		} else {
			if (sz) {
				memcpy(out_buf -> buf, out_buf -> buf + out_buf -> start_pos + sz, (buflen(out_buf) - sz) * sizeof(char));
				out_buf -> start_pos = 0;
				out_buf -> end_pos -= sz;
				out_buf -> buf[out_buf -> end_pos] = '\0';
			}
			return OK;
		}
	} else {
		return OK;
	}
}

int new_connection_handler(event_loop* el, int fd, kivi_client* client) {
	log_info("new connection %d", fd);
	int client_fd = accept(fd, NULL, NULL);
	if (client_fd > 0) {
		if (set_non_blocking(client_fd)) {
			log_warn("couldn't make client socket nonblocking %d", client_fd);
			return FAILED;
		}
		
		file_event file_ev;
		file_ev.mask = READ_EVENT | WRITE_EVENT;
		file_ev.read_handler = socket_reader;
		file_ev.write_handler = socket_writer;
		if (add_to_el(el, client_fd, file_ev) == FAILED) {
			return FAILED;
		} else {
			client = create_new_client(client_fd);
			server.clients[client_fd] = client;
		}
	} else {
		return FAILED;
	}

	return OK;
}

void process_request(int fd) {
	if (server.clients[fd] == NULL) {
		return;
	}

	buffer* in_buf = server.clients[fd] -> in_buf;
	buffer* out_buf = server.clients[fd] -> out_buf;
	int i, len = buflen(in_buf);
	int req_len_end_pos = -1;
	for (i = 0; i < len; i++) {
		if (in_buf -> buf[i] == SEPARATOR) {
			req_len_end_pos = i;
			break;
		}
	}
	
	if (req_len_end_pos == -1) {
		return;
	}
	int req_len = -1;
	char tmp;
	sscanf(in_buf -> buf, "%d%c", &req_len, &tmp);
	if (len - req_len_end_pos - 1 < req_len) {
		return;
	}
	
	char op[100], *rest, tmp_buf[1024];
	memcpy(tmp_buf, in_buf -> buf + req_len_end_pos + 1, req_len * sizeof(char));
	tmp_buf[req_len] = '\0';
	sscanf(tmp_buf, "%s", op);
	rest = tmp_buf + strlen(op) + 1;
	
	if (strcmp(op, "GET") == 0) {
		void* res = get_from_map(server.db, rest);
		if (res) {
			append_to_buffer(out_buf, (char*) res);
		} else {
			append_to_buffer(out_buf, "");
		}
	} else if (strcmp(op, "SET") == 0) {
		char key[100], value[100];
		sscanf(rest, "%s %s", key, value);
		free(remove_from_map(server.db, key));
		char* kivi_obj = malloc((strlen(value) + 1) * sizeof(char));
		memcpy(kivi_obj, value, (strlen(value) + 1) * sizeof(char));
		put_in_map(server.db, key, kivi_obj);
		append_to_buffer(out_buf, "OK");
	} else if (strcmp(op, "DEL") == 0) {
		free(remove_from_map(server.db, rest));
		append_to_buffer(out_buf, "OK");
	}
	
	memcpy(in_buf -> buf, in_buf -> buf + req_len_end_pos + 1 + req_len, (len - req_len_end_pos - 1 - req_len) * sizeof(char));
	in_buf -> end_pos -= req_len_end_pos + 1 + req_len;
	in_buf -> buf[in_buf -> end_pos] = '\0';
}

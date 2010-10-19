#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>

#include "kivi.h"
#include "constants.h"
#include "buffer.h"
#include "logging.c"
#include "map.c"
#include "el.h"
#include "el_epoll.c"
#include "networking.c"

kivi_server server;

int init_server(int addr, int port, int max_connections) {
	struct sockaddr_in srv;
	
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		return FAILED;
	}
	
	bzero(&srv, sizeof(srv));
	srv.sin_family = AF_INET;
	srv.sin_addr.s_addr = htonl(addr);
	srv.sin_port = htons(port);
	
	if (bind(fd, (struct sockaddr*) &srv, sizeof(srv)) < 0) {
		return FAILED;
	}
	if (listen(fd, max_connections) < 0) {
		return FAILED;
	} else {
		return fd;
	}
}

int main(int argc, char** argv) {
	create_event_loop_api();
	
	server.max_clients = MAX_EVENTS;
	server.el.max_events = MAX_EVENTS;
	server.el.state = create_el(&server.el);
	if (server.el.state == NULL) {
		log_error("Couldn't create event loop");
		return FAILED;
	}
	server.db = create_new_map();
	server.clients = malloc(server.max_clients * sizeof(kivi_client*));
	int j;
	for (j = 0; j < server.max_clients; j++) {
		server.clients[j] = NULL;
	}

	server.fd = init_server(INADDR_ANY, PORT, MAX_EVENTS);
	if (server.fd != FAILED) {
		log_info("server initialized: %d", server.fd);
	} else {
		log_error("server initialization failed");
		return FAILED;
	}

	file_event server_event;
	server_event.mask = READ_EVENT;
	server_event.read_handler = new_connection_handler;
	log_info("add_to_event_loop %d", add_to_el(&server.el, server.fd, server_event));
			
	int i, event_count;
	while (1) {
		event_count = wait_el(&server.el);

		// process fired events
		for (i = 0; i < event_count; i++) {
			if (server.el.fired_events[i].mask & READ_EVENT && server.el.registered_file_events[server.el.fired_events[i].fd].read_handler != NULL) {
				server.el.registered_file_events[server.el.fired_events[i].fd].read_handler(&server.el, server.el.fired_events[i].fd, server.clients[server.el.fired_events[i].fd]);
			}
			if (server.el.fired_events[i].mask & WRITE_EVENT && server.clients[server.el.fired_events[i].fd] != NULL && server.el.registered_file_events[server.el.fired_events[i].fd].write_handler != NULL) {
				server.el.registered_file_events[server.el.fired_events[i].fd].write_handler(&server.el, server.el.fired_events[i].fd, server.clients[server.el.fired_events[i].fd]);
			}
		}
		
		// process connection in_buffers
		for (i = 0; i < event_count; i++) {
			process_request(server.el.fired_events[i].fd);
		}
	}
	
	return OK;
}

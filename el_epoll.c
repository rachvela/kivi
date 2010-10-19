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
#include "el.h"

typedef struct epoll_state {
	int epoll_fd;
	struct epoll_event* events;
} epoll_state;

void* create_epoll_event_loop(event_loop* el) {
	int epoll_fd = epoll_create(el -> max_events);
	if (epoll_fd < 0) {
		return NULL;
	} else {
		epoll_state* state = malloc(sizeof(epoll_state));
		state -> epoll_fd = epoll_fd;
		state -> events = malloc(el -> max_events * sizeof(struct epoll_event));
		return state;
	}
}

int add_to_epoll_event_loop(event_loop* el, int fd, file_event file_ev) {
	struct epoll_event epoll_ev;
	epoll_ev.data.fd = fd;
	epoll_ev.events = 0;
	if (file_ev.mask & READ_EVENT) {
		epoll_ev.events |= EPOLLIN;
	}
	if (file_ev.mask & WRITE_EVENT) {
		epoll_ev.events |= EPOLLOUT;
	}
	epoll_state* state = (epoll_state*) el -> state;
	if (epoll_ctl(state -> epoll_fd, EPOLL_CTL_ADD, fd, &epoll_ev) < 0) {
		return FAILED;
	} else {
		el -> registered_file_events[fd] = file_ev;
		return OK;
	}
}

int remove_from_epoll_event_loop(event_loop* el, int fd) {
	struct epoll_event epoll_ev;
	if (epoll_ctl(((epoll_state*) el -> state) -> epoll_fd, EPOLL_CTL_DEL, fd, &epoll_ev) < 0) {
		return FAILED;
	} else {
		if (close(fd) < 0) {
			return FAILED;
		}
	}
	
	return OK;
}

int start_epoll_event_loop(event_loop* el) {
	el -> stop = 0;
	return OK;
}

int stop_epoll_event_loop(event_loop* el) {
	el -> stop = 1;
	return OK;
}

int get_epoll_fired_events(event_loop* el) {
	epoll_state* state = (epoll_state*) el -> state;
	int i, event_count;

	event_count = epoll_wait(state -> epoll_fd, state -> events, el -> max_events, 60000);
	for (i = 0; i < event_count; i++) {
		el -> fired_events[i].fd = state -> events[i].data.fd;
		el -> fired_events[i].mask = 0;
		if (state -> events[i].events & EPOLLIN) {
			el -> fired_events[i].mask |= READ_EVENT;
		}
		if (state -> events[i].events & EPOLLOUT) {
			el -> fired_events[i].mask |= WRITE_EVENT;
		}
	}
	
	return event_count;
}

void destroy_epoll_event_loop(event_loop* el) {
	epoll_state* state = (epoll_state*) el -> state;
	free(state -> events);
	free(state);
}

void create_event_loop_api() {
	create_el = create_epoll_event_loop;
	start_el = start_epoll_event_loop;
	stop_el = stop_epoll_event_loop;
	add_to_el = add_to_epoll_event_loop;
	remove_from_el = remove_from_epoll_event_loop;
	wait_el = get_epoll_fired_events;
	destroy_el = destroy_epoll_event_loop;
}

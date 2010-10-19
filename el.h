#ifndef __EL
#define __EL

#define READ_EVENT 1
#define WRITE_EVENT 2

struct event_loop;
struct file_event;
struct kivi_client;

typedef int file_event_handler(struct event_loop* el, int fd, struct kivi_client* client);

typedef struct file_event {
	int mask;
	file_event_handler* read_handler;
	file_event_handler* write_handler;
	struct kivi_client* client;
} file_event;

typedef struct fired_event {
	int fd;
	int mask;
} fired_event;

typedef struct event_loop {
	int max_events;
	int stop;
	
	file_event registered_file_events[1024];
	fired_event fired_events[1024];
	void* state;
} event_loop;

typedef void* create_event_loop(event_loop* el);
typedef int start_event_loop(event_loop* el);
typedef int stop_event_loop(event_loop* el);
typedef int add_to_event_loop(event_loop* el, int fd, file_event file_ev);
typedef int remove_from_event_loop(event_loop* el, int fd);
typedef int get_fired_events(event_loop* el);
typedef void destroy_event_loop(event_loop* el);

create_event_loop* create_el;
start_event_loop* start_el;
stop_event_loop* stop_el;
add_to_event_loop* add_to_el;
remove_from_event_loop* remove_from_el;
get_fired_events* wait_el;
destroy_event_loop* destroy_el;

#endif
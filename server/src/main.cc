#include <wayland-server.h>

#include "subsystem.h"
#include "subsystem/input.h"
#include "event.h"

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cerrno>

#define DEFAULT_SEAT "seat0"

int main(int argc, char *argv[])
{
	wl_display *display = wl_display_create();
	if (!display) {
		fprintf(stderr, "Unable to create the wayland display\n");
		return -EFAULT;
	}

	const char *socket = wl_display_add_socket_auto(display);
	if (!socket) {
		fprintf(stderr, "Unable to add socket to the wayland display\n");
		wl_display_destroy(display);
		return -EFAULT;
	}

	printf("Allocated socket: %s\n", socket);

	wl_event_loop *event_loop = wl_display_get_event_loop(display);
	if (!event_loop) {
		fprintf(stderr, "Failed to get the event_loop\n");
		wl_display_destroy(display);
		return -EFAULT;
	}

	ncway::Input *input = ncway::Input::Create(DEFAULT_SEAT);
	wl_event_source *event_source = wl_event_loop_add_fd(event_loop, input->getFD(), WL_EVENT_READABLE, ncway::Event::eventHandler, input);
	if (!event_source) {
		fprintf(stderr, "Failed to add the input fd to the wl_event_loop\n");
		delete input;
		wl_display_destroy(display);
		return -EFAULT;
	}

	printf("Running the wayland display on %s\n", socket);
	wl_display_run(display);

	delete input;
	wl_display_destroy(display);
	return 0;
}

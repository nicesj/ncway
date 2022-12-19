#include <wayland-server.h>

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cerrno>

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

	printf("Running the wayland display on %s\n", socket);
	wl_display_run(display);

	wl_display_destroy(display);
	return 0;
}

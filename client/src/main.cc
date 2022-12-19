#include <cstdio>
#include <cerrno>
#include <wayland-client.h>

int main(int argc, char *argv[])
{
	wl_display *display = wl_display_connect(nullptr);
	if (!display) {
		fprintf(stderr, "Unable to connect to a display");
		return -EFAULT;
	}

	printf("Connection established\n");

	wl_display_disconnect(display);
	return 0;
}

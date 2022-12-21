#include <wayland-server.h>
#include <libinput.h>
#include <libudev.h>

#include "subsystem.h"

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cerrno>

#define DEFAULT_SEAT "seat0"

static int input_event_callback(int fd, uint32_t mask, void *data)
{
	// NOTE:
	// casting void * to libinput *, it is recommended to use static_cast
	libinput *li = static_cast<libinput *>(data);
	if (libinput_dispatch(li) < 0) {
		fprintf(stderr, "Failed to dispatch the libinput event\n");
		return 1;
	}

	libinput_event *evt = libinput_get_event(li);
	if (!evt) {
		fprintf(stderr, "Failed to fetch an event\n");
		return 1;
	}

	// TODO:
	// Let's deal with the event object!!!

	libinput_event_destroy(evt);
	return 1;
}

static void close_restricted(int fd, void *user_data)
{
}

static int open_restricted(const char *path, int flags, void *user_data)
{
	return 0;
}

static libinput_interface interface {
	.open_restricted = open_restricted,
	.close_restricted = close_restricted,
};

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

	udev *ptr_udev = udev_new();
	if (!ptr_udev) {
		fprintf(stderr, "Failed to create a new udev object\n");
		wl_display_destroy(display);
		return -EFAULT;
	}

	libinput *li = libinput_udev_create_context(&interface, nullptr, ptr_udev);
	if (!li) {
		fprintf(stderr, "Failed to create an udev context\n");
		udev_unref(ptr_udev);
		wl_display_destroy(display);
		return -EFAULT;
	}

	// NOTE:
	// Do you want to know what is the "SEAT" here?
	// Reference: https://wayland.freedesktop.org/libinput/doc/latest/seats.html
	libinput_udev_assign_seat(li, DEFAULT_SEAT);

	int fd = libinput_get_fd(li);
	wl_event_source *event_source = wl_event_loop_add_fd(event_loop, fd, WL_EVENT_READABLE, input_event_callback, li);
	if (!event_source) {
		fprintf(stderr, "Failed to add the input fd to the wl_event_loop\n");
		libinput_unref(li);
		udev_unref(ptr_udev);
		wl_display_destroy(display);
		return -EFAULT;
	}

	printf("Running the wayland display on %s\n", socket);
	wl_display_run(display);

	libinput_unref(li);
	udev_unref(ptr_udev);
	wl_display_destroy(display);
	return 0;
}

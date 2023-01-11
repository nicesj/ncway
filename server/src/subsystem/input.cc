#include "input.h"
#include <string>
#include <memory>

#include <cstdio>
#include <cstdlib>

#include <wayland-server.h>
#include <libinput.h>
#include <libudev.h>
#include <unistd.h>
#include <fcntl.h>

namespace ncway {

void Input::closeRestricted(int fd, void *data)
{
	printf("Close FD: %d\n", fd);
	if (close(fd) < 0) {
		fprintf(stderr, "Close: %d, %m\n", fd);
	}
}

int Input::openRestricted(const char *path, int flags, void *data)
{
	printf("Open: %s (0x%X)\n", path, flags);
	int fd = open(path, flags);
	return fd < 0 ? -errno : fd;
}

int Input::getFD(void)
{
	return libinput_get_fd(li);
}

int Input::handler(int fd, uint32_t mask)
{
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
	switch (libinput_event_get_type(evt)) {
	case LIBINPUT_EVENT_KEYBOARD_KEY:
		printf("Keyboard event catched!\n");
		libinput_event_keyboard *keyEvt;
		keyEvt = libinput_event_get_keyboard_event(evt);
		if (!keyEvt) {
			fprintf(stderr, "Invalid keyboard event!\n");
			break;
		}
		uint32_t keyCode;
		keyCode	= libinput_event_keyboard_get_key(keyEvt);
		printf("KeyCode: %u\n", keyCode);
		if (keyCode == static_cast<uint32_t>(16)) {
			wl_display_terminate(display.get());
		}
		break;
	default:
		printf("Input event is not handled yet\n");
		break;
	}

	libinput_event_destroy(evt);
	return 1;
}

std::shared_ptr<Input> Input::create(std::shared_ptr<wl_display> display, std::string seat)
{
	// NOTE:
	// interface must be declared in the Data section (or heap)
	// the libinput just copy the pointer of interface structure
	static libinput_interface interface = {
		.open_restricted = Input::openRestricted,
		.close_restricted = Input::closeRestricted,
	};

	std::shared_ptr<Input> instance = std::shared_ptr<Input>(new Input());
	if (!instance) {
		return nullptr;
	}

	instance->display = display;

	instance->ud = udev_new();
	if (!instance->ud) {
		fprintf(stderr, "Failed to create a new udev object\n");
		return nullptr;
	}

	instance->li = libinput_udev_create_context(&interface, nullptr, instance->ud);
	if (!instance->li) {
		fprintf(stderr, "Failed to create an udev context\n");
		udev_unref(instance->ud);
		return nullptr;
	}

	// NOTE:
	// Do you want to know what is the "SEAT" here?
	// Reference: https://wayland.freedesktop.org/libinput/doc/latest/seats.html
	if (libinput_udev_assign_seat(instance->li, seat.c_str()) < 0) {
		return nullptr;
	}

	return instance;
}

Input::Input(void)
: ud(nullptr)
, li(nullptr)
{
}

Input::~Input(void)
{
	libinput_unref(li);
	udev_unref(ud);
	printf("Input is destructed\n");
}

std::string Input::name(void)
{
	return "input";
}

std::string Input::version(void)
{
	return "0.1";
}

bool Input::isCompatible(std::string ver)
{
	return true;
}

std::shared_ptr<wl_display> Input::getDisplay(void)
{
	return display;
}

}

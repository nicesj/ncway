#include "input.h"
#include <cstdio>
#include <string>
#include <memory>

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

	libinput_event_destroy(evt);
	return 1;
}

std::shared_ptr<Input> Input::Create(std::string seat)
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

}

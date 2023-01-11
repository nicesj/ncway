#include <memory>

#include <cstdio>
#include <cerrno>
#include <cstring>

#include <wayland-client.h>
#include <sys/epoll.h>
#include <unistd.h>

#define MAX_EVENTS 10
#define TIMEOUT_IN_MS -1

static void registry_handle_global(void *data, wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{
	printf("interface: '%s', version: %d, name: %d\n", interface, version, name);
}

static void registry_handle_global_remove(void *data, wl_registry *registry, uint32_t name)
{
	printf("registry_handle_global_remove\n");
}

static const wl_registry_listener registry_listener = {
	.global = registry_handle_global,
	.global_remove = registry_handle_global_remove,
};

int main(int argc, char *argv[])
{
	std::shared_ptr<wl_display> display = std::shared_ptr<wl_display>(wl_display_connect(nullptr),
		[](wl_display *ptr) {
			printf("Display object is disconnected %p\n", ptr);
			if (ptr) {
	       			wl_display_disconnect(ptr);
			}
		}
	);
	if (!display) {
		fprintf(stderr, "Unable to connect to a display\n");
		return -EFAULT;
	}

	wl_registry *registry = wl_display_get_registry(display.get());
	wl_registry_add_listener(registry, &registry_listener, nullptr);
	wl_display_roundtrip(display.get());

	epoll_event events[MAX_EVENTS + 1];

	int handle = epoll_create1(EPOLL_CLOEXEC);
	if (handle > 0) {
		epoll_event ev;
		ev.events = EPOLLIN | EPOLLPRI | EPOLLERR | EPOLLOUT;
		ev.data.ptr = reinterpret_cast<void *>(0x01);

		int wl_fd = wl_display_get_fd(display.get());
		if (wl_fd < 0) {
			if (close(handle) < 0) {
				fprintf(stderr, "close: %s\n", strerror(errno));
			}
			return 0;
		}

		if (epoll_ctl(handle, EPOLL_CTL_ADD, wl_fd, &ev) < 0) {
			fprintf(stderr, "epoll_ctl: %s\n", strerror(errno));
			if (close(handle) < 0) {
				fprintf(stderr, "close: %s\n", strerror(errno));
			}
			return 0;
		}

		do {
			int ret = epoll_pwait(handle, events, MAX_EVENTS, TIMEOUT_IN_MS, nullptr);
			if (ret < 0) {
				if (errno == EINTR) {
					printf("Interrupted\n");
					continue;
				}

				fprintf(stderr, "Something goes wrong: %s\n", strerror(errno));
				break;
			} else if (ret == 0) {
				printf("Timed out\n");
				continue;
			}

			for (int i = 0; i < ret; ++i) {
				if (events[i].data.ptr == nullptr) {
					printf("Interrupted\n");
					continue;
				}

				if (events[i].events & (EPOLLIN | EPOLLPRI)) {
					// Read
					if (wl_display_dispatch(display.get()) < 0) {
						fprintf(stderr, "dispatch: %s\n", strerror(errno));
					}
				}

				if (events[i].events & EPOLLOUT) {
					// Write
					// wl_display_flush(display.get())
				}

				if (events[i].events & EPOLLERR) {
					// Error
				}
			}
		} while (true);
	}

	printf("Connection established\n");
	return 0;
}

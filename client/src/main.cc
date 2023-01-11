#include <memory>

#include <cstdio>
#include <cerrno>
#include <wayland-client.h>

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

	printf("Connection established\n");
	return 0;
}

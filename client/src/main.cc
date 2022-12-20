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
	wl_display *display = wl_display_connect(nullptr);
	if (!display) {
		fprintf(stderr, "Unable to connect to a display");
		return -EFAULT;
	}

	wl_registry *registry = wl_display_get_registry(display);
	wl_registry_add_listener(registry, &registry_listener, nullptr);
	wl_display_roundtrip(display);

	printf("Connection established\n");

	wl_display_disconnect(display);
	return 0;
}

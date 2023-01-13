#include <wayland-server.h>

#include "subsystem.h"
#include "subsystem/input.h"
#include "subsystem/drm.h"
#include "subsystem/gbm.h"
#include "subsystem/egl.h"
#include "event.h"

#include <memory>
#include <vector>
#include <algorithm>

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <drm_fourcc.h>

#define DEFAULT_SEAT "seat0"

struct ClientOutput {
};

std::vector<ClientOutput*> gVectorClientOutput;

static void wl_output_handle_resource_destroy(wl_resource *resource)
{
	ClientOutput *clientOutput = reinterpret_cast<ClientOutput *>(wl_resource_get_user_data(resource));

	gVectorClientOutput.erase(std::remove(gVectorClientOutput.begin(), gVectorClientOutput.end(), clientOutput), gVectorClientOutput.end());
	delete clientOutput;
}

static void wl_output_handle_release(wl_client *client, wl_resource *resource)
{
	wl_resource_destroy(resource);
}

static const struct wl_output_interface wl_output_implementation = {
	.release = wl_output_handle_release,
};

static void wl_output_handle_bind(wl_client *client, void *data, uint32_t version, uint32_t id)
{
	ClientOutput *clientOutput = new ClientOutput();

	printf("output handle bound! %u %u\n", version, id);
	wl_resource *resource = wl_resource_create(client, &wl_output_interface, wl_output_interface.version, id);
	wl_resource_set_implementation(resource, &wl_output_implementation, clientOutput, wl_output_handle_resource_destroy);

	gVectorClientOutput.push_back(clientOutput);
}

int main(int argc, char *argv[])
{
	std::shared_ptr<wl_display> display = std::shared_ptr<wl_display>(wl_display_create(), [](wl_display *ptr) {
		wl_display_destroy(ptr);
		printf("Display object is destructed\n");
	});

	if (!display) {
		fprintf(stderr, "Unable to create the wayland display\n");
		return -EFAULT;
	}

	const char *socket = wl_display_add_socket_auto(display.get());
	if (!socket) {
		fprintf(stderr, "Unable to add socket to the wayland display\n");
		return -EFAULT;
	}

	printf("Allocated socket: %s\n", socket);

	wl_event_loop *event_loop = wl_display_get_event_loop(display.get());
	if (!event_loop) {
		fprintf(stderr, "Failed to get the event_loop\n");
		return -EFAULT;
	}

	std::shared_ptr<ncway::Input> input = ncway::Input::create(display, DEFAULT_SEAT);
	if (!input) {
		fprintf(stderr, "Failed to create the Input");
		return -EFAULT;
	}

	// NOTE:
	// The inputInfo object will be kept in the stack until terminating the process
	ncway::Event::EventInfo inputInfo = {
		.eventPtr = std::static_pointer_cast<ncway::Event>(input),
	};
	wl_event_source *event_source = wl_event_loop_add_fd(event_loop, input->getFD(), WL_EVENT_READABLE, ncway::Event::eventHandler, &inputInfo);
	if (!event_source) {
		fprintf(stderr, "Failed to add the input fd to the wl_event_loop\n");
		return -EFAULT;
	}

	std::shared_ptr<ncway::DRM> drm = ncway::DRM::create(display, "/dev/dri/card0", true, false);
	if (!drm) {
		fprintf(stderr, "Failed to create the DRM");
		return -EFAULT;
	}

	// NOTE:
	// The drmInfo object will be kept in the stack until terminating the process
	ncway::Event::EventInfo drmInfo = {
		.eventPtr = std::static_pointer_cast<ncway::Event>(drm),
	};

	wl_event_source *drm_event_source = wl_event_loop_add_fd(event_loop, drm->getFD(), WL_EVENT_READABLE, ncway::Event::eventHandler, &drmInfo);
	if (!drm_event_source) {
		fprintf(stderr, "Failed to add the DRM fd to the wl_event_loop\n");
		return -EFAULT;
	}

	printf("%zu connectors are found\n", drm->count());
	drm->select(0);
	printf("%d modes found\n", drm->getModeCount());
	drm->setMode(0);

	std::shared_ptr<ncway::GBM> gbm = ncway::GBM::create(drm, DRM_FORMAT_XRGB8888, DRM_FORMAT_MOD_LINEAR);
	std::shared_ptr<ncway::EGL> egl = ncway::EGL::create(gbm, 1);

	egl->startRender(nullptr);

	wl_global_create(display.get(), &wl_output_interface, 1, nullptr, wl_output_handle_bind);

	printf("Running the wayland display on %s\n", socket);
	wl_display_run(display.get());
	return 0;
}

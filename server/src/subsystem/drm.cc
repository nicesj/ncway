#include "drm.h"

#include <cstdio>
#include <cerrno>
#include <string>

#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <xf86drmMode.h>

namespace ncway {
int DRM::getFD(void)
{
	return -1;
}

int DRM::handler(int fd, uint32_t mask)
{
	return 1;
}

DRM *DRM::Create(std::string nodePath)
{
	DRM *instance = new DRM();
	if (!instance) {
		return nullptr;
	}

	instance->fd = open(nodePath.c_str(), O_RDWR);
	if (instance->fd < 0) {
		fprintf(stderr, "open(%s) returns %s\n", nodePath.c_str(), strerror(errno));
		delete instance;
		return nullptr;
	}

	instance->resources = drmModeGetResources(instance->fd);
	if (instance->resources == nullptr) {
		fprintf(stderr, "drmModeGetResources failed: %s\n", strerror(errno));
		delete instance;
		return nullptr;
	}

	for (int i = 0; i < instance->resources->count_connectors; ++i) {
		instance->connector = drmModeGetConnector(instance->fd, instance->resources->connectors[i]);
		if (instance->connector == nullptr) {
			fprintf(stderr, "Get a nullptr connector [%d]\n", i);
			continue;
		}

		if (instance->connector->connection == DRM_MODE_CONNECTED && instance->connector->count_modes > 0) {
			printf("Connector %d found\n", instance->connector->connector_id);
			break;
		}

		drmModeFreeConnector(instance->connector);
		instance->connector = nullptr;
	}

	if (!instance->connector) {
		fprintf(stderr, "No active connector found\n");
		delete instance;
		return nullptr;
	}

	drmModeModeInfo mode = instance->connector->modes[0];
	fprintf(stderr, "(%dx%d)\n", mode.hdisplay, mode.vdisplay);

	instance->encoder = nullptr;
	for (int i = 0; i < instance->resources->count_encoders; ++i) {
		instance->encoder = drmModeGetEncoder(instance->fd, instance->resources->encoders[i]);
		if (instance->encoder == nullptr) {
			fprintf(stderr, "Get a nullptr encoder [%d]\n", i);
			continue;
		}

		if (instance->encoder->encoder_id == instance->connector->encoder_id) {
			printf("Encoder %d found\n", instance->encoder->encoder_id);
			break;
		}

		drmModeFreeEncoder(instance->encoder);
		instance->encoder = nullptr;
	}

	if (!instance->encoder) {
		fprintf(stderr, "No matching encoder with connector\n");
		delete instance;
		return nullptr;
	}

	return instance;
}

DRM::DRM(void)
: fd(-1)
, resources(nullptr)
, connector(nullptr)
, encoder(nullptr)
{
}

DRM::~DRM(void)
{
	if (encoder) {
		drmModeFreeEncoder(encoder);
	}

	if (connector) {
		drmModeFreeConnector(connector);
	}

	if (resources) {
		drmModeFreeResources(resources);
	}

	if (fd >= 0 && close(fd) < 0) {
		fprintf(stderr, "close: %s\n", strerror(errno));
	}
}

std::string DRM::name(void)
{
	return "DRM";
}

}

#include "drm.h"

#include <cstdio>
#include <cerrno>
#include <string>
#include <vector>

#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <xf86drmMode.h>

namespace ncway {
DRM::DRM(void)
: fd(-1)
, resources(nullptr)
, encoder(nullptr)
, connector(nullptr)
{
}

DRM::~DRM(void)
{
	if (encoder) {
		drmModeFreeEncoder(encoder);
	}

	std::vector<drmModeConnector*>::iterator it;
	for (it = connectors.begin(); it != connectors.end(); ++it) {
		drmModeFreeConnector(*it);
	}
	connectors.clear();

	if (resources) {
		drmModeFreeResources(resources);
	}

	if (fd >= 0 && close(fd) < 0) {
		fprintf(stderr, "close: %s\n", strerror(errno));
	}
}

int DRM::getFD(void)
{
	return -1;
}

int DRM::handler(int fd, uint32_t mask)
{
	return 1;
}

int DRM::getModeCount(void) const
{
	if (!connector) {
		fprintf(stderr, "Connector must be selected first\n");
		return -EINVAL;
	}

	return connector->count_modes;
}

const drmModeModeInfo &DRM::getMode(void) const
{
	return modeInfo;
}

int DRM::selectMode(int idx)
{
	if (!connector) {
		fprintf(stderr, "Connector must be selected first\n");
		return -EINVAL;
	}

	if (idx < 0 || idx >= connector->count_modes) {
		fprintf(stderr, "Invalid modes(%d) must be in between %d and %d\n", idx, 0, connector->count_modes);
		return -EINVAL;
	}

	modeInfo = connector->modes[idx];
	fprintf(stderr, "(%dx%d)\n", modeInfo.hdisplay, modeInfo.vdisplay);

	return 0;
}

uint32_t DRM::findCRTC(drmModeConnector *connector)
{
	if (!connector) {
		fprintf(stderr, "connector is not declared\n");
		return 0;
	}

	for (int i = 0; i< connector->count_encoders; ++i) {
		const uint32_t encoder_id = connector->encoders[i];
		drmModeEncoder *_encoder = drmModeGetEncoder(fd, encoder_id);

		if (!_encoder) {
			continue;
		}

		const uint32_t crtc_id = findCRTC(_encoder);
		if (crtc_id == 0) {
			continue;
		}

		if (encoder != nullptr) {
			drmModeFreeEncoder(encoder);
		}

		encoder = _encoder;
		return crtc_id;
	}

	return 0;
}

uint32_t DRM::findCRTC(drmModeEncoder *encoder)
{
	if (!encoder) {
		fprintf(stderr, "encoder is not declared\n");
		return 0;
	}

	for (int i = 0; i < resources->count_crtcs; ++i) {
		const uint32_t crtc_mask = 1 << i;
		const uint32_t crtc_id = resources->crtcs[i];
		if (encoder->possible_crtcs & crtc_mask) {
			return crtc_id;
		}
	}

	return 0;
}

int DRM::selectConnector(int idx)
{
	if (idx < 0 || idx >= connectors.size()) {
		fprintf(stderr, "Invalid connector index (%d < %zu)\n", idx, connectors.size());
		return -EINVAL;
	}

	connector = connectors[idx];
	connector_id = connector->connector_id;

	if (encoder) {
		drmModeFreeEncoder(encoder);
		encoder = nullptr;
	}

	for (int i = 0; i < resources->count_encoders; ++i) {
		encoder = drmModeGetEncoder(fd, resources->encoders[i]);
		if (encoder == nullptr) {
			fprintf(stderr, "Get a nullptr encoder [%d]\n", i);
			continue;
		}

		if (encoder->encoder_id == connector->encoder_id) {
			printf("Encoder %d found\n", encoder->encoder_id);
			break;
		}

		drmModeFreeEncoder(encoder);
		encoder = nullptr;
	}

	if (!encoder) {
		fprintf(stderr, "Find Connector & Encoder in a different way\n");
		crtc_id = findCRTC(connector);
	} else {
		crtc_id = encoder->crtc_id;
	}

	if (crtc_id == 0) {
		fprintf(stderr, "No matching encoder with connector\n");
		connector = nullptr;
		if (encoder) {
			drmModeFreeEncoder(encoder);
			encoder = nullptr;
		}
		return -ENOENT;
	}

	return 0;
}

size_t DRM::getConnectorCount(void) const
{
	return connectors.size();
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
		drmModeConnector *connector = drmModeGetConnector(instance->fd, instance->resources->connectors[i]);
		if (connector == nullptr) {
			fprintf(stderr, "Get a nullptr connector [%d]\n", i);
			continue;
		}

		if (connector->connection == DRM_MODE_CONNECTED && connector->count_modes > 0) {
			instance->connectors.push_back(connector);
			printf("Connector %d found\n", connector->connector_id);
		} else {
			drmModeFreeConnector(connector);
		}
	}

	if (instance->connectors.size() == 0) {
		fprintf(stderr, "No active connector found\n");
		delete instance;
		return nullptr;
	}

	return instance;
}

std::string DRM::name(void)
{
	return "DRM";
}

drmModeEncoder *DRM::getEncoder(void)
{
	return encoder;
}

drmModeConnector *DRM::getConnector(void)
{
	return connector;
}

}

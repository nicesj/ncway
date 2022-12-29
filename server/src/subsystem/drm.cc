#include "drm.h"

#include <cstdio>
#include <cerrno>
#include <string>
#include <vector>

#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <xf86drm.h>
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
	return fd;
}

void DRM::page_flip_handler(int fd, unsigned int frame, unsigned int sec, unsigned int usec, void *data)
{
	printf("Frame: %u (%u sec %u usec)\n", frame, sec, usec);
}

int DRM::handler(int fd, uint32_t mask)
{
	drmEventContext evctx = {
		.version = 2,
		.page_flip_handler = DRM::page_flip_handler,
	};

	drmHandleEvent(fd, &evctx);
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

	if (idx < 0) {
		printf("Select mode automatically\n");
		// NOTE:
		// Find a preferred mode
		// or the highest resolution
		int area = 0;
		for (int i = 0; i < connector->count_modes; ++i) {
			drmModeModeInfo *current_mode = &connector->modes[i];
			if (current_mode->type & DRM_MODE_TYPE_PREFERRED) {
				printf("Selected mode is %d\n", i);
				modeInfo = *current_mode;
				idx = i;
				break;
			}

			int current_area = current_mode->hdisplay * current_mode->vdisplay;
			if (current_area > area) {
				modeInfo = *current_mode;
				area = current_area;
				idx = i;
				printf("Selected mode is %d (%dx%d)\n", i, current_mode->hdisplay, current_mode->vdisplay);
			}
		}
	} else if (idx >= connector->count_modes) {
		fprintf(stderr, "Invalid modes(%d) must be in between %d and %d\n", idx, 0, connector->count_modes);
		return -EINVAL;
	}

	modeInfo = connector->modes[idx];
	fprintf(stderr, "%s (%dx%d)\n", modeInfo.name, modeInfo.hdisplay, modeInfo.vdisplay);

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

DRM *DRM::Create(std::string nodePath, bool isMaster, bool isAtomic)
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

	if (isMaster) {
		int ret = drmSetMaster(instance->fd);
		if (ret < 0) {
			fprintf(stderr, "Failed to be a master\n");
			delete instance;
			return nullptr;
		}
	}

	if (isAtomic) {
		int ret = drmSetClientCap(instance->fd, DRM_CLIENT_CAP_ATOMIC, 1);
		if (ret < 0) {
			fprintf(stderr, "Failed to set atomic flag\n");
			delete instance;
			return nullptr;
		}
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

int DRM::addFramebuffer(Renderer::bufferDescription *desc)
{
	uint32_t fb_id;
	int ret = drmModeAddFB2WithModifiers(fd,
		       desc->width, desc->height, desc->format,
		       desc->handles, desc->strides, desc->offsets, desc->modifiers,
		       &fb_id, desc->flags);
	if (ret) {
		fprintf(stderr, "Failed to create FB: %s\n", strerror(errno));
		ret = drmModeAddFB2(fd, desc->width, desc->height, desc->format, desc->handles, desc->strides, desc->offsets, &fb_id, 0);
		if (ret) {
			fprintf(stderr, "Failed to create FB: %s\n", strerror(errno));
		}
	}

	if (ret) {
		return -EFAULT;
	}

	framebufferDescription *fbDesc = new framebufferDescription();
	if (!fbDesc) {
		fprintf(stderr, "Failed to allocate memory\n");
		drmModeRmFB(fd, fb_id);
		return -ENOMEM;
	}
	fbDesc->fb_id = fb_id;
	fbDesc->fd = fd;
	desc->user_data = fbDesc;
	desc->user_data_destructor = [](Renderer::bufferDescription *desc) {
		framebufferDescription *fbDesc = static_cast<framebufferDescription *>(desc->user_data);
		drmModeRmFB(fbDesc->fd, fbDesc->fb_id);
		delete fbDesc;
		desc->user_data = nullptr;
		desc->user_data_destructor = nullptr;
		return;
	};

	return 0;
}

}

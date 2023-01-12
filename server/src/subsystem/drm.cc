#include "drm.h"

#include <cstdio>
#include <cerrno>
#include <string>
#include <vector>
#include <memory>

#include <cstring>

#include <wayland-server.h>
#include <unistd.h>
#include <fcntl.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

namespace ncway {

DRM::DRMDevice::DRMDevice(void)
: connector(nullptr)
, crtc_id(0)
, modeInfo(nullptr)
{
}

DRM::DRMDevice::~DRMDevice(void)
{
	if (!connector) {
		return;
	}

	printf("Connector: %d is released\n", connector->connector_id);
	drmModeFreeConnector(connector);
	connector = nullptr;
}

DRM::DRM(void)
: fd(-1)
{
}

DRM::~DRM(void)
{
	DRMDevices.clear();

	if (fd >= 0 && close(fd) < 0) {
		fprintf(stderr, "close: %s\n", strerror(errno));
	}

	printf("DRM is destructed\n");
}

int DRM::getFD(void)
{
	return fd;
}

void DRM::page_flip_handler(int fd, unsigned int frame, unsigned int sec, unsigned int usec, void *data)
{
	static unsigned int lastSec;
	static int frameCount = 0;

	if (lastSec != sec) {
		printf("%u fps (%u sec)\n", frameCount, lastSec);
		frameCount = 0;
		lastSec = sec;
	}

	++frameCount;

	if (data) {
		DRM::EventData *evtData = static_cast<DRM::EventData *>(data);
		if (evtData->renderer) {
			evtData->renderer();
		}
	}
}

void DRM::vblank_handler(int fd, unsigned int sequence, unsigned int sec, unsigned int usec, void *data)
{
	printf("sequence: %u (%u sec %u usec)\n", sequence, sec, usec);
}

int DRM::handler(int fd, uint32_t mask)
{
	drmEventContext evctx = {
		.version = 2,
		.vblank_handler = DRM::vblank_handler,
		.page_flip_handler = DRM::page_flip_handler,
	};

	if (drmHandleEvent(fd, &evctx) < 0) {
		fprintf(stderr, "Unable to handle the drm event\n");
	}

	return 1;
}

int DRM::getModeCount(void) const
{
	return selectedDevice->connector->count_modes;
}

const drmModeModeInfo &DRM::getMode(void) const
{
	return *(selectedDevice->modeInfo);
}

int DRM::setMode(int idx)
{
	if (idx >= selectedDevice->connector->count_modes) {
		fprintf(stderr, "Invalid modes(%d) must be in between %d and %d\n", idx, 0, selectedDevice->connector->count_modes);
		return -EINVAL;
	}

	if (idx < 0) {
		printf("Select mode automatically\n");
		// NOTE:
		// Find a preferred mode
		// or the highest resolution
		int area = 0;
		for (int i = 0; i < selectedDevice->connector->count_modes; ++i) {
			drmModeModeInfo *current_mode = &selectedDevice->connector->modes[i];
			if (current_mode->type & DRM_MODE_TYPE_PREFERRED) {
				printf("Selected mode is %d\n", i);
				idx = i;
				break;
			}

			int current_area = current_mode->hdisplay * current_mode->vdisplay;
			if (current_area > area) {
				area = current_area;
				idx = i;
				printf("Selected mode is %d (%dx%d)\n", i, current_mode->hdisplay, current_mode->vdisplay);
			}
		}
		if (idx < 0) {
			fprintf(stderr, "Unable to find the proper mode\n");
			return -EINVAL;
		}
	}

	selectedDevice->modeInfo = &selectedDevice->connector->modes[idx];
	printf("%s (%dx%d)\n",
		selectedDevice->modeInfo->name,
	       	selectedDevice->modeInfo->hdisplay,
	       	selectedDevice->modeInfo->vdisplay
	);

	return 0;
}

int DRM::select(int idx)
{
	if (idx < 0 || idx > DRMDevices.size()) {
		return -EINVAL;
	}

	selectedDevice = DRMDevices[idx];
	return 0;
}

size_t DRM::count(void) const
{
	return DRMDevices.size();
}

std::shared_ptr<DRM> DRM::create(std::shared_ptr<wl_display> display, std::string nodePath, bool isMaster, bool isAtomic)
{
	std::shared_ptr<DRM> instance = std::shared_ptr<DRM>(new DRM());
	if (!instance) {
		return nullptr;
	}

	instance->display = display;

	instance->fd = open(nodePath.c_str(), O_RDWR);
	if (instance->fd < 0) {
		fprintf(stderr, "open(%s) returns %s\n", nodePath.c_str(), strerror(errno));
		return nullptr;
	}

	if (isMaster) {
		int ret = drmSetMaster(instance->fd);
		if (ret < 0) {
			fprintf(stderr, "Failed to be a master\n");
			return nullptr;
		}
	}

	if (isAtomic) {
		int ret = drmSetClientCap(instance->fd, DRM_CLIENT_CAP_ATOMIC, 1);
		if (ret < 0) {
			fprintf(stderr, "Failed to set atomic flag\n");
			return nullptr;
		}
	}

	// We have "resources"
	drmModeRes *resources = drmModeGetResources(instance->fd);
	if (resources == nullptr) {
		fprintf(stderr, "drmModeGetResources failed: %s\n", strerror(errno));
		return nullptr;
	}

	// Find a fully connected path of
	// "connector", "encoder" and "crtc"
	// in order to manipulate the GBM
	// Now, lets walk through the connectors
	for (int i = 0; i < resources->count_connectors; ++i) {
		drmModeConnector *connector = drmModeGetConnector(instance->fd, resources->connectors[i]);
		if (connector == nullptr) {
			fprintf(stderr, "Get a nullptr connector [%d]\n", i);
			continue;
		}

		if (connector->connection != DRM_MODE_CONNECTED || connector->count_modes == 0) {
			drmModeFreeConnector(connector);
			continue;
		}

		printf("Connector %d found\n", connector->connector_id);

		drmModeEncoder *encoder = drmModeGetEncoder(instance->fd, connector->encoder_id);
		if (encoder == nullptr) {
			drmModeFreeConnector(connector);
			continue;
		}

		uint32_t crtc_id = encoder->crtc_id;
		int j = 0;
		while (j < resources->count_crtcs && (crtc_id == 0 || instance->isBoundedCRTC(crtc_id))) {
			const uint32_t crtc_mask = 1 << j;
			if (!(encoder->possible_crtcs & crtc_mask)) {
				++j;
				continue;
			}

			if (resources->crtcs[j] == encoder->crtc_id) {
				++j;
				continue;
			}

			crtc_id = resources->crtcs[j];
			++j;
		}

		drmModeFreeEncoder(encoder);

		std::shared_ptr<DRMDevice> device = std::make_shared<DRMDevice>();
		device->connector = connector;
		device->crtc_id = crtc_id;
		instance->DRMDevices.push_back(std::move(device));
	}

	drmModeFreeResources(resources);
	return instance;
}

bool DRM::isBoundedCRTC(uint32_t crtc_id)
{
	std::vector<std::shared_ptr<DRMDevice>>::const_iterator it;
	for (it = DRMDevices.begin(); it != DRMDevices.end(); ++it) {
		if ((*it)->crtc_id == crtc_id) {
			// Alredy bounded crtc_id
			return true;
		}
	}

	return false;
}

std::shared_ptr<wl_display> DRM::getDisplay(void)
{
	return display;
}

std::string DRM::name(void)
{
	return "DRM";
}

std::string DRM::version(void)
{
	return "0.1";
}

bool DRM::isCompatible(std::string ver)
{
	return true;
}

uint32_t DRM::addFramebuffer(BufferDescriptor *desc)
{
	int ret = drmModeAddFB2WithModifiers(fd,
		       desc->width, desc->height, desc->format,
		       desc->handles, desc->strides, desc->offsets, desc->modifiers,
		       &desc->fb_id, desc->flags);
	if (ret) {
		fprintf(stderr, "Failed to create FB: %s (%d) Fallback to drmModeAddFB2()\n", strerror(errno), fd);
		ret = drmModeAddFB2(fd, desc->width, desc->height, desc->format, desc->handles, desc->strides, desc->offsets, &desc->fb_id, 0);
		if (ret) {
			fprintf(stderr, "Failed to create FB: %s\n", strerror(errno));
			return 0;
		}
	}

	desc->customDestructor = [&](BufferDescriptor *desc) -> void {
		printf("FB destroyed: %u\n", desc->fb_id);
		drmModeRmFB(fd, desc->fb_id);
	};

	return desc->fb_id;
}

uint32_t DRM::getFBID(BufferDescriptor *desc)
{
	return desc->fb_id;
}

int DRM::setCrtcMode(int fb_id, int x, int y)
{
	return drmModeSetCrtc(
		fd,
	       	selectedDevice->crtc_id,
	       	fb_id, x, y,
	       	&(selectedDevice->connector->connector_id), 1,
	       	selectedDevice->modeInfo
	);
}

int DRM::pageFlip(int fb_id, void *data)
{
	return drmModePageFlip(fd, selectedDevice->crtc_id, fb_id, DRM_MODE_PAGE_FLIP_EVENT, data);
}

}

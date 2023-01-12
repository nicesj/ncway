#pragma once
#include "../subsystem.h"
#include "../buffer_descriptor.h"

#include <vector>
#include <memory>
#include <functional>

#include <wayland-server.h>
#include <xf86drmMode.h>

#include <unistd.h>

namespace ncway {
class DRM : public Subsystem {
private:
	DRM(void);

public:
	struct EventData {
		std::function<void(void)> renderer;
		void *bo;
	};

public:
	static std::shared_ptr<DRM> create(std::shared_ptr<wl_display> display, std::string nodePath, bool isMaster, bool isAtomic);
	virtual ~DRM(void);

public:
	std::string name(void) override;
	std::string version(void) override;
	bool isCompatible(std::string ver) override;

public:
	int handler(int fd, uint32_t mask) override;
	int getFD(void) override;

	size_t count(void) const;
	int select(int idx);
	int setMode(int idx);
	int getModeCount(void) const;
	const drmModeModeInfo &getMode(void) const;

	int addFramebuffer(BufferDescriptor *desc);
	int setCrtcMode(int fb_id, int x, int y);
	int pageFlip(int fb_id, void *data);

	int getFBID(BufferDescriptor *desc);

public:
	std::shared_ptr<wl_display> getDisplay(void);

private:
	static void page_flip_handler(int fd, unsigned int frame, unsigned int sec, unsigned int usec, void *data);
	static void vblank_handler(int fd, unsigned int sequence, unsigned int sec, unsigned int usec, void *data);

private:
	int fd;

	struct DRMDevice {
		DRMDevice(void);
		virtual ~DRMDevice(void);

		drmModeConnector *connector;
		uint32_t crtc_id;
		drmModeModeInfo *modeInfo;
	};

	std::vector<std::shared_ptr<DRMDevice>> DRMDevices;
	std::shared_ptr<DRMDevice> selectedDevice;

	bool isBoundedCRTC(uint32_t crtc_id);

private:
	std::shared_ptr<wl_display> display;
};
}

#pragma once
#include "../subsystem.h"
#include "../buffer_descriptor.h"

#include <vector>
#include <memory>

#include <xf86drmMode.h>

#include <unistd.h>

namespace ncway {
class DRM : public Subsystem {
private:
	DRM(void);
public:
	static std::shared_ptr<DRM> Create(std::string nodePath, bool isMaster, bool isAtomic);
	virtual ~DRM(void);

public:
	std::string name(void) override;
	std::string version(void) override;
	bool isCompatible(std::string ver) override;

public:
	int handler(int fd, uint32_t mask) override;
	int getFD(void) override;

	size_t getConnectorCount(void) const;
	int selectConnector(int idx);
	int selectMode(int idx);
	int getModeCount(void) const;
	const drmModeModeInfo &getMode(void) const;

	drmModeEncoder *getEncoder(void);
	drmModeConnector *getConnector(void);

	int addFramebuffer(BufferDescriptor *desc);
	int setCrtcMode(int fb_id, int x, int y);
	int pageFlip(int fb_id, void *data);

	int getFBID(BufferDescriptor *desc);

private:
	uint32_t findCRTC(drmModeEncoder *encoder);
	uint32_t findCRTC(drmModeConnector *connector);
	static void page_flip_handler(int fd, unsigned int frame, unsigned int sec, unsigned int usec, void *data);

private:
	int fd;
	drmModeRes *resources;
	drmModeEncoder *encoder;
	drmModeConnector *connector;
	drmModeModeInfo modeInfo;
	uint32_t crtc_id;
	uint32_t connector_id;

	std::vector<drmModeConnector *> connectors;
};
}

#pragma once
#include "../subsystem.h"

#include <vector>

#include <xf86drmMode.h>

#include <unistd.h>

namespace ncway {
class DRM : public Subsystem {
private:
	DRM(void);
public:
	static DRM *Create(std::string nodePath, bool isMaster, bool isAtomic);
	virtual ~DRM(void);

public:
	std::string name(void);
	int handler(int fd, uint32_t mask);
	int getFD(void);

	size_t getConnectorCount(void) const;
	int selectConnector(int idx);
	int selectMode(int idx);
	int getModeCount(void) const;
	const drmModeModeInfo &getMode(void) const;

	drmModeEncoder *getEncoder(void);
	drmModeConnector *getConnector(void);

	static void page_flip_handler(int fd, unsigned int frame, unsigned int sec, unsigned int usec, void *data);

private:
	uint32_t findCRTC(drmModeEncoder *encoder);
	uint32_t findCRTC(drmModeConnector *connector);

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

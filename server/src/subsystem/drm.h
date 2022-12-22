#pragma once
#include "../subsystem.h"

#include <vector>

#include <xf86drmMode.h>

namespace ncway {
class DRM : public Subsystem {
private:
	DRM(void);
public:
	static DRM *Create(std::string nodePath);
	virtual ~DRM(void);

public:
	std::string name(void);
	int handler(int fd, uint32_t mask);
	int getFD(void);

	size_t getConnectorCount(void);
	int selectConnector(int idx);
	int selectMode(int idx);
	int getModeCount(void);

private:
	int fd;
	drmModeRes *resources;
	drmModeEncoder *encoder;
	drmModeConnector *connector;

	std::vector<drmModeConnector *> connectors;
};
}

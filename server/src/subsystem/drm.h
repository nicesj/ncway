#pragma once
#include "../subsystem.h"
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

private:
	int fd;
	drmModeRes *resources;
	drmModeConnector *connector;
	drmModeEncoder *encoder;
};
}

#pragma once

#include "./renderer.h"
#include "./drm.h"

#include <unistd.h>
#include <gbm.h>

namespace ncway {

class GBM : public Renderer {
private:
	GBM(void);

public:
	virtual ~GBM(void);

public:
	int getFD(void);
	int handler(int fd, uint32_t mask);

public:
	static GBM *Create(DRM *drm, uint32_t format, uint64_t modifier);
	gbm_device *getDevice(void) const;
	gbm_surface *getSurface(void) const;
	int getWidth(void) const;
	int getHeight(void) const;
	uint32_t getFormat(void) const;
	std::string name(void);
	Renderer::bufferDescription *getBufferDescription(gbm_bo *bo, bool applyModifiers);

private:
	gbm_device *dev;
	gbm_surface *surface;
	uint32_t format;
	int width;
	int height;
};

}

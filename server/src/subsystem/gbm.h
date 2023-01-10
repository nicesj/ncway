#pragma once

#include "../subsystem.h"
#include "drm.h"
#include "../buffer_descriptor.h"

#include <unistd.h>
#include <gbm.h>

namespace ncway {

class GBM : public Subsystem {
private:
	GBM(void);

public:
	virtual ~GBM(void);

public:
	int getFD(void);
	int handler(int fd, uint32_t mask);

public:
	std::string name(void);
	std::string version(void);
	bool isCompatible(std::string ver);

public:
	static GBM *Create(DRM *drm, uint32_t format, uint64_t modifier);
	gbm_device *getDevice(void) const;
	gbm_surface *getSurface(void) const;
	int getWidth(void) const;
	int getHeight(void) const;
	uint32_t getFormat(void) const;
	BufferDescriptor *getBufferDescriptor(gbm_bo *bo, bool applyModifiers);
	gbm_bo *getBufferObject(void);
	void releaseBufferObject(gbm_bo *bo);
	DRM *getDRM(void);

private:
	DRM *drm;
	gbm_device *dev;
	gbm_surface *surface;
	uint32_t format;
	int width;
	int height;
};

}

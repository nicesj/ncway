#pragma once

#include "../subsystem.h"
#include "drm.h"
#include "../buffer_descriptor.h"

#include <memory>

#include <unistd.h>
#include <gbm.h>

namespace ncway {

class GBM : public Subsystem {
private:
	GBM(void);

public:
	virtual ~GBM(void);

public:
	int getFD(void) override;
	int handler(int fd, uint32_t mask) override;

public:
	std::string name(void) override;
	std::string version(void) override;
	bool isCompatible(std::string ver) override;

public:
	static std::shared_ptr<GBM> Create(std::shared_ptr<DRM> drm, uint32_t format, uint64_t modifier);
	gbm_device *getDevice(void) const;
	gbm_surface *getSurface(void) const;
	int getWidth(void) const;
	int getHeight(void) const;
	uint32_t getFormat(void) const;
	BufferDescriptor *getBufferDescriptor(gbm_bo *bo, bool applyModifiers);
	gbm_bo *getBufferObject(void);
	void releaseBufferObject(gbm_bo *bo);
	std::shared_ptr<DRM> getDRM(void);

private:
	std::shared_ptr<DRM> drm;
	gbm_device *dev;
	gbm_surface *surface;
	uint32_t format;
	int width;
	int height;
};

}

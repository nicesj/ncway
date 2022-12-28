#pragma once
#include "subsystem.h"
#include "subsystem/egl.h"
#include "subsystem/gbm.h"
#include "subsystem/drm.h"

#include <string>
#include <unistd.h>

namespace ncway {
class Renderer : public Subsystem {
public:
	Renderer *Create(DRM *drm, GBM *gbm, EGL *egl);

public:
	virtual ~Renderer(void);
	std::string name(void);
	int getFD(void);
	int handler(int fd, uint32_t mask);

private:
	Renderer(DRM *drm, GBM *gbm, EGL *egl);

private:
	DRM *drm;
	GBM *gbm;
	EGL *egl;
}
}

#include "renderer.h"
#include "subsystem/egl.h"
#include "subsystem/gbm.h"
#include "subsystem/drm.h"

#include <string>

#include <unistd.h>

namespace ncway {
Renderer::Renderer(DRM *drm, GBM *gbm, EGL *egl)
: drm(drm)
, gbm(gbm)
, egl(egl)
{
}

Renderer::~Renderer(void)
{
}

std::string Renderer::name(void)
{
	return "renderer";
}

int Renderer::getFD(void)
{
	return -1;
}

int Renderer::handler(int fd, uint32_t mask)
{
	return 1;
}

Renderer *Renderer::Create(DRM *drm, GBM *gbm, EGL *egl)
{
	Renderer *instance = new Renderer(drm, gbm, egl);
	if (!instance) {
		fprintf(stderr, "Failed to create a Renderer instance\n");
		return nullptr;
	}

	return instance;
}



}

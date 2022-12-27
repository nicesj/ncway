#include "./gbm.h"
#include "./drm.h"

#include <string>
#include <cstdio>

#include <drm_fourcc.h>
#include <unistd.h>
#include <gbm.h>

namespace ncway {

GBM::GBM(void)
: dev(nullptr)
, surface(nullptr)
, format(0)
, width(0)
, height(0)
{
}

GBM::~GBM(void)
{
	if (surface != nullptr) {
		gbm_surface_destroy(surface);
	}
}

GBM *GBM::Create(DRM *drm, uint32_t format, uint64_t modifier)
{
	GBM *instance = new GBM();
	if (instance == nullptr) {
		fprintf(stderr, "Failed to allocate memory for the GBM instance\n");
		return nullptr;
	}

	instance->dev = gbm_create_device(drm->getFD());
	if (instance->dev == nullptr) {
		fprintf(stderr, "Failed to create the GBM Device (%d)\n", drm->getFD());
		delete instance;
		return nullptr;
	}
	instance->format = format;

	if (gbm_surface_create_with_modifiers) {
		const drmModeModeInfo mode = drm->getMode();
		printf("Screen resolution: [%ux%u]\n", mode.hdisplay, mode.vdisplay);
		printf("Instance Device: %p\n", instance->dev);
		instance->surface = gbm_surface_create_with_modifiers(instance->dev,
			       	mode.hdisplay, mode.vdisplay,
			       	format,
			       	&modifier, 1);
		printf("surface is created: %p\n", instance->surface);
		instance->width = mode.hdisplay;
		instance->height = mode.vdisplay;
	}

	if (instance->surface == nullptr) {
		if (modifier != DRM_FORMAT_MOD_LINEAR) {
			fprintf(stderr, "Modifiers requested but it is not supported\n");
			delete instance;
			return nullptr;
		}

		const drmModeModeInfo mode = drm->getMode();
		instance->surface = gbm_surface_create(instance->dev,
			       	mode.hdisplay, mode.vdisplay,
			       	instance->format,
			       	GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
		instance->width = mode.hdisplay;
		instance->height = mode.vdisplay;
	}

	if (instance->surface == nullptr) {
		fprintf(stderr, "Surface is not available\n");
		return nullptr;
	}

	printf("Surface is prepared\n");
	return instance;
}

gbm_device *GBM::getDevice(void) const
{
	return dev;
}

gbm_surface *GBM::getSurface(void) const
{
	return surface;
}

int GBM::getWidth(void) const
{
	return width;
}

int GBM::getHeight(void) const
{
	return height;
}

uint32_t GBM::getFormat(void) const
{
	return format;
}

std::string GBM::name(void)
{
	return "gbm";
}

int GBM::getFD(void)
{
	return -1;
}

int GBM::handler(int fd, uint32_t mask)
{
	return 1;
}

}

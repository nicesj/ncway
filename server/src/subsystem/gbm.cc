#include "./gbm.h"
#include "./drm.h"
#include "../buffer_descriptor.h"

#include <string>
#include <cstdio>
#include <cstring>
#include <memory>

#include <wayland-server.h>
#include <drm_fourcc.h>
#include <unistd.h>
#include <gbm.h>
#include <inttypes.h>

__attribute__((weak)) uint64_t
gbm_bo_get_modifier(struct gbm_bo *bo);

__attribute__((weak)) int
gbm_bo_get_plane_count(struct gbm_bo *bo);

__attribute__((weak)) uint32_t
gbm_bo_get_stride_for_plane(struct gbm_bo *bo, int plane);

__attribute__((weak)) uint32_t
gbm_bo_get_offset(struct gbm_bo *bo, int plane);

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
	printf("GBM is destructed\n");
}

std::shared_ptr<GBM> GBM::create(std::shared_ptr<DRM> drm, uint32_t format, uint64_t modifier)
{
	std::shared_ptr<GBM> instance = std::shared_ptr<GBM>(new GBM());
	if (instance == nullptr) {
		fprintf(stderr, "Failed to allocate memory for the GBM instance\n");
		return nullptr;
	}

	instance->drm = drm;

	instance->dev = gbm_create_device(drm->getFD());
	if (instance->dev == nullptr) {
		fprintf(stderr, "Failed to create the GBM Device (%d)\n", drm->getFD());
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

std::string GBM::version(void)
{
	return "0.1";
}

bool GBM::isCompatible(std::string ver)
{
	return true;
}

int GBM::getFD(void)
{
	return -1;
}

int GBM::handler(int fd, uint32_t mask)
{
	return 1;
}

std::shared_ptr<wl_display> GBM::getDisplay(void)
{
	return drm->getDisplay();
}

BufferDescriptor *GBM::getBufferDescriptor(gbm_bo *bo, bool applyModifiers)
{
	BufferDescriptor *desc = static_cast<BufferDescriptor *>(gbm_bo_get_user_data(bo));
	if (desc != nullptr) {
		return desc;
	}

	desc = new BufferDescriptor();
	if (desc == nullptr) {
		fprintf(stderr, "Failed to allocate memory\n");
		return nullptr;
	}

	desc->width = gbm_bo_get_width(bo);
	desc->height = gbm_bo_get_height(bo);
	desc->format = gbm_bo_get_format(bo);
	desc->fb_id = 0;

	if (applyModifiers && gbm_bo_get_modifier && gbm_bo_get_plane_count && gbm_bo_get_stride_for_plane && gbm_bo_get_offset) {
		desc->modifiers[0] = gbm_bo_get_modifier(bo);
		const int num_planes = gbm_bo_get_plane_count(bo);
		for (int i = 0; i < num_planes; ++i) {
			desc->strides[i] = gbm_bo_get_stride_for_plane(bo, i);
			desc->handles[i] = gbm_bo_get_handle(bo).u32;
			desc->offsets[i] = gbm_bo_get_offset(bo, i);
			desc->modifiers[i] = desc->modifiers[0];
		}

		if (desc->modifiers[0]) {
			desc->flags = DRM_MODE_FB_MODIFIERS;
			printf("Using modifier %" PRIu64 "\n", desc->modifiers[0]);
		}
	} else {
		uint32_t tmp[4] = { 0, };
		tmp[0] = gbm_bo_get_handle(bo).u32;
		memcpy(desc->handles, tmp, 16);
		tmp[0] = gbm_bo_get_stride(bo);
		memcpy(desc->strides, tmp, 16);
		memset(desc->offsets, 0, 16);
	}

	gbm_bo_set_user_data(bo, desc, [](gbm_bo *bo, void *data) {
		BufferDescriptor *desc = static_cast<BufferDescriptor *>(data);
		printf("Destroy bufferDesctiptor\n");
		delete desc;
	});

	return desc;
}

gbm_bo *GBM::getBufferObject(void)
{
	return gbm_surface_lock_front_buffer(surface);
}

void GBM::releaseBufferObject(gbm_bo *bo)
{
	gbm_surface_release_buffer(surface, bo);
}

std::shared_ptr<DRM> GBM::getDRM(void)
{
	return drm;
}

}

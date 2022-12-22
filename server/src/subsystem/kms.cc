#include "kms.h"
#include "drm.h"

#include <string>
#include <cstdint>

namespace ncway {
KMS::KMS(const DRM *_drm)
: fd(-1)
, drm(_drm)
{
}

KMS::~KMS(void)
{
}

int KMS::getFD(void)
{
	return fd;
}

int KMS::handler(int fd, uint32_t mask)
{
	return 1;
}

std::string KMS::name(void)
{
	return "kms";
}

KMS *KMS::Create(const DRM *drm)
{
	if (!drm) {
		fprintf(stderr, "DRM object required\n");
		return nullptr;
	}

	KMS *instance = new KMS(drm);
	return instance;
}

}

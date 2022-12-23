#include "kms.h"
#include "drm.h"

#include <string>
#include <cstdint>
#include <libkms/libkms.h>

namespace ncway {
KMS::KMS(const DRM *_drm, kms_driver *_kms)
: fd(-1)
, drm(_drm)
, kms(_kms)
{
}

KMS::~KMS(void)
{
	kms_destroy(&kms);
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

	kms_dirver *kms = nullptr;
	int ret = kms_create(drm->getFD(), &kms);
	if (ret != 0) {
		fprintf(stderr, "Unable to create the KMS object\n");
		return nullptr;
	}

	KMS *instance = new KMS(drm, kms);
	if (!instance) {
		fprintf(stderr, "Failed to allocate heap for the KMS\n");
		kms_destroy(&kms);
		return nullptr;
	}

	return instance;
}

}

#pragma once
#include "../subsystem.h"
#include "drm.h"

#include <string>
#include <cstdint>

namespace ncway {
class KMS : public Subsystem {
private:
	KMS(const DRM *drm);

public:
	static KMS *Create(const DRM *drm);
	virtual ~KMS(void);

public:
	std::string name(void);
	int handler(int fd, uint32_t mask);
	int getFD(void);

private:
	int fd;
	const DRM *drm;
	kms_driver *kms;
};
}

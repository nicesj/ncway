#include "kms.h"

#include <string>
#include <cstdint>

namespace ncway {
KMS::KMS(void)
: fd(-1)
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

KMS *KMS::Create(void)
{
	KMS *instance = new KMS();
	return instance;
}

}

#pragma once
#include "../subsystem.h"

#include <string>
#include <cstdint>

namespace ncway {
class KMS : public Subsystem {
private:
	KMS(void);

public:
	static KMS *Create(void);
	virtual ~KMS(void);

public:
	std::string name(void);
	int handler(int fd, uint32_t mask);
	int getFD(void);

private:
	int fd;
};
}

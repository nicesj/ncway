#pragma once
#include "../subsystem.h"

namespace ncway {
class Renderer : public Subsystem {
public:
	virtual ~Renderer(void) = default;
	Renderer(void) = default;

public:
	int handler(int fd, uint32_t mask) override;
	int getFD(void) override;

public:
	std::string name(void) override;
	std::string version(void) override;
	bool isCompatible(std::string ver) override;
};
}

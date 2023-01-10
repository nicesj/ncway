#pragma once
#include "../subsystem.h"

namespace ncway {
class Renderer : public Subsystem {
public:
	virtual ~Renderer(void) = default;
	Renderer(void) = default;

public:
	std::string name(void);
	std::string version(void);
	bool isCompatible(std::string ver);
};
}

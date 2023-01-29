#pragma once
#include "../subsystem.h"

namespace ncway {
class Compositor : public Subsystem {
private:
	Compositor(void);
	virtual ~Compositor(void);

private:
	static std::shared_ptr<Compositor *> instance;

public:
	static std::shared_ptr<Compositor *> getInstance(void);
	static void destroy(void);

public:
	int handler(int fd, uint32_t mask) override;
	int getFD(void) override;

public:
	std::string name(void) override;
	std::string version(void) override;
	bool isCompatible(std::string ver) override;
};
}

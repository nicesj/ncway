#include "compositor.h"

#include <memory>

namespace ncway {

std::shared_ptr<Compositor *> Compositor::instance;

Compositor::Compositor(void)
{
}

Compositor::~Compositor(void)
{
}

std::shared_ptr<Compositor *> Compositor::getInstance(void)
{
	if (Compositor::instance == nullptr) {
		Compositor::instance = std::make_shared<Compositor *>(new Compositor());
	}

	return Compositor::instance;
}

void Compositor::destroy(void)
{
	Compositor::instance = nullptr;
}

int Compositor::handler(int fd, uint32_t mask)
{
	return 1;
}

int Compositor::getFD(void)
{
	return -1;
}

std::string Compositor::name(void)
{
	return "compositor";
}

std::string Compositor::version(void)
{
	return "0.1";
}

bool Compositor::isCompatible(std::string ver)
{
	return true;
}

}

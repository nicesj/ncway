#pragma once
#include <string>
#include "event.h"

namespace ncway {
class Subsystem : public Event {
public:
	Subsystem(void);
	virtual ~Subsystem(void);

	virtual std::string name(void) = 0;
	virtual std::string version(void) = 0;
	virtual bool isCompatible(std::string ver) = 0;
};
}

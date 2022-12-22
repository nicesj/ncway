#pragma once
#include <string>
#include "event.h"

namespace ncway {
class Subsystem : public Event {
public:
	Subsystem(void);
	virtual ~Subsystem(void);

	virtual std::string name(void) = 0;
};
}

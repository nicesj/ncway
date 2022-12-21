#pragma once
#include <string>

namespace ncway {

class Subsystem {
public:
	Subsystem(void);
	virtual ~Subsystem(void);

	virtual std::string name(void) = 0;
};

}

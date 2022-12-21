#pragma once

#include <vector>
#include <string>

#include "subsystem.h"

namespace ncway {

class SubsystemManager {
private:
	SubsystemManager(void);
	virtual ~SubsystemManager(void);

public:
	static SubsystemManager *create(void);
	static void destroy(SubsystemManager *manager);

public:
	int load(void);

	int getSubsystemList(std::vector<std::string> &nameVector);
	Subsystem *getSubsystem(std::string name);
};

}

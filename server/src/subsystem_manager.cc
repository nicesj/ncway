#include "subsystem_manager.h"
#include "subsystem.h"

namespace ncway {

SubsystemManager::SubsystemManager(void)
{
}

SubsystemManager::~SubsystemManager(void)
{
}

SubsystemManager *SubsystemManager::create(void)
{
	SubsystemManager *manager = new SubsystemManager();
	return manager;
}

void SubsystemManager::destroy(SubsystemManager *manager)
{
	delete manager;
}

int SubsystemManager::load(void)
{
	return 0;
}

int SubsystemManager::getSubsystemList(std::vector<std::string> &nameVector)
{
	return 0;
}

Subsystem *SubsystemManager::getSubsystem(std::string name)
{
	return nullptr;
}

}

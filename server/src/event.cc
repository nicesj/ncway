#include "event.h"

#include <memory>

#include <cstdint>

namespace ncway {

int Event::eventHandler(int fd, uint32_t mask, void *data)
{
	Event::EventInfo *info = static_cast<Event::EventInfo *>(data);
	return info->eventPtr->handler(fd, mask);
}

}

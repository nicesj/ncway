#include "event.h"
#include <cstdint>

namespace ncway {

int Event::eventHandler(int fd, uint32_t mask, void *data)
{
	Event *evt = static_cast<Event *>(data);
	return evt->handler(fd, mask);
}

}

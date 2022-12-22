#pragma once
#include <cstdint>

namespace ncway {
class Event {
public:
	Event(void) = default;
	virtual ~Event(void) = default;

	virtual int getFD(void) = 0;
	virtual int handler(int fd, uint32_t mask) = 0;
public:
	static int eventHandler(int fd, uint32_t mask, void *data);
};
}

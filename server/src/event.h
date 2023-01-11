#pragma once

#include <memory>

#include <cstdint>

namespace ncway {
class Event : public std::enable_shared_from_this<Event> {
public:
	Event(void) = default;
	virtual ~Event(void) = default;

public:
	struct EventInfo {
		std::shared_ptr<Event> eventPtr;
	};

public:
	virtual int getFD(void) = 0;
	virtual int handler(int fd, uint32_t mask) = 0;

public:
	static int eventHandler(int fd, uint32_t mask, void *data);
};
}

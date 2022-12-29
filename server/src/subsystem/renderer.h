#pragma once
#include "../subsystem.h"

#include <string>
#include <unistd.h>

namespace ncway {
class Renderer : public Subsystem {
public:
	virtual ~Renderer(void) = default;
	Renderer(void) = default;

public:
	struct bufferDescription {
		uint32_t width;
		uint32_t height;
		uint32_t format;
		uint32_t flags;
		uint32_t strides[4];
		uint32_t handles[4];
		uint32_t offsets[4];
		uint64_t modifiers[4];

		void *user_data;
		void (*user_data_destructor)(bufferDescription *desc);
	};
};
}

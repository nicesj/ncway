#pragma once

#include <string>
#include <functional>

#include <unistd.h>

#define PIXEL_SIZE 4

namespace ncway {

struct BufferDescriptor {
	uint32_t width;
	uint32_t height;
	uint32_t format;
	uint32_t flags;
	uint32_t strides[PIXEL_SIZE];
	uint32_t handles[PIXEL_SIZE];
	uint32_t offsets[PIXEL_SIZE];
	uint64_t modifiers[PIXEL_SIZE];
	uint32_t fb_id;

	std::function<void(BufferDescriptor *)> customDestructor;

	virtual ~BufferDescriptor(void);
	BufferDescriptor(void);
};

}

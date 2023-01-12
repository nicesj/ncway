#include "./buffer_descriptor.h"

namespace ncway {
BufferDescriptor::BufferDescriptor(void)
{
}

BufferDescriptor::~BufferDescriptor(void)
{
	if (customDestructor) {
		customDestructor(this);
	}
}
}

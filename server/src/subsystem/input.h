#include "../subsystem.h"
#include <string>

namespace ncway
{
class Input : public Subsystem {
public:
	Input(void);
	virtual ~Input(void);

	std::string name(void);
};
}

#include "../subsystem.h"
#include <libudev.h>
#include <libinput.h>

#include <string>

namespace ncway
{
class Input : public Subsystem {
private:
	Input(void);

public:
	static Input *Create(std::string seat);
	virtual ~Input(void);

public:
	std::string name(void);
	int handler(int fd, uint32_t mask);
	int getFD(void);

private:
	static void closeRestricted(int fd, void *data);
	static int openRestricted(const char *path, int flags, void *data);

private:
	udev *ud;
	libinput *li;
};
}

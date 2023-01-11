#include "../subsystem.h"

#include <memory>
#include <string>

#include <libudev.h>
#include <libinput.h>
#include <wayland-server.h>

namespace ncway
{
class Input : public Subsystem {
private:
	Input(void);

public:
	static std::shared_ptr<Input> create(std::shared_ptr<wl_display> display, std::string seat);
	virtual ~Input(void);

public:
	std::string name(void) override;
	std::string version(void) override;
	bool isCompatible(std::string ver) override;

public:
	int handler(int fd, uint32_t mask) override;
	int getFD(void) override;

public:
	std::shared_ptr<wl_display> getDisplay(void);

private:
	static void closeRestricted(int fd, void *data);
	static int openRestricted(const char *path, int flags, void *data);

private:
	udev *ud;
	libinput *li;

private:
	std::shared_ptr<wl_display> display;
};
}

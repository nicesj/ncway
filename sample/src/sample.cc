#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl31.h>

#include <fcntl.h>
#include <unistd.h>

#include <gbm.h>

#include <cstdio>
#include <cerrno>
#include <cstring>

int main(int argc, char *argv[])
{
	int fd = open("/dev/dri/card0", O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "open: %s\n", strerror(errno));
	}

	gbm_device *gbm = gbm_create_device(fd);
	if (!gbm) {
		if (close(fd) < 0) {
			fprintf(stderr, "close: %s\n", strerror(errno));
		}
		return -1;
	}


	EGLDisplay dpy = eglGetPlatformDisplay(EGL_PLATFORM_GBM_MESA, gbm, nullptr);
	bool res = eglInitialize(dpy, nullptr, nullptr);
	const char *egl_ext = eglQueryString(dpy, EGL_EXTENSIONS);
	printf("EGL Extension: %s\n", egl_ext);

	static const EGLint config_attribs[] = {
		EGL_RENDERABLE_TYPE,
		EGL_OPENGL_ES3_BIT_KHR,
		EGL_NONE,
	};

	EGLConfig cfg;
	EGLint count;

	res = eglChooseConfig(dpy, config_attribs, &cfg, 1, &count);
	res = eglBindAPI(EGL_OPENGL_ES_API);

	static const EGLint attribs[] = {
		EGL_CONTEXT_CLIENT_VERSION,
		3,
		EGL_NONE,
	};

	EGLContext core_ctx = eglCreateContext(dpy, cfg, EGL_NO_CONTEXT, attribs);

	glDispatchCompute(1, 1, 1);
	eglDestroyContext(dpy, core_ctx);
	eglTerminate(dpy);
	gbm_device_destroy(gbm);
	if (close(fd) < 0) {
		fprintf(stderr, "close: %s\n", strerror(errno));
	}
	return 0;
}

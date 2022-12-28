#include "egl.h"

#include <cstdlib>
#include <cerrno>
#include <cstring>

namespace ncway {

EGL::EGL(void)
{
}

EGL::~EGL(void)
{
}

EGLDisplay& EGL::getDisplay()
{
	return display;
}

EGLConfig& EGL::getConfig()
{
	return config;
}

EGLContext& EGL::getContext()
{
	return context;
}

EGLSurface& EGL::getSurface()
{
	return surface;
}

bool EGL::chooseConfig(EGLDisplay display, const EGLint *attribs, EGLint visual_id, EGLConfig *config_out)
{
	EGLint count = 0;
	EGLint matched = 0;
	EGLConfig *configs;
	int config_index = -1;

	if (!eglGetConfigs(display, nullptr, 0, &count) || count < 1) {
		fprintf(stderr, "No EGL configs to choose from\n");
		return false;
	}

	configs = reinterpret_cast<EGLConfig *>(malloc(count * sizeof(*configs)));
	if (!configs) {
		fprintf(stderr, "malloc: %s\n", strerror(errno));
		return false;
	}

	if (!eglChooseConfig(display, attribs, configs, count, &matched) || !matched) {
		fprintf(stderr, "No EGL configs with appropriate attributes\n");
		goto out;
	}

	if (!visual_id) {
		config_index = 0;
	}

	if (config_index == -1) {
		config_index = matchConfigToVisual(display, visual_id, configs, matched);
	}

	if (config_index != -1) {
		*config_out = configs[config_index];
	}

out:
	free(configs);

	if (config_index == -1) {
		return false;
	}

	return true;
}

int EGL::matchConfigToVisual(EGLDisplay display, EGLint visualId, EGLConfig *configs, int count)
{
	for (int i = 0; i < count; ++i) {
		EGLint id;

		if (!eglGetConfigAttrib(display, configs[i], EGL_NATIVE_VISUAL_ID, &id))
			continue;

		if (id == visualId)
			return i;
	}

	return -1;
}

bool EGL::hasExt(const char *extensionList, const char *ext)
{
	const char *ptr = extensionList;
	int len = strlen(ext);

	if (ptr == NULL || *ptr == '\0')
		return false;

	while (true) {
		ptr = strstr(ptr, ext);
		if (!ptr)
			return false;

		if (ptr[len] == ' ' || ptr[len] == '\0')
			return true;

		ptr += len;
	}
}

EGL *EGL::Create(GBM *gbm, int samples)
{
	EGL *egl = new EGL();
	if (egl == nullptr) {
		fprintf(stderr, "Failed to allocate memory for the EGL\n");
		return nullptr;
	}

	static const EGLint context_attribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};

	const EGLint config_attribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RED_SIZE, 1,
		EGL_GREEN_SIZE, 1,
		EGL_BLUE_SIZE, 1,
		EGL_ALPHA_SIZE, 0,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_SAMPLES, samples,
		EGL_NONE
	};

#define get_proc_client(ext, type, name) do { \
		if (egl->hasExt(egl_exts_client, #ext)) \
			egl->name = reinterpret_cast<type>(eglGetProcAddress(#name)); \
	} while(0)

	const char *egl_exts_client = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
	get_proc_client(EGL_EXT_platform_base, PFNEGLGETPLATFORMDISPLAYEXTPROC, eglGetPlatformDisplayEXT);

	if (egl->eglGetPlatformDisplayEXT) {
		egl->display = egl->eglGetPlatformDisplayEXT(EGL_PLATFORM_GBM_KHR, gbm->getDevice(), nullptr);
	} else {
		egl->display = eglGetDisplay(gbm->getDevice());
	}

	EGLint major, minor;
	if (!eglInitialize(egl->display, &major, &minor)) {
		printf("Failed to initialize\n");
		delete egl;
		return nullptr;
	}

#define get_proc_dpy(ext, type, name) do { \
		if (egl->hasExt(egl_exts_dpy, #ext)) \
			egl->name = reinterpret_cast<type>(eglGetProcAddress(#name)); \
	} while (0)

	const char *egl_exts_dpy = eglQueryString(egl->display, EGL_EXTENSIONS);
	get_proc_dpy(EGL_KHR_image_base, PFNEGLCREATEIMAGEKHRPROC, eglCreateImageKHR);
	get_proc_dpy(EGL_KHR_image_base, PFNEGLDESTROYIMAGEKHRPROC, eglDestroyImageKHR);
	get_proc_dpy(EGL_KHR_fence_sync, PFNEGLCREATESYNCKHRPROC, eglCreateSyncKHR);
	get_proc_dpy(EGL_KHR_fence_sync, PFNEGLDESTROYSYNCKHRPROC, eglDestroySyncKHR);
	get_proc_dpy(EGL_KHR_fence_sync, PFNEGLWAITSYNCKHRPROC, eglWaitSyncKHR);
	get_proc_dpy(EGL_KHR_fence_sync, PFNEGLCLIENTWAITSYNCKHRPROC, eglClientWaitSyncKHR);
	get_proc_dpy(EGL_ANDROID_native_fence_sync, PFNEGLDUPNATIVEFENCEFDANDROIDPROC, eglDupNativeFenceFDANDROID);

	egl->modifiers_supported = egl->hasExt(egl_exts_dpy, "EGL_EXT_image_dma_buf_import_modifiers");
	
	printf("Using display %p with EGL version %d.%d\n", egl->display, major, minor);
	printf("===================\n");
	printf("EGL information: \n");
	printf("  version: \"%s\"\n", eglQueryString(egl->display, EGL_VERSION));
	printf("  vendor: \"%s\"\n", eglQueryString(egl->display, EGL_VENDOR));
	printf("  client extensions: \"%s\"\n", egl_exts_client);
	printf("  display extensions: \"%s\"\n", egl_exts_dpy);
	printf("===================\n");

	if (!eglBindAPI(EGL_OPENGL_ES_API)) {
		printf("Failed to bind api EGL_OPENGL_ES_API\n");
		delete egl;
		return nullptr;
	}

	if (!egl->chooseConfig(egl->display, config_attribs, gbm->getFormat(), &egl->config)) {
		printf("Failed to choose config\n");
		delete egl;
		return nullptr;
	}

	egl->context = eglCreateContext(egl->display, egl->config, EGL_NO_CONTEXT, context_attribs);
	if (egl->context == nullptr) {
		fprintf(stderr, "Failed to create a context\n");
		delete egl;
		return nullptr;
	}

	egl->surface = eglCreateWindowSurface(egl->display, egl->config, static_cast<EGLNativeWindowType>(gbm->getSurface()), nullptr);
	if (egl->surface == EGL_NO_SURFACE) {
		printf("Failed to create egl surface\n");
		delete egl;
		return nullptr;
	}

	eglMakeCurrent(egl->display, egl->surface, egl->surface, egl->context);

#define get_proc_gl(ext, type, name) do { \
		if (egl->hasExt(gl_exts, #ext)) \
			egl->name = reinterpret_cast<type>(eglGetProcAddress(#name)); \
	} while (0)

	const char *gl_exts = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));
	printf("OpenGL ES 2.x information:\n");
	printf("  version: \"%s\"\n", glGetString(GL_VERSION));
	printf("  shading language version: \"%s\"\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	printf("  vendor: \"%s\"\n", glGetString(GL_VENDOR));
	printf("  renderer: \"%s\"\n", glGetString(GL_RENDERER));
	printf("  extensions: \"%s\"\n", gl_exts);
	printf("====================================\n");

	get_proc_gl(GL_OES_EGL_image, PFNGLEGLIMAGETARGETTEXTURE2DOESPROC, glEGLImageTargetTexture2DOES);

	get_proc_gl(GL_AND_performance_monitor, PFNGLGETPERFMONITORGROUPSAMDPROC, glGetPerfMonitorGroupsAMD);
	get_proc_gl(GL_AND_performance_monitor, PFNGLGETPERFMONITORCOUNTERSAMDPROC, glGetPerfMonitorCountersAMD);
	get_proc_gl(GL_AMD_performance_monitor, PFNGLGETPERFMONITORGROUPSTRINGAMDPROC, glGetPerfMonitorGroupStringAMD);
	get_proc_gl(GL_AMD_performance_monitor, PFNGLGETPERFMONITORCOUNTERSTRINGAMDPROC, glGetPerfMonitorCounterStringAMD);
	get_proc_gl(GL_AMD_performance_monitor, PFNGLGETPERFMONITORCOUNTERINFOAMDPROC, glGetPerfMonitorCounterInfoAMD);
	get_proc_gl(GL_AMD_performance_monitor, PFNGLGENPERFMONITORSAMDPROC, glGenPerfMonitorsAMD);
	get_proc_gl(GL_AMD_performance_monitor, PFNGLDELETEPERFMONITORSAMDPROC, glDeletePerfMonitorsAMD);
	get_proc_gl(GL_AMD_performance_monitor, PFNGLSELECTPERFMONITORCOUNTERSAMDPROC, glSelectPerfMonitorCountersAMD);
	get_proc_gl(GL_AMD_performance_monitor, PFNGLBEGINPERFMONITORAMDPROC, glBeginPerfMonitorAMD);
	get_proc_gl(GL_AMD_performance_monitor, PFNGLENDPERFMONITORAMDPROC, glEndPerfMonitorAMD);
	get_proc_gl(GL_AMD_performance_monitor, PFNGLGETPERFMONITORCOUNTERDATAAMDPROC, glGetPerfMonitorCounterDataAMD);

	return egl;
}

}
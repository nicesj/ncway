#include "./egl.h"
#include "./gbm.h"
#include "./drm.h"
#include "../buffer_descriptor.h"

#include <memory>
#include <functional>

#include <cstdlib>
#include <cerrno>
#include <cstring>

namespace ncway {

EGL::EGL(void)
{
}

EGL::~EGL(void)
{
	printf("EGL is destructed\n");
}

std::string EGL::name(void)
{
	return "egl";
}

std::string EGL::version(void)
{
	return "0.1";
}

bool EGL::isCompatible(std::string ver)
{
	return true;
}

int EGL::getFD(void)
{
	return -1;
}

int EGL::handler(int fd, uint32_t mask)
{
	return 0;
}

EGLDisplay& EGL::getEGLDisplay(void)
{
	return display;
}

EGLConfig& EGL::getEGLConfig(void)
{
	return config;
}

EGLContext& EGL::getEGLContext(void)
{
	return context;
}

EGLSurface& EGL::getEGLSurface(void)
{
	return surface;
}

std::shared_ptr<wl_display> EGL::getDisplay(void)
{
	return gbm->getDisplay();
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

std::shared_ptr<EGL> EGL::create(std::shared_ptr<GBM> gbm, int samples)
{
	std::shared_ptr<EGL> egl = std::shared_ptr<EGL>(new EGL());
	if (egl == nullptr) {
		fprintf(stderr, "Failed to allocate memory for the EGL\n");
		return nullptr;
	}

	egl->gbm = gbm;

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

#define get_proc_client(ext, type, name) \
	do { \
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
		return nullptr;
	}

#define get_proc_dpy(ext, type, name) \
	do { \
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
		return nullptr;
	}

	if (!egl->chooseConfig(egl->display, config_attribs, gbm->getFormat(), &egl->config)) {
		printf("Failed to choose config\n");
		return nullptr;
	}

	egl->context = eglCreateContext(egl->display, egl->config, EGL_NO_CONTEXT, context_attribs);
	if (egl->context == nullptr) {
		fprintf(stderr, "Failed to create a context\n");
		return nullptr;
	}

	egl->surface = eglCreateWindowSurface(egl->display, egl->config, static_cast<EGLNativeWindowType>(gbm->getSurface()), nullptr);
	if (egl->surface == EGL_NO_SURFACE) {
		printf("Failed to create egl surface\n");
		return nullptr;
	}

	eglMakeCurrent(egl->display, egl->surface, egl->surface, egl->context);

#define get_proc_gl(ext, type, name) \
	do { \
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
	return egl;
}

int EGL::startRender(std::function<void(void)> renderer)
{
	eglSwapBuffers(display, surface);
	gbm_bo *bo = gbm->getBufferObject();
	BufferDescriptor *desc = gbm->getBufferDescriptor(bo, false);
	uint32_t fb_id = gbm->getDRM()->addFramebuffer(desc);
	gbm->getDRM()->setCrtcMode(fb_id, 0, 0);

	eglSwapBuffers(display, surface);
	gbm_bo *next_bo = gbm->getBufferObject();
	BufferDescriptor *next_desc = gbm->getBufferDescriptor(next_bo, false);
	uint32_t next_fb_id = gbm->getDRM()->addFramebuffer(next_desc);
	DRM::EventData *evtData = new DRM::EventData();
	evtData->bo = next_bo;
	evtData->renderer = [&, renderer, evtData](void) -> void {
		gbm_bo *bo = static_cast<gbm_bo *>(evtData->bo);
		if (renderer) {
			renderer();
		}
		eglSwapBuffers(display, surface);

		gbm_bo *next_bo = gbm->getBufferObject();
		BufferDescriptor *next_desc = gbm->getBufferDescriptor(next_bo, false);
		uint32_t next_fb_id = gbm->getDRM()->getFBID(next_desc);
		evtData->bo = static_cast<void *>(next_bo);

		gbm->getDRM()->pageFlip(next_fb_id, evtData);
		gbm->releaseBufferObject(bo);
		return;
	};
	gbm->getDRM()->pageFlip(next_fb_id, evtData);
	gbm->releaseBufferObject(bo);

	return 0;
}

std::shared_ptr<GBM> EGL::getGBM(void)
{
	return gbm;
}

}

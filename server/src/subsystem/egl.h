#pragma once

#include "../subsystem.h"
#include "gbm.h"

#include <memory>
#include <functional>

#ifndef GL_ES_VERSION_2_0
#include <GLES2/gl2.h>
#endif
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <wayland-server.h>
#include <gbm.h>
#include <drm_fourcc.h>

namespace ncway {

class EGL : public Subsystem {
private:
	EGL(void);

public:
	virtual ~EGL(void);

public:
	int getFD(void) override;
	int handler(int fd, uint32_t mask) override;

public:
	std::string name(void) override;
	std::string version(void) override;
	bool isCompatible(std::string ver) override;

public:
	static std::shared_ptr<EGL> create(std::shared_ptr<GBM> gbm, int samples);
	EGLDisplay& getEGLDisplay(void);
	EGLConfig& getEGLConfig(void);
	EGLContext& getEGLContext(void);
	EGLSurface& getEGLSurface(void);

public:
	int startRender(std::function<void(void)> renderer);
	std::shared_ptr<GBM> getGBM(void);
	std::shared_ptr<wl_display> getDisplay(void);

private:
	bool chooseConfig(EGLDisplay display, const EGLint *attrib, EGLint visual_id, EGLConfig *config_out);
	bool hasExt(const char *extension_list, const char *et);
	int matchConfigToVisual(EGLDisplay display, EGLint visualId, EGLConfig *configs, int count);

private:
	EGLDisplay display;
	EGLConfig config;
	EGLContext context;
	EGLSurface surface;

	std::shared_ptr<GBM> gbm;

	PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT;
	PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR;
	PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR;
	PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES;
	PFNEGLCREATESYNCKHRPROC eglCreateSyncKHR;
	PFNEGLDESTROYSYNCKHRPROC eglDestroySyncKHR;
	PFNEGLWAITSYNCKHRPROC eglWaitSyncKHR;
	PFNEGLCLIENTWAITSYNCKHRPROC eglClientWaitSyncKHR;
	PFNEGLDUPNATIVEFENCEFDANDROIDPROC eglDupNativeFenceFDANDROID;

	/* AMD performance monitor */
	PFNGLGETPERFMONITORGROUPSAMDPROC glGetPerfMonitorGroupsAMD;
	PFNGLGETPERFMONITORCOUNTERSAMDPROC glGetPerfMonitorCountersAMD;
	PFNGLGETPERFMONITORGROUPSTRINGAMDPROC glGetPerfMonitorGroupStringAMD;
	PFNGLGETPERFMONITORCOUNTERSTRINGAMDPROC glGetPerfMonitorCounterStringAMD;
	PFNGLGETPERFMONITORCOUNTERINFOAMDPROC glGetPerfMonitorCounterInfoAMD;
	PFNGLGENPERFMONITORSAMDPROC glGenPerfMonitorsAMD;
	PFNGLDELETEPERFMONITORSAMDPROC glDeletePerfMonitorsAMD;
	PFNGLSELECTPERFMONITORCOUNTERSAMDPROC glSelectPerfMonitorCountersAMD;
	PFNGLBEGINPERFMONITORAMDPROC glBeginPerfMonitorAMD;
	PFNGLENDPERFMONITORAMDPROC glEndPerfMonitorAMD;
	PFNGLGETPERFMONITORCOUNTERDATAAMDPROC glGetPerfMonitorCounterDataAMD;
	bool modifiers_supported;
};

}

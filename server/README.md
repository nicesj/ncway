# Overview

![Wayland Architecture](https://wayland.freedesktop.org/wayland-architecture.png)

![DRI and Wayland](https://en.wikipedia.org/wiki/Direct_Rendering_Infrastructure#/media/File:Linux_graphics_drivers_DRI_Wayland.svg)

# Prerequisite

## Vulkan
```bash
$ sudo apt install vulkan-tools
$ sudo apt install libvulkan-dev
$ sudo apt install vulkan-validationlayers-dev spirv-tools
```

## DRI
```bash
$ sudo apt install libinput-dev
$ sudo apt install libdrm-dev
$ sudo apt install libkms-dev
```

# References

 * [vulkan-tutorial](https://vulkan-tutorial.com/Introduction)
 * [KMS pageflip example](https://github.com/liujunming/GPU_learning/blob/master/drm/kms-pageflip.c)
 * [kmscube EGL example](https://gitlab.freedesktop.org/mesa/kmscube/-/blob/master/common.c)
 * [egl on DRM example](https://blogs.igalia.com/elima/2016/10/06/example-run-an-opengl-es-compute-shader-on-a-drm-render-node/)
 * [DRM and KMS](https://events.static.linuxfound.org/sites/events/files/lcjpcojp13_pinchart.pdf)
 * [KMS implementation](https://github.com/CPFL/drm/blob/master/libkms/api.c)

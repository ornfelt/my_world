#ifndef GFX_DEVICES_H
#define GFX_DEVICES_H

#include "config.h"

struct gfx_window;

#ifdef GFX_ENABLE_DEVICE_D3D11
#include <dxgi.h>

struct gfx_device *gfx_d3d11_device_new(struct gfx_window *window, DXGI_SWAP_CHAIN_DESC *swap_chain_desc, IDXGISwapChain **swap_chain);
void gfx_d3d11_resize(struct gfx_device *device);
#endif

#ifdef GFX_ENABLE_DEVICE_D3D9
#include <dxgi.h>

struct gfx_device *gfx_d3d9_device_new(struct gfx_window *window, DXGI_SWAP_CHAIN_DESC *swap_chain_desc, IDXGISwapChain **swap_chain);
#endif

#if defined(GFX_ENABLE_DEVICE_GL4) || defined(GFX_ENABLE_DEVICE_GL3) || defined(GFX_ENABLE_DEVICE_GLES3)
typedef void *(gfx_gl_load_addr_t)(const char *name);
#endif

#ifdef GFX_ENABLE_DEVICE_GL3
struct gfx_device *gfx_gl3_device_new(struct gfx_window *window, gfx_gl_load_addr_t *load_addr);
#endif

#ifdef GFX_ENABLE_DEVICE_GL4
struct gfx_device *gfx_gl4_device_new(struct gfx_window *window, gfx_gl_load_addr_t *load_addr);
#endif

#ifdef GFX_ENABLE_DEVICE_GLES3
struct gfx_device *gfx_gles3_device_new(struct gfx_window *window, gfx_gl_load_addr_t *load_addr);
#endif

#ifdef GFX_ENABLE_DEVICE_VK
#include <vulkan/vulkan.h>

struct gfx_device *gfx_vk_device_new(struct gfx_window *window, VkInstance instance, VkSurfaceKHR surface);

void gfx_vk_set_swap_interval(struct gfx_device *device, int interval);
void gfx_vk_swap_buffers(struct gfx_device *device);
#endif

#endif

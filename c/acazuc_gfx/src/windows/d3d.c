#define COBJMACROS
#define CINTERFACE
#define WIN32_LEAN_AND_MEAN
#define INITGUID

#include "windows/win32.h"

#include "window_vtable.h"
#include "devices.h"
#include "window.h"
#include "config.h"

DEFINE_GUID(UIID_IDXGIFactory, 0x7b7166ec, 0x21c7, 0x44ae, 0xb2, 0x1a, 0xc9, 0xae, 0x32, 0x1a, 0xe3, 0x69);

struct gfx_d3d_window
{
	struct gfx_window window;
	struct gfx_win32_window win32;
	DXGI_SWAP_CHAIN_DESC swap_chain_desc;
	IDXGISwapChain *swap_chain;
	int interval;
};

#define WIN32_WINDOW (&D3D_WINDOW->win32)
#define D3D_WINDOW ((struct gfx_d3d_window*)window)

static void d3d_on_resize(struct gfx_window *window);

static bool d3d_ctr(struct gfx_window *window, struct gfx_window_properties *properties)
{
	D3D_WINDOW->interval = 1;
	return gfx_window_vtable.ctr(window, properties);
}

static void d3d_dtr(struct gfx_window *window)
{
	if (D3D_WINDOW->swap_chain)
		IDXGISwapChain_Release(D3D_WINDOW->swap_chain);
	gfx_win32_dtr(&D3D_WINDOW->win32);
	gfx_window_vtable.dtr(window);
}

static bool d3d_create_device(struct gfx_window *window)
{
	switch (window->properties.device_backend)
	{
#ifdef GFX_ENABLE_DEVICE_D3D9
		case GFX_DEVICE_D3D9:
			window->device = gfx_d3d9_device_new(&D3D_WINDOW->window, &D3D_WINDOW->swap_chain_desc, &D3D_WINDOW->swap_chain);
			d3d_on_resize(window);
			return true;
#endif
#ifdef GFX_ENABLE_DEVICE_D3D11
		case GFX_DEVICE_D3D11:
			window->device = gfx_d3d11_device_new(&D3D_WINDOW->window, &D3D_WINDOW->swap_chain_desc, &D3D_WINDOW->swap_chain);
			d3d_on_resize(window);
			return true;
#endif
		default:
			break;
	}
	return false;
}

static void d3d_show(struct gfx_window *window)
{
	gfx_win32_show(WIN32_WINDOW);
}

static void d3d_hide(struct gfx_window *window)
{
	gfx_win32_hide(WIN32_WINDOW);
}

static void d3d_set_title(struct gfx_window *window, const char *title)
{
	gfx_win32_set_title(WIN32_WINDOW, title);
}

static void d3d_set_icon(struct gfx_window *window, const void *data, uint32_t width, uint32_t height)
{
	gfx_win32_set_icon(WIN32_WINDOW, data, width, height);
}

static void d3d_poll_events(struct gfx_window *window)
{
	gfx_win32_poll_events(WIN32_WINDOW);
}

static void d3d_wait_events(struct gfx_window *window)
{
	gfx_win32_wait_events(WIN32_WINDOW);
}

static void d3d_set_swap_interval(struct gfx_window *window, int interval)
{
	if (interval == 0)
		D3D_WINDOW->interval = 0;
	else
		D3D_WINDOW->interval = 1;
}

static void d3d_swap_buffers(struct gfx_window *window)
{
	IDXGISwapChain_Present(D3D_WINDOW->swap_chain, D3D_WINDOW->interval, 0);
}

static void d3d_make_current(struct gfx_window *window)
{
	(void)window;
}

static void d3d_resize(struct gfx_window *window, uint32_t width, uint32_t height)
{
	gfx_win32_resize(WIN32_WINDOW, width, height);
}

static void d3d_grab_cursor(struct gfx_window *window)
{
	gfx_win32_grab_cursor(WIN32_WINDOW);
}

static void d3d_ungrab_cursor(struct gfx_window *window)
{
	gfx_win32_ungrab_cursor(WIN32_WINDOW);
}

static char *d3d_get_clipboard(struct gfx_window *window)
{
	return gfx_win32_get_clipboard(WIN32_WINDOW);
}

static void d3d_set_clipboard(struct gfx_window *window, const char *text)
{
	gfx_win32_set_clipboard(WIN32_WINDOW, text);
}

static gfx_cursor_t d3d_create_native_cursor(struct gfx_window *window, enum gfx_native_cursor cursor)
{
	return gfx_win32_create_native_cursor(WIN32_WINDOW, cursor);
}

static gfx_cursor_t d3d_create_cursor(struct gfx_window *window, const void *data, uint32_t width, uint32_t height, uint32_t xhot, uint32_t yhot)
{
	return gfx_win32_create_cursor(WIN32_WINDOW, data, width, height, xhot, yhot);
}

static void d3d_delete_cursor(struct gfx_window *window, gfx_cursor_t cursor)
{
	gfx_win32_delete_cursor(WIN32_WINDOW, cursor);
}

static void d3d_set_cursor(struct gfx_window *window, gfx_cursor_t cursor)
{
	gfx_win32_set_cursor(WIN32_WINDOW, cursor);
}

static void d3d_set_mouse_position(struct gfx_window *window, int32_t x, int32_t y)
{
	gfx_win32_set_mouse_position(WIN32_WINDOW, x, y);
}

static void d3d_on_resize(struct gfx_window *window)
{
	switch (window->properties.device_backend)
	{
		case GFX_DEVICE_GL3:
			break;
		case GFX_DEVICE_GL4:
			break;
		case GFX_DEVICE_GLES3:
			break;
		case GFX_DEVICE_D3D9:
#ifdef GFX_ENABLE_DEVICE_D3D9
			if (window->device)
				gfx_d3d9_resize(window->device);
#endif
			break;
		case GFX_DEVICE_D3D11:
#ifdef GFX_ENABLE_DEVICE_D3D11
			if (window->device)
				gfx_d3d11_resize(window->device);
#endif
			break;
		case GFX_DEVICE_VK:
			break;
	}
}

static const struct gfx_window_vtable d3d_vtable =
{
	GFX_WINDOW_VTABLE_DEF(d3d)
};

struct gfx_window *gfx_d3d_window_new(const char *title, uint32_t width, uint32_t height, struct gfx_window_properties *properties)
{
	struct gfx_window *window = GFX_MALLOC(sizeof(struct gfx_d3d_window));
	if (!window)
	{
		GFX_ERROR_CALLBACK("malloc failed");
		return NULL;
	}
	memset(window, 0, sizeof(struct gfx_d3d_window));
	window->vtable = &d3d_vtable;
	if (!window->vtable->ctr(window, properties))
	{
		window->vtable->dtr(window);
		GFX_FREE(window);
		return NULL;
	}
	gfx_win32_ctr(WIN32_WINDOW, window);
	WIN32_WINDOW->on_resize = d3d_on_resize;
	if (!gfx_win32_create_window(WIN32_WINDOW, title, width, height))
	{
		GFX_ERROR_CALLBACK("failed to create window");
		window->vtable->dtr(window);
		GFX_FREE(window);
		return NULL;
	}
	IDXGIFactory *factory;
	IDXGIAdapter *adapter;
	IDXGIOutput* adapter_output;
	DXGI_ADAPTER_DESC adapter_desc;
	unsigned num_modes;
	unsigned numerator;
	unsigned denominator;
	DXGI_MODE_DESC *modes = NULL;
	if (FAILED(CreateDXGIFactory(&UIID_IDXGIFactory, (void**)&factory)))
	{
		GFX_ERROR_CALLBACK("failed to create DXGI factory");
		goto err;
	}
	if (FAILED(IDXGIFactory_EnumAdapters(factory, 0, &adapter)))
	{
		GFX_ERROR_CALLBACK("failed to enumerate adapters");
		goto err;
	}
	if (FAILED(IDXGIAdapter_EnumOutputs(adapter, 0, &adapter_output)))
	{
		GFX_ERROR_CALLBACK("failed to enumerate outputs");
		goto err;
	}
	if (FAILED(IDXGIOutput_GetDisplayModeList(adapter_output, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &num_modes, NULL)))
	{
		GFX_ERROR_CALLBACK("failed to get display mode list number");
		goto err;
	}
	modes = (DXGI_MODE_DESC*)GFX_MALLOC(sizeof(*modes) * num_modes);
	if (!modes)
	{
		GFX_ERROR_CALLBACK("failed to malloc dxgi modes");
		goto err;
	}
	if (FAILED(IDXGIOutput_GetDisplayModeList(adapter_output, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &num_modes, modes)))
	{
		GFX_ERROR_CALLBACK("failed to get dispaly mode list");
		goto err;
	}
	for (uint32_t i = 0; i < num_modes; ++i)
	{
		if (modes[i].Width == width && modes[i].Height == height)
		{
			numerator = modes[i].RefreshRate.Numerator;
			denominator = modes[i].RefreshRate.Denominator;
		}
	}
	if (FAILED(IDXGIAdapter_GetDesc(adapter, &adapter_desc)))
	{
		GFX_ERROR_CALLBACK("failed to get desc");
		goto err;
	}
	D3D_WINDOW->swap_chain_desc.BufferCount = 1;
	D3D_WINDOW->swap_chain_desc.BufferDesc.Width = width;
	D3D_WINDOW->swap_chain_desc.BufferDesc.Height = height;
	D3D_WINDOW->swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	D3D_WINDOW->swap_chain_desc.BufferDesc.RefreshRate.Numerator = numerator;
	D3D_WINDOW->swap_chain_desc.BufferDesc.RefreshRate.Denominator = denominator;
	D3D_WINDOW->swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	D3D_WINDOW->swap_chain_desc.OutputWindow = WIN32_WINDOW->window;
	D3D_WINDOW->swap_chain_desc.SampleDesc.Count = 1;
	D3D_WINDOW->swap_chain_desc.SampleDesc.Quality = 0;
	D3D_WINDOW->swap_chain_desc.Windowed = true;
	D3D_WINDOW->swap_chain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	D3D_WINDOW->swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	D3D_WINDOW->swap_chain_desc.Flags = 0;
	IDXGIOutput_Release(adapter_output);
	IDXGIFactory_Release(factory);
	GFX_FREE(modes);
	return window;

err:
	if (adapter_output)
		IDXGIOutput_Release(adapter_output);
	if (factory)
		IDXGIFactory_Release(factory);
	GFX_FREE(modes);
	window->vtable->dtr(window);
	GFX_FREE(window);
	return NULL;
}


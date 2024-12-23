#include "windows/win32.h"

#include <jks/utf16.h>
#include <jks/utf8.h>
#include <windowsx.h>

#ifndef WM_MOUSEHWHEEL
# define WM_MOUSEHWHEEL 0x020E
#endif

static const char *cursors[GFX_CURSOR_LAST] =
{
	[GFX_CURSOR_ARROW] = IDC_ARROW,
	[GFX_CURSOR_CROSS] = IDC_CROSS,
	[GFX_CURSOR_HAND] = IDC_HAND,
	[GFX_CURSOR_IBEAM] = IDC_IBEAM,
	[GFX_CURSOR_NO] = IDC_NO,
	[GFX_CURSOR_SIZEALL] = IDC_SIZEALL,
	[GFX_CURSOR_VRESIZE] = IDC_SIZENS,
	[GFX_CURSOR_HRESIZE] = IDC_SIZEWE,
	[GFX_CURSOR_WAIT] = IDC_WAIT,
	[GFX_CURSOR_BLANK] = NULL,
};

static LRESULT WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
static void update_cursor(struct gfx_win32_window *window);
static uint32_t get_mods(void);
static enum gfx_key_code get_key_code(int key_code);

void gfx_win32_ctr(struct gfx_win32_window *window, struct gfx_window *winref)
{
	window->winref = winref;
	window->window = NULL;
	window->cursor = NULL;
	window->hidden_cursor = false;
	window->mouse_hover = false;
	window->on_resize = NULL;
}

void gfx_win32_dtr(struct gfx_win32_window *window)
{
	DestroyWindow(window->window);
	UnregisterClass(window->classname, window->hinstance);
}

bool gfx_win32_create_window(struct gfx_win32_window *window, const char *title, uint32_t width, uint32_t height)
{
	WNDCLASSEX wc;
	int pos_x, pos_y;

	window->classname = strdup(title);
	if (!window->classname)
	{
		GFX_ERROR_CALLBACK("strdup failed");
		return false;
	}
	window->hinstance = GetModuleHandle(NULL);
	if (!window->hinstance)
	{
		GFX_ERROR_CALLBACK("GetModuleHandle failed");
		return false;
	}
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = window->hinstance;
	wc.hIcon         = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm       = wc.hIcon;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = window->classname;
	wc.cbSize        = sizeof(WNDCLASSEX);
	if (!RegisterClassEx(&wc))
	{
		GFX_ERROR_CALLBACK("RegisterCallEx failed: %lu", (long unsigned)GetLastError());
		return false;
	}
	pos_x = (GetSystemMetrics(SM_CXSCREEN) - width)  / 2;
	pos_y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
	window->window = CreateWindowExA(WS_EX_APPWINDOW, window->classname, title, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_SYSMENU | WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SIZEBOX, pos_x, pos_y, width, height, NULL, NULL, window->hinstance, NULL);
	if (!window->window)
	{
		GFX_ERROR_CALLBACK("CreateWindowEx failed: %lu", (long unsigned)GetLastError());
		return false;
	}
	SetWindowLongPtr(window->window, GWLP_USERDATA, (LONG_PTR)window);
	SetForegroundWindow(window->window);
	SetFocus(window->window);
	ShowCursor(true);
	return true;
}

void gfx_win32_show(struct gfx_win32_window *window)
{
	ShowWindow(window->window, SW_SHOW);
}

void gfx_win32_hide(struct gfx_win32_window *window)
{
	ShowWindow(window->window, SW_HIDE);
}

void gfx_win32_set_title(struct gfx_win32_window *window, const char *title)
{
	SetWindowTextA(window->window, title);
}

static HANDLE build_icon(bool cursor, const void *data, uint32_t width, uint32_t height, uint32_t xhot, uint32_t yhot)
{
	HANDLE icon = NULL;
	char *tmp = NULL;
	HBITMAP mask;
	HBITMAP color;
	ICONINFO iconinfo;
	tmp = GFX_MALLOC(width * height * 4);
	if (!tmp)
		goto end;
	for (size_t i = 0; i < width * height * 4; i += 4)
	{
		((uint8_t*)tmp)[i + 0] = ((uint8_t*)data)[i + 2];
		((uint8_t*)tmp)[i + 1] = ((uint8_t*)data)[i + 1];
		((uint8_t*)tmp)[i + 2] = ((uint8_t*)data)[i + 0];
		((uint8_t*)tmp)[i + 3] = ((uint8_t*)data)[i + 3];
	}
	mask = CreateBitmap(width, height, 1, 1, NULL);
	color = CreateBitmap(width, height, 1, 32, tmp);
	if (!mask || !color)
		goto end;
	iconinfo.fIcon = !cursor;
	iconinfo.xHotspot = xhot;
	iconinfo.yHotspot = yhot;
	iconinfo.hbmMask = mask;
	iconinfo.hbmColor = color;
	icon = CreateIconIndirect(&iconinfo);
end:
	DeleteObject(color);
	DeleteObject(mask);
	GFX_FREE(tmp);
	return icon;
}

void gfx_win32_set_icon(struct gfx_win32_window *window, const void *data, uint32_t width, uint32_t height)
{
	HANDLE icon = build_icon(false, data, width, height, 0, 0);
	SendMessage(window->window, WM_SETICON, ICON_BIG, (LPARAM)icon);
	SendMessage(window->window, WM_SETICON, ICON_SMALL, (LPARAM)icon);
	DestroyIcon(icon);
}

static LRESULT WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	struct gfx_win32_window *window = (struct gfx_win32_window*)GetWindowLongPtrA(hwnd, GWLP_USERDATA);
	switch (message)
	{
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_XBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_XBUTTONUP:
		{
			bool down;
			enum gfx_mouse_button button;
			switch (message)
			{
				case WM_LBUTTONDOWN:
				case WM_RBUTTONDOWN:
				case WM_MBUTTONDOWN:
				case WM_XBUTTONDOWN:
					down = true;
					break;
				default:
					down = false;
					break;
			}
			switch (message)
			{
				case WM_LBUTTONDOWN:
				case WM_LBUTTONUP:
					button = GFX_MOUSE_BUTTON_LEFT;
					break;
				case WM_RBUTTONDOWN:
				case WM_RBUTTONUP:
					button = GFX_MOUSE_BUTTON_RIGHT;
					break;
				case WM_MBUTTONDOWN:
				case WM_MBUTTONUP:
					button = GFX_MOUSE_BUTTON_MIDDLE;
					break;
				default:
					if (GET_XBUTTON_WPARAM(wparam) == XBUTTON1)
						button = (enum gfx_mouse_button)(GFX_MOUSE_BUTTON_MIDDLE + 1);
					else
						button = (enum gfx_mouse_button)(GFX_MOUSE_BUTTON_MIDDLE + 2);
					break;
			}
			if (down)
			{
				if (button >= 0 && button <= GFX_MOUSE_BUTTON_LAST)
					window->winref->mouse_buttons |= 1 << button;
				if (window->winref->mouse_down_callback)
				{
					struct gfx_mouse_event event;
					event.used = false;
					event.x = gfx_get_mouse_x(window->winref);
					event.y = gfx_get_mouse_y(window->winref);
					event.button = button;
					event.mods = get_mods();
					window->winref->mouse_down_callback(window->winref, &event);
				}
			}
			else
			{
				if (button >= 0 && button <= GFX_MOUSE_BUTTON_LAST)
					window->winref->mouse_buttons &= ~(1 << button);
				if (window->winref->mouse_up_callback)
				{
					struct gfx_mouse_event event;
					event.used = false;
					event.x = gfx_get_mouse_x(window->winref);
					event.y = gfx_get_mouse_y(window->winref);
					event.button = button;
					event.mods = get_mods();
					window->winref->mouse_up_callback(window->winref, &event);
				}
			}
			break;
		}
		case WM_CLOSE:
			window->winref->close_requested = true;
			if (window->winref->close_callback)
				window->winref->close_callback(window->winref);
			break;
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			enum gfx_key_code key = get_key_code(HIWORD(lparam) & 0x1FF);
			if (key == GFX_KEY_UNKNOWN)
				break;
			if (window->winref->keys[key / 8] & (1 << (key % 8)))
			{
				if (window->winref->key_press_callback)
				{
					struct gfx_key_event event;
					event.used = false;
					event.key = key;
					event.mods = get_mods();
					window->winref->key_press_callback(window->winref, &event);
				}
			}
			else
			{
				window->winref->keys[key / 8] |= 1 << (key % 8);
				if (window->winref->key_down_callback)
				{
					struct gfx_key_event event;
					event.used = false;
					event.key = key;
					event.mods = get_mods();
					window->winref->key_down_callback(window->winref, &event);
				}
			}
			break;
		}
		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			enum gfx_key_code key = get_key_code(HIWORD(lparam) & 0x1FF);
			if (key == GFX_KEY_UNKNOWN)
				break;
			if (!(window->winref->keys[key / 8] & (1 << (key % 8))))
				break;
			window->winref->keys[key / 8] &= ~(1 << (key % 8));
			if (window->winref->key_up_callback)
			{
				struct gfx_key_event event;
				event.used = false;
				event.key = key;
				event.mods = get_mods();
				window->winref->key_up_callback(window->winref, &event);
			}
			break;
		}
		case WM_MOUSEMOVE:
		{
			if (!window->mouse_hover)
			{
				window->mouse_hover = false;
				TRACKMOUSEEVENT tme;
				tme.cbSize = sizeof(tme);
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = window->window;
				tme.dwHoverTime = 0;
				TrackMouseEvent(&tme);
				if (window->winref->cursor_enter_callback)
					window->winref->cursor_enter_callback(window->winref);
			}
			if (window->winref->grabbed)
				break;
			int x = GET_X_LPARAM(lparam);
			int y = GET_Y_LPARAM(lparam);
			window->winref->mouse_x = x;
			window->winref->mouse_y = y;
			if (window->winref->mouse_move_callback)
			{
				struct gfx_pointer_event event;
				event.used = false;
				event.x = x;
				event.y = y;
				window->winref->mouse_move_callback(window->winref, &event);
			}
			break;
		}
		case WM_INPUT:
		{
			if (!window->winref->grabbed)
				break;
			HRAWINPUT hRawInput = (HRAWINPUT)lparam;
			RAWINPUT ri;
			UINT size = sizeof(ri);
			if (GetRawInputData(hRawInput, RID_INPUT, &ri, &size, sizeof(ri.header)) == (UINT)-1)
			{
				GFX_ERROR_CALLBACK("GetRawInputData failed");
				break;
			}
			if (ri.header.dwType != RIM_TYPEMOUSE)
			{
				GFX_ERROR_CALLBACK("invalid WM_INPUT type: %u", (unsigned int)ri.header.dwType);
				break;
			}
			if (ri.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE)
			{
				window->winref->virtual_x = ri.data.mouse.lLastX;
				window->winref->virtual_y = ri.data.mouse.lLastY;
			}
			else if (ri.data.mouse.lLastX || ri.data.mouse.lLastY)
			{
				window->winref->virtual_x += ri.data.mouse.lLastX;
				window->winref->virtual_y += ri.data.mouse.lLastY;
			}
			else
			{
				GFX_ERROR_CALLBACK("not RELATIVE or ABSOLUTE mouse move: %u", ri.data.mouse.usFlags);
				break;
			}
			if (window->winref->mouse_move_callback)
			{
				struct gfx_pointer_event event;
				event.used = false;
				event.x = window->winref->virtual_x;
				event.y = window->winref->virtual_y;
				window->winref->mouse_move_callback(window->winref, &event);
			}
			break;
		}
		case WM_MOUSELEAVE:
			window->mouse_hover = false;
			if (window->winref->cursor_leave_callback)
				window->winref->cursor_leave_callback(window->winref);
			break;
		case WM_MOUSEHWHEEL:
		{
			if (window->winref->scroll_callback)
			{
				struct gfx_scroll_event event;
				event.used = false;
				event.mouse_x = gfx_get_mouse_x(window->winref);
				event.mouse_y = gfx_get_mouse_y(window->winref);
				event.x = -GET_WHEEL_DELTA_WPARAM(wparam) / (float)WHEEL_DELTA;
				event.y = 0;
				window->winref->scroll_callback(window->winref, &event);
			}
			break;
		}
		case WM_MOUSEWHEEL:
		{
			if (window->winref->scroll_callback)
			{
				struct gfx_scroll_event event;
				event.used = false;
				event.mouse_x = gfx_get_mouse_x(window->winref);
				event.mouse_y = gfx_get_mouse_y(window->winref);
				event.x = 0;
				event.y = GET_WHEEL_DELTA_WPARAM(wparam) / (float)WHEEL_DELTA;
				window->winref->scroll_callback(window->winref, &event);
			}
			break;
		}
		case WM_SETFOCUS:
			if (window->winref->focus_in_callback)
				window->winref->focus_in_callback(window->winref);
			break;
		case WM_KILLFOCUS:
			if (window->winref->focus_out_callback)
				window->winref->focus_out_callback(window->winref);
			break;
		case WM_MOVE:
		{
			int32_t x = GET_X_LPARAM(lparam);
			int32_t y = GET_Y_LPARAM(lparam);
			if (x != window->winref->x || y != window->winref->y)
			{
				window->winref->x = x;
				window->winref->y = y;
				if (window->winref->move_callback)
				{
					struct gfx_move_event event;
					event.used = false;
					event.x = x;
					event.y = y;
					window->winref->move_callback(window->winref, &event);
				}
			}
			break;
		}
		case WM_SIZE:
		{
			int32_t width = LOWORD(lparam);
			int32_t height = HIWORD(lparam);
			if (width != window->winref->width || height != window->winref->height)
			{
				window->winref->width = width;
				window->winref->height = height;
				if (window->on_resize)
					window->on_resize(window->winref);
				if (window->winref->resize_callback)
				{
					struct gfx_resize_event event;
					event.used = false;
					event.width = width;
					event.height = height;
					window->winref->resize_callback(window->winref, &event);
				}
			}
			break;
		}
		case WM_SETCURSOR:
		{
			if (LOWORD(lparam) == HTCLIENT)
			{
				update_cursor(window);
				return true;
			}
			break;
		}
		case WM_CHAR:
		{
			if (wparam == UNICODE_NOCHAR)
				return true;
			if (window->winref->char_callback)
			{
				struct gfx_char_event event;
				event.used = false;
				char tmp[2];
				tmp[0] = wparam >> 8;
				tmp[1] = wparam;
				char *iter = tmp;
				memset(event.utf8, 0, sizeof(event.utf8));
				if (utf16_decode((const char**)&iter, &event.codepoint))
				{
					iter = event.utf8;
					if (utf8_encode(&iter, event.codepoint))
					{
						window->winref->char_callback(window->winref, &event);
					}
					else
					{
						GFX_ERROR_CALLBACK("invalid event codepoint");
					}
				}
				else
				{
					GFX_ERROR_CALLBACK("invalid event codepoint");
				}
			}
			break;
		}
		case WM_UNICHAR:
		{
			if (wparam == UNICODE_NOCHAR)
				return true;
			if (window->winref->char_callback)
			{
				struct gfx_char_event event;
				event.used = false;
				event.codepoint = wparam;
				char *iter = event.utf8;
				if (utf8_encode(&iter, event.codepoint))
				{
					window->winref->char_callback(window->winref, &event);
				}
				else
				{
					GFX_ERROR_CALLBACK("invalid event codepoint");
				}
			}
			break;
		}
		case WM_NCHITTEST:
			/* FALLTHROUGH */
		case WM_SETTEXT:
			/* FALLTHROUGH */
		case WM_WINDOWPOSCHANGING:
			/* FALLTHROUGH */
		case WM_WINDOWPOSCHANGED:
			/* FALLTHROUGH */
		case WM_NCACTIVATE:
			/* FALLTHROUGH */
		case WM_DESTROY:
			/* FALLTHROUGH */
		case WM_ACTIVATE:
			/* FALLTHROUGH */
		case WM_ACTIVATEAPP:
			/* FALLTHROUGH */
		case WM_NCDESTROY:
			return DefWindowProc(hwnd, message, wparam, lparam);
		default:
			GFX_ERROR_CALLBACK("unhandled window message: %u", message);
			return DefWindowProc(hwnd, message, wparam, lparam);
	}
	return 0;
}

void gfx_win32_poll_events(struct gfx_win32_window *window)
{
	(void)window;
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void gfx_win32_wait_events(struct gfx_win32_window *window)
{
	WaitMessage();
	gfx_win32_poll_events(window);
}

void gfx_win32_resize(struct gfx_win32_window *window, uint32_t width, uint32_t height)
{
	SetWindowPos(window->window, HWND_TOP, 0, 0, width, height, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);
}

void gfx_win32_grab_cursor(struct gfx_win32_window *window)
{
	RAWINPUTDEVICE rid;
	rid.usUsagePage = 1;
	rid.usUsage = 2;
	rid.dwFlags = 0;
	rid.hwndTarget = window->window;
	RegisterRawInputDevices(&rid, 1, sizeof(rid));
	window->hidden_cursor = true;
	update_cursor(window);
	RECT rect;
	GetClientRect(window->window, &rect);
	ClientToScreen(window->window, (POINT*)&rect.left);
	ClientToScreen(window->window, (POINT*)&rect.right);
	ClipCursor(&rect);
	window->winref->grabbed = true;
	window->winref->virtual_x = window->winref->mouse_x;
	window->winref->virtual_y = window->winref->mouse_y;
	POINT point;
	GetCursorPos(&point);
	ScreenToClient(window->window, &point);
	window->prev_mouse_x = point.x;
	window->prev_mouse_y = point.y;
}

void gfx_win32_ungrab_cursor(struct gfx_win32_window *window)
{
	RAWINPUTDEVICE rid;
	rid.usUsagePage = 1;
	rid.usUsage = 2;
	rid.dwFlags = RIDEV_REMOVE;
	rid.hwndTarget = NULL;
	RegisterRawInputDevices(&rid, 1, sizeof(rid));
	window->hidden_cursor = false;
	update_cursor(window);
	ClipCursor(NULL);
	POINT point;
	point.x = window->prev_mouse_x;
	point.y = window->prev_mouse_y;
	ClientToScreen(window->window, &point);
	gfx_win32_set_mouse_position(window, point.x, point.y);
	window->winref->grabbed = false;
}

char *gfx_win32_get_clipboard(struct gfx_win32_window *window)
{
	if (!OpenClipboard(window->window))
		return NULL;
	HANDLE object = GetClipboardData(CF_TEXT); // CF_UNICODETEXT
	char *str = GlobalLock(object);
	char *ret;
	if (str != NULL)
	{
		ret = strdup(str);
		GlobalUnlock(str);
	}
	else
	{
		ret = NULL;
	}
	CloseClipboard();
	return ret;
}

void gfx_win32_set_clipboard(struct gfx_win32_window *window, const char *text)
{
	if (!OpenClipboard(window->window))
		return;
	HANDLE object = GlobalAlloc(GMEM_MOVEABLE, strlen(text) + 1);
	if (!object)
	{
		CloseClipboard();
		return;
	}
	char *str = GlobalLock(object);
	if (str)
	{
		strcpy(str, text);
		GlobalUnlock(str);
	}
	EmptyClipboard();
	SetClipboardData(CF_TEXT, object); //CF_UNICODETEXT
	CloseClipboard();
}

gfx_cursor_t gfx_win32_create_native_cursor(struct gfx_win32_window *window, enum gfx_native_cursor cursor)
{
	(void)window;
	return LoadCursor(NULL, cursors[cursor]);
}

gfx_cursor_t gfx_win32_create_cursor(struct gfx_win32_window *window, const void *data, uint32_t width, uint32_t height, uint32_t xhot, uint32_t yhot)
{
	(void)window;
	return build_icon(true, data, width, height, xhot, yhot);
}

void gfx_win32_delete_cursor(struct gfx_win32_window *window, gfx_cursor_t cursor)
{
	(void)window;
	if (!cursor)
		return;
	DestroyCursor((HCURSOR)cursor);
}

void gfx_win32_set_cursor(struct gfx_win32_window *window, gfx_cursor_t cursor)
{
	window->cursor = cursor;
	update_cursor(window);
}

void gfx_win32_set_mouse_position(struct gfx_win32_window *window, int32_t x, int32_t y)
{
	(void)window;
	SetCursorPos(x, y);
}

static void update_cursor(struct gfx_win32_window *window)
{
	if (window->hidden_cursor)
	{
		SetCursor(NULL);
		return;
	}
	SetCursor(window->cursor);
}

static enum gfx_key_code get_key_code(int key_code)
{
	switch (key_code)
	{
		case 0x1E:  return GFX_KEY_A;
		case 0x30:  return GFX_KEY_B;
		case 0x2E:  return GFX_KEY_C;
		case 0x20:  return GFX_KEY_D;
		case 0x12:  return GFX_KEY_E;
		case 0x21:  return GFX_KEY_F;
		case 0x22:  return GFX_KEY_G;
		case 0x23:  return GFX_KEY_H;
		case 0x17:  return GFX_KEY_I;
		case 0x24:  return GFX_KEY_J;
		case 0x25:  return GFX_KEY_K;
		case 0x26:  return GFX_KEY_L;
		case 0x32:  return GFX_KEY_M;
		case 0x31:  return GFX_KEY_N;
		case 0x18:  return GFX_KEY_O;
		case 0x19:  return GFX_KEY_P;
		case 0x10:  return GFX_KEY_Q;
		case 0x13:  return GFX_KEY_R;
		case 0x1f:  return GFX_KEY_S;
		case 0x14:  return GFX_KEY_T;
		case 0x16:  return GFX_KEY_U;
		case 0x2f:  return GFX_KEY_V;
		case 0x11:  return GFX_KEY_W;
		case 0x2d:  return GFX_KEY_X;
		case 0x15:  return GFX_KEY_Y;
		case 0x2c:  return GFX_KEY_Z;
		case 0xB:   return GFX_KEY_0;
		case 0x2:   return GFX_KEY_1;
		case 0x3:   return GFX_KEY_2;
		case 0x4:   return GFX_KEY_3;
		case 0x5:   return GFX_KEY_4;
		case 0x6:   return GFX_KEY_5;
		case 0x7:   return GFX_KEY_6;
		case 0x8:   return GFX_KEY_7;
		case 0x9:   return GFX_KEY_8;
		case 0xA:   return GFX_KEY_9;
		case 0x52:  return GFX_KEY_KP_0;
		case 0x4F:  return GFX_KEY_KP_1;
		case 0x50:  return GFX_KEY_KP_2;
		case 0x51:  return GFX_KEY_KP_3;
		case 0x4B:  return GFX_KEY_KP_4;
		case 0x4C:  return GFX_KEY_KP_5;
		case 0x4D:  return GFX_KEY_KP_6;
		case 0x47:  return GFX_KEY_KP_7;
		case 0x48:  return GFX_KEY_KP_8;
		case 0x49:  return GFX_KEY_KP_9;
		case 0x135: return GFX_KEY_KP_DIVIDE;
		case 0x37:  return GFX_KEY_KP_MULTIPLY;
		case 0x4A:  return GFX_KEY_KP_SUBTRACT;
		case 0x4E:  return GFX_KEY_KP_ADD;
		case 0x59:  return GFX_KEY_KP_EQUAL;
		case 0x53:  return GFX_KEY_KP_DECIMAL;
		case 0x11C: return GFX_KEY_KP_ENTER;
		case 0x3B:  return GFX_KEY_F1;
		case 0x3C:  return GFX_KEY_F2;
		case 0x3D:  return GFX_KEY_F3;
		case 0x3E:  return GFX_KEY_F4;
		case 0x3F:  return GFX_KEY_F5;
		case 0x40:  return GFX_KEY_F6;
		case 0x41:  return GFX_KEY_F7;
		case 0x42:  return GFX_KEY_F8;
		case 0x43:  return GFX_KEY_F9;
		case 0x44:  return GFX_KEY_F10;
		case 0x57:  return GFX_KEY_F11;
		case 0x58:  return GFX_KEY_F12;
		case 0x64:  return GFX_KEY_F13;
		case 0x65:  return GFX_KEY_F14;
		case 0x66:  return GFX_KEY_F15;
		case 0x67:  return GFX_KEY_F16;
		case 0x68:  return GFX_KEY_F17;
		case 0x69:  return GFX_KEY_F18;
		case 0x6A:  return GFX_KEY_F19;
		case 0x6B:  return GFX_KEY_F20;
		case 0x6C:  return GFX_KEY_F21;
		case 0x6D:  return GFX_KEY_F22;
		case 0x6E:  return GFX_KEY_F23;
		case 0x76:  return GFX_KEY_F24;
		case 0x2A:  return GFX_KEY_LSHIFT;
		case 0x36:  return GFX_KEY_RSHIFT;
		case 0x1D:  return GFX_KEY_LCONTROL;
		case 0x11D: return GFX_KEY_RCONTROL;
		case 0x38:  return GFX_KEY_LALT;
		case 0x138: return GFX_KEY_RALT;
		case 0x15B: return GFX_KEY_LSUPER;
		case 0x15C: return GFX_KEY_RSUPER;
		case 0x14B: return GFX_KEY_LEFT;
		case 0x14D: return GFX_KEY_RIGHT;
		case 0x148: return GFX_KEY_UP;
		case 0x150: return GFX_KEY_DOWN;
		case 0x39:  return GFX_KEY_SPACE;
		case 0xE:   return GFX_KEY_BACKSPACE;
		case 0x1C:  return GFX_KEY_ENTER;
		case 0xF:   return GFX_KEY_TAB;
		case 0x1:   return GFX_KEY_ESCAPE;
		case 0x45:  return GFX_KEY_PAUSE;
		case 0x153: return GFX_KEY_DELETE;
		case 0x152: return GFX_KEY_INSERT;
		case 0x147: return GFX_KEY_HOME;
		case 0x149: return GFX_KEY_PAGE_UP;
		case 0x151: return GFX_KEY_PAGE_DOWN;
		case 0x14F: return GFX_KEY_END;
		case 0x33:  return GFX_KEY_COMMA;
		case 0x34:  return GFX_KEY_PERIOD;
		case 0x35:  return GFX_KEY_SLASH;
		case 0x28:  return GFX_KEY_APOSTROPHE;
		case 0x27:  return GFX_KEY_SEMICOLON;
		case 0x29:  return GFX_KEY_GRAVE;
		case 0x1A:  return GFX_KEY_LBRACKET;
		case 0x1B:  return GFX_KEY_RBRACKET;
		case 0x2B:  return GFX_KEY_BACKSLASH;
		case 0xD:   return GFX_KEY_EQUAL;
		case 0xC:   return GFX_KEY_SUBTRACT;
		case 0x46:  return GFX_KEY_SCROLL_LOCK;
		case 0x145: return GFX_KEY_NUM_LOCK;
		case 0x3A:  return GFX_KEY_CAPS_LOCK;
		case 0x137: return GFX_KEY_PRINT;
	}
	return GFX_KEY_UNKNOWN;
}

static uint32_t get_mods(void)
{
	uint32_t mods = 0;
	if (GetKeyState(VK_CONTROL) & 0x8000)
		mods |= GFX_KEY_MOD_CONTROL;
	if (GetKeyState(VK_SHIFT) & 0x8000)
		mods |= GFX_KEY_MOD_SHIFT;
	if (GetKeyState(VK_CAPITAL) & 0x1)
		mods |= GFX_KEY_MOD_CAPS_LOCK;
	if (GetKeyState(VK_MENU) & 0x8000)
		mods |= GFX_KEY_MOD_ALT;
	if (GetKeyState(VK_NUMLOCK) & 0x1)
		mods |= GFX_KEY_MOD_NUM_LOCK;
	if ((GetKeyState(VK_LWIN) & 0x8000) | (GetKeyState(VK_RWIN) & 0x8000))
		mods |= GFX_KEY_MOD_SUPER;
	return mods;
}

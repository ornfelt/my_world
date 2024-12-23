#include "Window.h"
#include "../GL.h"
#include <libunicode/utf8.h>
#include <iostream>
#include <cstring>
#include <cmath>

namespace librender
{

	int32_t Window::maxMSAA = -1;

	Window::Window(std::string title, int width, int height)
	: focused(true)
	, mouseX(0)
	, mouseY(0)
	, width(width)
	, height(height)
	{
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		this->semiDiag = std::sqrt(this->width * this->width + this->height * this->height);
		this->fullscreen = false;
		if (!(this->window = glfwCreateWindow(this->width, this->height, title.c_str(), nullptr, nullptr)))
			throw std::exception();
		glfwMakeContextCurrent(this->window);
		glfwSetWindowUserPointer(this->window, this);
		this->currentCursor = GLFW_ARROW_CURSOR;
		this->hResizeCursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
		this->vResizeCursor = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
		this->arrowCursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
		this->crossCursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
		this->ibeamCursor = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
		this->handCursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
		glfwSetCursorPosCallback(this->window, EventsManager::cursorListener);
		glfwSetFramebufferSizeCallback(this->window, EventsManager::windowResizeListener);
		glfwSetWindowFocusCallback(this->window, EventsManager::windowFocusListener);
		/*int count = 0;
		GLFWmonitor** monitors = glfwGetMonitors(&count);
		for (int i = 0; i < count; ++i)
		{
			int mWidth = 0;
			int mHeight = 0;
			glfwGetMonitorPhysicalSize(monitors[i], &mWidth, &mHeight);
			std::cout << glfwGetMonitorName(monitors[i]) << " (" << mWidth << "x" << mHeight << ") :" << std::endl;
			int modesCount = 0;
			const GLFWvidmode *modes = glfwGetVideoModes(monitors[i], &modesCount);
			for (int j = 0; j < modesCount; ++j)
			{
				std::cout << "width: " << modes[j].width << ", height: " << modes[j].height << ", redBits: " << modes[j].redBits << ", greenBits: " << modes[j].greenBits << ", blueBits: " << modes[j].blueBits << ", refreshRate: " << modes[j].refreshRate << std::endl;
			}
			std::cout << std::endl;
		}*/
	}

	Window::~Window()
	{
		if (this->window)
			glfwDestroyWindow(this->window);
		if (this->hResizeCursor)
			glfwDestroyCursor(this->hResizeCursor);
		if (this->vResizeCursor)
			glfwDestroyCursor(this->vResizeCursor);
		if (this->arrowCursor)
			glfwDestroyCursor(this->arrowCursor);
		if (this->crossCursor)
			glfwDestroyCursor(this->crossCursor);
		if (this->ibeamCursor)
			glfwDestroyCursor(this->ibeamCursor);
		if (this->handCursor)
			glfwDestroyCursor(this->handCursor);
	}

	void Window::show()
	{
		glfwShowWindow(this->window);
	}

	void Window::setTitle(std::string &title)
	{
		glfwSetWindowTitle(this->window, title.c_str());
	}

	void Window::setIcon(char *data, uint32_t width, uint32_t height)
	{
		GLFWimage img = {static_cast<int>(width), static_cast<int>(height), reinterpret_cast<unsigned char*>(data)};
		glfwSetWindowIcon(this->window, 1, &img);
	}

	void Window::enableFullscreen()
	{
		if (this->fullscreen)
			return;
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		this->fullscreen = true;
		this->prevWidth = this->width;
		this->prevHeight = this->height;
		glfwSetWindowMonitor(this->window, monitor, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
	}

	void Window::disableFullscreen()
	{
		if (!this->fullscreen)
			return;
		setBorders(true);
		glfwSetWindowMonitor(window, nullptr, 0, 0, this->prevWidth, this->prevHeight, GLFW_DONT_CARE);
		this->fullscreen = false;
	}

	void Window::switchFullscreen()
	{
		if (this->fullscreen)
			disableFullscreen();
		else
			enableFullscreen();
	}

	void Window::maximize()
	{
		glfwMaximizeWindow(this->window);
	}

	void Window::resized(int width, int height)
	{
		this->semiDiag = std::sqrt(width * width + height * height) / 2;
		this->width = width;
		this->height = height;
	}

	bool Window::isButtonDown(int button)
	{
		return glfwGetMouseButton(this->window, button) == GLFW_PRESS;
	}

	bool Window::isKeyDown(int key)
	{
		return glfwGetKey(this->window, key) == GLFW_PRESS;
	}

	std::string Window::getClipboard()
	{
		std::string str = glfwGetClipboardString(this->window);
		if (!utf8::is_valid(str.begin(), str.end()))
		{
			std::string tmp;
			utf8::replace_invalid(str.begin(), str.end(), back_inserter(tmp));
			str = tmp;
		}
		return str;
	}

	void Window::setClipboard(std::string str)
	{
		glfwSetClipboardString(this->window, str.c_str());
	}

	bool Window::closeRequested()
	{
		return glfwWindowShouldClose(this->window);
	}

	void Window::setBorders(bool borders)
	{
		glfwWindowHint(GLFW_DECORATED, borders ? GL_TRUE : GL_FALSE);
	}

	void Window::setResizable(bool resizable)
	{
		glfwWindowHint(GLFW_RESIZABLE, resizable ? GL_TRUE : GL_FALSE);
	}

	void Window::setVSync(bool vsync)
	{
		glfwSwapInterval(vsync ? 1 : 0);
	}

	void Window::update()
	{
		glfwSwapBuffers(this->window);
	}

	void Window::pollEvents()
	{
		glfwPollEvents();
	}

	void Window::setHResizeCursor()
	{
		if (this->currentCursor != GLFW_HRESIZE_CURSOR)
		{
			this->currentCursor = GLFW_HRESIZE_CURSOR;
			glfwSetCursor(this->window, this->hResizeCursor);
		}
	}

	void Window::setVResizeCursor()
	{
		if (this->currentCursor != GLFW_VRESIZE_CURSOR)
		{
			this->currentCursor = GLFW_VRESIZE_CURSOR;
			glfwSetCursor(window, this->vResizeCursor);
		}
	}

	void Window::setArrowCursor()
	{
		if (this->currentCursor != GLFW_ARROW_CURSOR)
		{
			this->currentCursor = GLFW_ARROW_CURSOR;
			glfwSetCursor(window, this->arrowCursor);
		}
	}

	void Window::setCrossCursor()
	{
		if (this->currentCursor != GLFW_CROSSHAIR_CURSOR)
		{
			this->currentCursor = GLFW_CROSSHAIR_CURSOR;
			glfwSetCursor(window, this->crossCursor);
		}
	}

	void Window::setIbeamCursor()
	{
		if (this->currentCursor != GLFW_IBEAM_CURSOR)
		{
			this->currentCursor = GLFW_IBEAM_CURSOR;
			glfwSetCursor(window, this->ibeamCursor);
		}
	}

	void Window::setHandCursor()
	{
		if (this->currentCursor != GLFW_HAND_CURSOR)
		{
			this->currentCursor = GLFW_HAND_CURSOR;
			glfwSetCursor(window, this->handCursor);
		}
	}

	void Window::setCursor(enum WindowCursor cursor)
	{
		switch (cursor)
		{
			case WINDOW_CURSOR_HRESIZE:
				setHResizeCursor();
				break;
			case WINDOW_CURSOR_VRESIZE:
				setVResizeCursor();
				break;
			case WINDOW_CURSOR_ARROW:
				setArrowCursor();
				break;
			case WINDOW_CURSOR_CROSS:
				setCrossCursor();
				break;
			case WINDOW_CURSOR_IBEAM:
				setIbeamCursor();
				break;
			case WINDOW_CURSOR_HAND:
				setHandCursor();
				break;
		}
	}

	void Window::setFocused(bool focused)
	{
		this->focused = focused;
	}

	void Window::setWindowResizedCallback(WindowResizedCallback callback)
	{
		//Window resize is always set in constructor
		this->eventsManager.setWindowResizedCallback(callback);
	}

	void Window::setScrollCallback(ScrollCallback callback)
	{
		this->eventsManager.setScrollCallback(callback);
		if (!callback)
		{
			glfwSetScrollCallback(this->window, nullptr);
			return;
		}
		glfwSetScrollCallback(this->window, EventsManager::scrollListener);
	}

	void Window::setMouseMoveCallback(MouseMoveCallback callback)
	{
		//Mouse move is always set in constructor
		this->eventsManager.setMouseMoveCallback(callback);
	}

	void Window::setMouseDownCallback(MouseDownCallback callback)
	{
		this->eventsManager.setMouseDownCallback(callback);
		if (!callback && !this->eventsManager.getMouseUpCallback())
		{
			glfwSetMouseButtonCallback(this->window, nullptr);
			return;
		}
		glfwSetMouseButtonCallback(this->window, EventsManager::mouseListener);
	}

	void Window::setMouseUpCallback(MouseUpCallback callback)
	{
		this->eventsManager.setMouseUpCallback(callback);
		if (!callback && !this->eventsManager.getMouseDownCallback())
		{
			glfwSetMouseButtonCallback(this->window, nullptr);
			return;
		}
		glfwSetMouseButtonCallback(this->window, EventsManager::mouseListener);
	}

	void Window::setKeyDownCallback(KeyDownCallback callback)
	{
		this->eventsManager.setKeyDownCallback(callback);
		if (!callback && !this->eventsManager.getKeyPressCallback() && !this->eventsManager.getKeyUpCallback())
		{
			glfwSetKeyCallback(this->window, nullptr);
			return;
		}
		glfwSetKeyCallback(this->window, EventsManager::keyListener);
	}

	void Window::setKeyPressCallback(KeyPressCallback callback)
	{
		this->eventsManager.setKeyPressCallback(callback);
		if (!callback && !this->eventsManager.getKeyDownCallback() && !this->eventsManager.getKeyUpCallback())
		{
			glfwSetKeyCallback(this->window, nullptr);
			return;
		}
		glfwSetKeyCallback(this->window, EventsManager::keyListener);
	}

	void Window::setKeyUpCallback(KeyUpCallback callback)
	{
		this->eventsManager.setKeyUpCallback(callback);
		if (!callback && !this->eventsManager.getKeyDownCallback() && !this->eventsManager.getKeyPressCallback())
		{
			glfwSetKeyCallback(this->window, nullptr);
			return;
		}
		glfwSetKeyCallback(this->window, EventsManager::keyListener);
	}

	void Window::setCharCallback(CharCallback callback)
	{
		this->eventsManager.setCharCallback(callback);
		if (!callback)
		{
			glfwSetCharCallback(this->window, nullptr);
			return;
		}
		glfwSetCharCallback(this->window, EventsManager::charListener);
	}

	uint8_t Window::getMaxMSAA()
	{
		if (maxMSAA == -1)
		{
			glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
			glfwWindowHint(GLFW_SAMPLES, 128);
			GLFWwindow *testWindow = glfwCreateWindow(1, 1, "", nullptr, nullptr);
			glfwMakeContextCurrent(testWindow);
			glGetIntegerv(GL_SAMPLES, &maxMSAA);
			glfwDestroyWindow(testWindow);
		}
		if (maxMSAA == -1)
			return 0;
		return (uint8_t)maxMSAA;
	}

}

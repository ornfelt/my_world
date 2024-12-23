#include "EventsManager.h"
#include "./Window.h"
#include "../GL.h"
#include <libunicode/utf8.h>
#include <iostream>
#include <vector>

namespace librender
{

	EventsManager::EventsManager()
	: windowResizedCallback(nullptr)
	, scrollCallback(nullptr)
	, mouseMoveCallback(nullptr)
	, mouseDownCallback(nullptr)
	, mouseUpCallback(nullptr)
	, keyDownCallback(nullptr)
	, keyPressCallback(nullptr)
	, keyUpCallback(nullptr)
	, charCallback(nullptr)
	{
	}

	void EventsManager::charListener(GLFWwindow *glfwWindow, unsigned int codepoint)
	{
		Window *window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
		try
		{
			std::string result(4, 0);
			std::string::iterator iter(result.begin());
			utf8::encode(iter, codepoint);
			CharEvent event;
			event.charcode = const_cast<char*>(result.c_str());
			if (window->getEventsManager().charCallback)
				window->getEventsManager().charCallback(event);
		}
		catch (std::exception &e)
		{
			std::cout << e.what() << std::endl;
		}
	}

	void EventsManager::keyListener(GLFWwindow *glfwWindow, int key, int scancode, int action, int mods)
	{
		Window *window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
		try
		{
			KeyEvent event;
			event.key = key;
			event.scancode = scancode;
			event.mods = mods;
			if (action == GLFW_PRESS)
			{
				if (window->getEventsManager().keyDownCallback)
					window->getEventsManager().keyDownCallback(event);
				if (window->getEventsManager().keyPressCallback)
					window->getEventsManager().keyPressCallback(event);
			}
			else if (action == GLFW_REPEAT)
			{
				if (window->getEventsManager().keyPressCallback)
					window->getEventsManager().keyPressCallback(event);
			}
			else if (action == GLFW_RELEASE)
			{
				if (window->getEventsManager().keyUpCallback)
					window->getEventsManager().keyUpCallback(event);
			}
		}
		catch (std::exception &e)
		{
			std::cout << e.what() << std::endl;
		}
	}

	void EventsManager::scrollListener(GLFWwindow *glfwWindow, double xOffset, double yOffset)
	{
		Window *window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
		ScrollEvent event;
		event.yScroll = yOffset;
		event.xScroll = xOffset;
		if (window->getEventsManager().scrollCallback)
			window->getEventsManager().scrollCallback(event);
	}

	void EventsManager::mouseListener(GLFWwindow *glfwWindow, int button, int action, int mods)
	{
		Window *window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
		MouseEvent event;
		event.button = button;
		event.mods = mods;
		if (action == GLFW_PRESS)
		{
			if (window->getEventsManager().mouseDownCallback)
				window->getEventsManager().mouseDownCallback(event);
		}
		else if (action == GLFW_RELEASE)
		{
			if (window->getEventsManager().mouseUpCallback)
				window->getEventsManager().mouseUpCallback(event);
		}
	}

	void EventsManager::cursorListener(GLFWwindow *glfwWindow, double x, double y)
	{
		Window *window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
		window->setMouseX(x);
		window->setMouseY(y);
		if (window->getEventsManager().mouseMoveCallback)
			window->getEventsManager().mouseMoveCallback();
	}

	void EventsManager::windowResizeListener(GLFWwindow *glfwWindow, int width, int height)
	{
		Window *window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
		window->resized(width, height);
		if (window->getEventsManager().windowResizedCallback)
			window->getEventsManager().windowResizedCallback();
	}

	void EventsManager::windowFocusListener(GLFWwindow *glfwWindow, int focused)
	{
		Window *window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
		window->setFocused(focused == GLFW_TRUE);
	}

}

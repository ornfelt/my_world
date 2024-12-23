#ifndef LIBRENDER_EVENTS_LISTENER_H
# define LIBRENDER_EVENTS_LISTENER_H

# include "./Events.h"

struct GLFWwindow;

namespace librender
{

	typedef void (*WindowResizedCallback)();
	typedef void (*ScrollCallback)(ScrollEvent &event);
	typedef void (*MouseMoveCallback)();
	typedef void (*MouseDownCallback)(MouseEvent &event);
	typedef void (*MouseUpCallback)(MouseEvent &event);
	typedef void (*KeyDownCallback)(KeyEvent &event);
	typedef void (*KeyPressCallback)(KeyEvent &event);
	typedef void (*KeyUpCallback)(KeyEvent &event);
	typedef void (*CharCallback)(CharEvent &event);

	class EventsManager
	{

	private:
		WindowResizedCallback windowResizedCallback;
		ScrollCallback scrollCallback;
		MouseMoveCallback mouseMoveCallback;
		MouseDownCallback mouseDownCallback;
		MouseUpCallback mouseUpCallback;
		KeyDownCallback keyDownCallback;
		KeyPressCallback keyPressCallback;
		KeyUpCallback keyUpCallback;
		CharCallback charCallback;

	public:
		EventsManager();
		inline void setWindowResizedCallback(WindowResizedCallback callback) {this->windowResizedCallback = callback;};
		inline WindowResizedCallback getWindowResizeCallback() {return this->windowResizedCallback;};
		inline void setScrollCallback(ScrollCallback callback) {this->scrollCallback = callback;};
		inline ScrollCallback getScrollCallback() {return this->scrollCallback;};
		inline void setMouseMoveCallback(MouseMoveCallback callback) {this->mouseMoveCallback = callback;};
		inline MouseMoveCallback getMouseMoveCallback() {return this->mouseMoveCallback;};
		inline void setMouseDownCallback(MouseDownCallback callback) {this->mouseDownCallback = callback;};
		inline MouseDownCallback getMouseDownCallback() {return this->mouseDownCallback;};
		inline void setMouseUpCallback(MouseUpCallback callback) {this->mouseUpCallback = callback;};
		inline MouseUpCallback getMouseUpCallback() {return this->mouseUpCallback;};
		inline void setKeyDownCallback(KeyDownCallback callback) {this->keyDownCallback = callback;};
		inline KeyDownCallback getKeyDownCallback() {return this->keyDownCallback;};
		inline void setKeyPressCallback(KeyPressCallback callback) {this->keyPressCallback = callback;};
		inline KeyPressCallback getKeyPressCallback() {return this->keyPressCallback;};
		inline void setKeyUpCallback(KeyUpCallback callback) {this->keyUpCallback = callback;};
		inline KeyUpCallback getKeyUpCallback() {return this->keyUpCallback;};
		inline void setCharCallback(CharCallback callback) {this->charCallback = callback;};
		inline CharCallback getCharCallback() {return this->charCallback;};
		static void charListener(GLFWwindow *glfwWindow, unsigned int codepoint);
		static void keyListener(GLFWwindow *glfwWindow, int key, int scancode, int action, int mods);
		static void scrollListener(GLFWwindow *glfwWindow, double xOffset, double yOffset);
		static void mouseListener(GLFWwindow *glfwWindow, int button, int action, int mods);
		static void cursorListener(GLFWwindow *glfwWindow, double x, double y);
		static void windowResizeListener(GLFWwindow *glfwWindow, int width, int height);
		static void windowFocusListener(GLFWwindow *glfwWindow, int focused);

	};

}

#endif

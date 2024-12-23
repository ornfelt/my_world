#include "Monitor.h"
#include "../GL.h"

namespace librender
{

	Monitor::Monitor(GLFWmonitor *handle)
	: handle(handle)
	{
		glfwGetMonitorPhysicalSize(this->handle, &this->width, &this->height);
		this->name = glfwGetMonitorName(this->handle);
		int videoModesCount = 0;
		const GLFWvidmode *videoModes = glfwGetVideoModes(this->handle, &videoModesCount);
		for (int i = 0; i < videoModesCount; ++i)
		{
			const GLFWvidmode &videoMode = videoModes[i];
			this->videoModes.push_back(VideoMode(videoMode.refreshRate, videoMode.redBits, videoMode.greenBits, videoMode.blueBits, videoMode.width, videoMode.height));
		}
	}

}

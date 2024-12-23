#ifndef LIBRENDER_MONITOR_H
# define LIBRENDER_MONITOR_H

# include "./VideoMode.h"
# include <cstdint>
# include <string>
# include <vector>

struct GLFWmonitor;

namespace librender
{

	class Monitor
	{

	private:
		std::vector<VideoMode> videoModes;
		GLFWmonitor *handle;
		std::string name;
		int32_t width;
		int32_t height;

	public:
		Monitor(GLFWmonitor *handle);
		std::vector<VideoMode> &getVideoModes() {return this->videoModes;};
		std::string &getName() {return this->name;};
		inline int32_t getWidth() {return this->width;};
		inline int32_t getHeight() {return this->height;};

	};

}

#endif

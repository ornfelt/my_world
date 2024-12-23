#include "Device.h"

namespace libaudio
{

	Device::Device(PaDeviceIndex index)
	: device(Pa_GetDeviceInfo(index))
	, index(index)
	{
	}

	Device::~Device()
	{
	}

	std::string Device::getName() const
	{
		if (!this->device || !this->device->name)
			return "";
		return this->device->name;
	}

	std::vector<Device> Device::getDevices()
	{
		std::vector<Device> devices;
		for (PaDeviceIndex i = 0; i < Pa_GetDeviceCount(); ++i)
			devices.emplace_back(i);
		return devices;
	}

	std::vector<Device> Device::getInputDevices()
	{
		std::vector<Device> devices;
		for (PaDeviceIndex i = 0; i < Pa_GetDeviceCount(); ++i)
		{
			const PaDeviceInfo *info = Pa_GetDeviceInfo(i);
			if (info->maxInputChannels > 0)
				devices.emplace_back(i);
		}
		return devices;
	}

	std::vector<Device> Device::getOutputDevices()
	{
		std::vector<Device> devices;
		for (PaDeviceIndex i = 0; i < Pa_GetDeviceCount(); ++i)
		{
			const PaDeviceInfo *info = Pa_GetDeviceInfo(i);
			if (info->maxOutputChannels > 0)
				devices.emplace_back(i);
		}
		return devices;
	}

	Device *Device::getDefaultInputDevice()
	{
		PaDeviceIndex index = Pa_GetDefaultInputDevice();
		if (index == paNoDevice)
			return nullptr;
		return new Device(index);
	}

	Device *Device::getDefaultOutputDevice()
	{
		PaDeviceIndex index = Pa_GetDefaultOutputDevice();
		if (index == paNoDevice)
			return nullptr;
		return new Device(index);
	}

}

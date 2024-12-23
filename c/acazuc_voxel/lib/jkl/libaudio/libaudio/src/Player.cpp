#include "Player.h"
#include <algorithm>
#include <vector>

namespace libaudio
{

	Player::Player()
	: pitch(1)
	, gain(1)
	, pan(.5)
	, active(false)
	, loop(false)
	{
	}

	Player::~Player()
	{
	}

	bool Player::callback(float *out, unsigned long frameCount, uint32_t sampling)
	{
		if (!frameCount)
			return false;
		if (!this->active)
			return false;
		getData(out, frameCount * 2, this->sampling / static_cast<float>(sampling) * frameCount * 2 * this->pitch);
		if (this->gain == 0)
			return false;
		for (size_t i = 0; i < this->filters.size(); ++i)
			this->filters[i]->filter(out, frameCount);
		if (this->gain != 1)
		{
			for (size_t i = 0; i < frameCount * 2; ++i)
				out[i] *= this->gain;
		}
		if (this->pan != .5)
		{
			for (size_t i = 0; i < frameCount; ++i)
			{
				out[i * 2 + 0] *= std::max(0.f, (1 - this->pan) * 2);
				out[i * 2 + 1] *= std::max(0.f, this->pan * 2);
			}
		}
		return true;
	}

	void Player::start()
	{
		this->active = true;
	}

	void Player::stop()
	{
		this->active = false;
	}

	void Player::addFilter(Filter *filter)
	{
		for (size_t i = 0; i < this->filters.size(); ++i)
		{
			if (this->filters[i] == filter)
				return;
		}
		this->filters.push_back(filter);
	}

	void Player::removeFilter(Filter *filter)
	{
		for (size_t i = 0; i < this->filters.size(); ++i)
		{
			if (this->filters[i] != filter)
				continue;
			this->filters.erase(this->filters.begin() + i);
			return;
		}
	}

	void Player::setPitch(float pitch)
	{
		this->pitch = pitch;
	}

	void Player::setGain(float gain)
	{
		this->gain = std::max(0.f, std::min(1.f, gain));
	}

	void Player::setPan(float pan)
	{
		this->pan = std::max(0.f, std::min(1.f, pan));
	}

	void Player::setLoop(bool loop)
	{
		this->loop = loop;
	}

}

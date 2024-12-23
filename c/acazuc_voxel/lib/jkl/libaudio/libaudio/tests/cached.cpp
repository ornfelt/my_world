#include "Stream.h"
#include "loadDatas.h"
#include "CachedPlayer.h"
#include <cstdlib>
#include <unistd.h>
#include <iostream>

using libaudio::Stream;
using libaudio::CachedPlayer;

int main()
{
	srand(time(NULL));
	PaError error = Pa_Initialize();
	if (error)
	{
		std::cerr << "Failed to initialize portaudio" << std::endl;
		return EXIT_FAILURE;
	}
	Stream *stream = new Stream(2, 44100);
	std::vector<float> datas;
	int channels;
	int rate;
	if (!libaudio::loadDatas("./test.ogg", datas, &rate, &channels))
	{
		std::cerr << "Failed to open ogg file" << std::endl;
		return EXIT_FAILURE;
	}
	CachedPlayer *player = new CachedPlayer(datas.data(), datas.size(), rate, channels);
	player->setLoop(true);
	stream->addPlayer(player);
	player->start();
	stream->start();
	sleep(5);
	return EXIT_SUCCESS;
}

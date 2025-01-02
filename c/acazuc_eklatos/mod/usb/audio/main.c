#define ENABLE_TRACE

#include "usb.h"
#include "snd.h"

#include <kmod.h>

static const struct usb_itf_probe probes[] =
{
	{USB_CLASS_AUDIO, USB_SUBCLASS_AUDIO_STREAMING, 0},
};

struct usb_audio
{
	struct usb_device *device;
	struct usb_interface *interface;
	struct usb_endpoint *endpoint;
	struct usb_isoc_pipe *isoc_pipe;
	struct snd *snd;
};

static void isoc_pipe_fn(struct usb_isoc_pipe *pipe, void *data, size_t *sizes,
                         size_t nbufs)
{
	struct usb_audio *audio = pipe->userdata;

	for (size_t i = 0; i < nbufs; ++i)
	{
		snd_read(audio->snd, data, sizes[i]);
		data = (uint8_t*)data + sizes[i];
	}
}

static void usb_audio_free(struct usb_audio *audio)
{
	if (!audio)
		return;
	usb_isoc_pipe_free(audio->isoc_pipe);
	snd_free(audio->snd);
	free(audio);
}

static int usb_init(struct usb_device *device, struct usb_interface *interface,
                    void *userdata)
{
	struct usb_audio *audio = NULL;
	int ret;

	(void)userdata;
	audio = malloc(sizeof(*audio), M_ZERO);
	if (!audio)
	{
		TRACE("usb_audio: allocation failed");
		ret = -ENOMEM;
		goto err;
	}
	audio->device = device;
	audio->interface = interface;
	audio->endpoint = TAILQ_FIRST(&interface->endpoints);
	if (!audio->endpoint)
	{
		TRACE("usb_audio: no endpoint");
		ret = -EINVAL;
		goto err;
	}
	/* XXX this is a hardcoded way to tell qemu to use the non-empty
	 * audio stream
	 */
	ret = usb_use_interface(audio->device, interface);
	if (ret)
	{
		TRACE("usb_audio: failed to set interface");
		goto err;
	}
	ret = snd_alloc(&audio->snd);
	if (ret)
	{
		TRACE("usb_audio: failed to create snd");
		goto err;
	}
	ret = usb_isoc_pipe_alloc(device, audio->endpoint, isoc_pipe_fn,
	                          audio, &audio->isoc_pipe);
	if (ret)
	{
		TRACE("usb_audio: isoc pipe allocation failed");
		goto err;
	}
	ret = usb_isoc_transfer(audio->isoc_pipe);
	if (ret)
	{
		TRACE("usb_audio: isoc pipe setup failed");
		goto err;
	}
	interface->userdata = audio;
	return 0;

err:
	usb_audio_free(audio);
	return ret;
}

int init(void)
{
	int ret;

	ret = usb_register_itf_probes(probes,
	                              sizeof(probes) / sizeof(*probes),
	                              usb_init, NULL);
	if (ret)
	{
		TRACE("usb_audio: failed to register probes");
		return ret;
	}
	return 0;
}

void fini(void)
{
}

struct kmod_info kmod =
{
	.magic = KMOD_MAGIC,
	.version = 1,
	.name = "usb/audio",
	.init = init,
	.fini = fini,
};

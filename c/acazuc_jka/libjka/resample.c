#include "libjka.h"

#include <samplerate.h>
#include <stdlib.h>
#include <string.h>

struct jka_stream_resample
{
	struct jka_stream stream;
	struct jka_stream *input;
	SRC_STATE *src_state;
	float outbuf[1024];
	size_t outbuf_size;
	float inbuf[1024];
	size_t inbuf_size;
	size_t inbuf_rate;
	size_t output_rate;
	int stopped;
};

static ssize_t read_samples(struct jka_stream *stream, float *samples, size_t count, size_t *sample_rate)
{
	struct jka_stream_resample *resample = (struct jka_stream_resample*)stream;
	*sample_rate = resample->output_rate;
	size_t ret = 0;
	while (count)
	{
		if (resample->outbuf_size >= count * 2)
		{
			memcpy(samples, resample->outbuf, count * 2 * sizeof(*resample->outbuf));
			resample->outbuf_size -= count * 2;
			memmove(resample->outbuf,
			        &resample->outbuf[count * 2],
			        resample->outbuf_size * sizeof(*resample->outbuf));
			ret += count;
			return ret;
		}
		if (resample->outbuf_size > 0)
		{
			memcpy(samples, resample->outbuf, resample->outbuf_size * sizeof(*resample->outbuf));
			samples = &((float*)samples)[resample->outbuf_size];
			count -= resample->outbuf_size / 2;
			ret += resample->outbuf_size / 2;
			resample->outbuf_size = 0;
		}
		if (!resample->inbuf_size)
		{
			ssize_t rd = resample->input->op->read_samples(resample->input, resample->inbuf, sizeof(resample->inbuf) / sizeof(*resample->inbuf) / 2, &resample->inbuf_rate);
			if (rd <= 0)
			{
				if (ret)
					return ret;
				return rd;
			}
			resample->inbuf_size = rd * 2;
		}
		float ratio = resample->output_rate / (float)resample->inbuf_rate;
		if (src_set_ratio(resample->src_state, ratio))
			break; /* XXX handle error */
		SRC_DATA data;
		data.data_in = resample->inbuf;
		data.data_out = resample->outbuf;
		data.input_frames = resample->inbuf_size / 2;
		data.output_frames = sizeof(resample->outbuf) / sizeof(*resample->outbuf) / 2;
		data.end_of_input = 0;
		data.src_ratio = ratio;
		if (src_process(resample->src_state, &data))
			break; /* XXX handle error */
		resample->inbuf_size -= data.input_frames_used * 2;
		memmove(resample->inbuf,
		        &resample->inbuf[data.input_frames_used * 2],
		        resample->inbuf_size * sizeof(*resample->inbuf));
		resample->outbuf_size = data.output_frames_gen * 2;
	}
	return ret;
}

static void dtr(struct jka_stream *stream)
{
	struct jka_stream_resample *resample = (struct jka_stream_resample*)stream;
	src_delete(resample->src_state);
}

static const struct jka_stream_op op =
{
	.dtr = dtr,
	.read_samples = read_samples,
};

struct jka_stream *jka_stream_resample_new(struct jka_stream *input, size_t output_rate)
{
	struct jka_stream_resample *resample = calloc(1, sizeof(*resample));
	if (!resample)
		return NULL;
	resample->stream.op = &op;
	resample->input = input;
	int err;
	resample->src_state = src_new(SRC_SINC_MEDIUM_QUALITY, 2, &err);
	if (!resample->src_state)
	{
		free(resample);
		return NULL;
	}
	resample->output_rate = output_rate;
	return &resample->stream;
}

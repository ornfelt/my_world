#include "libjka.h"

#include <stdlib.h>
#include <math.h>

#ifndef M_PI
# define M_PI 3.14159265358979323846
#endif

struct jka_stream_bilinear
{
	struct jka_stream stream;
	struct jka_stream *input;
	float back_samples[2][3];
	float prev_samples[2][2];
	float a0;
	float a1;
	float a2;
	float b0;
	float b1;
	float b2;
};

static ssize_t bilinear_read_samples(struct jka_stream *stream, float *samples, size_t count, size_t *sample_rate)
{
	struct jka_stream_bilinear *bilinear = (struct jka_stream_bilinear*)stream;
	ssize_t ret = bilinear->input->op->read_samples(bilinear->input, samples, count, sample_rate);
	if (ret <= 0)
		return ret;
	float *dst = samples;
	for (ssize_t i = 0; i < ret; ++i)
	{
		for (size_t j = 0; j < 2; ++j)
		{
			bilinear->back_samples[j][2] = bilinear->back_samples[j][1];
			bilinear->back_samples[j][1] = bilinear->back_samples[j][0];
			bilinear->back_samples[j][0] = *dst;
			float pcm =
				(bilinear->b0 * bilinear->back_samples[j][0]) +
				(bilinear->b1 * bilinear->back_samples[j][1]) +
				(bilinear->b2 * bilinear->back_samples[j][2]) -
				(bilinear->a1 * bilinear->prev_samples[j][0]) -
				(bilinear->a2 * bilinear->prev_samples[j][1]);
			*dst = pcm;
			bilinear->prev_samples[j][1] = bilinear->prev_samples[j][0];
			bilinear->prev_samples[j][0] = pcm;
			++dst;
		}
	}
	return ret;
}

static const struct jka_stream_op bilinear_op =
{
	.read_samples = bilinear_read_samples,
};

static struct jka_stream_bilinear *bilinear_new(struct jka_stream *input)
{
	struct jka_stream_bilinear *bilinear = calloc(1, sizeof(*bilinear));
	if (!bilinear)
		return NULL;
	bilinear->stream.op = &bilinear_op;
	bilinear->input = input;
	return bilinear;
}

struct jka_stream *jka_stream_lowpass_new(struct jka_stream *input, size_t sampling, size_t cutoff, float resonance)
{
	struct jka_stream_bilinear *stream = bilinear_new(input);
	if (!stream)
		return NULL;
	float w0 = 2 * M_PI * cutoff / (float)sampling;
	float alpha = sinf(w0) / (2 * resonance);
	float cw0 = cosf(w0);
	stream->a0 = 1 + alpha;
	stream->a1 = (-2 * cw0) / stream->a0;
	stream->a2 = (1 - alpha) / stream->a0;
	stream->b0 = ((1 - cw0) / 2) / stream->a0;
	stream->b1 = (1 - cw0) / stream->a0;
	stream->b2 = ((1 - cw0) / 2) / stream->a0;
	return &stream->stream;
}

struct jka_stream *jka_stream_highpass_new(struct jka_stream *input, size_t sampling, size_t cutoff, float resonance)
{
	struct jka_stream_bilinear *stream = bilinear_new(input);
	if (!stream)
		return NULL;
	float w0 = 2 * M_PI * cutoff / (float)sampling;
	float alpha = sinf(w0) / (2 * resonance);
	float cw0 = cosf(w0);
	stream->a0 = 1 + alpha;
	stream->a1 = (-2 * cw0) / stream->a0;
	stream->a2 = (1 - alpha) / stream->a0;
	stream->b0 = ((1 + cw0) / 2) / stream->a0;
	stream->b1 = (-1 - cw0) / stream->a0;
	stream->b2 = ((1 + cw0) / 2) / stream->a0;
	return &stream->stream;
}

struct jka_stream *jka_stream_bandpass_new(struct jka_stream *input, size_t sampling, size_t cutoff, float resonance)
{
	struct jka_stream_bilinear *stream = bilinear_new(input);
	if (!stream)
		return NULL;
	float w0 = 2 * M_PI * cutoff / (float)sampling;
	float alpha = sinf(w0) / (2 * resonance);
	float cw0 = cosf(w0);
	stream->a0 = 1 + alpha;
	stream->a1 = (-2 * cw0) / stream->a0;
	stream->a2 = (1 - alpha) / stream->a0;
	stream->b0 = alpha / stream->a0;
	stream->b1 = 0;
	stream->b2 = alpha / stream->a0;
	return &stream->stream;
}

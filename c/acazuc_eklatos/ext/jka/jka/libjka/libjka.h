#ifndef LIBJKA_H
#define LIBJKA_H

#include <sys/types.h>

#include <stddef.h>
#include <stdio.h>

struct jka_stream;

struct jka_stream_op
{
	void (*ctr)(struct jka_stream *stream);
	void (*dtr)(struct jka_stream *stream);
	ssize_t (*read_samples)(struct jka_stream *stream, float *samples, size_t count, size_t *sample_rate);
};

struct jka_stream
{
	const struct jka_stream_op *op;
	int eof;
};

struct jka_stream *jka_stream_wave_new(FILE *fp);
struct jka_stream *jka_stream_resample_new(struct jka_stream *input, size_t output_rate);
struct jka_stream *jka_stream_lowpass_new(struct jka_stream *input, size_t sampling, size_t cutoff, float resonance);
struct jka_stream *jka_stream_highpass_new(struct jka_stream *input, size_t sampling, size_t cutoff, float resonance);
struct jka_stream *jka_stream_bandpass_new(struct jka_stream *input, size_t sampling, size_t cutoff, float resonance);

void jka_stream_delete(struct jka_stream *stream);

#endif

#include "libjka.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct jka_stream_wave
{
	struct jka_stream stream;
	FILE *fp;
	size_t file_begin;
	struct
	{
		uint32_t data_size;
		uint16_t fmt_tag;
		uint16_t channels;
		uint32_t samplerate;
		uint32_t byte_rate;
		uint16_t align;
		uint16_t bps;
	} fmt;
	uint32_t chunk_size;
	uint32_t chunk_pos;
	uint8_t buffer[4096];
	size_t buffer_size;
};

static int decode_next_chunk(struct jka_stream_wave *wave)
{
	uint8_t sig[4];
	if (fread(sig, 1, 4, wave->fp) != 4)
		return 0;
	if (!memcmp(sig, "fmt ", 4))
	{
		if (fread(&wave->fmt, 1, sizeof(wave->fmt), wave->fp) != sizeof(wave->fmt))
			return 0;
		wave->chunk_pos = 0;
		wave->chunk_size = 0;
		return 1;
	}
	if (!memcmp(sig, "data", 4))
	{
		if (fread(&wave->chunk_size, 1, 4, wave->fp) != 4)
			return 0;
		return 1;
	}
	return 0;
}

static size_t refill_buffer(struct jka_stream_wave *wave, size_t count)
{
	size_t n = count;
	size_t avail = sizeof(wave->buffer) - wave->buffer_size;
	if (n > avail)
		n = avail;
	size_t rd = fread(&wave->buffer[wave->buffer_size], 1, n, wave->fp);
	wave->buffer_size += rd;
	return rd;
}

static void consume_buffer(struct jka_stream_wave *wave, size_t count)
{
	memmove(&wave->buffer, &wave->buffer[count], wave->buffer_size - count);
	wave->buffer_size -= count;
}

static ssize_t decode_samples(struct jka_stream_wave *wave, float *samples, size_t count, size_t *sample_rate)
{
	*sample_rate = wave->fmt.samplerate;
	if (wave->fmt.fmt_tag != 1)
		return -1;
	size_t avail_bytes = wave->chunk_size - wave->chunk_pos;
	size_t bytes_per_sample;
	switch (wave->fmt.bps)
	{
		case 8:
			bytes_per_sample = wave->fmt.channels;
			break;
		case 16:
			bytes_per_sample = wave->fmt.channels * 2;
			break;
		case 32:
			bytes_per_sample = wave->fmt.channels * 4;
			break;
		default:
			return -1;
	}
	size_t avail_samples = avail_bytes / bytes_per_sample;
	if (count > avail_samples)
		count = avail_samples;
	switch (wave->fmt.bps)
	{
		case 8:
			if (wave->fmt.channels == 1)
			{
				while (count)
				{
					if (!refill_buffer(wave, count))
						break; /* XXX error handling */
					size_t i = 0;
					while (i + 1 <= wave->buffer_size)
					{
						samples[0] = wave->buffer[i] / 128.f;
						samples[1] = samples[0];
						samples += 2;
						++i;
					}
					consume_buffer(wave, i);
					wave->chunk_pos += i;
				}
			}
			else
			{
				while (count)
				{
					if (!refill_buffer(wave, count))
						break; /* XXX error handling */
					size_t i = 0;
					while (i + wave->fmt.channels <= wave->buffer_size)
					{
						samples[0] = wave->buffer[i + 0] / 128.f;
						samples[1] = wave->buffer[i + 1] / 128.f;
						samples += 2;
						i += wave->fmt.channels;
					}
					consume_buffer(wave, i);
					wave->chunk_pos += i;
				}
			}
			break;
		case 16:
			if (wave->fmt.channels == 1)
			{
				while (count)
				{
					if (!refill_buffer(wave, count))
						break; /* XXX error handling */
					size_t i = 0;
					while (i + 2 <= wave->buffer_size)
					{
						samples[0] = *(int16_t*)&wave->buffer[i] / 32768.f;
						samples[1] = samples[0];
						samples += 2;
						i += 2;
					}
					consume_buffer(wave, i);
					wave->chunk_pos += i;
				}
			}
			else
			{
				while (count)
				{
					if (!refill_buffer(wave, count))
						break; /* XXX error handling */
					size_t i = 0;
					while (i + 2 * wave->fmt.channels <= wave->buffer_size)
					{
						samples[0] = ((int16_t*)wave->buffer)[i + 0] / 32768.f;
						samples[1] = ((int16_t*)wave->buffer)[i + 1] / 32768.f;
						samples += 2;
						i += 2 * wave->fmt.channels;
					}
					consume_buffer(wave, i);
					wave->chunk_pos += i;
				}
			}
			break;
		case 32:
			if (wave->fmt.channels == 1)
			{
				while (count)
				{
					if (!refill_buffer(wave, count))
						break; /* XXX error handling */
					size_t i = 0;
					while (i + 4 <= wave->buffer_size)
					{
						samples[0] = *(int32_t*)&wave->buffer[i] / 2147483648.f;
						samples[1] = samples[0];
						samples += 4;
						i += 4;
					}
					consume_buffer(wave, i);
					wave->chunk_pos += i;
				}
			}
			else
			{
				while (count)
				{
					if (!refill_buffer(wave, count))
						break; /* XXX error handling */
					size_t i = 0;
					while (i + 4 * wave->fmt.channels <= wave->buffer_size)
					{
						samples[0] = ((int32_t*)wave->buffer)[i + 0] / 2147483648.f;
						samples[1] = ((int32_t*)wave->buffer)[i + 1] / 2147483648.f;
						samples += 4;
						i += 4 * wave->fmt.channels;
					}
					consume_buffer(wave, i);
					wave->chunk_pos += i;
				}
			}
			break;
	}
	return count;
}

static ssize_t read_samples(struct jka_stream *stream, float *samples, size_t count, size_t *sample_rate)
{
	struct jka_stream_wave *wave = (struct jka_stream_wave*)stream;
	while (count)
	{
		if (wave->chunk_pos != wave->chunk_size)
			return decode_samples(wave, samples, count, sample_rate);
		if (feof(wave->fp))
		{
			if (1) /* stop on end */
				return 0;
			if (fseek(wave->fp, wave->file_begin, SEEK_SET) == -1)
				return -1;
		}
		if (!decode_next_chunk(wave))
			return -1;
	}
	return 0;
}

static void dtr(struct jka_stream *stream)
{
	struct jka_stream_wave *wave = (struct jka_stream_wave*)stream;
	fclose(wave->fp);
}

static const struct jka_stream_op op =
{
	.dtr = dtr,
	.read_samples = read_samples,
};

struct jka_stream *jka_stream_wave_new(FILE *fp)
{
	char riff_sig[4];
	if (fread(riff_sig, 1, 4, fp) != 4)
		return NULL;
	if (memcmp(riff_sig, "RIFF", 4))
		return NULL;
	uint32_t length;
	if (fread(&length, 1, 4, fp) != 4)
		return NULL;
	char wave_sig[4];
	if (fread(&wave_sig, 1, 4, fp) != 4)
		return NULL;
	if (memcmp(wave_sig, "WAVE", 4))
		return NULL;
	struct jka_stream_wave *wave = calloc(1, sizeof(*wave));
	if (!wave)
		return NULL;
	wave->stream.op = &op;
	wave->file_begin = ftell(fp);
	wave->fp = fp;
	return &wave->stream;
}

#include "simplex_octave.h"
#include "simplex.h"
#include "log.h"

#include <math.h>

bool simplex_noise_init(struct simplex_noise *simplex_noise, uint32_t octaves_number, float persistance, int32_t seed)
{
	jks_array_init(&simplex_noise->frequencies, sizeof(float), NULL, NULL);
	jks_array_init(&simplex_noise->amplitudes, sizeof(float), NULL, NULL);
	jks_array_init(&simplex_noise->octaves, sizeof(struct simplex_noise_octave), NULL, NULL);
	if (!jks_array_resize(&simplex_noise->frequencies, octaves_number))
	{
		LOG_ERROR("allocation failed");
		return false;
	}
	if (!jks_array_resize(&simplex_noise->amplitudes, octaves_number))
	{
		LOG_ERROR("allocation failed");
		return false;
	}
	if (!jks_array_resize(&simplex_noise->octaves, octaves_number))
	{
		LOG_ERROR("allocation failed");
		return false;
	}
	for (uint32_t i = 0; i < octaves_number; ++i)
	{
		simplex_noise_octave_init(JKS_ARRAY_GET(&simplex_noise->octaves, i, struct simplex_noise_octave), seed);
		*JKS_ARRAY_GET(&simplex_noise->frequencies, i, float) = pow(2, i);
		*JKS_ARRAY_GET(&simplex_noise->amplitudes, i, float) = pow(persistance, octaves_number - i);
	}
	simplex_noise->persistance = persistance;
	simplex_noise->seed = seed;
	return true;
}

void simplex_noise_destroy(struct simplex_noise *simplex_noise)
{
	jks_array_destroy(&simplex_noise->frequencies);
	jks_array_destroy(&simplex_noise->amplitudes);
	jks_array_destroy(&simplex_noise->octaves);
}

float simplex_noise_get1(struct simplex_noise *simplex_noise, float x)
{
	float result = 0;
	for (uint32_t i = 0; i < simplex_noise->octaves.size; ++i)
	{
		float frequency = *JKS_ARRAY_GET(&simplex_noise->frequencies, i, float);
		float amplitude = *JKS_ARRAY_GET(&simplex_noise->amplitudes, i, float);
		result += simplex_noise_octave_get1(JKS_ARRAY_GET(&simplex_noise->octaves, i, struct simplex_noise_octave), x / frequency) * amplitude;
	}
	return result;
}

float simplex_noise_get2(struct simplex_noise *simplex_noise, float x, float y)
{
	float result = 0;
	for (uint32_t i = 0; i < simplex_noise->octaves.size; ++i)
	{
		float frequency = *JKS_ARRAY_GET(&simplex_noise->frequencies, i, float);
		float amplitude = *JKS_ARRAY_GET(&simplex_noise->amplitudes, i, float);
		result += simplex_noise_octave_get2(JKS_ARRAY_GET(&simplex_noise->octaves, i, struct simplex_noise_octave), x / frequency, y / frequency) * amplitude;
	}
	return result;
}

float simplex_noise_get3(struct simplex_noise *simplex_noise, float x, float y, float z)
{
	float result = 0;
	for (uint32_t i = 0 ; i < simplex_noise->octaves.size; ++i)
	{
		float frequency = *JKS_ARRAY_GET(&simplex_noise->frequencies, i, float);
		float amplitude = *JKS_ARRAY_GET(&simplex_noise->amplitudes, i, float);
		result += simplex_noise_octave_get3(JKS_ARRAY_GET(&simplex_noise->octaves, i, struct simplex_noise_octave), x / frequency, y / frequency, z / frequency) * amplitude;
	}
	return result;
}

float simplex_noise_get4(struct simplex_noise *simplex_noise, float x, float y, float z, float w)
{
	float result = 0;
	for (uint32_t i = 0; i < simplex_noise->octaves.size; ++i)
	{
		float frequency = *JKS_ARRAY_GET(&simplex_noise->frequencies, i, float);
		float amplitude = *JKS_ARRAY_GET(&simplex_noise->amplitudes, i, float);
		result += simplex_noise_octave_get4(JKS_ARRAY_GET(&simplex_noise->octaves, i, struct simplex_noise_octave), x / frequency, y / frequency, z / frequency, w / frequency) * amplitude;
	}
	return result;
}

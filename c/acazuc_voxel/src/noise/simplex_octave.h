#ifndef SIMPLEX_NOISE_OCTAVE_H
#define SIMPLEX_NOISE_OCTAVE_H

#include <stdint.h>

struct simplex_noise_octave
{
	uint8_t perm[512];
	uint8_t perm_mod12[512];
};

void simplex_noise_octave_init(struct simplex_noise_octave *octave, uint64_t seed);
float simplex_noise_octave_get1(struct simplex_noise_octave *octave, float x);
float simplex_noise_octave_get2(struct simplex_noise_octave *octave, float x, float y);
float simplex_noise_octave_get3(struct simplex_noise_octave *octave, float x, float y, float z);
float simplex_noise_octave_get4(struct simplex_noise_octave *octave, float x, float y, float z, float w);

#endif

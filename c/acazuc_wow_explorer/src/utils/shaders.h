#ifndef SHADERS_H
#define SHADERS_H

#include <stddef.h>

void decode_dx9_shader(char *buffer, size_t buffer_size, const void *data, size_t size);
void decode_nv_register_shader(char *buffer, size_t buffer_size, const void *data, size_t size);
void decode_nv_texture_shader(char *buffer, size_t buffer_size, const void *data, size_t size);

#endif

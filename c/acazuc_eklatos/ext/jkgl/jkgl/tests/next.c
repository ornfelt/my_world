#define __BSD_VISIBLE 1
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "rast/rast.h"
#include "common.h"
#include "gl.h"

void
vs_fn(struct rast_vert *vert)
{
	vert->x = vert->attribs[0].x;
	vert->y = vert->attribs[0].y;
	vert->z = vert->attribs[0].z;
	vert->w = vert->attribs[0].w;
	vert->varying[0] = vert->attribs[1];
	vert->varying[1] = vert->attribs[2];
}

bool
fs_fn(const struct rast_vert *vert, float *color)
{
	float tex_color[4];
	gl_sample(0, &vert->varying[1].x, tex_color);
	color[0] = vert->varying[0].x * tex_color[0];
	color[1] = vert->varying[0].y * tex_color[1];
	color[2] = vert->varying[0].z * tex_color[2];
	color[3] = vert->varying[0].w * tex_color[3];
	return false;
}

int
main(int argc, char **argv)
{
	GLuint vertex_buffer;
	GLuint vertex_array;
	GLuint index_buffer;
	GLuint program;
	GLuint vs;
	GLuint fs;
	GLuint texture;
	struct window window;

	(void)argc;
	if (setup_window(argv[0], &window))
		return EXIT_FAILURE;
	{
		static const uint16_t indexes[] = {0, 1, 2, 0, 2, 3};
		GL_CALL(glGenBuffers, 1, &index_buffer);
		GL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, index_buffer);
		GL_CALL(glBufferData, GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), indexes, GL_STATIC_DRAW);
	}
	{
		static const float vertexes[] =
		{
			0.25, 0.25, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0,
			0.50, 0.25, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0,
			0.50, 0.50, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
			0.25, 0.50, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 1.0,
		};
		GL_CALL(glGenBuffers, 1, &vertex_buffer);
		GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, vertex_buffer);
		GL_CALL(glBufferData, GL_ARRAY_BUFFER, sizeof(vertexes), vertexes, GL_STATIC_DRAW);
	}
	{
		GL_CALL(glGenVertexArrays, 1, &vertex_array);
		GL_CALL(glBindVertexArray, vertex_array);
		GL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, index_buffer);
		GL_CALL(glBindVertexBuffer, 0, vertex_buffer, 0, sizeof(float) * 10);
		GL_CALL(glEnableVertexAttribArray, 0);
		GL_CALL(glVertexAttribBinding, 0, 0);
		GL_CALL(glVertexBindingDivisor, 0, 0);
		GL_CALL(glVertexAttribFormat, 0, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 0);
		GL_CALL(glEnableVertexAttribArray, 1);
		GL_CALL(glVertexAttribBinding, 1, 0);
		GL_CALL(glVertexBindingDivisor, 1, 0);
		GL_CALL(glVertexAttribFormat, 1, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4);
		GL_CALL(glEnableVertexAttribArray, 2);
		GL_CALL(glVertexAttribBinding, 2, 0);
		GL_CALL(glVertexBindingDivisor, 2, 0);
		GL_CALL(glVertexAttribFormat, 2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8);
	}
	{
		GL_CALL(glEnable, GL_BLEND);
		GL_CALL(glBlendFunc, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		GL_CALL(glBlendEquation, GL_FUNC_ADD);
		GL_CALL(glColorMask, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		GL_CALL(glDisable, GL_COLOR_LOGIC_OP);
	}
	{
		GL_CALL(glPointSize, 1);
		GL_CALL(glLineWidth, 1);
		GL_CALL(glDisable, GL_POINT_SMOOTH);
		GL_CALL(glDisable, GL_LINE_SMOOTH);
		GL_CALL(glDisable, GL_SCISSOR_TEST);
		GL_CALL(glFrontFace, GL_CW);
		GL_CALL(glDisable, GL_CULL_FACE);
		GL_CALL(glPolygonMode, GL_FRONT_AND_BACK, GL_FILL);
	}
	{
		GLint result = GL_FALSE;
		const GLchar *src = (void*)vs_fn;
		GLuint len = sizeof(src);
		vs = glCreateShader(GL_VERTEX_SHADER);
		GL_CALL(glShaderSource, vs, 1, &src, &len);
		GL_CALL(glCompileShader, vs);
		GL_CALL(glGetShaderiv, vs, GL_COMPILE_STATUS, &result);
		if (!result)
		{
			fprintf(stderr, "vertex shader creation failed\n");
			return EXIT_FAILURE;
		}
	}
	{
		GLint result = GL_FALSE;
		const GLchar *src = (void*)fs_fn;
		GLuint len = sizeof(src);
		fs = glCreateShader(GL_FRAGMENT_SHADER);
		GL_CALL(glShaderSource, fs, 1, &src, &len);
		GL_CALL(glCompileShader, fs);
		GL_CALL(glGetShaderiv, fs, GL_COMPILE_STATUS, &result);
		if (!result)
		{
			fprintf(stderr, "fragment shader creation failed\n");
			return EXIT_FAILURE;
		}
	}
	{
		program = glCreateProgram();
		GLint result = GL_FALSE;
		GL_CALL(glAttachShader, program, vs);
		GL_CALL(glAttachShader, program, fs);
		GL_CALL(glLinkProgram, program);
		GL_CALL(glGetProgramiv, program, GL_LINK_STATUS, &result);
		if (!result)
		{
			fprintf(stderr, "program creation failed\n");
			return EXIT_FAILURE;
		}
	}
	{
		GL_CALL(glEnable, GL_DEPTH_TEST);
		GL_CALL(glDepthMask, GL_TRUE);
		GL_CALL(glDepthFunc, GL_LEQUAL);
		GL_CALL(glDisable, GL_STENCIL_TEST);
	}
	{
		static const uint8_t texture_data[] =
		{
			0xFF, 0x00, 0x00, 0xFF,
			0x00, 0xFF, 0x00, 0xFF,
			0x00, 0x00, 0xFF, 0xFF,
			0xFF, 0xFF, 0x00, 0xFF,
		};
		GL_CALL(glGenTextures, 1, &texture);
		GL_CALL(glBindTexture, GL_TEXTURE_2D, texture);
		GL_CALL(glTexImage2D, GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);
#if 0
		GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#else
		GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_CUBIC);
		GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_CUBIC);
#endif
		GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	while (1)
	{
		handle_events(&window);
		uint64_t frametime = nanotime();
		uint64_t period = 10 * 1000000000ULL;
		float percent = (frametime % period) / (float)period;
		float f = 1 + 0.5 * cosf(percent * M_PI * 2);
		float hsl[3];
		float rgb[3];
		hsl[0] = percent;
		hsl[1] = 0.5;
		hsl[2] = 0.5;
		hsl2rgb(rgb, hsl);
		int32_t fw = window.width * f;
		int32_t fh = window.height * f;
		GL_CALL(glViewport, window.width - fw, window.height - fh, fw, fw);
		GL_CALL(glClearColor, rgb[0], rgb[1], rgb[2], 1);
		GL_CALL(glClearDepth, 1);
		GL_CALL(glClearStencil, 0);
		GL_CALL(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		GL_CALL(glViewport, 0, 0, fw, fh);
		GL_CALL(glBindVertexArray, vertex_array);
		GL_CALL(glUseProgram, program);
		GL_CALL(glDrawElements, GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0);
		swap_buffers(&window);
	}
	return EXIT_SUCCESS;
}

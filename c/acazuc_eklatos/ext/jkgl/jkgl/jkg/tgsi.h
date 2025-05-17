/**************************************************************************
 * 
 * Copyright 2008 VMware, Inc.
 * Copyright 2009-2010 VMware, Inc.
 * All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 **************************************************************************/

/* cleaned up headers from mesa files:
 * p_shader_tokens.h
 */

#ifndef JKGL_TGSI_H
#define JKGL_TGSI_H

#include <stdint.h>

enum tgsi_processor_type
{
	TGSI_PROCESSOR_FRAGMENT,
	TGSI_PROCESSOR_VERTEX,
	TGSI_PROCESSOR_GEOMETRY,
	TGSI_PROCESSOR_TESS_CTRL,
	TGSI_PROCESSOR_TESS_EVAL,
	TGSI_PROCESSOR_COMPUTE,
};

struct tgsi_processor
{
	uint32_t processor : 4;  /* TGSI_PROCESSOR_ */
	uint32_t padding   : 28;
};

enum tgsi_token_type
{
	TGSI_TOKEN_TYPE_DECLARATION,
	TGSI_TOKEN_TYPE_IMMEDIATE,
	TGSI_TOKEN_TYPE_INSTRUCTION,
	TGSI_TOKEN_TYPE_PROPERTY,
};

struct tgsi_token
{
	uint32_t type      : 4;  /**< TGSI_TOKEN_TYPE_x */
	uint32_t tokens_nb : 8;  /**< UINT */
	uint32_t padding   : 20;
};

enum tgsi_file_type
{
	TGSI_FILE_NULL,
	TGSI_FILE_CONSTANT,
	TGSI_FILE_INPUT,
TGSI_FILE_OUTPUT,
	TGSI_FILE_TEMPORARY,
	TGSI_FILE_SAMPLER,
	TGSI_FILE_ADDRESS,
	TGSI_FILE_IMMEDIATE,
	TGSI_FILE_PREDICATE,
	TGSI_FILE_SYSTEM_VALUE,
	TGSI_FILE_IMAGE,
	TGSI_FILE_SAMPLER_VIEW,
	TGSI_FILE_BUFFER,
	TGSI_FILE_MEMORY,
	TGSI_FILE_HW_ATOMIC,
	TGSI_FILE_COUNT /**< how many TGSI_FILE_ types */
};

#define TGSI_WRITEMASK_NONE 0x00
#define TGSI_WRITEMASK_X    0x01
#define TGSI_WRITEMASK_Y    0x02
#define TGSI_WRITEMASK_XY   0x03
#define TGSI_WRITEMASK_Z    0x04
#define TGSI_WRITEMASK_XZ   0x05
#define TGSI_WRITEMASK_YZ   0x06
#define TGSI_WRITEMASK_XYZ  0x07
#define TGSI_WRITEMASK_W    0x08
#define TGSI_WRITEMASK_XW   0x09
#define TGSI_WRITEMASK_YW   0x0A
#define TGSI_WRITEMASK_XYW  0x0B
#define TGSI_WRITEMASK_ZW   0x0C
#define TGSI_WRITEMASK_XZW  0x0D
#define TGSI_WRITEMASK_YZW  0x0E
#define TGSI_WRITEMASK_XYZW 0x0F

enum tgsi_interpolate_mode
{
	TGSI_INTERPOLATE_CONSTANT,
	TGSI_INTERPOLATE_LINEAR,
	TGSI_INTERPOLATE_PERSPECTIVE,
	TGSI_INTERPOLATE_COLOR, /* special color case for smooth/flat */
	TGSI_INTERPOLATE_COUNT,
};

enum tgsi_interpolate_loc
{
	TGSI_INTERPOLATE_LOC_CENTER,
	TGSI_INTERPOLATE_LOC_CENTROID,
	TGSI_INTERPOLATE_LOC_SAMPLE,
	TGSI_INTERPOLATE_LOC_COUNT,
};

enum tgsi_memory_type
{
	TGSI_MEMORY_TYPE_GLOBAL, /* OpenCL global              */
	TGSI_MEMORY_TYPE_SHARED, /* OpenCL local / GLSL shared */
	TGSI_MEMORY_TYPE_PRIVATE,/* OpenCL private             */
	TGSI_MEMORY_TYPE_INPUT,  /* OpenCL kernel input params */
	TGSI_MEMORY_TYPE_COUNT,
};

struct tgsi_declaration
{
	uint32_t type        : 4;  /**< TGSI_TOKEN_TYPE_DECLARATION */
	uint32_t tokens_nb   : 8;  /**< UINT */
	uint32_t file        : 4;  /**< one of TGSI_FILE_x */
	uint32_t usage_mask  : 4;  /**< bitmask of TGSI_WRITEMASK_x flags */
	uint32_t dimension   : 1;  /**< any extra dimension info? */
	uint32_t semantic    : 1;  /**< BOOL, any semantic info? */
	uint32_t interpolate : 1;  /**< any interpolation info? */
	uint32_t invariant   : 1;  /**< invariant optimization? */
	uint32_t local       : 1;  /**< optimize as subroutine local variable? */
	uint32_t array       : 1;  /**< extra array info? */
	uint32_t atomic      : 1;  /**< atomic only? for TGSI_FILE_BUFFER */
	uint32_t mem_type    : 2;  /**< TGSI_MEMORY_TYPE_x for TGSI_FILE_MEMORY */
	uint32_t padding     : 3;
};

#endif

#include "test_utils.h"

#include <stdio.h>
#include <stdlib.h>

static void test_utils_compare_buffers(const unsigned char* buffer,
                                       const unsigned char* write_buffer,
                                       const size_t buffer_size,
                                       const char* location,
                                       size_t i)
{
    if (buffer[i] != write_buffer[i])
    {
        int line_number;
        size_t j;

        printf("%s failed write:\n", location);
        printf(__FILE__ ":" STRINGIFY(__LINE__) " Failed comparison at index %lu:\n", (long unsigned int)i);
        printf("    buffer:       '0x%02x'\n", buffer[i]);
        printf("    write_buffer: '0x%02x'\n", write_buffer[i]);
        puts("");

        line_number = 0;
        for (j = 0; j < buffer_size; ++j)
        {
            if (j == i)
            {
                break;
            }

            if (line_number >= 60)
            {
                puts("");
                line_number = 0;
            }

            printf("0x%02x ", buffer[j]);
            line_number += 5;
        }

        puts("");
        puts("");

        printf("buffer:       ");
        line_number = sizeof("buffer:       ");
        for (j = i; j < buffer_size; ++j)
        {
            if (line_number >= 60)
            {
                puts("");
                line_number = 0;
            }

            printf("0x%02x ", buffer[j]);
            line_number += 5;
        }
        puts("");

        printf("write_buffer: ");
        line_number = sizeof("buffer:       ");
        for (j = i; j < buffer_size; ++j)
        {
            if (line_number >= 60)
            {
                puts("");
                line_number = 0;
            }

            printf("0x%02x ", write_buffer[j]);
            line_number += 5;
        }
        puts("");

        fflush(NULL);
        abort();
    }
}

void world_test_compare_buffers(const unsigned char* buffer,
                                const unsigned char* write_buffer,
                                size_t buffer_size,
                                const char* location,
                                TestUtilsSide side)
{
    size_t i;
    uint32_t opcode = 0;
    uint32_t write_opcode = 0;
    int failed = 0;

    uint32_t size = buffer[0] << 8 | buffer[1];
    uint32_t write_size = write_buffer[0] << 8 | write_buffer[1];

    if (side == TEST_UTILS_SIDE_CLIENT)
    {
        opcode = (uint32_t)buffer[2] | (uint32_t)buffer[3] << 8 | (uint32_t)buffer[4] << 16 | (uint32_t)buffer[5] << 24;
        write_opcode = (uint32_t)write_buffer[2] | (uint32_t)write_buffer[3] << 8 | (uint32_t)write_buffer[4] << 16 |
            (uint32_t)write_buffer[5] << 24;
    }
    else
    {
        opcode = (uint32_t)buffer[2] | (uint32_t)buffer[3] << 8;
        write_opcode = (uint32_t)write_buffer[2] | (uint32_t)write_buffer[3] << 8;
    }

    if (size != write_size)
    {
        printf("%s wrote incorrect size:\n", location);
        printf(__FILE__ ":" STRINGIFY(__LINE__) " expected 0x%04x but got 0x%04x:\n", size, write_size);
        failed = 1;
    }

    if (opcode != write_opcode)
    {
        printf("%s wrote incorrect opcode:\n", location);
        printf(__FILE__ ":" STRINGIFY(__LINE__) " expected 0x%04x but got 0x%04x:\n", opcode, write_opcode);
        failed = 1;
    }

    i = side == TEST_UTILS_SIDE_CLIENT ? 6 : 4;
    for (; i < buffer_size; ++i)
    {
        if (buffer[i] != write_buffer[i])
        {
            test_utils_compare_buffers(buffer, write_buffer, buffer_size, location, i);
        }
    }

    if (failed)
    {
        test_utils_compare_buffers(buffer, write_buffer, buffer_size, location, 0);
    }
}

void wlm_test_compare_buffers(const unsigned char* buffer,
                              const unsigned char* write_buffer,
                              const size_t buffer_size,
                              const char* location)
{
    size_t i;
    for (i = 0; i < buffer_size; ++i)
    {
        if (buffer[i] != write_buffer[i])
        {
            test_utils_compare_buffers(buffer, write_buffer, buffer_size, location, i);
        }
    }
}

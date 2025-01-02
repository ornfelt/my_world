#ifndef WOW_LOGIN_MESSAGES_TEST_UTILS_H
#define WOW_LOGIN_MESSAGES_TEST_UTILS_H

#include <stdio.h>
#include <stdint.h>

#define STRINGIFY2(x) #x
#define STRINGIFY(x) STRINGIFY2(x)

#ifdef __cplusplus
extern "C" {
#endif


void wlm_test_compare_buffers(const unsigned char* buffer,
                              const unsigned char* write_buffer,
                              size_t buffer_size,
                              const char* location);

typedef enum
{
    TEST_UTILS_SIDE_SERVER,
    TEST_UTILS_SIDE_CLIENT
} TestUtilsSide;

void world_test_compare_buffers(const unsigned char* buffer,
                                const unsigned char* write_buffer,
                                size_t buffer_size,
                                const char* location,
                                TestUtilsSide side);


#ifdef __cplusplus
}
#endif


#endif

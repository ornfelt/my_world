#ifndef EKLAT_REBOOT_H
#define EKLAT_REBOOT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REBOOT_SHUTDOWN  0x19971203
#define REBOOT_REBOOT    0xAC15DEAD
#define REBOOT_SUSPEND   0x531F8888
#define REBOOT_HIBERNATE 0xEFFACEAC

int reboot(uint32_t cmd);

#ifdef __cplusplus
}
#endif

#endif

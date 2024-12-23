#ifndef SOCKET_PLATFORM_H
# define SOCKET_PLATFORM_H

# include "Platform.h"

# ifdef LIBNET_PLATFORM_WINDOWS
#  include <winsock2.h>
#  include <ws2tcpip.h>
   typedef int SOCKLEN_T;
# elif defined LIBNET_PLATFORM_LINUX
#  include <netinet/tcp.h>
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <sys/types.h>
#  include <sys/ioctl.h>
#  include <unistd.h>
#  include <netdb.h>
#  include <fcntl.h>
#  define closesocket(s) ::close(s)
#  define INVALID_SOCKET -1
#  define SOCKET_ERROR -1
   typedef socklen_t SOCKLEN_T;
   typedef int SOCKET;
# else
#  error Platform not supported
# endif

#endif

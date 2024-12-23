#ifndef LIBNET_PLATFORM_H
# define LIBNET_PLATFORM_H

# ifdef _WIN32
#  define LIBNET_PLATFORM_WINDOWS
# elif defined __linux__
#  define LIBNET_PLATFORM_LINUX
# endif

#endif

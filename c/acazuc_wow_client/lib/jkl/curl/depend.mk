curl: nghttp2 nghttp3 zlib

ifneq ($(filter $(TARGET), linux_32 linux_64 host),)
curl: openssl
endif

ifneq ($(filter $(TARGET), linux_32 linux_64 host),)
nghttp2: openssl
endif

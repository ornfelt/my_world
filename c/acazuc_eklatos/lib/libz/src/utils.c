#include <zlib.h>

int
compress(uint8_t *dst,
         unsigned *dst_len,
         const uint8_t *src,
         unsigned src_len)
{
	return compress2(dst, dst_len, src, src_len, Z_DEFAULT_COMPRESSION);
}

int
compress2(uint8_t *dst,
          unsigned *dst_len,
          const uint8_t *src,
          unsigned src_len,
          int level)
{
	z_stream stream;
	int ret;

	ret = deflateInit(&stream, level);
	if (ret != Z_OK)
		return ret;
	stream.avail_in = src_len;
	stream.next_in = src;
	stream.avail_out = *dst_len;
	stream.next_out = dst;
	ret = deflate(&stream, Z_FINISH);
	*dst_len -= stream.avail_out;
	deflateEnd(&stream);
	return ret;
}

int
uncompress(uint8_t *dst,
           unsigned *dst_len,
           const uint8_t *src,
           unsigned src_len)
{
	return uncompress2(dst, dst_len, src, &src_len);
}

int
uncompress2(uint8_t *dst,
            unsigned *dst_len,
            const uint8_t *src,
            unsigned *src_len)
{
	z_stream stream;
	int ret;

	ret = inflateInit(&stream);
	if (ret != Z_OK)
		return ret;
	stream.avail_in = *src_len;
	stream.next_in = src;
	stream.avail_out = *dst_len;
	stream.next_out = dst;
	ret = inflate(&stream, Z_NO_FLUSH);
	*dst_len -= stream.avail_out;
	*src_len -= stream.avail_in;
	inflateEnd(&stream);
	if (ret == Z_OK)
		return Z_BUF_ERROR;
	return ret;
}

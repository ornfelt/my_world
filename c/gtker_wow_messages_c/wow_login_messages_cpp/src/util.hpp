#ifndef WOW_LOGIN_MESSAGES_CPP_UTIL_H
#define WOW_LOGIN_MESSAGES_CPP_UTIL_H

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace wow_login_messages {

class Writer
{
public:
    explicit Writer(const size_t size) { m_buf.reserve(size); }

    void write_u8(const uint8_t value) { m_buf.push_back(value); }

    void write_u16(const uint16_t value)
    {
        m_buf.push_back(static_cast<unsigned char>(value));
        m_buf.push_back(static_cast<unsigned char>(value >> 8));
    }

    void write_u32(const uint32_t value)
    {
        m_buf.push_back(static_cast<unsigned char>(value));
        m_buf.push_back(static_cast<unsigned char>(value >> 8));
        m_buf.push_back(static_cast<unsigned char>(value >> 16));
        m_buf.push_back(static_cast<unsigned char>(value >> 24));
    }

    void write_u64(const uint64_t value)
    {
        m_buf.push_back(static_cast<unsigned char>(value));
        m_buf.push_back(static_cast<unsigned char>(value >> 8));
        m_buf.push_back(static_cast<unsigned char>(value >> 16));
        m_buf.push_back(static_cast<unsigned char>(value >> 24));
        m_buf.push_back(static_cast<unsigned char>(value >> 32));
        m_buf.push_back(static_cast<unsigned char>(value >> 40));
        m_buf.push_back(static_cast<unsigned char>(value >> 48));
        m_buf.push_back(static_cast<unsigned char>(value >> 56));
    }

    void write_i32(const int32_t value)
    {
        uint32_t val;
        memcpy(&val, &value, 4);
        return write_u32(val);
    }

    void write_float(const float value)
    {
        uint32_t val;
        memcpy(&val, &value, 4);
        return write_u32(val);
    }

    void write_bool8(const bool value) { return write_u8(value ? 1 : 0); }

    void write_string(const std::string& value)
    {
        const uint8_t length = static_cast<uint8_t>(value.size());
        write_u8(length);

        for (const auto& v : value)
        {
            write_u8(v);
        }
    }

    void write_cstring(const std::string& value)
    {
        for (const auto& v : value)
        {
            write_u8(v);
        }

        write_u8(0);
    }

    std::vector<unsigned char> m_buf;
};

}  // namespace wow_login_messages
#endif /* WOW_LOGIN_MESSAGES_CPP_UTIL_H */

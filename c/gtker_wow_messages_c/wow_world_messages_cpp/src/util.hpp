#ifndef WOW_WORLD_MESSAGES_CPP_UTIL_HPP
#define WOW_WORLD_MESSAGES_CPP_UTIL_HPP

#include "wow_world_messages_cpp/wow_world_messages.hpp"

namespace wow_world_messages {

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

    void write_u16_be_at_first_index(const uint16_t value)
    {
        m_buf[1] = static_cast<uint8_t>(value);
        m_buf[0] = static_cast<uint8_t>(value >> 8);
    }

    void write_u16_be(const uint16_t value)
    {
        m_buf.push_back(static_cast<unsigned char>(value >> 8));
        m_buf.push_back(static_cast<unsigned char>(value));
    }

    void write_u32(const uint32_t value)
    {
        m_buf.push_back(static_cast<unsigned char>(value));
        m_buf.push_back(static_cast<unsigned char>(value >> 8));
        m_buf.push_back(static_cast<unsigned char>(value >> 16));
        m_buf.push_back(static_cast<unsigned char>(value >> 24));
    }

    void write_u48(const uint64_t value)
    {
        m_buf.push_back(static_cast<unsigned char>(value));
        m_buf.push_back(static_cast<unsigned char>(value >> 8));
        m_buf.push_back(static_cast<unsigned char>(value >> 16));
        m_buf.push_back(static_cast<unsigned char>(value >> 24));
        m_buf.push_back(static_cast<unsigned char>(value >> 40));
        m_buf.push_back(static_cast<unsigned char>(value >> 32));
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
        std::memcpy(&val, &value, 4);
        return write_u32(val);
    }

    void write_float(const float value)
    {
        uint32_t val;
        std::memcpy(&val, &value, 4);
        return write_u32(val);
    }

    void write_bool8(const bool value) { return write_u8(value ? 1 : 0); }

    void write_bool32(const bool value) { return write_u32(value ? 1 : 0); }

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

    void write_sized_cstring(const std::string& value)
    {
        write_u32(static_cast<uint32_t>(value.size() + 1));
        write_cstring(value);
    }

    void write_packed_guid(uint64_t value)
    {
        uint8_t output[8];
        int output_index = 0;
        uint8_t header = 0;

        for (int i = 0; i < 8; ++i)
        {
            const uint8_t v = (value >> i * 8) & 0xff;
            const bool byte_has_value = v != 0;
            if (byte_has_value)
            {
                output[output_index++] = v;
                header |= 1 << i;
            }
        }

        write_u8(header);
        for (int i = 0; i < output_index; ++i)
        {
            write_u8(output[i]);
        }
    }


    std::vector<unsigned char> m_buf;
};

class ByteReader final : public wow_world_messages::Reader
{
public:
    explicit ByteReader(std::vector<unsigned char> buf) : m_buf(std::move(buf)) {}

    uint8_t read_u8() override
    {
        const uint8_t value = m_buf[m_index];
        m_index += 1;

        return value;
    }

    bool is_at_end() const { return m_index >= m_buf.size(); }

    std::vector<unsigned char> m_buf;
    size_t m_index = 0;
};

namespace util {
size_t wwm_packed_guid_size(uint64_t value);
size_t wwm_monster_move_spline_size(const std::vector<all::Vector3d>& splines);
void wwm_write_monster_move_spline(Writer& writer, const std::vector<all::Vector3d>& splines);
std::vector<all::Vector3d> wwm_read_monster_move_spline(Reader& reader);

size_t wwm_named_guid_size(const NamedGuid& guid);
void wwm_write_named_guid(Writer& writer, const NamedGuid& guid);
NamedGuid wwm_read_named_guid(Reader& reader);

size_t wwm_variable_item_random_property_size(const VariableItemRandomProperty& property);
void wwm_write_variable_item_random_property(Writer& writer, const VariableItemRandomProperty& property);
VariableItemRandomProperty wwm_read_variable_item_random_property(Reader& reader);

std::vector<unsigned char> compress_data(const std::vector<unsigned char>& buffer);
std::vector<unsigned char> decompress_data(const std::vector<unsigned char>& buffer);

void update_mask_set_u32(uint32_t* headers, uint32_t* values, uint32_t offset, uint32_t value);
uint32_t update_mask_get_u32(const uint32_t* headers, const uint32_t* values, uint32_t offset);

void update_mask_set_u64(uint32_t* headers, uint32_t* values, uint32_t offset, uint64_t value);
uint64_t update_mask_get_u64(const uint32_t* headers, const uint32_t* values, uint32_t offset);

void update_mask_set_float(uint32_t* headers, uint32_t* values, uint32_t offset, float value);
float update_mask_get_float(const uint32_t* headers, const uint32_t* values, uint32_t offset);

void update_mask_set_two_shorts(uint32_t* headers, uint32_t* values, uint32_t offset, std::pair<uint16_t, uint16_t> value);
std::pair<uint16_t, uint16_t> update_mask_get_two_shorts(const uint32_t* headers, const uint32_t* values, uint32_t offset);

void update_mask_set_bytes(uint32_t* headers, uint32_t* values, uint32_t offset, const std::array<uint8_t, 4>& value);
std::array<uint8_t, 4> update_mask_get_bytes(const uint32_t* headers, const uint32_t* values, uint32_t offset);

}  // namespace util

} /* namespace wow_world_messages */

#endif /* WOW_WORLD_MESSAGES_CPP_UTIL_HPP */

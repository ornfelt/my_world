#ifndef WWM_WOW_WORLD_MESSAGES_CPP_H
#define WWM_WOW_WORLD_MESSAGES_CPP_H


#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "wow_world_messages_cpp/all.hpp"

#include "wow_world_messages_cpp_export.h"

namespace wow_world_messages {

class Reader
{
public:
    virtual uint8_t read_u8() = 0;

    virtual uint16_t read_u16()
    {
        const uint8_t lower = read_u8();
        const uint8_t upper = read_u8();

        return static_cast<uint16_t>(lower) | static_cast<uint16_t>(upper) << 8;
    }

    virtual void read_bytes(unsigned char* destination, size_t length)
    {
        for (size_t i = 0; i < length; ++i)
        {
            destination[i] = read_u8();
        }
    }

    virtual uint16_t read_u16_be()
    {
        const uint8_t upper = read_u8();
        const uint8_t lower = read_u8();

        return static_cast<uint16_t>(lower) | static_cast<uint16_t>(upper) << 8;
    }

    virtual uint32_t read_u32()
    {
        const uint16_t lower = read_u16();
        const uint16_t upper = read_u16();

        return static_cast<uint32_t>(lower) | static_cast<uint32_t>(upper) << 16;
    }

    virtual uint64_t read_u48()
    {
        const uint32_t lower = read_u32();
        const uint16_t upper = read_u16();

        return static_cast<uint64_t>(lower) | static_cast<uint64_t>(upper) << 32;
    }

    virtual uint64_t read_u64()
    {
        const uint32_t lower = read_u32();
        const uint32_t upper = read_u32();

        return static_cast<uint64_t>(lower) | static_cast<uint64_t>(upper) << 32;
    }

    virtual int32_t read_i32()
    {
        auto v = read_u32();
        int32_t value;
        std::memcpy(&value, &v, 4);
        return value;
    }

    virtual float read_float()
    {
        auto v = read_u32();
        float value;
        std::memcpy(&value, &v, 4);

        return value;
    }

    virtual bool read_bool8() { return read_u8() == 1; }

    virtual bool read_bool32() { return read_u32() == 1; }

    virtual std::string read_string()
    {
        const uint8_t length = read_u8();
        std::string str;

        for (uint8_t i = 0; i < length; ++i)
        {
            str.push_back(static_cast<char>(read_u8()));
        }

        return str;
    }

    virtual std::string read_cstring()
    {
        std::string str;

        uint8_t val = read_u8();
        while (val != 0)
        {
            str.push_back(static_cast<char>(val));
            val = read_u8();
        }

        return str;
    }

    virtual std::string read_sized_cstring()
    {
        const uint32_t length = read_u32();
        (void)length; /* this could be used in a specialized method */
        return read_cstring();
    }

    virtual uint64_t read_packed_guid()
    {
        uint64_t value = 0;

        const auto header = read_u8();

        for (uint64_t i = 0; i < 8; ++i)
        {
            const bool byte_has_value = (header & (1 << i)) != 0;
            if (byte_has_value)
            {
                value |= static_cast<uint64_t>(read_u8()) << i * 8;
            }
        }

        return value;
    }

    virtual ~Reader() = default;
};

class NamedGuid
{
public:
    WOW_WORLD_MESSAGES_CPP_EXPORT NamedGuid() : m_guid(0), m_name() {}
    WOW_WORLD_MESSAGES_CPP_EXPORT NamedGuid(uint64_t guid, std::string&& name) : m_guid(guid), m_name(name) {}

    WOW_WORLD_MESSAGES_CPP_EXPORT const std::string* name() const
    {
        if (m_guid != 0)
        {
            return &m_name;
        }

        return nullptr;
    }
    WOW_WORLD_MESSAGES_CPP_EXPORT uint64_t guid() const { return m_guid; }

private:
    uint64_t m_guid;
    std::string m_name;
};

class VariableItemRandomProperty
{
public:
    WOW_WORLD_MESSAGES_CPP_EXPORT VariableItemRandomProperty() : m_item_random_property_id(0), m_item_suffix_factor(0)
    {
    }

    WOW_WORLD_MESSAGES_CPP_EXPORT VariableItemRandomProperty(uint32_t item_random_property_id,
                                                             uint32_t item_suffix_factor) :
        m_item_random_property_id(item_random_property_id), m_item_suffix_factor(item_suffix_factor)
    {
    }

    WOW_WORLD_MESSAGES_CPP_EXPORT uint32_t item_random_property_id() const { return m_item_random_property_id; }

    WOW_WORLD_MESSAGES_CPP_EXPORT const uint32_t* item_suffix_factor() const
    {
        if (m_item_random_property_id != 0)
        {
            return &m_item_suffix_factor;
        }

        return nullptr;
    }

private:
    uint32_t m_item_random_property_id;
    uint32_t m_item_suffix_factor;
};

struct bad_opcode_access final : std::exception
{
};

} /* namespace wow_world_messages */

#endif /* WWM_WOW_WORLD_MESSAGES_CPP_H */

#include "DBC.hpp"

#include "parser/MpqManager.hpp"
#include "utility/BinaryStream.hpp"
#include "utility/Exception.hpp"

#include <cassert>
#include <cstdint>
#include <memory>
#include <string>

#pragma pack(push, 1)
// taken from https://wowdev.wiki/DBC
struct dbc_header
{
    std::uint32_t magic;        // always 'WDBC'
    std::uint32_t record_count; // records per file
    std::uint32_t field_count;  // fields per record
    std::uint32_t
        record_size; // sum (sizeof (field_type_i)) | 0 <= i < field_count.
                     // field_type_i is NOT defined in the files.
    std::uint32_t string_block_size;
};
#pragma pack(pop)

namespace parser
{
DBC::DBC(const std::string& filename)
{
    auto in = sMpqManager.OpenFile(filename);

    if (!in)
        THROW_MSG("Failed to open DBC " + filename, Result::FAILED_TO_OPEN_DBC);

    dbc_header header;
    *in >> header;

    if (header.magic != Magic)
        THROW(Result::UNRECOGNIZED_DBC_FILE);

    assert(header.field_count * sizeof(std::uint32_t) == header.record_size);

    m_recordCount = header.record_count;
    m_fieldCount = header.field_count;

    m_data.resize(header.record_count * header.field_count);
    in->ReadBytes(&m_data[0], m_data.size() * sizeof(std::uint32_t));

    // ensure that we have precisely enough space left for the string block
    assert(header.string_block_size + in->rpos() == in->wpos());
    m_string.resize(header.string_block_size);
    in->ReadBytes(&m_string[0], m_string.size());
}

std::uint32_t DBC::GetField(int row, int column) const
{
    auto const offset = row * m_fieldCount + column;

    if (offset > m_data.size() || column >= m_fieldCount)
        THROW(Result::INVALID_ROW_COLUMN_FROM_DBC);

    return m_data[offset];
}

std::string DBC::GetStringField(int row, int column) const
{
    auto const pos = GetField(row, column);
    return std::string(reinterpret_cast<const char*>(&m_string[pos]));
}
} // namespace parser

/* clang-format off */

#include "util.hpp"

#include "wow_world_messages_cpp/all.hpp"

namespace wow_world_messages {
namespace all {
Vector3d Vector3d_read(Reader& reader) {
    Vector3d obj{};

    obj.x = reader.read_float();

    obj.y = reader.read_float();

    obj.z = reader.read_float();

    return obj;
}

void Vector3d_write(Writer& writer, const Vector3d& obj) {
    writer.write_float(obj.x);

    writer.write_float(obj.y);

    writer.write_float(obj.z);

}

Vector2d Vector2d_read(Reader& reader) {
    Vector2d obj{};

    obj.x = reader.read_float();

    obj.y = reader.read_float();

    return obj;
}

void Vector2d_write(Writer& writer, const Vector2d& obj) {
    writer.write_float(obj.x);

    writer.write_float(obj.y);

}

} // namespace all
} // namespace wow_world_messages

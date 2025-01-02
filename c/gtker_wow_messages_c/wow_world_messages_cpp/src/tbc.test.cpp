/* clang-format off */
#include "wow_world_messages_cpp/tbc.hpp"
#include "test_utils.h"

class ByteReader final : public ::wow_world_messages::Reader
{
public:
    explicit ByteReader(std::vector<unsigned char> buf) : m_buf(std::move(buf)) { }

    uint8_t read_u8() override
    {
        const uint8_t value = m_buf[m_index];
        m_index += 1;

        return value;
    }

    std::vector<unsigned char> m_buf;
    size_t m_index = 0;
};

int main() {
    /* CMSG_TELEPORT_TO_UNIT */ {
        const std::vector<unsigned char> buffer = {0, 11, 9, 0, 0, 0, 86, 117, 114, 116, 110, 101, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::CMSG_TELEPORT_TO_UNIT>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_TELEPORT_TO_UNIT 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_TELEPORT_TO_UNIT 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* CMSG_CHAR_ENUM */ {
        const std::vector<unsigned char> buffer = {0, 4, 55, 0, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::CMSG_CHAR_ENUM>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_CHAR_ENUM 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_CHAR_ENUM 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* CMSG_CHAR_DELETE */ {
        const std::vector<unsigned char> buffer = {0, 12, 56, 0, 0, 0, 239, 190, 173, 222, 0, 0, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::CMSG_CHAR_DELETE>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_CHAR_DELETE 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_CHAR_DELETE 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* CMSG_PLAYER_LOGIN */ {
        const std::vector<unsigned char> buffer = {0, 12, 61, 0, 0, 0, 239, 190, 173, 222, 0, 0, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::CMSG_PLAYER_LOGIN>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_PLAYER_LOGIN 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_PLAYER_LOGIN 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* SMSG_LOGIN_SETTIMESPEED */ {
        const std::vector<unsigned char> buffer = {0, 10, 66, 0, 10, 50, 115, 22, 137, 136, 136, 60, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ServerOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::SMSG_LOGIN_SETTIMESPEED>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " SMSG_LOGIN_SETTIMESPEED 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " SMSG_LOGIN_SETTIMESPEED 0", TEST_UTILS_SIDE_SERVER);
    }
    /* CMSG_PLAYER_LOGOUT */ {
        const std::vector<unsigned char> buffer = {0, 4, 74, 0, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::CMSG_PLAYER_LOGOUT>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_PLAYER_LOGOUT 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_PLAYER_LOGOUT 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* CMSG_LOGOUT_REQUEST */ {
        const std::vector<unsigned char> buffer = {0, 4, 75, 0, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::CMSG_LOGOUT_REQUEST>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_LOGOUT_REQUEST 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_LOGOUT_REQUEST 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* SMSG_LOGOUT_RESPONSE */ {
        const std::vector<unsigned char> buffer = {0, 7, 76, 0, 0, 0, 0, 0, 1, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ServerOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::SMSG_LOGOUT_RESPONSE>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " SMSG_LOGOUT_RESPONSE 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " SMSG_LOGOUT_RESPONSE 0", TEST_UTILS_SIDE_SERVER);
    }
    /* SMSG_LOGOUT_COMPLETE */ {
        const std::vector<unsigned char> buffer = {0, 2, 77, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ServerOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::SMSG_LOGOUT_COMPLETE>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " SMSG_LOGOUT_COMPLETE 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " SMSG_LOGOUT_COMPLETE 0", TEST_UTILS_SIDE_SERVER);
    }
    /* CMSG_PET_NAME_QUERY */ {
        const std::vector<unsigned char> buffer = {0, 16, 82, 0, 0, 0, 239, 190, 173, 222, 239, 190, 173, 222, 222, 202, 250, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::CMSG_PET_NAME_QUERY>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_PET_NAME_QUERY 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_PET_NAME_QUERY 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* SMSG_DESTROY_OBJECT */ {
        const std::vector<unsigned char> buffer = {0, 10, 170, 0, 6, 0, 0, 0, 0, 0, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ServerOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::SMSG_DESTROY_OBJECT>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " SMSG_DESTROY_OBJECT 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " SMSG_DESTROY_OBJECT 0", TEST_UTILS_SIDE_SERVER);
    }
    /* MSG_MOVE_TELEPORT_ACK_Client */ {
        const std::vector<unsigned char> buffer = {0, 13, 199, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 38, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::MSG_MOVE_TELEPORT_ACK_Client>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " MSG_MOVE_TELEPORT_ACK_Client 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " MSG_MOVE_TELEPORT_ACK_Client 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* SMSG_TUTORIAL_FLAGS */ {
        const std::vector<unsigned char> buffer = {0, 34, 253, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ServerOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::SMSG_TUTORIAL_FLAGS>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " SMSG_TUTORIAL_FLAGS 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " SMSG_TUTORIAL_FLAGS 0", TEST_UTILS_SIDE_SERVER);
    }
    /* CMSG_STANDSTATECHANGE */ {
        const std::vector<unsigned char> buffer = {0, 8, 1, 1, 0, 0, 1, 0, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::CMSG_STANDSTATECHANGE>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_STANDSTATECHANGE 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_STANDSTATECHANGE 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* CMSG_AUTOEQUIP_ITEM */ {
        const std::vector<unsigned char> buffer = {0, 6, 10, 1, 0, 0, 255, 24, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::CMSG_AUTOEQUIP_ITEM>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_AUTOEQUIP_ITEM 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_AUTOEQUIP_ITEM 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* CMSG_INITIATE_TRADE */ {
        const std::vector<unsigned char> buffer = {0, 12, 22, 1, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::CMSG_INITIATE_TRADE>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_INITIATE_TRADE 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_INITIATE_TRADE 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* CMSG_CANCEL_TRADE */ {
        const std::vector<unsigned char> buffer = {0, 4, 28, 1, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::CMSG_CANCEL_TRADE>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_CANCEL_TRADE 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_CANCEL_TRADE 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* SMSG_INITIAL_SPELLS */ {
        const std::vector<unsigned char> buffer = {0, 167, 42, 1, 0, 40, 0, 78, 0, 0, 0, 81, 0, 0, 0, 107, 0, 0, 0, 196, 0, 0, 0, 198, 0, 0, 0, 201, 0, 0, 0, 203, 0, 0, 0, 204, 0, 0, 0, 10, 2, 0, 0, 156, 2, 0, 0, 78, 9, 0, 0, 153, 9, 0, 0, 175, 9, 0, 0, 234, 11, 0, 0, 37, 13, 0, 0, 181, 20, 0, 0, 89, 24, 0, 0, 102, 24, 0, 0, 103, 24, 0, 0, 77, 25, 0, 0, 78, 25, 0, 0, 203, 25, 0, 0, 98, 28, 0, 0, 99, 28, 0, 0, 187, 28, 0, 0, 194, 32, 0, 0, 33, 34, 0, 0, 117, 35, 0, 0, 118, 35, 0, 0, 156, 35, 0, 0, 165, 35, 0, 0, 117, 80, 0, 0, 118, 80, 0, 0, 119, 80, 0, 0, 120, 80, 0, 0, 128, 81, 0, 0, 147, 84, 0, 0, 148, 84, 0, 0, 11, 86, 0, 0, 26, 89, 0, 0, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ServerOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::SMSG_INITIAL_SPELLS>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " SMSG_INITIAL_SPELLS 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " SMSG_INITIAL_SPELLS 0", TEST_UTILS_SIDE_SERVER);
    }
    /* CMSG_CANCEL_CAST */ {
        const std::vector<unsigned char> buffer = {0, 8, 47, 1, 0, 0, 120, 80, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::CMSG_CANCEL_CAST>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_CANCEL_CAST 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_CANCEL_CAST 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* CMSG_CANCEL_CAST */ {
        const std::vector<unsigned char> buffer = {0, 8, 47, 1, 0, 0, 242, 33, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::CMSG_CANCEL_CAST>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_CANCEL_CAST 1 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_CANCEL_CAST 1", TEST_UTILS_SIDE_CLIENT);
    }
    /* CMSG_SET_SELECTION */ {
        const std::vector<unsigned char> buffer = {0, 12, 61, 1, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::CMSG_SET_SELECTION>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_SET_SELECTION 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_SET_SELECTION 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* CMSG_ATTACKSWING */ {
        const std::vector<unsigned char> buffer = {0, 12, 65, 1, 0, 0, 100, 0, 0, 0, 0, 0, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::CMSG_ATTACKSWING>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_ATTACKSWING 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_ATTACKSWING 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* SMSG_ATTACKSTART */ {
        const std::vector<unsigned char> buffer = {0, 18, 67, 1, 23, 0, 0, 0, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ServerOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::SMSG_ATTACKSTART>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " SMSG_ATTACKSTART 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " SMSG_ATTACKSTART 0", TEST_UTILS_SIDE_SERVER);
    }
    /* SMSG_ATTACKSTOP */ {
        const std::vector<unsigned char> buffer = {0, 10, 68, 1, 1, 23, 1, 100, 0, 0, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ServerOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::SMSG_ATTACKSTOP>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " SMSG_ATTACKSTOP 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " SMSG_ATTACKSTOP 0", TEST_UTILS_SIDE_SERVER);
    }
    /* CMSG_QUERY_TIME */ {
        const std::vector<unsigned char> buffer = {0, 4, 206, 1, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::CMSG_QUERY_TIME>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_QUERY_TIME 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_QUERY_TIME 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* CMSG_PING */ {
        const std::vector<unsigned char> buffer = {0, 12, 220, 1, 0, 0, 239, 190, 173, 222, 222, 202, 250, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::CMSG_PING>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_PING 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_PING 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* SMSG_PONG */ {
        const std::vector<unsigned char> buffer = {0, 6, 221, 1, 239, 190, 173, 222, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ServerOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::SMSG_PONG>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " SMSG_PONG 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " SMSG_PONG 0", TEST_UTILS_SIDE_SERVER);
    }
    /* CMSG_SETSHEATHED */ {
        const std::vector<unsigned char> buffer = {0, 8, 224, 1, 0, 0, 1, 0, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::CMSG_SETSHEATHED>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_SETSHEATHED 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_SETSHEATHED 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* SMSG_AUTH_CHALLENGE */ {
        const std::vector<unsigned char> buffer = {0, 6, 236, 1, 239, 190, 173, 222, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ServerOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::SMSG_AUTH_CHALLENGE>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " SMSG_AUTH_CHALLENGE 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " SMSG_AUTH_CHALLENGE 0", TEST_UTILS_SIDE_SERVER);
    }
    /* CMSG_AUTH_SESSION */ {
        const std::vector<unsigned char> buffer = {0, 172, 237, 1, 0, 0, 243, 22, 0, 0, 0, 0, 0, 0, 65, 0, 136, 2, 216, 73, 136, 157, 239, 5, 37, 187, 193, 171, 167, 138, 219, 164, 251, 163, 231, 126, 103, 172, 234, 198, 86, 1, 0, 0, 120, 156, 117, 204, 189, 14, 194, 48, 12, 4, 224, 242, 30, 188, 12, 97, 64, 149, 200, 66, 195, 140, 76, 226, 34, 11, 199, 169, 140, 203, 79, 159, 30, 22, 36, 6, 115, 235, 119, 119, 129, 105, 89, 64, 203, 105, 51, 103, 163, 38, 199, 190, 91, 213, 199, 122, 223, 125, 18, 190, 22, 192, 140, 113, 36, 228, 18, 73, 168, 194, 228, 149, 72, 10, 201, 197, 61, 216, 182, 122, 6, 75, 248, 52, 15, 21, 70, 115, 103, 187, 56, 204, 122, 199, 151, 139, 189, 220, 38, 204, 254, 48, 66, 214, 230, 202, 1, 168, 184, 144, 128, 81, 252, 183, 164, 80, 112, 184, 18, 243, 63, 38, 65, 253, 181, 55, 144, 25, 102, 143, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::CMSG_AUTH_SESSION>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_AUTH_SESSION 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        auto reader2 = ByteReader(write_buffer);

        auto opcode2 = ::wow_world_messages::tbc::ClientOpcode::read(reader2, [](unsigned char*, size_t){});
        if (opcode2.get_if<::wow_world_messages::tbc::CMSG_AUTH_SESSION>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_AUTH_SESSION 0 read invalid second opcode %s\n", opcode2.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer2 = opcode2.write([](unsigned char*, size_t){});

        world_test_compare_buffers(write_buffer.data(), write_buffer2.data(), write_buffer2.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_AUTH_SESSION 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* SMSG_ACCOUNT_DATA_TIMES */ {
        const std::vector<unsigned char> buffer = {0, 130, 9, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ServerOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::SMSG_ACCOUNT_DATA_TIMES>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " SMSG_ACCOUNT_DATA_TIMES 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " SMSG_ACCOUNT_DATA_TIMES 0", TEST_UTILS_SIDE_SERVER);
    }
    /* CMSG_REQUEST_ACCOUNT_DATA */ {
        const std::vector<unsigned char> buffer = {0, 8, 10, 2, 0, 0, 6, 0, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::CMSG_REQUEST_ACCOUNT_DATA>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_REQUEST_ACCOUNT_DATA 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_REQUEST_ACCOUNT_DATA 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* CMSG_UPDATE_ACCOUNT_DATA */ {
        const std::vector<unsigned char> buffer = {0, 12, 11, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::CMSG_UPDATE_ACCOUNT_DATA>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_UPDATE_ACCOUNT_DATA 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        auto reader2 = ByteReader(write_buffer);

        auto opcode2 = ::wow_world_messages::tbc::ClientOpcode::read(reader2, [](unsigned char*, size_t){});
        if (opcode2.get_if<::wow_world_messages::tbc::CMSG_UPDATE_ACCOUNT_DATA>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_UPDATE_ACCOUNT_DATA 0 read invalid second opcode %s\n", opcode2.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer2 = opcode2.write([](unsigned char*, size_t){});

        world_test_compare_buffers(write_buffer.data(), write_buffer2.data(), write_buffer2.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_UPDATE_ACCOUNT_DATA 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* CMSG_UPDATE_ACCOUNT_DATA */ {
        const std::vector<unsigned char> buffer = {4, 160, 11, 2, 0, 0, 7, 0, 0, 0, 24, 20, 0, 0, 120, 1, 189, 87, 219, 150, 171, 40, 16, 125, 231, 43, 252, 4, 181, 147, 182, 243, 72, 20, 35, 211, 68, 178, 4, 59, 39, 231, 133, 255, 255, 139, 41, 174, 130, 74, 39, 15, 51, 89, 221, 174, 104, 93, 54, 80, 84, 21, 155, 31, 50, 9, 202, 199, 162, 70, 8, 119, 29, 233, 126, 150, 111, 126, 147, 160, 81, 151, 153, 178, 78, 77, 164, 157, 102, 42, 175, 100, 148, 170, 29, 240, 56, 18, 86, 224, 89, 114, 132, 220, 151, 64, 100, 236, 16, 250, 203, 71, 226, 37, 69, 93, 158, 154, 234, 120, 4, 27, 206, 248, 36, 144, 192, 143, 162, 62, 30, 253, 131, 110, 120, 146, 143, 162, 106, 74, 243, 128, 6, 77, 152, 118, 70, 93, 213, 77, 81, 34, 51, 118, 241, 121, 48, 162, 207, 3, 226, 125, 79, 91, 50, 105, 73, 117, 170, 225, 7, 61, 8, 99, 78, 171, 63, 239, 3, 21, 55, 48, 208, 163, 84, 245, 151, 254, 245, 50, 69, 199, 158, 79, 215, 68, 69, 174, 92, 146, 32, 1, 127, 73, 254, 72, 181, 145, 138, 135, 144, 196, 122, 106, 224, 18, 93, 249, 8, 146, 73, 197, 11, 170, 142, 167, 32, 95, 205, 202, 155, 111, 128, 125, 36, 205, 116, 97, 65, 176, 40, 31, 79, 245, 15, 167, 163, 89, 165, 94, 7, 60, 65, 193, 8, 254, 33, 251, 26, 42, 228, 174, 98, 228, 18, 226, 102, 85, 171, 97, 172, 74, 205, 2, 162, 166, 99, 234, 30, 132, 251, 239, 16, 24, 29, 197, 110, 244, 27, 99, 163, 74, 47, 35, 159, 136, 149, 149, 16, 18, 241, 77, 97, 39, 190, 142, 250, 95, 219, 51, 206, 101, 97, 55, 182, 132, 253, 191, 158, 177, 84, 87, 42, 90, 179, 13, 102, 111, 252, 254, 248, 232, 248, 189, 171, 154, 19, 228, 131, 121, 188, 163, 32, 172, 87, 3, 149, 194, 76, 73, 71, 75, 15, 225, 80, 141, 18, 160, 5, 217, 85, 223, 136, 204, 186, 106, 221, 47, 158, 58, 61, 243, 190, 70, 155, 247, 238, 39, 10, 21, 193, 30, 55, 134, 31, 144, 42, 185, 217, 175, 204, 242, 120, 3, 23, 146, 50, 242, 4, 46, 181, 202, 163, 181, 19, 193, 114, 158, 136, 250, 17, 42, 141, 238, 161, 41, 14, 141, 143, 237, 198, 44, 66, 204, 26, 154, 178, 206, 46, 56, 134, 180, 150, 17, 230, 106, 107, 99, 219, 240, 158, 139, 100, 48, 128, 37, 133, 247, 60, 182, 15, 188, 234, 32, 18, 195, 94, 98, 185, 88, 230, 13, 254, 220, 212, 5, 235, 42, 173, 42, 243, 232, 164, 132, 246, 195, 152, 13, 105, 135, 175, 248, 98, 219, 139, 94, 23, 212, 200, 162, 59, 207, 125, 31, 198, 244, 26, 157, 142, 43, 167, 5, 82, 43, 19, 175, 72, 101, 82, 49, 239, 105, 212, 25, 95, 31, 6, 151, 87, 89, 144, 149, 93, 6, 45, 77, 191, 44, 88, 106, 150, 193, 10, 123, 232, 83, 212, 195, 149, 117, 209, 124, 22, 117, 213, 184, 128, 110, 12, 95, 0, 180, 153, 231, 17, 109, 67, 49, 77, 197, 238, 81, 12, 105, 77, 95, 192, 12, 78, 47, 193, 6, 235, 12, 178, 156, 112, 71, 76, 87, 77, 219, 154, 157, 160, 29, 66, 12, 148, 176, 78, 40, 56, 166, 117, 17, 135, 132, 90, 82, 99, 99, 199, 229, 0, 71, 252, 142, 37, 158, 39, 172, 46, 112, 124, 231, 160, 22, 3, 19, 145, 95, 33, 204, 40, 59, 22, 20, 206, 81, 69, 70, 125, 238, 25, 46, 177, 55, 145, 51, 180, 166, 111, 165, 71, 219, 1, 128, 195, 157, 242, 142, 182, 187, 21, 182, 44, 59, 53, 211, 17, 222, 27, 41, 88, 61, 201, 134, 149, 221, 83, 184, 85, 173, 100, 211, 33, 224, 174, 28, 158, 14, 144, 214, 207, 115, 252, 212, 254, 41, 124, 72, 205, 231, 200, 193, 52, 7, 218, 99, 56, 180, 58, 197, 120, 139, 153, 237, 49, 201, 174, 186, 51, 124, 224, 192, 38, 108, 51, 173, 107, 160, 123, 80, 226, 85, 137, 206, 23, 101, 169, 151, 26, 201, 12, 245, 96, 153, 94, 85, 3, 173, 136, 149, 152, 49, 138, 71, 224, 55, 154, 110, 128, 243, 199, 41, 242, 28, 248, 212, 217, 30, 172, 89, 138, 27, 174, 199, 173, 225, 182, 58, 15, 161, 67, 59, 134, 165, 39, 166, 41, 28, 89, 88, 106, 105, 24, 169, 2, 202, 213, 57, 86, 89, 87, 192, 76, 190, 62, 172, 252, 142, 167, 145, 142, 23, 187, 34, 167, 0, 146, 73, 128, 28, 41, 233, 153, 169, 231, 57, 154, 219, 170, 51, 23, 34, 34, 152, 30, 173, 167, 12, 248, 100, 68, 167, 128, 46, 73, 70, 46, 19, 159, 3, 239, 210, 132, 56, 22, 239, 205, 202, 17, 202, 202, 204, 200, 83, 57, 39, 172, 247, 132, 31, 123, 66, 203, 183, 87, 238, 150, 182, 175, 132, 208, 134, 161, 119, 174, 132, 205, 158, 208, 176, 198, 181, 229, 105, 207, 178, 42, 19, 169, 185, 86, 220, 233, 216, 241, 123, 81, 33, 65, 255, 194, 62, 195, 62, 194, 133, 2, 246, 219, 252, 1, 215, 108, 191, 33, 118, 21, 234, 252, 139, 24, 248, 29, 206, 101, 132, 174, 68, 8, 56, 132, 225, 238, 97, 56, 188, 190, 130, 152, 59, 131, 191, 21, 216, 59, 136, 189, 104, 32, 159, 204, 158, 106, 91, 82, 107, 168, 172, 189, 221, 184, 72, 254, 122, 215, 137, 231, 91, 191, 54, 95, 48, 51, 243, 45, 163, 249, 174, 41, 179, 79, 221, 64, 214, 18, 129, 165, 57, 94, 228, 41, 111, 252, 157, 90, 164, 13, 65, 51, 42, 111, 155, 106, 82, 47, 31, 159, 132, 52, 122, 199, 141, 50, 245, 245, 45, 206, 178, 41, 239, 228, 70, 75, 133, 142, 86, 197, 116, 201, 54, 162, 88, 162, 251, 141, 251, 94, 104, 83, 36, 136, 244, 233, 146, 18, 168, 84, 21, 249, 108, 86, 147, 184, 109, 180, 145, 103, 116, 110, 187, 233, 88, 215, 213, 89, 237, 116, 203, 161, 170, 55, 214, 9, 55, 167, 164, 147, 47, 39, 163, 19, 132, 3, 196, 100, 69, 50, 197, 84, 165, 231, 7, 53, 96, 248, 103, 208, 164, 139, 223, 247, 78, 109, 118, 97, 66, 48, 246, 17, 130, 218, 58, 111, 26, 190, 79, 133, 180, 39, 219, 62, 252, 180, 234, 160, 98, 226, 122, 251, 120, 173, 222, 128, 111, 111, 234, 205, 192, 252, 82, 223, 171, 145, 14, 111, 27, 9, 174, 20, 175, 244, 188, 255, 96, 77, 159, 111, 27, 9, 152, 251, 155, 214, 244, 245, 182, 145, 78, 111, 27, 9, 200, 207, 255, 30, 190, 127, 1, 52, 153, 2, 36, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::CMSG_UPDATE_ACCOUNT_DATA>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_UPDATE_ACCOUNT_DATA 1 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        auto reader2 = ByteReader(write_buffer);

        auto opcode2 = ::wow_world_messages::tbc::ClientOpcode::read(reader2, [](unsigned char*, size_t){});
        if (opcode2.get_if<::wow_world_messages::tbc::CMSG_UPDATE_ACCOUNT_DATA>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_UPDATE_ACCOUNT_DATA 1 read invalid second opcode %s\n", opcode2.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer2 = opcode2.write([](unsigned char*, size_t){});

        world_test_compare_buffers(write_buffer.data(), write_buffer2.data(), write_buffer2.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_UPDATE_ACCOUNT_DATA 1", TEST_UTILS_SIDE_CLIENT);
    }
    /* CMSG_GMTICKET_GETTICKET */ {
        const std::vector<unsigned char> buffer = {0, 4, 17, 2, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::CMSG_GMTICKET_GETTICKET>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_GMTICKET_GETTICKET 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_GMTICKET_GETTICKET 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* MSG_AUCTION_HELLO_Client */ {
        const std::vector<unsigned char> buffer = {0, 12, 85, 2, 0, 0, 239, 190, 173, 222, 0, 0, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::MSG_AUCTION_HELLO_Client>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " MSG_AUCTION_HELLO_Client 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " MSG_AUCTION_HELLO_Client 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* CMSG_SET_ACTIVE_MOVER */ {
        const std::vector<unsigned char> buffer = {0, 12, 106, 2, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::CMSG_SET_ACTIVE_MOVER>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_SET_ACTIVE_MOVER 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_SET_ACTIVE_MOVER 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* MSG_QUERY_NEXT_MAIL_TIME_Client */ {
        const std::vector<unsigned char> buffer = {0, 4, 132, 2, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::MSG_QUERY_NEXT_MAIL_TIME_Client>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " MSG_QUERY_NEXT_MAIL_TIME_Client 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " MSG_QUERY_NEXT_MAIL_TIME_Client 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* CMSG_MEETINGSTONE_INFO */ {
        const std::vector<unsigned char> buffer = {0, 4, 150, 2, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::CMSG_MEETINGSTONE_INFO>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_MEETINGSTONE_INFO 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_MEETINGSTONE_INFO 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* CMSG_CHAR_RENAME */ {
        const std::vector<unsigned char> buffer = {0, 21, 199, 2, 0, 0, 239, 190, 173, 222, 0, 0, 0, 0, 68, 101, 97, 100, 98, 101, 101, 102, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::CMSG_CHAR_RENAME>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_CHAR_RENAME 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_CHAR_RENAME 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* CMSG_REQUEST_RAID_INFO */ {
        const std::vector<unsigned char> buffer = {0, 4, 205, 2, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::CMSG_REQUEST_RAID_INFO>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_REQUEST_RAID_INFO 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_REQUEST_RAID_INFO 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* CMSG_MOVE_TIME_SKIPPED */ {
        const std::vector<unsigned char> buffer = {0, 16, 206, 2, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, 32, 0, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::CMSG_MOVE_TIME_SKIPPED>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_MOVE_TIME_SKIPPED 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_MOVE_TIME_SKIPPED 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* CMSG_BATTLEFIELD_STATUS */ {
        const std::vector<unsigned char> buffer = {0, 4, 211, 2, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::CMSG_BATTLEFIELD_STATUS>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_BATTLEFIELD_STATUS 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_BATTLEFIELD_STATUS 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* SMSG_SPLINE_SET_RUN_SPEED */ {
        const std::vector<unsigned char> buffer = {0, 8, 254, 2, 1, 6, 0, 0, 224, 64, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ServerOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::SMSG_SPLINE_SET_RUN_SPEED>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " SMSG_SPLINE_SET_RUN_SPEED 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " SMSG_SPLINE_SET_RUN_SPEED 0", TEST_UTILS_SIDE_SERVER);
    }
    /* SMSG_MOTD */ {
        const std::vector<unsigned char> buffer = {0, 116, 61, 3, 2, 0, 0, 0, 87, 101, 108, 99, 111, 109, 101, 32, 116, 111, 32, 97, 110, 32, 65, 122, 101, 114, 111, 116, 104, 67, 111, 114, 101, 32, 115, 101, 114, 118, 101, 114, 46, 0, 124, 99, 102, 102, 70, 70, 52, 65, 50, 68, 84, 104, 105, 115, 32, 115, 101, 114, 118, 101, 114, 32, 114, 117, 110, 115, 32, 111, 110, 32, 65, 122, 101, 114, 111, 116, 104, 67, 111, 114, 101, 124, 114, 32, 124, 99, 102, 102, 51, 67, 69, 55, 70, 70, 119, 119, 119, 46, 97, 122, 101, 114, 111, 116, 104, 99, 111, 114, 101, 46, 111, 114, 103, 124, 114, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::tbc::ServerOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::tbc::SMSG_MOTD>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " SMSG_MOTD 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " SMSG_MOTD 0", TEST_UTILS_SIDE_SERVER);
    }
    return 0;
}

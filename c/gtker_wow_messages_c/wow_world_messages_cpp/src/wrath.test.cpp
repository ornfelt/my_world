/* clang-format off */
#include "wow_world_messages_cpp/wrath.hpp"
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
    /* CMSG_WORLD_TELEPORT */ {
        const std::vector<unsigned char> buffer = {0, 36, 8, 0, 0, 0, 239, 190, 173, 222, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 63, 0, 0, 0, 64, 0, 0, 64, 64, 0, 0, 128, 64, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::wrath::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::CMSG_WORLD_TELEPORT>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_WORLD_TELEPORT 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_WORLD_TELEPORT 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* CMSG_WORLD_TELEPORT */ {
        const std::vector<unsigned char> buffer = {0, 36, 8, 0, 0, 0, 154, 61, 9, 2, 213, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 226, 67, 0, 176, 201, 69, 0, 128, 30, 69, 219, 15, 73, 64, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::wrath::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::CMSG_WORLD_TELEPORT>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_WORLD_TELEPORT 1 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_WORLD_TELEPORT 1", TEST_UTILS_SIDE_CLIENT);
    }
    /* CMSG_TELEPORT_TO_UNIT */ {
        const std::vector<unsigned char> buffer = {0, 11, 9, 0, 0, 0, 86, 117, 114, 116, 110, 101, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::wrath::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::CMSG_TELEPORT_TO_UNIT>() == nullptr) {
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

        auto opcode = ::wow_world_messages::wrath::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::CMSG_CHAR_ENUM>() == nullptr) {
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

        auto opcode = ::wow_world_messages::wrath::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::CMSG_CHAR_DELETE>() == nullptr) {
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

        auto opcode = ::wow_world_messages::wrath::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::CMSG_PLAYER_LOGIN>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_PLAYER_LOGIN 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_PLAYER_LOGIN 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* CMSG_PLAYER_LOGOUT */ {
        const std::vector<unsigned char> buffer = {0, 4, 74, 0, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::wrath::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::CMSG_PLAYER_LOGOUT>() == nullptr) {
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

        auto opcode = ::wow_world_messages::wrath::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::CMSG_LOGOUT_REQUEST>() == nullptr) {
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

        auto opcode = ::wow_world_messages::wrath::ServerOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::SMSG_LOGOUT_RESPONSE>() == nullptr) {
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

        auto opcode = ::wow_world_messages::wrath::ServerOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::SMSG_LOGOUT_COMPLETE>() == nullptr) {
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

        auto opcode = ::wow_world_messages::wrath::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::CMSG_PET_NAME_QUERY>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_PET_NAME_QUERY 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_PET_NAME_QUERY 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* SMSG_UPDATE_OBJECT */ {
        const std::vector<unsigned char> buffer = {0, 115, 169, 0, 1, 0, 0, 0, 3, 1, 8, 4, 33, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 205, 215, 11, 198, 53, 126, 4, 195, 249, 15, 167, 66, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 63, 0, 0, 140, 66, 0, 0, 144, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 208, 15, 73, 64, 0, 0, 0, 0, 3, 7, 0, 0, 0, 0, 0, 128, 0, 24, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 25, 0, 0, 0, 1, 0, 0, 0, 12, 77, 0, 0, 12, 77, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::wrath::ServerOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::SMSG_UPDATE_OBJECT>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " SMSG_UPDATE_OBJECT 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " SMSG_UPDATE_OBJECT 0", TEST_UTILS_SIDE_SERVER);
    }
    /* MSG_MOVE_TELEPORT_ACK_Client */ {
        const std::vector<unsigned char> buffer = {0, 13, 199, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 38, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::wrath::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::MSG_MOVE_TELEPORT_ACK_Client>() == nullptr) {
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

        auto opcode = ::wow_world_messages::wrath::ServerOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::SMSG_TUTORIAL_FLAGS>() == nullptr) {
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

        auto opcode = ::wow_world_messages::wrath::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::CMSG_STANDSTATECHANGE>() == nullptr) {
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

        auto opcode = ::wow_world_messages::wrath::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::CMSG_AUTOEQUIP_ITEM>() == nullptr) {
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

        auto opcode = ::wow_world_messages::wrath::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::CMSG_INITIATE_TRADE>() == nullptr) {
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

        auto opcode = ::wow_world_messages::wrath::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::CMSG_CANCEL_TRADE>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_CANCEL_TRADE 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_CANCEL_TRADE 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* CMSG_CANCEL_CAST */ {
        const std::vector<unsigned char> buffer = {0, 8, 47, 1, 0, 0, 120, 80, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::wrath::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::CMSG_CANCEL_CAST>() == nullptr) {
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

        auto opcode = ::wow_world_messages::wrath::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::CMSG_CANCEL_CAST>() == nullptr) {
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

        auto opcode = ::wow_world_messages::wrath::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::CMSG_SET_SELECTION>() == nullptr) {
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

        auto opcode = ::wow_world_messages::wrath::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::CMSG_ATTACKSWING>() == nullptr) {
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

        auto opcode = ::wow_world_messages::wrath::ServerOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::SMSG_ATTACKSTART>() == nullptr) {
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

        auto opcode = ::wow_world_messages::wrath::ServerOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::SMSG_ATTACKSTOP>() == nullptr) {
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

        auto opcode = ::wow_world_messages::wrath::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::CMSG_QUERY_TIME>() == nullptr) {
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

        auto opcode = ::wow_world_messages::wrath::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::CMSG_PING>() == nullptr) {
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

        auto opcode = ::wow_world_messages::wrath::ServerOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::SMSG_PONG>() == nullptr) {
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

        auto opcode = ::wow_world_messages::wrath::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::CMSG_SETSHEATHED>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_SETSHEATHED 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_SETSHEATHED 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* CMSG_REQUEST_ACCOUNT_DATA */ {
        const std::vector<unsigned char> buffer = {0, 8, 10, 2, 0, 0, 6, 0, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::wrath::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::CMSG_REQUEST_ACCOUNT_DATA>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_REQUEST_ACCOUNT_DATA 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_REQUEST_ACCOUNT_DATA 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* CMSG_GMTICKET_GETTICKET */ {
        const std::vector<unsigned char> buffer = {0, 4, 17, 2, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::wrath::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::CMSG_GMTICKET_GETTICKET>() == nullptr) {
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

        auto opcode = ::wow_world_messages::wrath::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::MSG_AUCTION_HELLO_Client>() == nullptr) {
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

        auto opcode = ::wow_world_messages::wrath::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::CMSG_SET_ACTIVE_MOVER>() == nullptr) {
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

        auto opcode = ::wow_world_messages::wrath::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::MSG_QUERY_NEXT_MAIL_TIME_Client>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " MSG_QUERY_NEXT_MAIL_TIME_Client 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " MSG_QUERY_NEXT_MAIL_TIME_Client 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* CMSG_CHAR_RENAME */ {
        const std::vector<unsigned char> buffer = {0, 21, 199, 2, 0, 0, 239, 190, 173, 222, 0, 0, 0, 0, 68, 101, 97, 100, 98, 101, 101, 102, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::wrath::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::CMSG_CHAR_RENAME>() == nullptr) {
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

        auto opcode = ::wow_world_messages::wrath::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::CMSG_REQUEST_RAID_INFO>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " CMSG_REQUEST_RAID_INFO 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " CMSG_REQUEST_RAID_INFO 0", TEST_UTILS_SIDE_CLIENT);
    }
    /* CMSG_BATTLEFIELD_STATUS */ {
        const std::vector<unsigned char> buffer = {0, 4, 211, 2, 0, 0, };
        auto reader = ByteReader(buffer);

        auto opcode = ::wow_world_messages::wrath::ClientOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::CMSG_BATTLEFIELD_STATUS>() == nullptr) {
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

        auto opcode = ::wow_world_messages::wrath::ServerOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::SMSG_SPLINE_SET_RUN_SPEED>() == nullptr) {
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

        auto opcode = ::wow_world_messages::wrath::ServerOpcode::read(reader, [](unsigned char*, size_t){});
        if (opcode.get_if<::wow_world_messages::wrath::SMSG_MOTD>() == nullptr) {
            printf(__FILE__ ":" STRINGIFY(__LINE__) " SMSG_MOTD 0 read invalid opcode %s\n", opcode.to_string());
            fflush(nullptr);
            abort();
        }

        const std::vector<unsigned char> write_buffer = opcode.write([](unsigned char*, size_t){});

        world_test_compare_buffers(buffer.data(), write_buffer.data(), write_buffer.size(),  __FILE__ ":" STRINGIFY(__LINE__) " SMSG_MOTD 0", TEST_UTILS_SIDE_SERVER);
    }
    return 0;
}

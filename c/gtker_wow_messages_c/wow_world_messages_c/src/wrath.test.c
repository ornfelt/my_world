/* clang-format off */
#include "wow_world_messages/wrath.h"
#include "test_utils.h"

#include <stdlib.h> /* abort() */

static void check_complete(const WowWorldResult result, const char* location, const char* object, const char* reason) {
    if (result != WWM_RESULT_SUCCESS) {
        printf("%s: %s %s %s\n", location, object, reason, wwm_error_code_to_string(result));
        fflush(NULL);
        abort();
    }
}

static void check_result(const WowWorldResult result, const char* location, const char* object, const char* reason) {
    if (result < WWM_RESULT_SUCCESS) {
        printf("%s: %s %s %s\n", location, object, reason, wwm_error_code_to_string(result));
        fflush(NULL);
        abort();
    }
}

static void check_opcode(const int opcode, const int expected, const char* location, const char* object) {
    if (opcode != expected) {
        printf("%s: %s read wrong opcode: '0x%x' instead of '0x%x'\n", location, object, opcode, expected);
        fflush(NULL);
        abort();
    }
}

unsigned char write_buffer[(1 << 16 ) - 1] = {0}; /* uint16_t max */
unsigned char write_buffer2[(1 << 16) - 1] = {0}; /* uint16_t max */
int main(void) {
    WowWorldReader reader;
    WowWorldWriter writer;

    WowWorldResult result;

    /* CMSG_WORLD_TELEPORT */
    do {
        unsigned char buffer[] = {0, 36, 8, 0, 0, 0, 239, 190, 173, 222, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 63, 0, 0, 0, 64, 0, 0, 64, 64, 0, 0, 128, 64, };

        WrathClientOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_client_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_WORLD_TELEPORT 0", "failed to read");
        check_opcode(opcode.opcode, W_CMSG_WORLD_TELEPORT, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_WORLD_TELEPORT 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_WORLD_TELEPORT 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_WORLD_TELEPORT 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_CMSG_WORLD_TELEPORT_write(&writer, &opcode.body.CMSG_WORLD_TELEPORT);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_WORLD_TELEPORT 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMSG_WORLD_TELEPORT 0", TEST_UTILS_SIDE_CLIENT);
    }while (0);

    do {
        unsigned char buffer[] = {0, 36, 8, 0, 0, 0, 154, 61, 9, 2, 213, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 226, 67, 0, 176, 201, 69, 0, 128, 30, 69, 219, 15, 73, 64, };

        WrathClientOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_client_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_WORLD_TELEPORT 1", "failed to read");
        check_opcode(opcode.opcode, W_CMSG_WORLD_TELEPORT, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_WORLD_TELEPORT 1");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_WORLD_TELEPORT 1", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_WORLD_TELEPORT 1", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_CMSG_WORLD_TELEPORT_write(&writer, &opcode.body.CMSG_WORLD_TELEPORT);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_WORLD_TELEPORT 1", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMSG_WORLD_TELEPORT 1", TEST_UTILS_SIDE_CLIENT);
    }while (0);

    /* CMSG_TELEPORT_TO_UNIT */
    do {
        unsigned char buffer[] = {0, 11, 9, 0, 0, 0, 86, 117, 114, 116, 110, 101, 0, };

        WrathClientOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_client_opcode_read(&reader, &opcode);

        wrath_client_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_TELEPORT_TO_UNIT 0", "failed to read");
        check_opcode(opcode.opcode, W_CMSG_TELEPORT_TO_UNIT, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_TELEPORT_TO_UNIT 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_TELEPORT_TO_UNIT 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_TELEPORT_TO_UNIT 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_CMSG_TELEPORT_TO_UNIT_write(&writer, &opcode.body.CMSG_TELEPORT_TO_UNIT);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_TELEPORT_TO_UNIT 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMSG_TELEPORT_TO_UNIT 0", TEST_UTILS_SIDE_CLIENT);
        wrath_client_opcode_free(&opcode);
    }while (0);

    /* CMSG_CHAR_ENUM */
    do {
        unsigned char buffer[] = {0, 4, 55, 0, 0, 0, };

        WrathClientOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_client_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_CHAR_ENUM 0", "failed to read");
        check_opcode(opcode.opcode, W_CMSG_CHAR_ENUM, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_CHAR_ENUM 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_CHAR_ENUM 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_CHAR_ENUM 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_CMSG_CHAR_ENUM_write(&writer);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_CHAR_ENUM 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMSG_CHAR_ENUM 0", TEST_UTILS_SIDE_CLIENT);
    }while (0);

    /* CMSG_CHAR_DELETE */
    do {
        unsigned char buffer[] = {0, 12, 56, 0, 0, 0, 239, 190, 173, 222, 0, 0, 0, 0, };

        WrathClientOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_client_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_CHAR_DELETE 0", "failed to read");
        check_opcode(opcode.opcode, W_CMSG_CHAR_DELETE, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_CHAR_DELETE 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_CHAR_DELETE 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_CHAR_DELETE 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_CMSG_CHAR_DELETE_write(&writer, &opcode.body.CMSG_CHAR_DELETE);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_CHAR_DELETE 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMSG_CHAR_DELETE 0", TEST_UTILS_SIDE_CLIENT);
    }while (0);

    /* CMSG_PLAYER_LOGIN */
    do {
        unsigned char buffer[] = {0, 12, 61, 0, 0, 0, 239, 190, 173, 222, 0, 0, 0, 0, };

        WrathClientOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_client_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_PLAYER_LOGIN 0", "failed to read");
        check_opcode(opcode.opcode, W_CMSG_PLAYER_LOGIN, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_PLAYER_LOGIN 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_PLAYER_LOGIN 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_PLAYER_LOGIN 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_CMSG_PLAYER_LOGIN_write(&writer, &opcode.body.CMSG_PLAYER_LOGIN);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_PLAYER_LOGIN 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMSG_PLAYER_LOGIN 0", TEST_UTILS_SIDE_CLIENT);
    }while (0);

    /* CMSG_PLAYER_LOGOUT */
    do {
        unsigned char buffer[] = {0, 4, 74, 0, 0, 0, };

        WrathClientOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_client_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_PLAYER_LOGOUT 0", "failed to read");
        check_opcode(opcode.opcode, W_CMSG_PLAYER_LOGOUT, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_PLAYER_LOGOUT 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_PLAYER_LOGOUT 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_PLAYER_LOGOUT 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_CMSG_PLAYER_LOGOUT_write(&writer);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_PLAYER_LOGOUT 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMSG_PLAYER_LOGOUT 0", TEST_UTILS_SIDE_CLIENT);
    }while (0);

    /* CMSG_LOGOUT_REQUEST */
    do {
        unsigned char buffer[] = {0, 4, 75, 0, 0, 0, };

        WrathClientOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_client_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_LOGOUT_REQUEST 0", "failed to read");
        check_opcode(opcode.opcode, W_CMSG_LOGOUT_REQUEST, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_LOGOUT_REQUEST 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_LOGOUT_REQUEST 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_LOGOUT_REQUEST 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_CMSG_LOGOUT_REQUEST_write(&writer);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_LOGOUT_REQUEST 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMSG_LOGOUT_REQUEST 0", TEST_UTILS_SIDE_CLIENT);
    }while (0);

    /* SMSG_LOGOUT_RESPONSE */
    do {
        unsigned char buffer[] = {0, 7, 76, 0, 0, 0, 0, 0, 1, };

        WrathServerOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_server_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_LOGOUT_RESPONSE 0", "failed to read");
        check_opcode(opcode.opcode, W_SMSG_LOGOUT_RESPONSE, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_LOGOUT_RESPONSE 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_LOGOUT_RESPONSE 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_LOGOUT_RESPONSE 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_SMSG_LOGOUT_RESPONSE_write(&writer, &opcode.body.SMSG_LOGOUT_RESPONSE);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_LOGOUT_RESPONSE 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " SMSG_LOGOUT_RESPONSE 0", TEST_UTILS_SIDE_SERVER);
    }while (0);

    /* SMSG_LOGOUT_COMPLETE */
    do {
        unsigned char buffer[] = {0, 2, 77, 0, };

        WrathServerOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_server_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_LOGOUT_COMPLETE 0", "failed to read");
        check_opcode(opcode.opcode, W_SMSG_LOGOUT_COMPLETE, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_LOGOUT_COMPLETE 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_LOGOUT_COMPLETE 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_LOGOUT_COMPLETE 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_SMSG_LOGOUT_COMPLETE_write(&writer);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_LOGOUT_COMPLETE 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " SMSG_LOGOUT_COMPLETE 0", TEST_UTILS_SIDE_SERVER);
    }while (0);

    /* CMSG_PET_NAME_QUERY */
    do {
        unsigned char buffer[] = {0, 16, 82, 0, 0, 0, 239, 190, 173, 222, 239, 190, 173, 222, 222, 202, 250, 0, };

        WrathClientOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_client_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_PET_NAME_QUERY 0", "failed to read");
        check_opcode(opcode.opcode, W_CMSG_PET_NAME_QUERY, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_PET_NAME_QUERY 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_PET_NAME_QUERY 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_PET_NAME_QUERY 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_CMSG_PET_NAME_QUERY_write(&writer, &opcode.body.CMSG_PET_NAME_QUERY);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_PET_NAME_QUERY 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMSG_PET_NAME_QUERY 0", TEST_UTILS_SIDE_CLIENT);
    }while (0);

    /* SMSG_UPDATE_OBJECT */
    do {
        unsigned char buffer[] = {0, 115, 169, 0, 1, 0, 0, 0, 3, 1, 8, 4, 33, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 205, 215, 11, 198, 53, 126, 4, 195, 249, 15, 167, 66, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 63, 0, 0, 140, 66, 0, 0, 144, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 208, 15, 73, 64, 0, 0, 0, 0, 3, 7, 0, 0, 0, 0, 0, 128, 0, 24, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 25, 0, 0, 0, 1, 0, 0, 0, 12, 77, 0, 0, 12, 77, 0, 0, };

        WrathServerOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_server_opcode_read(&reader, &opcode);

        wrath_server_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_UPDATE_OBJECT 0", "failed to read");
        check_opcode(opcode.opcode, W_SMSG_UPDATE_OBJECT, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_UPDATE_OBJECT 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_UPDATE_OBJECT 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_UPDATE_OBJECT 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_SMSG_UPDATE_OBJECT_write(&writer, &opcode.body.SMSG_UPDATE_OBJECT);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_UPDATE_OBJECT 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " SMSG_UPDATE_OBJECT 0", TEST_UTILS_SIDE_SERVER);
        wrath_server_opcode_free(&opcode);
    }while (0);

    /* MSG_MOVE_TELEPORT_ACK_Client */
    do {
        unsigned char buffer[] = {0, 13, 199, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 38, };

        WrathClientOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_client_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "MSG_MOVE_TELEPORT_ACK_Client 0", "failed to read");
        check_opcode(opcode.opcode, W_MSG_MOVE_TELEPORT_ACK, __FILE__ ":" STRINGIFY(__LINE__), "MSG_MOVE_TELEPORT_ACK_Client 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "MSG_MOVE_TELEPORT_ACK_Client 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "MSG_MOVE_TELEPORT_ACK_Client 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_MSG_MOVE_TELEPORT_ACK_Client_write(&writer, &opcode.body.MSG_MOVE_TELEPORT_ACK_Client);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "MSG_MOVE_TELEPORT_ACK_Client 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " MSG_MOVE_TELEPORT_ACK_Client 0", TEST_UTILS_SIDE_CLIENT);
    }while (0);

    /* SMSG_TUTORIAL_FLAGS */
    do {
        unsigned char buffer[] = {0, 34, 253, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, };

        WrathServerOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_server_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_TUTORIAL_FLAGS 0", "failed to read");
        check_opcode(opcode.opcode, W_SMSG_TUTORIAL_FLAGS, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_TUTORIAL_FLAGS 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_TUTORIAL_FLAGS 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_TUTORIAL_FLAGS 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_SMSG_TUTORIAL_FLAGS_write(&writer, &opcode.body.SMSG_TUTORIAL_FLAGS);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_TUTORIAL_FLAGS 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " SMSG_TUTORIAL_FLAGS 0", TEST_UTILS_SIDE_SERVER);
    }while (0);

    /* CMSG_STANDSTATECHANGE */
    do {
        unsigned char buffer[] = {0, 8, 1, 1, 0, 0, 1, 0, 0, 0, };

        WrathClientOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_client_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_STANDSTATECHANGE 0", "failed to read");
        check_opcode(opcode.opcode, W_CMSG_STANDSTATECHANGE, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_STANDSTATECHANGE 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_STANDSTATECHANGE 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_STANDSTATECHANGE 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_CMSG_STANDSTATECHANGE_write(&writer, &opcode.body.CMSG_STANDSTATECHANGE);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_STANDSTATECHANGE 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMSG_STANDSTATECHANGE 0", TEST_UTILS_SIDE_CLIENT);
    }while (0);

    /* CMSG_AUTOEQUIP_ITEM */
    do {
        unsigned char buffer[] = {0, 6, 10, 1, 0, 0, 255, 24, };

        WrathClientOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_client_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_AUTOEQUIP_ITEM 0", "failed to read");
        check_opcode(opcode.opcode, W_CMSG_AUTOEQUIP_ITEM, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_AUTOEQUIP_ITEM 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_AUTOEQUIP_ITEM 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_AUTOEQUIP_ITEM 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_CMSG_AUTOEQUIP_ITEM_write(&writer, &opcode.body.CMSG_AUTOEQUIP_ITEM);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_AUTOEQUIP_ITEM 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMSG_AUTOEQUIP_ITEM 0", TEST_UTILS_SIDE_CLIENT);
    }while (0);

    /* CMSG_INITIATE_TRADE */
    do {
        unsigned char buffer[] = {0, 12, 22, 1, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, };

        WrathClientOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_client_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_INITIATE_TRADE 0", "failed to read");
        check_opcode(opcode.opcode, W_CMSG_INITIATE_TRADE, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_INITIATE_TRADE 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_INITIATE_TRADE 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_INITIATE_TRADE 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_CMSG_INITIATE_TRADE_write(&writer, &opcode.body.CMSG_INITIATE_TRADE);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_INITIATE_TRADE 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMSG_INITIATE_TRADE 0", TEST_UTILS_SIDE_CLIENT);
    }while (0);

    /* CMSG_CANCEL_TRADE */
    do {
        unsigned char buffer[] = {0, 4, 28, 1, 0, 0, };

        WrathClientOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_client_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_CANCEL_TRADE 0", "failed to read");
        check_opcode(opcode.opcode, W_CMSG_CANCEL_TRADE, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_CANCEL_TRADE 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_CANCEL_TRADE 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_CANCEL_TRADE 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_CMSG_CANCEL_TRADE_write(&writer);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_CANCEL_TRADE 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMSG_CANCEL_TRADE 0", TEST_UTILS_SIDE_CLIENT);
    }while (0);

    /* CMSG_CANCEL_CAST */
    do {
        unsigned char buffer[] = {0, 8, 47, 1, 0, 0, 120, 80, 0, 0, };

        WrathClientOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_client_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_CANCEL_CAST 0", "failed to read");
        check_opcode(opcode.opcode, W_CMSG_CANCEL_CAST, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_CANCEL_CAST 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_CANCEL_CAST 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_CANCEL_CAST 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_CMSG_CANCEL_CAST_write(&writer, &opcode.body.CMSG_CANCEL_CAST);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_CANCEL_CAST 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMSG_CANCEL_CAST 0", TEST_UTILS_SIDE_CLIENT);
    }while (0);

    do {
        unsigned char buffer[] = {0, 8, 47, 1, 0, 0, 242, 33, 0, 0, };

        WrathClientOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_client_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_CANCEL_CAST 1", "failed to read");
        check_opcode(opcode.opcode, W_CMSG_CANCEL_CAST, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_CANCEL_CAST 1");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_CANCEL_CAST 1", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_CANCEL_CAST 1", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_CMSG_CANCEL_CAST_write(&writer, &opcode.body.CMSG_CANCEL_CAST);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_CANCEL_CAST 1", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMSG_CANCEL_CAST 1", TEST_UTILS_SIDE_CLIENT);
    }while (0);

    /* CMSG_SET_SELECTION */
    do {
        unsigned char buffer[] = {0, 12, 61, 1, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, };

        WrathClientOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_client_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_SET_SELECTION 0", "failed to read");
        check_opcode(opcode.opcode, W_CMSG_SET_SELECTION, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_SET_SELECTION 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_SET_SELECTION 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_SET_SELECTION 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_CMSG_SET_SELECTION_write(&writer, &opcode.body.CMSG_SET_SELECTION);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_SET_SELECTION 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMSG_SET_SELECTION 0", TEST_UTILS_SIDE_CLIENT);
    }while (0);

    /* CMSG_ATTACKSWING */
    do {
        unsigned char buffer[] = {0, 12, 65, 1, 0, 0, 100, 0, 0, 0, 0, 0, 0, 0, };

        WrathClientOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_client_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_ATTACKSWING 0", "failed to read");
        check_opcode(opcode.opcode, W_CMSG_ATTACKSWING, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_ATTACKSWING 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_ATTACKSWING 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_ATTACKSWING 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_CMSG_ATTACKSWING_write(&writer, &opcode.body.CMSG_ATTACKSWING);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_ATTACKSWING 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMSG_ATTACKSWING 0", TEST_UTILS_SIDE_CLIENT);
    }while (0);

    /* SMSG_ATTACKSTART */
    do {
        unsigned char buffer[] = {0, 18, 67, 1, 23, 0, 0, 0, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 0, };

        WrathServerOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_server_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_ATTACKSTART 0", "failed to read");
        check_opcode(opcode.opcode, W_SMSG_ATTACKSTART, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_ATTACKSTART 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_ATTACKSTART 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_ATTACKSTART 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_SMSG_ATTACKSTART_write(&writer, &opcode.body.SMSG_ATTACKSTART);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_ATTACKSTART 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " SMSG_ATTACKSTART 0", TEST_UTILS_SIDE_SERVER);
    }while (0);

    /* SMSG_ATTACKSTOP */
    do {
        unsigned char buffer[] = {0, 10, 68, 1, 1, 23, 1, 100, 0, 0, 0, 0, };

        WrathServerOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_server_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_ATTACKSTOP 0", "failed to read");
        check_opcode(opcode.opcode, W_SMSG_ATTACKSTOP, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_ATTACKSTOP 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_ATTACKSTOP 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_ATTACKSTOP 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_SMSG_ATTACKSTOP_write(&writer, &opcode.body.SMSG_ATTACKSTOP);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_ATTACKSTOP 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " SMSG_ATTACKSTOP 0", TEST_UTILS_SIDE_SERVER);
    }while (0);

    /* CMSG_QUERY_TIME */
    do {
        unsigned char buffer[] = {0, 4, 206, 1, 0, 0, };

        WrathClientOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_client_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_QUERY_TIME 0", "failed to read");
        check_opcode(opcode.opcode, W_CMSG_QUERY_TIME, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_QUERY_TIME 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_QUERY_TIME 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_QUERY_TIME 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_CMSG_QUERY_TIME_write(&writer);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_QUERY_TIME 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMSG_QUERY_TIME 0", TEST_UTILS_SIDE_CLIENT);
    }while (0);

    /* CMSG_PING */
    do {
        unsigned char buffer[] = {0, 12, 220, 1, 0, 0, 239, 190, 173, 222, 222, 202, 250, 0, };

        WrathClientOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_client_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_PING 0", "failed to read");
        check_opcode(opcode.opcode, W_CMSG_PING, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_PING 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_PING 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_PING 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_CMSG_PING_write(&writer, &opcode.body.CMSG_PING);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_PING 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMSG_PING 0", TEST_UTILS_SIDE_CLIENT);
    }while (0);

    /* SMSG_PONG */
    do {
        unsigned char buffer[] = {0, 6, 221, 1, 239, 190, 173, 222, };

        WrathServerOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_server_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_PONG 0", "failed to read");
        check_opcode(opcode.opcode, W_SMSG_PONG, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_PONG 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_PONG 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_PONG 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_SMSG_PONG_write(&writer, &opcode.body.SMSG_PONG);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_PONG 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " SMSG_PONG 0", TEST_UTILS_SIDE_SERVER);
    }while (0);

    /* CMSG_SETSHEATHED */
    do {
        unsigned char buffer[] = {0, 8, 224, 1, 0, 0, 1, 0, 0, 0, };

        WrathClientOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_client_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_SETSHEATHED 0", "failed to read");
        check_opcode(opcode.opcode, W_CMSG_SETSHEATHED, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_SETSHEATHED 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_SETSHEATHED 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_SETSHEATHED 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_CMSG_SETSHEATHED_write(&writer, &opcode.body.CMSG_SETSHEATHED);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_SETSHEATHED 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMSG_SETSHEATHED 0", TEST_UTILS_SIDE_CLIENT);
    }while (0);

    /* CMSG_REQUEST_ACCOUNT_DATA */
    do {
        unsigned char buffer[] = {0, 8, 10, 2, 0, 0, 6, 0, 0, 0, };

        WrathClientOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_client_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_REQUEST_ACCOUNT_DATA 0", "failed to read");
        check_opcode(opcode.opcode, W_CMSG_REQUEST_ACCOUNT_DATA, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_REQUEST_ACCOUNT_DATA 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_REQUEST_ACCOUNT_DATA 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_REQUEST_ACCOUNT_DATA 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_CMSG_REQUEST_ACCOUNT_DATA_write(&writer, &opcode.body.CMSG_REQUEST_ACCOUNT_DATA);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_REQUEST_ACCOUNT_DATA 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMSG_REQUEST_ACCOUNT_DATA 0", TEST_UTILS_SIDE_CLIENT);
    }while (0);

    /* CMSG_GMTICKET_GETTICKET */
    do {
        unsigned char buffer[] = {0, 4, 17, 2, 0, 0, };

        WrathClientOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_client_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_GMTICKET_GETTICKET 0", "failed to read");
        check_opcode(opcode.opcode, W_CMSG_GMTICKET_GETTICKET, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_GMTICKET_GETTICKET 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_GMTICKET_GETTICKET 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_GMTICKET_GETTICKET 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_CMSG_GMTICKET_GETTICKET_write(&writer);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_GMTICKET_GETTICKET 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMSG_GMTICKET_GETTICKET 0", TEST_UTILS_SIDE_CLIENT);
    }while (0);

    /* MSG_AUCTION_HELLO_Client */
    do {
        unsigned char buffer[] = {0, 12, 85, 2, 0, 0, 239, 190, 173, 222, 0, 0, 0, 0, };

        WrathClientOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_client_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "MSG_AUCTION_HELLO_Client 0", "failed to read");
        check_opcode(opcode.opcode, W_MSG_AUCTION_HELLO, __FILE__ ":" STRINGIFY(__LINE__), "MSG_AUCTION_HELLO_Client 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "MSG_AUCTION_HELLO_Client 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "MSG_AUCTION_HELLO_Client 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_MSG_AUCTION_HELLO_Client_write(&writer, &opcode.body.MSG_AUCTION_HELLO_Client);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "MSG_AUCTION_HELLO_Client 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " MSG_AUCTION_HELLO_Client 0", TEST_UTILS_SIDE_CLIENT);
    }while (0);

    /* CMSG_SET_ACTIVE_MOVER */
    do {
        unsigned char buffer[] = {0, 12, 106, 2, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, };

        WrathClientOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_client_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_SET_ACTIVE_MOVER 0", "failed to read");
        check_opcode(opcode.opcode, W_CMSG_SET_ACTIVE_MOVER, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_SET_ACTIVE_MOVER 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_SET_ACTIVE_MOVER 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_SET_ACTIVE_MOVER 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_CMSG_SET_ACTIVE_MOVER_write(&writer, &opcode.body.CMSG_SET_ACTIVE_MOVER);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_SET_ACTIVE_MOVER 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMSG_SET_ACTIVE_MOVER 0", TEST_UTILS_SIDE_CLIENT);
    }while (0);

    /* MSG_QUERY_NEXT_MAIL_TIME_Client */
    do {
        unsigned char buffer[] = {0, 4, 132, 2, 0, 0, };

        WrathClientOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_client_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "MSG_QUERY_NEXT_MAIL_TIME_Client 0", "failed to read");
        check_opcode(opcode.opcode, W_MSG_QUERY_NEXT_MAIL_TIME, __FILE__ ":" STRINGIFY(__LINE__), "MSG_QUERY_NEXT_MAIL_TIME_Client 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "MSG_QUERY_NEXT_MAIL_TIME_Client 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "MSG_QUERY_NEXT_MAIL_TIME_Client 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_MSG_QUERY_NEXT_MAIL_TIME_Client_write(&writer);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "MSG_QUERY_NEXT_MAIL_TIME_Client 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " MSG_QUERY_NEXT_MAIL_TIME_Client 0", TEST_UTILS_SIDE_CLIENT);
    }while (0);

    /* CMSG_CHAR_RENAME */
    do {
        unsigned char buffer[] = {0, 21, 199, 2, 0, 0, 239, 190, 173, 222, 0, 0, 0, 0, 68, 101, 97, 100, 98, 101, 101, 102, 0, };

        WrathClientOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_client_opcode_read(&reader, &opcode);

        wrath_client_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_CHAR_RENAME 0", "failed to read");
        check_opcode(opcode.opcode, W_CMSG_CHAR_RENAME, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_CHAR_RENAME 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_CHAR_RENAME 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_CHAR_RENAME 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_CMSG_CHAR_RENAME_write(&writer, &opcode.body.CMSG_CHAR_RENAME);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_CHAR_RENAME 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMSG_CHAR_RENAME 0", TEST_UTILS_SIDE_CLIENT);
        wrath_client_opcode_free(&opcode);
    }while (0);

    /* CMSG_REQUEST_RAID_INFO */
    do {
        unsigned char buffer[] = {0, 4, 205, 2, 0, 0, };

        WrathClientOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_client_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_REQUEST_RAID_INFO 0", "failed to read");
        check_opcode(opcode.opcode, W_CMSG_REQUEST_RAID_INFO, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_REQUEST_RAID_INFO 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_REQUEST_RAID_INFO 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_REQUEST_RAID_INFO 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_CMSG_REQUEST_RAID_INFO_write(&writer);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_REQUEST_RAID_INFO 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMSG_REQUEST_RAID_INFO 0", TEST_UTILS_SIDE_CLIENT);
    }while (0);

    /* CMSG_BATTLEFIELD_STATUS */
    do {
        unsigned char buffer[] = {0, 4, 211, 2, 0, 0, };

        WrathClientOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_client_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_BATTLEFIELD_STATUS 0", "failed to read");
        check_opcode(opcode.opcode, W_CMSG_BATTLEFIELD_STATUS, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_BATTLEFIELD_STATUS 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_BATTLEFIELD_STATUS 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_BATTLEFIELD_STATUS 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_CMSG_BATTLEFIELD_STATUS_write(&writer);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMSG_BATTLEFIELD_STATUS 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMSG_BATTLEFIELD_STATUS 0", TEST_UTILS_SIDE_CLIENT);
    }while (0);

    /* SMSG_SPLINE_SET_RUN_SPEED */
    do {
        unsigned char buffer[] = {0, 8, 254, 2, 1, 6, 0, 0, 224, 64, };

        WrathServerOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_server_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_SPLINE_SET_RUN_SPEED 0", "failed to read");
        check_opcode(opcode.opcode, W_SMSG_SPLINE_SET_RUN_SPEED, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_SPLINE_SET_RUN_SPEED 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_SPLINE_SET_RUN_SPEED 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_SPLINE_SET_RUN_SPEED 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_SMSG_SPLINE_SET_RUN_SPEED_write(&writer, &opcode.body.SMSG_SPLINE_SET_RUN_SPEED);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_SPLINE_SET_RUN_SPEED 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " SMSG_SPLINE_SET_RUN_SPEED 0", TEST_UTILS_SIDE_SERVER);
    }while (0);

    /* SMSG_MOTD */
    do {
        unsigned char buffer[] = {0, 116, 61, 3, 2, 0, 0, 0, 87, 101, 108, 99, 111, 109, 101, 32, 116, 111, 32, 97, 110, 32, 65, 122, 101, 114, 111, 116, 104, 67, 111, 114, 101, 32, 115, 101, 114, 118, 101, 114, 46, 0, 124, 99, 102, 102, 70, 70, 52, 65, 50, 68, 84, 104, 105, 115, 32, 115, 101, 114, 118, 101, 114, 32, 114, 117, 110, 115, 32, 111, 110, 32, 65, 122, 101, 114, 111, 116, 104, 67, 111, 114, 101, 124, 114, 32, 124, 99, 102, 102, 51, 67, 69, 55, 70, 70, 119, 119, 119, 46, 97, 122, 101, 114, 111, 116, 104, 99, 111, 114, 101, 46, 111, 114, 103, 124, 114, 0, };

        WrathServerOpcodeContainer opcode;

        reader = wwm_create_reader(buffer, sizeof(buffer));
        result = wrath_server_opcode_read(&reader, &opcode);

        wrath_server_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_MOTD 0", "failed to read");
        check_opcode(opcode.opcode, W_SMSG_MOTD, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_MOTD 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = wrath_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_MOTD 0", "failed partial");
            if (result == WWM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_MOTD 0", "gave too much index back");
            }
        }

        writer = wwm_create_writer(write_buffer, sizeof(write_buffer));
        result = wrath_SMSG_MOTD_write(&writer, &opcode.body.SMSG_MOTD);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "SMSG_MOTD 0", "failed to write");

        world_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " SMSG_MOTD 0", TEST_UTILS_SIDE_SERVER);
        wrath_server_opcode_free(&opcode);
    }while (0);

    return 0;
}

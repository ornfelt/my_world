/* clang-format off */
#include "wow_login_messages/all.h"
#include "wow_login_messages/version2.h"
#include "wow_login_messages/version3.h"
#include "wow_login_messages/version5.h"
#include "wow_login_messages/version6.h"
#include "wow_login_messages/version7.h"
#include "wow_login_messages/version8.h"
#include "test_utils.h"

#include <stdlib.h> /* abort() */

static void check_complete(const WowLoginResult result, const char* location, const char* object, const char* reason) {
    if (result != WLM_RESULT_SUCCESS) {
        printf("%s: %s %s %s\n", location, object, reason, wlm_error_code_to_string(result));
        fflush(NULL);
        abort();
    }
}

static void check_result(const WowLoginResult result, const char* location, const char* object, const char* reason) {
    if (result < WLM_RESULT_SUCCESS) {
        printf("%s: %s %s %s\n", location, object, reason, wlm_error_code_to_string(result));
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
    WowLoginReader reader;
    WowLoginWriter writer;

    WowLoginResult result;

    /* CMD_AUTH_LOGON_CHALLENGE_Client */
    do {
        unsigned char buffer[] = {0, 3, 31, 0, 87, 111, 87, 0, 1, 12, 1, 243, 22, 54, 56, 120, 0, 110, 105, 87, 0, 66, 71, 110, 101, 60, 0, 0, 0, 127, 0, 0, 1, 1, 65, };

        AllClientOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = all_client_opcode_read(&reader, &opcode);

        all_client_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Client 0", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_LOGON_CHALLENGE, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Client 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = all_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Client 0", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Client 0", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = all_CMD_AUTH_LOGON_CHALLENGE_Client_write(&writer, &opcode.body.CMD_AUTH_LOGON_CHALLENGE_Client);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Client 0", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_LOGON_CHALLENGE_Client 0");
        all_client_opcode_free(&opcode);
    }while (0);

    /* CMD_AUTH_RECONNECT_CHALLENGE_Client */
    do {
        unsigned char buffer[] = {2, 2, 31, 0, 87, 111, 87, 0, 1, 12, 1, 243, 22, 54, 56, 120, 0, 110, 105, 87, 0, 66, 71, 110, 101, 60, 0, 0, 0, 127, 0, 0, 1, 1, 65, };

        AllClientOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = all_client_opcode_read(&reader, &opcode);

        all_client_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_CHALLENGE_Client 0", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_RECONNECT_CHALLENGE, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_CHALLENGE_Client 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = all_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_CHALLENGE_Client 0", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_CHALLENGE_Client 0", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = all_CMD_AUTH_RECONNECT_CHALLENGE_Client_write(&writer, &opcode.body.CMD_AUTH_RECONNECT_CHALLENGE_Client);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_CHALLENGE_Client 0", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_RECONNECT_CHALLENGE_Client 0");
        all_client_opcode_free(&opcode);
    }while (0);

    do {
        unsigned char buffer[] = {2, 2, 46, 0, 87, 111, 87, 0, 1, 12, 1, 243, 22, 54, 56, 120, 0, 110, 105, 87, 0, 66, 71, 110, 101, 60, 0, 0, 0, 127, 0, 0, 1, 16, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, };

        AllClientOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = all_client_opcode_read(&reader, &opcode);

        all_client_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_CHALLENGE_Client 1", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_RECONNECT_CHALLENGE, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_CHALLENGE_Client 1");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = all_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_CHALLENGE_Client 1", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_CHALLENGE_Client 1", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = all_CMD_AUTH_RECONNECT_CHALLENGE_Client_write(&writer, &opcode.body.CMD_AUTH_RECONNECT_CHALLENGE_Client);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_CHALLENGE_Client 1", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_RECONNECT_CHALLENGE_Client 1");
        all_client_opcode_free(&opcode);
    }while (0);

    /* CMD_AUTH_LOGON_CHALLENGE_Server */
    do {
        unsigned char buffer[] = {0, 0, 0, 73, 216, 194, 188, 104, 92, 43, 206, 74, 244, 250, 7, 10, 71, 147, 120, 88, 120, 70, 181, 131, 212, 65, 130, 158, 36, 216, 135, 206, 218, 52, 70, 1, 7, 32, 183, 155, 62, 42, 135, 130, 60, 171, 143, 94, 191, 191, 142, 177, 1, 8, 83, 80, 6, 41, 139, 91, 173, 189, 91, 83, 225, 137, 94, 100, 75, 137, 199, 9, 135, 125, 140, 101, 82, 102, 165, 125, 184, 101, 61, 110, 166, 43, 181, 84, 242, 11, 207, 116, 214, 74, 119, 167, 211, 61, 243, 48, 144, 135, 186, 163, 30, 153, 160, 11, 33, 87, 252, 55, 63, 179, 105, 205, 210, 241, };

        Version2ServerOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version2_server_opcode_read(&reader, &opcode);

        version2_server_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 0", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_LOGON_CHALLENGE, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version2_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 0", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 0", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version2_CMD_AUTH_LOGON_CHALLENGE_Server_write(&writer, &opcode.body.CMD_AUTH_LOGON_CHALLENGE_Server);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 0", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_LOGON_CHALLENGE_Server 0");
        version2_server_opcode_free(&opcode);
    }while (0);

    /* CMD_AUTH_LOGON_PROOF_Client */
    do {
        unsigned char buffer[] = {1, 241, 62, 229, 209, 131, 196, 200, 169, 80, 14, 63, 90, 93, 138, 238, 78, 46, 69, 225, 247, 204, 143, 28, 245, 238, 142, 17, 206, 211, 29, 215, 8, 107, 30, 72, 27, 77, 4, 161, 24, 216, 242, 222, 92, 89, 213, 92, 129, 46, 101, 236, 62, 78, 245, 45, 225, 128, 94, 26, 103, 21, 236, 200, 65, 238, 184, 144, 138, 88, 187, 0, 208, 2, 255, 0, 239, 190, 173, 222, 1, 2, 3, 4, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 254, 0, 238, 190, 173, 222, 0, 1, 2, 3, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, };

        Version2ClientOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version2_client_opcode_read(&reader, &opcode);

        version2_client_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 0", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_LOGON_PROOF, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version2_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 0", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 0", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version2_CMD_AUTH_LOGON_PROOF_Client_write(&writer, &opcode.body.CMD_AUTH_LOGON_PROOF_Client);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 0", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_LOGON_PROOF_Client 0");
        version2_client_opcode_free(&opcode);
    }while (0);

    do {
        unsigned char buffer[] = {1, 241, 62, 229, 209, 131, 196, 200, 169, 80, 14, 63, 90, 93, 138, 238, 78, 46, 69, 225, 247, 204, 143, 28, 245, 238, 142, 17, 206, 211, 29, 215, 8, 107, 30, 72, 27, 77, 4, 161, 24, 216, 242, 222, 92, 89, 213, 92, 129, 46, 101, 236, 62, 78, 245, 45, 225, 128, 94, 26, 103, 21, 236, 200, 65, 238, 184, 144, 138, 88, 187, 0, 208, 1, 255, 0, 239, 190, 173, 222, 1, 2, 3, 4, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, };

        Version2ClientOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version2_client_opcode_read(&reader, &opcode);

        version2_client_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 1", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_LOGON_PROOF, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 1");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version2_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 1", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 1", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version2_CMD_AUTH_LOGON_PROOF_Client_write(&writer, &opcode.body.CMD_AUTH_LOGON_PROOF_Client);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 1", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_LOGON_PROOF_Client 1");
        version2_client_opcode_free(&opcode);
    }while (0);

    do {
        unsigned char buffer[] = {1, 241, 62, 229, 209, 131, 196, 200, 169, 80, 14, 63, 90, 93, 138, 238, 78, 46, 69, 225, 247, 204, 143, 28, 245, 238, 142, 17, 206, 211, 29, 215, 8, 107, 30, 72, 27, 77, 4, 161, 24, 216, 242, 222, 92, 89, 213, 92, 129, 46, 101, 236, 62, 78, 245, 45, 225, 128, 94, 26, 103, 21, 236, 200, 65, 238, 184, 144, 138, 88, 187, 0, 208, 0, };

        Version2ClientOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version2_client_opcode_read(&reader, &opcode);

        version2_client_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 2", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_LOGON_PROOF, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 2");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version2_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 2", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 2", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version2_CMD_AUTH_LOGON_PROOF_Client_write(&writer, &opcode.body.CMD_AUTH_LOGON_PROOF_Client);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 2", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_LOGON_PROOF_Client 2");
        version2_client_opcode_free(&opcode);
    }while (0);

    /* CMD_AUTH_LOGON_PROOF_Server */
    do {
        unsigned char buffer[] = {1, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 239, 190, 173, 222, };

        Version2ServerOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version2_server_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Server 0", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_LOGON_PROOF, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Server 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version2_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Server 0", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Server 0", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version2_CMD_AUTH_LOGON_PROOF_Server_write(&writer, &opcode.body.CMD_AUTH_LOGON_PROOF_Server);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Server 0", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_LOGON_PROOF_Server 0");
    }while (0);

    /* CMD_AUTH_RECONNECT_CHALLENGE_Server */
    do {
        unsigned char buffer[] = {2, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 255, 254, 253, 252, 251, 250, 249, 248, 247, 246, 245, 244, 243, 242, 241, 240, };

        Version2ServerOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version2_server_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_CHALLENGE_Server 0", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_RECONNECT_CHALLENGE, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_CHALLENGE_Server 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version2_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_CHALLENGE_Server 0", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_CHALLENGE_Server 0", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version2_CMD_AUTH_RECONNECT_CHALLENGE_Server_write(&writer, &opcode.body.CMD_AUTH_RECONNECT_CHALLENGE_Server);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_CHALLENGE_Server 0", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_RECONNECT_CHALLENGE_Server 0");
    }while (0);

    do {
        unsigned char buffer[] = {2, 3, };

        Version2ServerOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version2_server_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_CHALLENGE_Server 1", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_RECONNECT_CHALLENGE, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_CHALLENGE_Server 1");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version2_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_CHALLENGE_Server 1", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_CHALLENGE_Server 1", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version2_CMD_AUTH_RECONNECT_CHALLENGE_Server_write(&writer, &opcode.body.CMD_AUTH_RECONNECT_CHALLENGE_Server);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_CHALLENGE_Server 1", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_RECONNECT_CHALLENGE_Server 1");
    }while (0);

    /* CMD_AUTH_RECONNECT_PROOF_Server */
    do {
        unsigned char buffer[] = {3, 0, };

        Version2ServerOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version2_server_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Server 0", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_RECONNECT_PROOF, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Server 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version2_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Server 0", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Server 0", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version2_CMD_AUTH_RECONNECT_PROOF_Server_write(&writer, &opcode.body.CMD_AUTH_RECONNECT_PROOF_Server);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Server 0", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_RECONNECT_PROOF_Server 0");
    }while (0);

    do {
        unsigned char buffer[] = {3, 14, };

        Version2ServerOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version2_server_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Server 1", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_RECONNECT_PROOF, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Server 1");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version2_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Server 1", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Server 1", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version2_CMD_AUTH_RECONNECT_PROOF_Server_write(&writer, &opcode.body.CMD_AUTH_RECONNECT_PROOF_Server);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Server 1", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_RECONNECT_PROOF_Server 1");
    }while (0);

    do {
        unsigned char buffer[] = {3, 14, };

        Version2ServerOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version2_server_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Server 2", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_RECONNECT_PROOF, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Server 2");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version2_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Server 2", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Server 2", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version2_CMD_AUTH_RECONNECT_PROOF_Server_write(&writer, &opcode.body.CMD_AUTH_RECONNECT_PROOF_Server);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Server 2", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_RECONNECT_PROOF_Server 2");
    }while (0);

    /* CMD_AUTH_RECONNECT_PROOF_Client */
    do {
        unsigned char buffer[] = {3, 234, 250, 185, 198, 24, 21, 11, 242, 249, 50, 206, 39, 98, 121, 150, 153, 107, 109, 26, 13, 243, 165, 158, 106, 56, 2, 231, 11, 225, 47, 5, 113, 186, 71, 140, 163, 40, 167, 158, 154, 36, 40, 230, 130, 237, 236, 199, 201, 232, 110, 241, 59, 123, 225, 224, 245, 0, };

        Version2ClientOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version2_client_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Client 0", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_RECONNECT_PROOF, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Client 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version2_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Client 0", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Client 0", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version2_CMD_AUTH_RECONNECT_PROOF_Client_write(&writer, &opcode.body.CMD_AUTH_RECONNECT_PROOF_Client);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Client 0", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_RECONNECT_PROOF_Client 0");
    }while (0);

    /* CMD_REALM_LIST_Server */
    do {
        unsigned char buffer[] = {16, 23, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 65, 0, 65, 0, 0, 0, 200, 67, 1, 0, 2, 0, 0, };

        Version2ServerOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version2_server_opcode_read(&reader, &opcode);

        version2_server_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Server 0", "failed to read");
        check_opcode(opcode.opcode, CMD_REALM_LIST, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Server 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version2_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Server 0", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Server 0", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version2_CMD_REALM_LIST_Server_write(&writer, &opcode.body.CMD_REALM_LIST_Server);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Server 0", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_REALM_LIST_Server 0");
        version2_server_opcode_free(&opcode);
    }while (0);

    do {
        unsigned char buffer[] = {16, 23, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 3, 65, 0, 65, 0, 0, 0, 200, 67, 1, 0, 2, 0, 0, };

        Version2ServerOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version2_server_opcode_read(&reader, &opcode);

        version2_server_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Server 1", "failed to read");
        check_opcode(opcode.opcode, CMD_REALM_LIST, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Server 1");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version2_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Server 1", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Server 1", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version2_CMD_REALM_LIST_Server_write(&writer, &opcode.body.CMD_REALM_LIST_Server);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Server 1", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_REALM_LIST_Server 1");
        version2_server_opcode_free(&opcode);
    }while (0);

    /* CMD_REALM_LIST_Client */
    do {
        unsigned char buffer[] = {16, 0, 0, 0, 0, };

        Version2ClientOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version2_client_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Client 0", "failed to read");
        check_opcode(opcode.opcode, CMD_REALM_LIST, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Client 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version2_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Client 0", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Client 0", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version2_CMD_REALM_LIST_Client_write(&writer);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Client 0", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_REALM_LIST_Client 0");
    }while (0);

    /* CMD_XFER_INITIATE */
    do {
        unsigned char buffer[] = {48, 5, 80, 97, 116, 99, 104, 188, 9, 0, 0, 0, 0, 0, 0, 17, 91, 85, 89, 127, 183, 223, 14, 134, 217, 179, 174, 90, 235, 203, 98, };

        Version2ServerOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version2_server_opcode_read(&reader, &opcode);

        version2_server_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_XFER_INITIATE 0", "failed to read");
        check_opcode(opcode.opcode, CMD_XFER_INITIATE, __FILE__ ":" STRINGIFY(__LINE__), "CMD_XFER_INITIATE 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version2_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_XFER_INITIATE 0", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_XFER_INITIATE 0", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version2_CMD_XFER_INITIATE_write(&writer, &opcode.body.CMD_XFER_INITIATE);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_XFER_INITIATE 0", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_XFER_INITIATE 0");
        version2_server_opcode_free(&opcode);
    }while (0);

    /* CMD_XFER_DATA */
    do {
        unsigned char buffer[] = {49, 64, 0, 77, 80, 81, 26, 44, 0, 0, 0, 60, 224, 38, 0, 1, 0, 3, 0, 252, 217, 38, 0, 252, 221, 38, 0, 64, 0, 0, 0, 36, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 120, 218, 236, 125, 123, 124, 84, 197, 245, 248, 110, 246, 134, 92, 116, 37, 11, 174, 184, };

        Version2ServerOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version2_server_opcode_read(&reader, &opcode);

        version2_server_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_XFER_DATA 0", "failed to read");
        check_opcode(opcode.opcode, CMD_XFER_DATA, __FILE__ ":" STRINGIFY(__LINE__), "CMD_XFER_DATA 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version2_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_XFER_DATA 0", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_XFER_DATA 0", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version2_CMD_XFER_DATA_write(&writer, &opcode.body.CMD_XFER_DATA);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_XFER_DATA 0", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_XFER_DATA 0");
        version2_server_opcode_free(&opcode);
    }while (0);

    /* CMD_XFER_ACCEPT */
    do {
        unsigned char buffer[] = {50, };

        Version2ClientOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version2_client_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_XFER_ACCEPT 0", "failed to read");
        check_opcode(opcode.opcode, CMD_XFER_ACCEPT, __FILE__ ":" STRINGIFY(__LINE__), "CMD_XFER_ACCEPT 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version2_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_XFER_ACCEPT 0", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_XFER_ACCEPT 0", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version2_CMD_XFER_ACCEPT_write(&writer);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_XFER_ACCEPT 0", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_XFER_ACCEPT 0");
    }while (0);

    /* CMD_XFER_RESUME */
    do {
        unsigned char buffer[] = {51, 64, 0, 0, 0, 0, 0, 0, 0, };

        Version2ClientOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version2_client_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_XFER_RESUME 0", "failed to read");
        check_opcode(opcode.opcode, CMD_XFER_RESUME, __FILE__ ":" STRINGIFY(__LINE__), "CMD_XFER_RESUME 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version2_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_XFER_RESUME 0", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_XFER_RESUME 0", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version2_CMD_XFER_RESUME_write(&writer, &opcode.body.CMD_XFER_RESUME);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_XFER_RESUME 0", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_XFER_RESUME 0");
    }while (0);

    /* CMD_XFER_CANCEL */
    do {
        unsigned char buffer[] = {52, };

        Version2ClientOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version2_client_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_XFER_CANCEL 0", "failed to read");
        check_opcode(opcode.opcode, CMD_XFER_CANCEL, __FILE__ ":" STRINGIFY(__LINE__), "CMD_XFER_CANCEL 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version2_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_XFER_CANCEL 0", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_XFER_CANCEL 0", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version2_CMD_XFER_CANCEL_write(&writer);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_XFER_CANCEL 0", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_XFER_CANCEL 0");
    }while (0);

    /* CMD_AUTH_LOGON_CHALLENGE_Server */
    do {
        unsigned char buffer[] = {0, 0, 0, 73, 216, 194, 188, 104, 92, 43, 206, 74, 244, 250, 7, 10, 71, 147, 120, 88, 120, 70, 181, 131, 212, 65, 130, 158, 36, 216, 135, 206, 218, 52, 70, 1, 7, 32, 183, 155, 62, 42, 135, 130, 60, 171, 143, 94, 191, 191, 142, 177, 1, 8, 83, 80, 6, 41, 139, 91, 173, 189, 91, 83, 225, 137, 94, 100, 75, 137, 199, 9, 135, 125, 140, 101, 82, 102, 165, 125, 184, 101, 61, 110, 166, 43, 181, 84, 242, 11, 207, 116, 214, 74, 119, 167, 211, 61, 243, 48, 144, 135, 186, 163, 30, 153, 160, 11, 33, 87, 252, 55, 63, 179, 105, 205, 210, 241, 1, 239, 190, 173, 222, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, };

        Version3ServerOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version3_server_opcode_read(&reader, &opcode);

        version3_server_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 0", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_LOGON_CHALLENGE, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version3_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 0", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 0", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version3_CMD_AUTH_LOGON_CHALLENGE_Server_write(&writer, &opcode.body.CMD_AUTH_LOGON_CHALLENGE_Server);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 0", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_LOGON_CHALLENGE_Server 0");
        version3_server_opcode_free(&opcode);
    }while (0);

    do {
        unsigned char buffer[] = {0, 0, 0, 73, 216, 194, 188, 104, 92, 43, 206, 74, 244, 250, 7, 10, 71, 147, 120, 88, 120, 70, 181, 131, 212, 65, 130, 158, 36, 216, 135, 206, 218, 52, 70, 1, 7, 32, 183, 155, 62, 42, 135, 130, 60, 171, 143, 94, 191, 191, 142, 177, 1, 8, 83, 80, 6, 41, 139, 91, 173, 189, 91, 83, 225, 137, 94, 100, 75, 137, 199, 9, 135, 125, 140, 101, 82, 102, 165, 125, 184, 101, 61, 110, 166, 43, 181, 84, 242, 11, 207, 116, 214, 74, 119, 167, 211, 61, 243, 48, 144, 135, 186, 163, 30, 153, 160, 11, 33, 87, 252, 55, 63, 179, 105, 205, 210, 241, 0, };

        Version3ServerOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version3_server_opcode_read(&reader, &opcode);

        version3_server_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 1", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_LOGON_CHALLENGE, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 1");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version3_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 1", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 1", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version3_CMD_AUTH_LOGON_CHALLENGE_Server_write(&writer, &opcode.body.CMD_AUTH_LOGON_CHALLENGE_Server);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 1", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_LOGON_CHALLENGE_Server 1");
        version3_server_opcode_free(&opcode);
    }while (0);

    /* CMD_AUTH_LOGON_PROOF_Client */
    do {
        unsigned char buffer[] = {1, 241, 62, 229, 209, 131, 196, 200, 169, 80, 14, 63, 90, 93, 138, 238, 78, 46, 69, 225, 247, 204, 143, 28, 245, 238, 142, 17, 206, 211, 29, 215, 8, 107, 30, 72, 27, 77, 4, 161, 24, 216, 242, 222, 92, 89, 213, 92, 129, 46, 101, 236, 62, 78, 245, 45, 225, 128, 94, 26, 103, 21, 236, 200, 65, 238, 184, 144, 138, 88, 187, 0, 208, 2, 255, 0, 239, 190, 173, 222, 1, 2, 3, 4, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 254, 0, 238, 190, 173, 222, 0, 1, 2, 3, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 0, };

        Version3ClientOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version3_client_opcode_read(&reader, &opcode);

        version3_client_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 0", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_LOGON_PROOF, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version3_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 0", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 0", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version3_CMD_AUTH_LOGON_PROOF_Client_write(&writer, &opcode.body.CMD_AUTH_LOGON_PROOF_Client);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 0", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_LOGON_PROOF_Client 0");
        version3_client_opcode_free(&opcode);
    }while (0);

    do {
        unsigned char buffer[] = {1, 241, 62, 229, 209, 131, 196, 200, 169, 80, 14, 63, 90, 93, 138, 238, 78, 46, 69, 225, 247, 204, 143, 28, 245, 238, 142, 17, 206, 211, 29, 215, 8, 107, 30, 72, 27, 77, 4, 161, 24, 216, 242, 222, 92, 89, 213, 92, 129, 46, 101, 236, 62, 78, 245, 45, 225, 128, 94, 26, 103, 21, 236, 200, 65, 238, 184, 144, 138, 88, 187, 0, 208, 1, 255, 0, 239, 190, 173, 222, 1, 2, 3, 4, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 0, };

        Version3ClientOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version3_client_opcode_read(&reader, &opcode);

        version3_client_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 1", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_LOGON_PROOF, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 1");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version3_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 1", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 1", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version3_CMD_AUTH_LOGON_PROOF_Client_write(&writer, &opcode.body.CMD_AUTH_LOGON_PROOF_Client);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 1", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_LOGON_PROOF_Client 1");
        version3_client_opcode_free(&opcode);
    }while (0);

    do {
        unsigned char buffer[] = {1, 241, 62, 229, 209, 131, 196, 200, 169, 80, 14, 63, 90, 93, 138, 238, 78, 46, 69, 225, 247, 204, 143, 28, 245, 238, 142, 17, 206, 211, 29, 215, 8, 107, 30, 72, 27, 77, 4, 161, 24, 216, 242, 222, 92, 89, 213, 92, 129, 46, 101, 236, 62, 78, 245, 45, 225, 128, 94, 26, 103, 21, 236, 200, 65, 238, 184, 144, 138, 88, 187, 0, 208, 0, 0, };

        Version3ClientOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version3_client_opcode_read(&reader, &opcode);

        version3_client_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 2", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_LOGON_PROOF, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 2");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version3_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 2", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 2", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version3_CMD_AUTH_LOGON_PROOF_Client_write(&writer, &opcode.body.CMD_AUTH_LOGON_PROOF_Client);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 2", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_LOGON_PROOF_Client 2");
        version3_client_opcode_free(&opcode);
    }while (0);

    do {
        unsigned char buffer[] = {1, 241, 62, 229, 209, 131, 196, 200, 169, 80, 14, 63, 90, 93, 138, 238, 78, 46, 69, 225, 247, 204, 143, 28, 245, 238, 142, 17, 206, 211, 29, 215, 8, 107, 30, 72, 27, 77, 4, 161, 24, 216, 242, 222, 92, 89, 213, 92, 129, 46, 101, 236, 62, 78, 245, 45, 225, 128, 94, 26, 103, 21, 236, 200, 65, 238, 184, 144, 138, 88, 187, 0, 208, 0, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, };

        Version3ClientOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version3_client_opcode_read(&reader, &opcode);

        version3_client_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 3", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_LOGON_PROOF, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 3");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version3_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 3", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 3", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version3_CMD_AUTH_LOGON_PROOF_Client_write(&writer, &opcode.body.CMD_AUTH_LOGON_PROOF_Client);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 3", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_LOGON_PROOF_Client 3");
        version3_client_opcode_free(&opcode);
    }while (0);

    /* CMD_SURVEY_RESULT */
    do {
        unsigned char buffer[] = {4, 222, 250, 0, 0, 0, 1, 0, 255, };

        Version3ClientOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version3_client_opcode_read(&reader, &opcode);

        version3_client_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_SURVEY_RESULT 0", "failed to read");
        check_opcode(opcode.opcode, CMD_SURVEY_RESULT, __FILE__ ":" STRINGIFY(__LINE__), "CMD_SURVEY_RESULT 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version3_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_SURVEY_RESULT 0", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_SURVEY_RESULT 0", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version3_CMD_SURVEY_RESULT_write(&writer, &opcode.body.CMD_SURVEY_RESULT);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_SURVEY_RESULT 0", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_SURVEY_RESULT 0");
        version3_client_opcode_free(&opcode);
    }while (0);

    /* CMD_AUTH_LOGON_CHALLENGE_Server */
    do {
        unsigned char buffer[] = {0, 0, 0, 58, 43, 237, 162, 169, 101, 37, 78, 69, 4, 195, 168, 246, 106, 134, 201, 81, 114, 215, 99, 107, 54, 137, 237, 192, 63, 252, 193, 66, 165, 121, 50, 1, 7, 32, 183, 155, 62, 42, 135, 130, 60, 171, 143, 94, 191, 191, 142, 177, 1, 8, 83, 80, 6, 41, 139, 91, 173, 189, 91, 83, 225, 137, 94, 100, 75, 137, 174, 120, 124, 96, 218, 20, 21, 219, 130, 36, 67, 72, 71, 108, 63, 211, 188, 22, 60, 89, 21, 128, 86, 5, 146, 59, 82, 46, 114, 18, 41, 82, 70, 15, 184, 237, 114, 71, 169, 255, 31, 242, 228, 96, 253, 255, 127, 249, 3, 0, 0, 0, 0, 89, 29, 166, 11, 52, 253, 100, 94, 56, 108, 84, 192, 24, 182, 167, 47, 8, 8, 2, 1, 194, 216, 23, 56, 5, 251, 84, 143, };

        Version5ServerOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version5_server_opcode_read(&reader, &opcode);

        version5_server_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 0", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_LOGON_CHALLENGE, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version5_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 0", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 0", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version5_CMD_AUTH_LOGON_CHALLENGE_Server_write(&writer, &opcode.body.CMD_AUTH_LOGON_CHALLENGE_Server);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 0", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_LOGON_CHALLENGE_Server 0");
        version5_server_opcode_free(&opcode);
    }while (0);

    /* CMD_AUTH_LOGON_PROOF_Client */
    do {
        unsigned char buffer[] = {1, 4, 73, 87, 221, 32, 81, 98, 245, 250, 254, 179, 103, 7, 114, 9, 81, 86, 32, 8, 8, 32, 193, 38, 202, 200, 247, 59, 70, 251, 136, 50, 6, 130, 201, 151, 96, 66, 228, 117, 249, 124, 96, 98, 228, 84, 102, 166, 254, 220, 233, 170, 124, 254, 116, 218, 112, 136, 204, 118, 36, 196, 40, 136, 181, 239, 196, 29, 180, 107, 197, 44, 251, 0, 3, 221, 105, 240, 247, 88, 76, 88, 240, 134, 54, 58, 26, 190, 110, 30, 77, 90, 78, 192, 86, 88, 136, 230, 41, 1, 108, 191, 61, 247, 142, 130, 147, 111, 29, 190, 229, 105, 52, 205, 8, 130, 148, 239, 93, 15, 150, 159, 252, 23, 11, 228, 66, 8, 46, 209, 16, };

        Version5ClientOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version5_client_opcode_read(&reader, &opcode);

        version5_client_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 0", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_LOGON_PROOF, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version5_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 0", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 0", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version5_CMD_AUTH_LOGON_PROOF_Client_write(&writer, &opcode.body.CMD_AUTH_LOGON_PROOF_Client);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 0", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_LOGON_PROOF_Client 0");
        version5_client_opcode_free(&opcode);
    }while (0);

    /* CMD_AUTH_LOGON_PROOF_Server */
    do {
        unsigned char buffer[] = {1, 0, 25, 255, 233, 250, 100, 169, 155, 175, 246, 179, 141, 156, 17, 171, 220, 174, 128, 196, 210, 231, 0, 0, 0, 0, 0, 0, };

        Version5ServerOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version5_server_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Server 0", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_LOGON_PROOF, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Server 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version5_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Server 0", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Server 0", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version5_CMD_AUTH_LOGON_PROOF_Server_write(&writer, &opcode.body.CMD_AUTH_LOGON_PROOF_Server);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Server 0", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_LOGON_PROOF_Server 0");
    }while (0);

    /* CMD_AUTH_RECONNECT_PROOF_Server */
    do {
        unsigned char buffer[] = {3, 0, 0, 0, };

        Version5ServerOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version5_server_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Server 0", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_RECONNECT_PROOF, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Server 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version5_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Server 0", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Server 0", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version5_CMD_AUTH_RECONNECT_PROOF_Server_write(&writer, &opcode.body.CMD_AUTH_RECONNECT_PROOF_Server);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Server 0", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_RECONNECT_PROOF_Server 0");
    }while (0);

    /* CMD_REALM_LIST_Server */
    do {
        unsigned char buffer[] = {16, 80, 0, 0, 0, 0, 0, 2, 0, 0, 0, 84, 101, 115, 116, 32, 82, 101, 97, 108, 109, 50, 0, 108, 111, 99, 97, 108, 104, 111, 115, 116, 58, 56, 48, 56, 53, 0, 0, 0, 72, 67, 3, 1, 1, 0, 0, 0, 84, 101, 115, 116, 32, 82, 101, 97, 108, 109, 0, 108, 111, 99, 97, 108, 104, 111, 115, 116, 58, 56, 48, 56, 53, 0, 0, 0, 72, 67, 3, 2, 0, 0, 0, };

        Version5ServerOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version5_server_opcode_read(&reader, &opcode);

        version5_server_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Server 0", "failed to read");
        check_opcode(opcode.opcode, CMD_REALM_LIST, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Server 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version5_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Server 0", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Server 0", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version5_CMD_REALM_LIST_Server_write(&writer, &opcode.body.CMD_REALM_LIST_Server);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Server 0", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_REALM_LIST_Server 0");
        version5_server_opcode_free(&opcode);
    }while (0);

    /* CMD_REALM_LIST_Server */
    do {
        unsigned char buffer[] = {16, 81, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 84, 101, 115, 116, 32, 82, 101, 97, 108, 109, 50, 0, 108, 111, 99, 97, 108, 104, 111, 115, 116, 58, 56, 48, 56, 53, 0, 0, 0, 72, 67, 3, 1, 1, 0, 0, 0, 84, 101, 115, 116, 32, 82, 101, 97, 108, 109, 0, 108, 111, 99, 97, 108, 104, 111, 115, 116, 58, 56, 48, 56, 53, 0, 0, 0, 72, 67, 3, 2, 0, 0, 0, };

        Version6ServerOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version6_server_opcode_read(&reader, &opcode);

        version6_server_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Server 0", "failed to read");
        check_opcode(opcode.opcode, CMD_REALM_LIST, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Server 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version6_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Server 0", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Server 0", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version6_CMD_REALM_LIST_Server_write(&writer, &opcode.body.CMD_REALM_LIST_Server);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Server 0", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_REALM_LIST_Server 0");
        version6_server_opcode_free(&opcode);
    }while (0);

    /* CMD_AUTH_LOGON_CHALLENGE_Server */
    do {
        unsigned char buffer[] = {0, 0, 0, 73, 216, 194, 188, 104, 92, 43, 206, 74, 244, 250, 7, 10, 71, 147, 120, 88, 120, 70, 181, 131, 212, 65, 130, 158, 36, 216, 135, 206, 218, 52, 70, 1, 7, 32, 183, 155, 62, 42, 135, 130, 60, 171, 143, 94, 191, 191, 142, 177, 1, 8, 83, 80, 6, 41, 139, 91, 173, 189, 91, 83, 225, 137, 94, 100, 75, 137, 199, 9, 135, 125, 140, 101, 82, 102, 165, 125, 184, 101, 61, 110, 166, 43, 181, 84, 242, 11, 207, 116, 214, 74, 119, 167, 211, 61, 243, 48, 144, 135, 186, 163, 30, 153, 160, 11, 33, 87, 252, 55, 63, 179, 105, 205, 210, 241, 0, };

        Version8ServerOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version8_server_opcode_read(&reader, &opcode);

        version8_server_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 0", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_LOGON_CHALLENGE, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version8_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 0", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 0", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version8_CMD_AUTH_LOGON_CHALLENGE_Server_write(&writer, &opcode.body.CMD_AUTH_LOGON_CHALLENGE_Server);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 0", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_LOGON_CHALLENGE_Server 0");
        version8_server_opcode_free(&opcode);
    }while (0);

    do {
        unsigned char buffer[] = {0, 0, 0, 73, 216, 194, 188, 104, 92, 43, 206, 74, 244, 250, 7, 10, 71, 147, 120, 88, 120, 70, 181, 131, 212, 65, 130, 158, 36, 216, 135, 206, 218, 52, 70, 1, 7, 32, 183, 155, 62, 42, 135, 130, 60, 171, 143, 94, 191, 191, 142, 177, 1, 8, 83, 80, 6, 41, 139, 91, 173, 189, 91, 83, 225, 137, 94, 100, 75, 137, 199, 9, 135, 125, 140, 101, 82, 102, 165, 125, 184, 101, 61, 110, 166, 43, 181, 84, 242, 11, 207, 116, 214, 74, 119, 167, 211, 61, 243, 48, 144, 135, 186, 163, 30, 153, 160, 11, 33, 87, 252, 55, 63, 179, 105, 205, 210, 241, 1, 239, 190, 173, 222, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, };

        Version8ServerOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version8_server_opcode_read(&reader, &opcode);

        version8_server_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 1", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_LOGON_CHALLENGE, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 1");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version8_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 1", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 1", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version8_CMD_AUTH_LOGON_CHALLENGE_Server_write(&writer, &opcode.body.CMD_AUTH_LOGON_CHALLENGE_Server);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 1", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_LOGON_CHALLENGE_Server 1");
        version8_server_opcode_free(&opcode);
    }while (0);

    do {
        unsigned char buffer[] = {0, 0, 0, 73, 216, 194, 188, 104, 92, 43, 206, 74, 244, 250, 7, 10, 71, 147, 120, 88, 120, 70, 181, 131, 212, 65, 130, 158, 36, 216, 135, 206, 218, 52, 70, 1, 7, 32, 183, 155, 62, 42, 135, 130, 60, 171, 143, 94, 191, 191, 142, 177, 1, 8, 83, 80, 6, 41, 139, 91, 173, 189, 91, 83, 225, 137, 94, 100, 75, 137, 199, 9, 135, 125, 140, 101, 82, 102, 165, 125, 184, 101, 61, 110, 166, 43, 181, 84, 242, 11, 207, 116, 214, 74, 119, 167, 211, 61, 243, 48, 144, 135, 186, 163, 30, 153, 160, 11, 33, 87, 252, 55, 63, 179, 105, 205, 210, 241, 4, 1, };

        Version8ServerOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version8_server_opcode_read(&reader, &opcode);

        version8_server_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 2", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_LOGON_CHALLENGE, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 2");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version8_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 2", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 2", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version8_CMD_AUTH_LOGON_CHALLENGE_Server_write(&writer, &opcode.body.CMD_AUTH_LOGON_CHALLENGE_Server);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 2", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_LOGON_CHALLENGE_Server 2");
        version8_server_opcode_free(&opcode);
    }while (0);

    do {
        unsigned char buffer[] = {0, 0, 0, 73, 216, 194, 188, 104, 92, 43, 206, 74, 244, 250, 7, 10, 71, 147, 120, 88, 120, 70, 181, 131, 212, 65, 130, 158, 36, 216, 135, 206, 218, 52, 70, 1, 7, 32, 183, 155, 62, 42, 135, 130, 60, 171, 143, 94, 191, 191, 142, 177, 1, 8, 83, 80, 6, 41, 139, 91, 173, 189, 91, 83, 225, 137, 94, 100, 75, 137, 199, 9, 135, 125, 140, 101, 82, 102, 165, 125, 184, 101, 61, 110, 166, 43, 181, 84, 242, 11, 207, 116, 214, 74, 119, 167, 211, 61, 243, 48, 144, 135, 186, 163, 30, 153, 160, 11, 33, 87, 252, 55, 63, 179, 105, 205, 210, 241, 2, 255, 238, 221, 204, 222, 202, 250, 239, 190, 173, 222, 0, };

        Version8ServerOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version8_server_opcode_read(&reader, &opcode);

        version8_server_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 3", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_LOGON_CHALLENGE, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 3");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version8_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 3", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 3", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version8_CMD_AUTH_LOGON_CHALLENGE_Server_write(&writer, &opcode.body.CMD_AUTH_LOGON_CHALLENGE_Server);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 3", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_LOGON_CHALLENGE_Server 3");
        version8_server_opcode_free(&opcode);
    }while (0);

    do {
        unsigned char buffer[] = {0, 0, 5, };

        Version8ServerOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version8_server_opcode_read(&reader, &opcode);

        version8_server_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 4", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_LOGON_CHALLENGE, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 4");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version8_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 4", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 4", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version8_CMD_AUTH_LOGON_CHALLENGE_Server_write(&writer, &opcode.body.CMD_AUTH_LOGON_CHALLENGE_Server);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 4", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_LOGON_CHALLENGE_Server 4");
        version8_server_opcode_free(&opcode);
    }while (0);

    do {
        unsigned char buffer[] = {0, 0, 0, 73, 216, 194, 188, 104, 92, 43, 206, 74, 244, 250, 7, 10, 71, 147, 120, 88, 120, 70, 181, 131, 212, 65, 130, 158, 36, 216, 135, 206, 218, 52, 70, 1, 7, 32, 183, 155, 62, 42, 135, 130, 60, 171, 143, 94, 191, 191, 142, 177, 1, 8, 83, 80, 6, 41, 139, 91, 173, 189, 91, 83, 225, 137, 94, 100, 75, 137, 199, 9, 135, 125, 140, 101, 82, 102, 165, 125, 184, 101, 61, 110, 166, 43, 181, 84, 242, 11, 207, 116, 214, 74, 119, 167, 211, 61, 243, 48, 144, 135, 186, 163, 30, 153, 160, 11, 33, 87, 252, 55, 63, 179, 105, 205, 210, 241, 6, 255, 238, 221, 204, 222, 202, 250, 239, 190, 173, 222, 0, 1, };

        Version8ServerOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version8_server_opcode_read(&reader, &opcode);

        version8_server_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 5", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_LOGON_CHALLENGE, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 5");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version8_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 5", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 5", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version8_CMD_AUTH_LOGON_CHALLENGE_Server_write(&writer, &opcode.body.CMD_AUTH_LOGON_CHALLENGE_Server);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 5", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_LOGON_CHALLENGE_Server 5");
        version8_server_opcode_free(&opcode);
    }while (0);

    do {
        unsigned char buffer[] = {0, 0, 5, };

        Version8ServerOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version8_server_opcode_read(&reader, &opcode);

        version8_server_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 6", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_LOGON_CHALLENGE, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 6");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version8_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 6", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 6", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version8_CMD_AUTH_LOGON_CHALLENGE_Server_write(&writer, &opcode.body.CMD_AUTH_LOGON_CHALLENGE_Server);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_CHALLENGE_Server 6", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_LOGON_CHALLENGE_Server 6");
        version8_server_opcode_free(&opcode);
    }while (0);

    /* CMD_AUTH_LOGON_PROOF_Client */
    do {
        unsigned char buffer[] = {1, 241, 62, 229, 209, 131, 196, 200, 169, 80, 14, 63, 90, 93, 138, 238, 78, 46, 69, 225, 247, 204, 143, 28, 245, 238, 142, 17, 206, 211, 29, 215, 8, 107, 30, 72, 27, 77, 4, 161, 24, 216, 242, 222, 92, 89, 213, 92, 129, 46, 101, 236, 62, 78, 245, 45, 225, 128, 94, 26, 103, 21, 236, 200, 65, 238, 184, 144, 138, 88, 187, 0, 208, 2, 255, 0, 239, 190, 173, 222, 1, 2, 3, 4, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 254, 0, 238, 190, 173, 222, 0, 1, 2, 3, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 0, };

        Version8ClientOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version8_client_opcode_read(&reader, &opcode);

        version8_client_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 0", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_LOGON_PROOF, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version8_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 0", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 0", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version8_CMD_AUTH_LOGON_PROOF_Client_write(&writer, &opcode.body.CMD_AUTH_LOGON_PROOF_Client);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 0", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_LOGON_PROOF_Client 0");
        version8_client_opcode_free(&opcode);
    }while (0);

    do {
        unsigned char buffer[] = {1, 241, 62, 229, 209, 131, 196, 200, 169, 80, 14, 63, 90, 93, 138, 238, 78, 46, 69, 225, 247, 204, 143, 28, 245, 238, 142, 17, 206, 211, 29, 215, 8, 107, 30, 72, 27, 77, 4, 161, 24, 216, 242, 222, 92, 89, 213, 92, 129, 46, 101, 236, 62, 78, 245, 45, 225, 128, 94, 26, 103, 21, 236, 200, 65, 238, 184, 144, 138, 88, 187, 0, 208, 1, 255, 0, 239, 190, 173, 222, 1, 2, 3, 4, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 0, };

        Version8ClientOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version8_client_opcode_read(&reader, &opcode);

        version8_client_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 1", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_LOGON_PROOF, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 1");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version8_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 1", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 1", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version8_CMD_AUTH_LOGON_PROOF_Client_write(&writer, &opcode.body.CMD_AUTH_LOGON_PROOF_Client);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 1", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_LOGON_PROOF_Client 1");
        version8_client_opcode_free(&opcode);
    }while (0);

    do {
        unsigned char buffer[] = {1, 241, 62, 229, 209, 131, 196, 200, 169, 80, 14, 63, 90, 93, 138, 238, 78, 46, 69, 225, 247, 204, 143, 28, 245, 238, 142, 17, 206, 211, 29, 215, 8, 107, 30, 72, 27, 77, 4, 161, 24, 216, 242, 222, 92, 89, 213, 92, 129, 46, 101, 236, 62, 78, 245, 45, 225, 128, 94, 26, 103, 21, 236, 200, 65, 238, 184, 144, 138, 88, 187, 0, 208, 0, 0, };

        Version8ClientOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version8_client_opcode_read(&reader, &opcode);

        version8_client_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 2", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_LOGON_PROOF, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 2");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version8_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 2", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 2", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version8_CMD_AUTH_LOGON_PROOF_Client_write(&writer, &opcode.body.CMD_AUTH_LOGON_PROOF_Client);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 2", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_LOGON_PROOF_Client 2");
        version8_client_opcode_free(&opcode);
    }while (0);

    do {
        unsigned char buffer[] = {1, 241, 62, 229, 209, 131, 196, 200, 169, 80, 14, 63, 90, 93, 138, 238, 78, 46, 69, 225, 247, 204, 143, 28, 245, 238, 142, 17, 206, 211, 29, 215, 8, 107, 30, 72, 27, 77, 4, 161, 24, 216, 242, 222, 92, 89, 213, 92, 129, 46, 101, 236, 62, 78, 245, 45, 225, 128, 94, 26, 103, 21, 236, 200, 65, 238, 184, 144, 138, 88, 187, 0, 208, 0, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, };

        Version8ClientOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version8_client_opcode_read(&reader, &opcode);

        version8_client_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 3", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_LOGON_PROOF, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 3");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version8_client_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 3", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 3", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version8_CMD_AUTH_LOGON_PROOF_Client_write(&writer, &opcode.body.CMD_AUTH_LOGON_PROOF_Client);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Client 3", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_LOGON_PROOF_Client 3");
        version8_client_opcode_free(&opcode);
    }while (0);

    /* CMD_AUTH_LOGON_PROOF_Server */
    do {
        unsigned char buffer[] = {1, 7, 0, 0, };

        Version8ServerOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version8_server_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Server 0", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_LOGON_PROOF, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Server 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version8_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Server 0", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Server 0", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version8_CMD_AUTH_LOGON_PROOF_Server_write(&writer, &opcode.body.CMD_AUTH_LOGON_PROOF_Server);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Server 0", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_LOGON_PROOF_Server 0");
    }while (0);

    do {
        unsigned char buffer[] = {1, 8, 0, 0, };

        Version8ServerOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version8_server_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Server 1", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_LOGON_PROOF, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Server 1");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version8_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Server 1", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Server 1", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version8_CMD_AUTH_LOGON_PROOF_Server_write(&writer, &opcode.body.CMD_AUTH_LOGON_PROOF_Server);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_LOGON_PROOF_Server 1", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_LOGON_PROOF_Server 1");
    }while (0);

    /* CMD_AUTH_RECONNECT_CHALLENGE_Server */
    do {
        unsigned char buffer[] = {2, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 255, 254, 253, 252, 251, 250, 249, 248, 247, 246, 245, 244, 243, 242, 241, 240, };

        Version8ServerOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version8_server_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_CHALLENGE_Server 0", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_RECONNECT_CHALLENGE, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_CHALLENGE_Server 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version8_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_CHALLENGE_Server 0", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_CHALLENGE_Server 0", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version8_CMD_AUTH_RECONNECT_CHALLENGE_Server_write(&writer, &opcode.body.CMD_AUTH_RECONNECT_CHALLENGE_Server);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_CHALLENGE_Server 0", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_RECONNECT_CHALLENGE_Server 0");
    }while (0);

    do {
        unsigned char buffer[] = {2, 3, };

        Version8ServerOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version8_server_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_CHALLENGE_Server 1", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_RECONNECT_CHALLENGE, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_CHALLENGE_Server 1");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version8_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_CHALLENGE_Server 1", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_CHALLENGE_Server 1", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version8_CMD_AUTH_RECONNECT_CHALLENGE_Server_write(&writer, &opcode.body.CMD_AUTH_RECONNECT_CHALLENGE_Server);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_CHALLENGE_Server 1", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_RECONNECT_CHALLENGE_Server 1");
    }while (0);

    /* CMD_AUTH_RECONNECT_PROOF_Server */
    do {
        unsigned char buffer[] = {3, 0, 0, 0, };

        Version8ServerOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version8_server_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Server 0", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_RECONNECT_PROOF, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Server 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version8_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Server 0", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Server 0", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version8_CMD_AUTH_RECONNECT_PROOF_Server_write(&writer, &opcode.body.CMD_AUTH_RECONNECT_PROOF_Server);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Server 0", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_RECONNECT_PROOF_Server 0");
    }while (0);

    do {
        unsigned char buffer[] = {3, 16, 0, 0, };

        Version8ServerOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version8_server_opcode_read(&reader, &opcode);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Server 1", "failed to read");
        check_opcode(opcode.opcode, CMD_AUTH_RECONNECT_PROOF, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Server 1");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version8_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Server 1", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Server 1", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version8_CMD_AUTH_RECONNECT_PROOF_Server_write(&writer, &opcode.body.CMD_AUTH_RECONNECT_PROOF_Server);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_AUTH_RECONNECT_PROOF_Server 1", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_AUTH_RECONNECT_PROOF_Server 1");
    }while (0);

    /* CMD_REALM_LIST_Server */
    do {
        unsigned char buffer[] = {16, 22, 0, 0, 0, 0, 0, 1, 0, 0, 0, 3, 65, 0, 65, 0, 0, 0, 200, 67, 1, 0, 2, 0, 0, };

        Version8ServerOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version8_server_opcode_read(&reader, &opcode);

        version8_server_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Server 0", "failed to read");
        check_opcode(opcode.opcode, CMD_REALM_LIST, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Server 0");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version8_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Server 0", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Server 0", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version8_CMD_REALM_LIST_Server_write(&writer, &opcode.body.CMD_REALM_LIST_Server);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Server 0", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_REALM_LIST_Server 0");
        version8_server_opcode_free(&opcode);
    }while (0);

    do {
        unsigned char buffer[] = {16, 27, 0, 0, 0, 0, 0, 1, 0, 0, 0, 4, 65, 0, 65, 0, 0, 0, 200, 67, 1, 0, 2, 1, 12, 1, 243, 22, 0, 0, };

        Version8ServerOpcodeContainer opcode;

        reader = wlm_create_reader(buffer, sizeof(buffer));
        result = version8_server_opcode_read(&reader, &opcode);

        version8_server_opcode_free(&opcode);
        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Server 1", "failed to read");
        check_opcode(opcode.opcode, CMD_REALM_LIST, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Server 1");

        reader.index = 0;
        reader.length = 1;

        while (true) {
            result = version8_server_opcode_read(&reader, &opcode);
            check_result(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Server 1", "failed partial");
            if (result == WLM_RESULT_SUCCESS) {
                break;
            }
            reader.index = 0;
            reader.length += result;
            if(reader.length > sizeof(buffer)) {
                check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Server 1", "gave too much index back");
            }
        }

        writer = wlm_create_writer(write_buffer, sizeof(write_buffer));
        result = version8_CMD_REALM_LIST_Server_write(&writer, &opcode.body.CMD_REALM_LIST_Server);

        check_complete(result, __FILE__ ":" STRINGIFY(__LINE__), "CMD_REALM_LIST_Server 1", "failed to write");

        wlm_test_compare_buffers(buffer, write_buffer, sizeof(buffer), __FILE__ ":" STRINGIFY(__LINE__) " CMD_REALM_LIST_Server 1");
        version8_server_opcode_free(&opcode);
    }while (0);

    return 0;
}

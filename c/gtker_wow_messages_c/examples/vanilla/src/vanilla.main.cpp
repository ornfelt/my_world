#include "server_socket.h"

#include "wow_login_messages_cpp/all.hpp"
#include "wow_login_messages_cpp/version3.hpp"
#include "wow_login_messages_cpp/wow_login_messages.hpp"

#include "wow_world_messages_cpp/vanilla.hpp"
#include "wow_world_messages_cpp/wow_world_messages.hpp"

#include "wow_srp_cpp/server.hpp"
#include "wow_srp_cpp/vanilla.hpp"

class NetworkReader final : public wow_login_messages::Reader, public wow_world_messages::Reader
{
public:
    explicit NetworkReader(const char* port) : m_socket(initialize_socket(port)) {}

    void accept() { server_accept(&m_socket); }

    void send(std::vector<unsigned char>&& data) { server_send(&m_socket, data.data(), data.size()); }

    void send_encrypted(const wow_world_messages::vanilla::ServerOpcode& opcode, wow_srp::VanillaHeaderCrypto& header)
    {
        send(opcode.write([&header](auto data, auto length) { header.encrypt(data, static_cast<uint16_t>(length)); }));
    }

private:
    uint8_t read_u8() override
    {
        unsigned char buf;

        server_recv(&m_socket, &buf, 1);

        return static_cast<uint8_t>(buf);
    }
    ServerSocket m_socket;
};

void auth_flow(NetworkReader& auth, NetworkReader& world)
{
    namespace all = wow_login_messages::all;

    auto all_client = all::ClientOpcode::read(auth);
    auto challenge = all_client.get<all::CMD_AUTH_LOGON_CHALLENGE_Client>();

    auto verifier =
        wow_srp::Verifier::from_username_and_password(std::move(challenge.account_name), challenge.account_name);
    auto proof = verifier.into_proof();

    namespace v3 = wow_login_messages::version3;

    v3::CMD_AUTH_LOGON_CHALLENGE_Server s{};
    s.result = v3::LoginResult::SUCCESS;
    s.server_public_key = proof.server_public_key();
    s.generator = {wow_srp::GENERATOR};
    s.large_safe_prime = std::vector(wow_srp::LARGE_SAFE_PRIME.begin(), wow_srp::LARGE_SAFE_PRIME.end());
    s.salt = proof.salt();
    s.crc_salt = {0};
    s.security_flag = v3::SecurityFlag::NONE;

    auth.send(s.write());

    auto client = v3::ClientOpcode::read(auth);
    auto c = client.get<v3::CMD_AUTH_LOGON_PROOF_Client>();

    auto server = proof.into_server(c.client_public_key, c.client_proof);

    v3::CMD_AUTH_LOGON_PROOF_Server p{};
    p.result = v3::LoginResult::SUCCESS;
    p.server_proof = server.server_proof();

    auth.send(p.write());
    (void)v3::ClientOpcode::read(auth).get<v3::CMD_REALM_LIST_Client>();

    v3::CMD_REALM_LIST_Server realms;
    realms.realms = {v3::Realm{v3::RealmType::PLAYER_VS_ENVIRONMENT, v3::RealmFlag::REALM_FLAG_NONE, "Test Realm",
                               "localhost:8085", 0.0f, 0, v3::RealmCategory ::DEFAULT, 1}};

    auth.send(realms.write());
    world.accept();

    auto seed = wow_srp::VanillaProofSeed();

    namespace vanilla = wow_world_messages::vanilla;
    world.send(vanilla::SMSG_AUTH_CHALLENGE{seed.proof_seed()}.write([](auto, auto) {}));

    auto opcode = vanilla::ClientOpcode::read(world, [](unsigned char*, size_t) {});
    auto session = opcode.get<vanilla::CMSG_AUTH_SESSION>();

    auto header_crypto = seed.into_server_header_crypto(challenge.account_name, server.session_key(),
                                                        session.client_proof, session.client_seed);

    world.send_encrypted(vanilla::SMSG_AUTH_RESPONSE{vanilla::WorldResult::AUTH_OK, 0, 0, 0, 0}, header_crypto);

    constexpr uint64_t GUID = 1;

    while (opcode.get_if<vanilla::CMSG_PLAYER_LOGIN>() == nullptr)
    {
        opcode = vanilla::ClientOpcode::read(world, [&header_crypto](unsigned char* data, const size_t length)
                                             { header_crypto.decrypt(data, static_cast<uint16_t>(length)); });

        printf("%s\n", opcode.to_string());

        if (opcode.get_if<vanilla::CMSG_CHAR_ENUM>() != nullptr)
        {
            auto character = vanilla::Character{GUID,
                                                "Test Char",
                                                vanilla::Race::HUMAN,
                                                vanilla::Class::WARRIOR,
                                                vanilla::Gender::MALE,
                                                0,
                                                0,
                                                0,
                                                0,
                                                0,
                                                0,
                                                vanilla::Area::NORTHSHIRE_ABBEY,
                                                vanilla::Map::EASTERN_KINGDOMS,
                                                {0.0f, 0.0f, 0.0f},
                                                0,
                                                vanilla::CharacterFlags::CHARACTER_FLAGS_NONE,
                                                false,
                                                0,
                                                0,
                                                vanilla::CreatureFamily::NONE,
                                                std::array<vanilla::CharacterGear, 19>()};
            world.send_encrypted(vanilla::SMSG_CHAR_ENUM{std::vector{character}}, header_crypto);
        };
    }
    printf("Logging into %lu\n", (unsigned long)opcode.get<vanilla::CMSG_PLAYER_LOGIN>().guid);

    auto verify_world =
        vanilla::SMSG_LOGIN_VERIFY_WORLD{vanilla::Map::EASTERN_KINGDOMS, {-8949.95f, -132.493f, 83.5312f}, 0.0f};
    world.send_encrypted(std::move(verify_world), header_crypto);

    world.send_encrypted(vanilla::SMSG_TUTORIAL_FLAGS{{
                             0xffffffff,
                             0xffffffff,
                             0xffffffff,
                             0xffffffff,
                             0xffffffff,
                             0xffffffff,
                             0xffffffff,
                             0xffffffff,
                         }},
                         header_crypto);

    vanilla::Object object;
    object.mask2.object_guid(GUID);
    object.mask2.object_scale_x(1.0f);
    object.mask2.object_type(25);
    object.mask2.unit_bytes_0({1, 1, 1, 1});
    object.mask2.game_object_displayid(50);
    object.mask2.unit_factiontemplate(1);
    object.mask2.unit_health(1);
    object.mask2.unit_level(1);
    object.mask2.unit_nativedisplayid(50);


    object.update_type = vanilla::UpdateType::CREATE_OBJECT2;
    object.guid3 = 1;
    object.object_type = vanilla::ObjectType::PLAYER;
    object.movement2.update_flag = static_cast<vanilla::UpdateFlag>(
        vanilla::UPDATE_FLAG_SELF | vanilla::UPDATE_FLAG_ALL | vanilla::UPDATE_FLAG_LIVING);
    object.movement2.flags = vanilla::MOVEMENT_FLAGS_NONE;
    object.movement2.timestamp = 0;
    object.movement2.living_position.x = -8949.95f;
    object.movement2.living_position.y = -132.493f;
    object.movement2.living_position.z = 83.5312f;
    object.movement2.living_orientation = 0.0f;
    object.movement2.fall_time = 0.0f;
    object.movement2.walking_speed = 1.0f;
    object.movement2.running_speed = 70.0f;
    object.movement2.backwards_running_speed = 4.5f;
    object.movement2.swimming_speed = 0.0f;
    object.movement2.backwards_swimming_speed = 0.0f;
    object.movement2.turn_rate = 3.1415f;
    object.movement2.unknown1 = 0;

    world.send_encrypted(vanilla::SMSG_UPDATE_OBJECT{0, {object}}, header_crypto);
}

int main()
{
    NetworkReader auth{"3724"};
    auth.accept();

    NetworkReader world("8085");
    auth_flow(auth, world);

    while (true)
    {
    }
}

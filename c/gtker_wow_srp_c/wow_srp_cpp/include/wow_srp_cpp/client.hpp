#pragma once
#include "wow_srp_cpp/wow_srp.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <memory>

#include "wow_srp_cpp_export.h"

struct WowSrpClientChallenge;
struct WowSrpClient;

namespace wow_srp {
class Client;

class ClientChallenge {
public:
  WOW_SRP_CPP_EXPORT ClientChallenge(const std::string &username, const std::string &password,
                  uint8_t generator,
                  KeyArray large_safe_prime,
                  KeyArray server_public_key,
                  KeyArray salt);

  WOW_SRP_CPP_EXPORT ClientChallenge(const ClientChallenge &) = delete;
  WOW_SRP_CPP_EXPORT ClientChallenge(ClientChallenge &&) = default;

  WOW_SRP_CPP_EXPORT ClientChallenge &operator=(const ClientChallenge &) = delete;
  WOW_SRP_CPP_EXPORT ClientChallenge &operator=(ClientChallenge &&) = default;

  WOW_SRP_CPP_EXPORT ~ClientChallenge() = default;

  WOW_SRP_CPP_EXPORT Client verify_server_proof(ProofArray server_proof);

  [[nodiscard]] WOW_SRP_CPP_EXPORT const KeyArray &
  client_public_key() const noexcept;
  [[nodiscard]] WOW_SRP_CPP_EXPORT const ProofArray &
  client_proof() const noexcept;

private:
  std::unique_ptr<WowSrpClientChallenge, void(*)(WowSrpClientChallenge*)> m_inner;
  KeyArray m_client_public_key;
  ProofArray m_client_proof;
};

class Client {
public:
  Client() = delete;
  WOW_SRP_CPP_EXPORT ~Client() = default;
  Client(const Client&) = delete;
  WOW_SRP_CPP_EXPORT Client(Client&&) = default;

  Client &operator=(const Client&) = delete;
  WOW_SRP_CPP_EXPORT Client& operator=(Client&&) = default;

  [[nodiscard]] WOW_SRP_CPP_EXPORT const SessionKeyArray& session_key() const noexcept;
  [[nodiscard]] WOW_SRP_CPP_EXPORT std::pair<ReconnectDataArray, ProofArray> calculate_reconnect_values(ReconnectDataArray server_challenge_data);

private:
  friend ClientChallenge;
  WOW_SRP_CPP_EXPORT Client(WowSrpClient* inner, SessionKeyArray session_key);

  std::unique_ptr<WowSrpClient, void(*)(WowSrpClient*)> m_inner;
  SessionKeyArray m_session_key;
};

} // namespace wow_srp


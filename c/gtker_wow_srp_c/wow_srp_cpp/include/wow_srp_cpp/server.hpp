#pragma once
#include <array>
#include <cstdint>
#include <memory>
#include <string>

#include "wow_srp_cpp/wow_srp.hpp"

#include "wow_srp_cpp_export.h"

struct WowSrpVerifier;
struct WowSrpProof;
struct WowSrpServer;

namespace wow_srp {
class Proof;
class Server;

class Verifier {
public:
  WOW_SRP_CPP_EXPORT Verifier(std::string &&username, const KeyArray &password_verifier,
           const KeyArray &salt);

  WOW_SRP_CPP_EXPORT Verifier(const Verifier &) = delete;
  WOW_SRP_CPP_EXPORT Verifier(Verifier &&) = default;

  WOW_SRP_CPP_EXPORT Verifier &operator=(const Verifier &) = delete;
  WOW_SRP_CPP_EXPORT Verifier &operator=(Verifier &&) = default;

  WOW_SRP_CPP_EXPORT ~Verifier() = default;

  WOW_SRP_CPP_EXPORT static Verifier from_username_and_password(std::string &&username,
                                             const std::string &password);

  [[nodiscard]] WOW_SRP_CPP_EXPORT Proof into_proof() noexcept;

  [[nodiscard]] WOW_SRP_CPP_EXPORT const KeyArray &salt() const noexcept;

  [[nodiscard]] WOW_SRP_CPP_EXPORT const KeyArray &verifier() const noexcept;

private:
  std::string m_username;
  KeyArray m_salt;
  KeyArray m_verifier;
  std::unique_ptr<WowSrpVerifier, void (*)(WowSrpVerifier *)> m_inner;
};

class Proof {
public:
  Proof() = delete;
  Proof(const Proof &) = delete;
  WOW_SRP_CPP_EXPORT Proof(Proof &&) = default;

  Proof &operator=(const Proof &) = delete;
  WOW_SRP_CPP_EXPORT Proof &operator=(Proof &&) = default;

  WOW_SRP_CPP_EXPORT ~Proof() = default;

  [[nodiscard]] WOW_SRP_CPP_EXPORT const KeyArray &salt() const noexcept;
  [[nodiscard]] WOW_SRP_CPP_EXPORT const KeyArray &server_public_key() const noexcept;

  WOW_SRP_CPP_EXPORT Server into_server(const KeyArray &client_public_key,
                     const ProofArray &client_proof);

private:
  friend Verifier;
  WOW_SRP_CPP_EXPORT Proof(WowSrpProof *inner, KeyArray salt, KeyArray server_public_key) noexcept;

  std::unique_ptr<WowSrpProof, void (*)(WowSrpProof *)> m_inner;
  KeyArray m_salt;
  KeyArray m_server_public_key;
};

class Server {
public:
  Server() = delete;
  Server(const Server &) = delete;
  WOW_SRP_CPP_EXPORT Server(Server &&) = default;

  Server &operator=(const Server &) = delete;
  WOW_SRP_CPP_EXPORT Server &operator=(Server &&) = default;

  WOW_SRP_CPP_EXPORT ~Server() = default;

  [[nodiscard]] WOW_SRP_CPP_EXPORT bool
  verify_reconnection_attempt(const ReconnectDataArray &client_data,
                              const ProofArray &client_proof);

  [[nodiscard]] WOW_SRP_CPP_EXPORT const ProofArray &server_proof() const noexcept;
  [[nodiscard]] WOW_SRP_CPP_EXPORT const SessionKeyArray &session_key() const noexcept;
  [[nodiscard]] WOW_SRP_CPP_EXPORT const ReconnectDataArray &reconnect_data() const noexcept;

private:
  friend Proof;
  WOW_SRP_CPP_EXPORT Server(WowSrpServer *inner, ProofArray server_proof,
         SessionKeyArray session_key,
         ReconnectDataArray reconnect_data) noexcept;

  std::unique_ptr<WowSrpServer, void (*)(WowSrpServer *)> m_inner;
  ProofArray m_server_proof;
  SessionKeyArray m_session_key;
  ReconnectDataArray m_reconnect_data;
};

} // namespace wow_srp


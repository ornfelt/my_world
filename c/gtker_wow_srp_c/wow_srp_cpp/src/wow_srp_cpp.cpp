#include <algorithm>
#include <array>
#include <string>

#include "wow_srp/client.h"
#include "wow_srp/server.h"
#include "wow_srp/vanilla.h"
#include "wow_srp/wow_srp.h"

#include "wow_srp_cpp/client.hpp"
#include "wow_srp_cpp/server.hpp"
#include "wow_srp_cpp/vanilla.hpp"
#include "wow_srp_cpp/wow_srp.hpp"

// Server
namespace wow_srp {
// Verifier

WOW_SRP_CPP_EXPORT Verifier::Verifier(std::string &&username, const KeyArray &password_verifier,
                   const KeyArray &salt)
    : m_username(username), m_salt{salt}, m_verifier{password_verifier},
      m_inner(nullptr, wow_srp_verifier_free) {
  char out_error = wow_srp::SUCCESS;

  m_inner.reset(wow_srp_verifier_from_database_values(
      m_username.c_str(), m_verifier.data(), m_salt.data(), &out_error));

  if (out_error != wow_srp::SUCCESS) {
    throw std::exception();
  }
}

WOW_SRP_CPP_EXPORT Verifier Verifier::from_username_and_password(std::string &&username,
                                              const std::string &password) {
  char out_error = WOW_SRP_SUCCESS;
  auto verifier = wow_srp_verifier_from_username_and_password(
      username.c_str(), password.c_str(), &out_error);

  if (out_error != WOW_SRP_SUCCESS || verifier == nullptr) {
    throw std::exception();
  }

  KeyArray password_verifier{};
  std::copy_n(wow_srp_verifier_password_verifier(verifier),
              password_verifier.size(), password_verifier.begin());

  KeyArray salt{};
  std::copy_n(wow_srp_verifier_salt(verifier), salt.size(), salt.begin());

  return {std::move(username), password_verifier, salt};
}

WOW_SRP_CPP_EXPORT Proof Verifier::into_proof() noexcept {
  auto proof = wow_srp_verifier_into_proof(m_inner.release());

  auto public_key = wow_srp_proof_server_public_key(proof);
  KeyArray server_public_key{};
  std::copy_n(public_key, server_public_key.size(), server_public_key.begin());

  return Proof{proof, m_salt, server_public_key};
}

WOW_SRP_CPP_EXPORT const KeyArray &Verifier::salt() const noexcept { return m_salt; }

WOW_SRP_CPP_EXPORT const KeyArray &Verifier::verifier() const noexcept { return m_verifier; }

// Proof

WOW_SRP_CPP_EXPORT Proof::Proof(WowSrpProof *inner, KeyArray salt,
             KeyArray server_public_key) noexcept
    : m_inner{inner, wow_srp_proof_free}, m_salt(salt),
      m_server_public_key(server_public_key) {}

WOW_SRP_CPP_EXPORT const KeyArray &Proof::salt() const noexcept { return m_salt; }

WOW_SRP_CPP_EXPORT const KeyArray &Proof::server_public_key() const noexcept {
  return m_server_public_key;
}

WOW_SRP_CPP_EXPORT Server Proof::into_server(const KeyArray &client_public_key,
                          const ProofArray &client_proof) {
  ProofArray out_server_proof{};
  char out_error{wow_srp::SUCCESS};

  auto server = wow_srp_proof_into_server(
      m_inner.get(), client_public_key.data(), client_proof.data(),
      out_server_proof.data(), &out_error);

  if (out_error != wow_srp::SUCCESS || server == nullptr) {
    throw std::exception();
  }
  (void)m_inner.release();

  SessionKeyArray session_key{};
  std::copy_n(wow_srp_server_session_key(server), session_key.size(),
              session_key.data());

  ReconnectDataArray reconnect_data{};
  std::copy_n(wow_srp_server_reconnect_challenge_data(server),
              reconnect_data.size(), reconnect_data.data());

  return {server, out_server_proof, session_key, reconnect_data};
}

WOW_SRP_CPP_EXPORT Server::Server(WowSrpServer *inner, ProofArray server_proof,
               SessionKeyArray session_key,
               ReconnectDataArray reconnect_data) noexcept
    : m_inner{inner, wow_srp_server_free}, m_server_proof(server_proof),
      m_session_key(session_key), m_reconnect_data(reconnect_data) {}

[[nodiscard]] WOW_SRP_CPP_EXPORT const ProofArray &Server::server_proof() const noexcept {
  return m_server_proof;
}

[[nodiscard]] WOW_SRP_CPP_EXPORT const SessionKeyArray &Server::session_key() const noexcept {
  return m_session_key;
}

[[nodiscard]] WOW_SRP_CPP_EXPORT const ReconnectDataArray &
Server::reconnect_data() const noexcept {
  return m_reconnect_data;
}
WOW_SRP_CPP_EXPORT bool Server::verify_reconnection_attempt(const ReconnectDataArray &client_data,
                                         const ProofArray &client_proof) {
  char out_error = wow_srp::SUCCESS;
  const auto success = wow_srp_server_verify_reconnection_attempt(
      m_inner.get(), client_data.data(), client_proof.data(), &out_error);

  if (out_error != wow_srp::SUCCESS) {
    throw std::exception();
  }

  return success;
}

} // namespace wow_srp

// Client
namespace wow_srp {

WOW_SRP_CPP_EXPORT ClientChallenge::ClientChallenge(const std::string &username,
                                 const std::string &password, uint8_t generator,
                                 KeyArray large_safe_prime,
                                 KeyArray server_public_key, KeyArray salt)
    : m_inner(nullptr, wow_srp_client_challenge_free), m_client_public_key{},
      m_client_proof{} {
  char out_error = wow_srp::SUCCESS;

  m_inner.reset(wow_srp_client_challenge_create(
      username.c_str(), password.c_str(), generator, large_safe_prime.data(),
      server_public_key.data(), salt.data(), &out_error));

  if (out_error != wow_srp::SUCCESS) {
    throw std::exception();
  }

  auto public_key = wow_srp_client_challenge_client_public_key(m_inner.get());
  std::copy_n(public_key, m_client_public_key.size(),
              m_client_public_key.begin());

  auto proof = wow_srp_client_challenge_client_proof(m_inner.get());
  std::copy_n(proof, m_client_proof.size(), m_client_proof.begin());
}

WOW_SRP_CPP_EXPORT const KeyArray &ClientChallenge::client_public_key() const noexcept {
  return m_client_public_key;
}

WOW_SRP_CPP_EXPORT const ProofArray &ClientChallenge::client_proof() const noexcept {
  return m_client_proof;
}

WOW_SRP_CPP_EXPORT Client ClientChallenge::verify_server_proof(ProofArray server_proof) {
  char out_error{wow_srp::SUCCESS};
  auto client = wow_srp_client_challenge_verify_server_proof(
      m_inner.get(), server_proof.data(), &out_error);

  if (out_error != wow_srp::SUCCESS || client == nullptr) {
    throw std::exception();
  }
  (void)m_inner.release();

  SessionKeyArray session_key{};
  std::copy_n(wow_srp_client_session_key(client), session_key.size(),
              session_key.begin());

  return {client, session_key};
}

// Client

WOW_SRP_CPP_EXPORT Client::Client(WowSrpClient *inner, SessionKeyArray session_key)
    : m_inner{inner, wow_srp_client_free}, m_session_key(session_key) {}

WOW_SRP_CPP_EXPORT const SessionKeyArray &Client::session_key() const noexcept {
  return m_session_key;
}

WOW_SRP_CPP_EXPORT std::pair<ReconnectDataArray, ProofArray>
Client::calculate_reconnect_values(ReconnectDataArray server_challenge_data) {
  ReconnectDataArray out_client_challenge_data{};
  ProofArray out_client_proof{};

  char out_error = wow_srp::SUCCESS;

  wow_srp_client_calculate_reconnect_values(
      m_inner.get(), server_challenge_data.data(),
      out_client_challenge_data.data(), out_client_proof.data(), &out_error);

  if (out_error != wow_srp::SUCCESS) {
    throw std::exception();
  }

  return std::make_pair(out_client_challenge_data, out_client_proof);
}

} // namespace wow_srp

// Vanilla
namespace wow_srp {
WOW_SRP_CPP_EXPORT VanillaProofSeed::VanillaProofSeed() noexcept
    : m_inner{wow_srp_vanilla_proof_seed_new(),
              wow_srp_vanilla_proof_seed_free},
      m_seed(wow_srp_vanilla_proof_seed(m_inner.get(), nullptr)) {}

WOW_SRP_CPP_EXPORT uint32_t VanillaProofSeed::proof_seed() const noexcept { return m_seed; }

WOW_SRP_CPP_EXPORT std::pair<VanillaHeaderCrypto, ProofArray>
VanillaProofSeed::into_client_header_crypto(const std::string &username,
                                            SessionKeyArray &session_key,
                                            uint32_t server_seed) {

  char out_error = wow_srp::SUCCESS;
  ProofArray out_client_proof{};

  auto header_crypto = wow_srp_vanilla_proof_seed_into_client_header_crypto(
      m_inner.get(), username.c_str(), session_key.data(), server_seed,
      out_client_proof.data(), &out_error);

  if (out_error != wow_srp::SUCCESS || header_crypto == nullptr) {
    throw std::exception();
  }
  (void)m_inner.release();

  return std::make_pair(VanillaHeaderCrypto(header_crypto), out_client_proof);
}

WOW_SRP_CPP_EXPORT VanillaHeaderCrypto VanillaProofSeed::into_server_header_crypto(
    const std::string &username, const SessionKeyArray &session_key,
    const ProofArray &client_proof, uint32_t client_seed) {

  char out_error = wow_srp::SUCCESS;
  auto header_crypto = wow_srp_vanilla_proof_seed_into_server_header_crypto(
      m_inner.get(), username.c_str(), session_key.data(), client_proof.data(),
      client_seed, &out_error);

  if (out_error != wow_srp::SUCCESS || header_crypto == nullptr) {
    throw std::exception();
  }
  (void)m_inner.release();

  return VanillaHeaderCrypto(header_crypto);
}

WOW_SRP_CPP_EXPORT VanillaHeaderCrypto::VanillaHeaderCrypto(
    WowSrpVanillaHeaderCrypto *inner) noexcept
    : m_inner{inner, wow_srp_vanilla_header_crypto_free} {}

WOW_SRP_CPP_EXPORT void VanillaHeaderCrypto::encrypt(uint8_t *data, uint16_t length) {
  char out_error = wow_srp::SUCCESS;
  wow_srp_vanilla_header_crypto_encrypt(m_inner.get(), data, length,
                                        &out_error);

  if (out_error != wow_srp::SUCCESS) {
    throw std::exception();
  }
}

WOW_SRP_CPP_EXPORT void VanillaHeaderCrypto::decrypt(uint8_t *data, uint16_t length) {
  char out_error = wow_srp::SUCCESS;
  wow_srp_vanilla_header_crypto_decrypt(m_inner.get(), data, length,
                                        &out_error);

  if (out_error != wow_srp::SUCCESS) {
    throw std::exception();
  }
}

} // namespace wow_srp
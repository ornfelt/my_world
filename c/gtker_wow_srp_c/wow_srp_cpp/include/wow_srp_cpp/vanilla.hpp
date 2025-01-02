#pragma once

#include "wow_srp_cpp/wow_srp.hpp"
#include <cstdint>
#include <string>
#include <utility>
#include <memory>

#include "wow_srp_cpp_export.h"

struct WowSrpVanillaProofSeed;
struct WowSrpVanillaHeaderCrypto;

namespace wow_srp {

class VanillaProofSeed;

class VanillaHeaderCrypto {
public:
  VanillaHeaderCrypto() = delete;
  WOW_SRP_CPP_EXPORT ~VanillaHeaderCrypto() = default;

  VanillaHeaderCrypto(const VanillaHeaderCrypto &) = delete;
  WOW_SRP_CPP_EXPORT VanillaHeaderCrypto(VanillaHeaderCrypto &&) = default;

  VanillaHeaderCrypto &operator=(const VanillaHeaderCrypto &) = delete;
  WOW_SRP_CPP_EXPORT VanillaHeaderCrypto &operator=(VanillaHeaderCrypto &&) = default;

  WOW_SRP_CPP_EXPORT void encrypt(uint8_t *data, uint16_t length);
  WOW_SRP_CPP_EXPORT void decrypt(uint8_t *data, uint16_t length);

private:
  friend VanillaProofSeed;
  WOW_SRP_CPP_EXPORT explicit VanillaHeaderCrypto(WowSrpVanillaHeaderCrypto *inner) noexcept;

  std::unique_ptr<WowSrpVanillaHeaderCrypto, void(*)(WowSrpVanillaHeaderCrypto*)> m_inner;
};

class VanillaProofSeed {
public:
  WOW_SRP_CPP_EXPORT VanillaProofSeed() noexcept;
  WOW_SRP_CPP_EXPORT ~VanillaProofSeed() noexcept = default;

  VanillaProofSeed(const VanillaProofSeed &) = delete;
  WOW_SRP_CPP_EXPORT VanillaProofSeed(VanillaProofSeed &&) = default;

  VanillaProofSeed &operator=(const VanillaProofSeed &) = delete;
  WOW_SRP_CPP_EXPORT VanillaProofSeed &operator=(VanillaProofSeed &&) = default;

  [[nodiscard]] WOW_SRP_CPP_EXPORT uint32_t proof_seed() const noexcept;

  WOW_SRP_CPP_EXPORT std::pair<VanillaHeaderCrypto, ProofArray> into_client_header_crypto(
      const std::string &username,
      SessionKeyArray &session_key, uint32_t server_seed);

  WOW_SRP_CPP_EXPORT VanillaHeaderCrypto into_server_header_crypto(
      const std::string &username,
      const SessionKeyArray &session_key,
      const ProofArray &client_proof, uint32_t client_seed);

private:
  std::unique_ptr<WowSrpVanillaProofSeed, void(*)(WowSrpVanillaProofSeed*)> m_inner;
  uint32_t m_seed;
};

} // namespace wow_srp


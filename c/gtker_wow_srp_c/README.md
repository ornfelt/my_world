# `wow_srp_c`

C bindings for the [`wow_srp` library](https://github.com/gtker/wow_srp) used for World of Warcraft authentication
server
key negotiation and world server header encryption/decryption.

Header files for both C (`wow_srp/wow_srp.h`) and C++ (`wow_srp/wow_srp.hpp`) are provided.
You should only use one of these, and probably the C++ one unless you know what you're doing.

## Quick Use for Cmake

* [Install Rust](https://www.rust-lang.org/tools/install).
* Add this repository to your project as
  either [a git submodule](https://git-scm.com/book/en/v2/Git-Tools-Submodules), [a git subtree](https://www.atlassian.com/git/tutorials/git-subtree),
  or just a straight copy.
* Add the following to your `CMakeLists.txt`:

```cmake
add_subdirectory(wow_srp_c)

# For C API
target_link_libraries(YOUR_TARGET_HERE PRIVATE wow_srp::wow_srp)
# For C++ API
target_link_libraries(YOUR_TARGET_HERE PRIVATE wow_srp_cpp::wow_srp_cpp)
```

* Import the `wow_srp/{client, server, wow_srp, wrath, tbc, vanilla}.h` headers file.

## Slow use for everybody else

By default, both static libraries (`.a`/`.lib`) and shared libraries (`.so`/`.dll`) are built.

* [Install Rust](https://www.rust-lang.org/tools/install).
* Build the library with `cargo build --release`.
* Add the `include/` folder to your include path.
* Either add the `target/release` directory to your library path or copy the `target/release/wow_srp.(dll/so/a/lib)`
  files to your build directory.
* Import the relevant header files from `wow_srp/`.
* For C++, compile `wow_srp_c/wow_srp_cpp/src` with include path in `wow_srp_cpp/include`.

# Server Usage

## Authentication

Import the `wow_srp/server.h` header file.

The general flow is:

```text
WowSrpVerifier -> WowSrpProof -> WowSrpServer
```

When creating users, generate a salt and password verifier from `wow_srp_verifier_from_username_and_password`.
Save these in the database.

When the client attempts a login, create a `WowSrpVerifier` from `wow_srp_verifier_from_database_values` and use the previously
stored salt and password verifier. Then convert it into a `WowSrpProof` with `wow_srp_verifier_into_proof`.

After receiving the client public key the `WowSrpProof` should be turned into a `WowSrpServer` with `wow_srp_proof_into_server`.

Temporarily save the session key for header decryption.

# Client Usage

Import the `wow_srp/client.h` header file.

The general flow is:

```text
WowSrpClientChallenge -> WowSrpClient
```

Create a `WowSrpClienChallenge` through `wow_srp_client_challenge` with your username/password and the values received from
[CMD_AUTH_LOGON_CHALLENGE_Server](https://gtker.com/wow_messages/docs/cmd_auth_logon_challenge_server.html).
Send the `client_public_key` and `client_proof` to the server with `wow_srp_client_challenge_client_public_key`
and `wow_srp_client_challenge_client_proof`.

After receiving [CMD_AUTH_LOGON_PROOF_Server](https://gtker.com/wow_messages/docs/cmd_auth_logon_proof_server.html) create a 
`WowSrpClient` through `wow_srp_client_challenge_verify_server_proof`.

Save the session key for header decryption.

# Header Decryption

The general flow for both is:
```text
WowSrpVanillaProofSeed -> WowSrpVanillaHeaderCrypto
```

Create the `WowSrpVanillaProofSeed` using `wow_srp_vanilla_proof_seed_new`.
Get the proof seed with `wow_srp_vanilla_prood_seed` and send it to the opposite party.

## Server

Create a `WowSrpVanillaHeaderCrypto` using `wow_srp_vanilla_proof_seed_into_server_header_crypto`.
Then encrypt with `wow_srp_vanilla_header_crypto_encrypt` and decrypt with `wow_srp_vanilla_header_crypto_decrypt`.

## Client

Create a `WowSrpVanillaHeaderCrypto` using `wow_srp_vanilla_proof_seed_into_client_header_crypto`.
Then encrypt with `wow_srp_vanilla_header_crypto_encrypt` and decrypt with `wow_srp_vanilla_header_crypto_decrypt`.


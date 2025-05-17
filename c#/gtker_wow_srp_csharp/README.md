# WowSrp for .NET (C#)

.NET Standard 2.1 library for World of Warcraft authentication servers.
Includes functionality for:

* Logging in (SRP6)
* Header encryption/decryption for Vanilla, TBC, and Wrath
* Calculation of world proof sent in [`CMSG_AUTH_SESSION`](https://gtker.com/wow_messages/docs/cmsg_auth_session.html)
* Integrity checking
* PIN codes for 1.12+
* Matrix Cards for 2.0+

## Getting started

Install the library from [nuget](https://www.nuget.org/packages/WowSrp):
```bash
dotnet add package WowSrp
```

A full example [can be found in the `wow_messages_csharp` repo](https://github.com/gtker/wow_messages_csharp/tree/main/ServerTest) which also contains a library for the message protocol.

## Usage

### Authentication

#### For Servers

The module is split into functionality used by a server implementation and a client implementation.

### Server

```text
SrpVerifier -> SrpProof -> SrpServer
```

You will only want to save the username, salt, and password verifier for an account.
Do not save the raw passwords on the server.

Construct an `SrpVerifier` through

```csharp
var verifier = new SrpVerifier(username, password);
```

Save the `Username`, `Salt`, and `PasswordVerifier` in your database.

When a client connects, retrieve the `Username`, `Salt`, and `PasswordVerifier` from your database and create
an `SrpVerifier` through the constructor and convert it to an `SrpProof`:

```csharp
var proof = new SrpVerifier(username, passwordVerifier, salt).IntoProof();
```

The `Salt`, `ServerPublicKey`, `Generator`, and `LargeSafePrime` can then be sent to the client:
The internal calculations use the generator and large safe prime from the functions, and these MUST
be the ones sent to the client.

After receiving the `clientPublicKey` and `clientProof`, the proof can be attempted converted to an `SrpServer`.

```csharp
var success = proof.IntoServer(client_public_key, client_proof)
if (!success.HasValue) {
    // Password was incorrect
}
var (server, serverProof) = success!.Value;
```

The client is now logged in and can be sent the realm list.

If the client loses connection it will attempt to reconnect.
This requires a valid `SrpServer` to exist.
In my opinion the reconnect method is insecure since it uses the session key that can easily be deduced
by any third party and it should not be implemented in a production auth server.

```csharp
var success = server.VerifyReconnect(clientData, clientProof);
```

#### For Clients

```text
SrpClientUser -> SrpClientChallenge -> SrpClient | -> SrpClientReconnection
```
The `SrpClientReconnection` is just a data struct that contains reconnection values.

The client does not have to save any values except for the username and password.

```csharp
var challenge = new SrpClientChallenge(username, password, generator, largeSafePrime, serverPublicKey, salt)
```


The client can then verify that the server also has the correct password through the `serverProof`:
This creates an `SrpClient`.

```csharp
var success = challenge.VerifyServerProof(serverProof)
if (!success.HasValue) {
    // Server didn't get the same hash as you did
}
var client = success!.Value;
```


The `SrpClient` can attempt to reconnect using the `serverReconnectData`:

```csharp
var reconnect_data = client.CalculateReconnectValues(serverReconnectData)
```

And then access the reconnect values from `reconnectData`:

```csharp
var challenge_data = reconnectData.ChallengeData;
var client_proof = reconnectData.ClientProof;
```

## Header Encryption

### Server

Instantiate a `NullCrypter` (Vanilla/TBC) or `NullCrypterWrath` (Wrath) for the first message, then use `Vanilla`/`Tbc`/`Wrath` for encrypted headers.

```csharp
var encrypter = new NullCrypter();
encrypter.WriteServerHeader(stream, size, opcode);
```

First, create a random seed from `WorldProof.RandomSeed()` and send it to the client in [SMSG_AUTH_CHALLENGE](https://gtker.com/wow_messages/docs/smsg_auth_challenge.html).

After receiving [CMSG_AUTH_SESSION](https://gtker.com/wow_messages/docs/cmsg_auth_session.html) from the client, instantiate the encrypter and decrypter for your version.

```csharp
var encrypter = new VanillaEncrypter();
encrypter.WriteServerHeader(stream, size, opcode);

var decrypter = new VanillaDecrypter();
var header = decrypter.ReadServerHeader(stream);
```

### Integrity Checking

Use the relevant function for either Windows, Mac, or the general purpose version:

```csharp
var hash = Integrity.GenericCheck(allFiles, checksumSalt, clientPublicKey)
```

### PIN Code

For both server and client, create a `PinCode` object and call `CalculateHash`.
Other arguments are random values sent to both client and server.

```csharp
var pinGridSeed = Pin.RandomPinGridSeed();
var server/clientSalt = Pin.RandomPinSalt();
// Send these to client/server

// Each byte in the array is a digit of the pin
// Must be at least 4 and no longer than 10, values can not be greater than 9
var pin = [1, 2, 3, 4];
var hash = Pin.CalculateHash(pinGridSeed, serverSalt, clientSalt);
```

If the server hash matches the client hash, the client has the correct PIN.

### Matrix Card

The `MatrixCard` class represents the physical card and the data on it.
The `VerifyHash` function takes in all the necessary parameters and should be preferred.

`MatrixCardVerifier` is used for verification whenever the full `MatrixCard` data is not available, such as when a physical card has been issued.

```csharp
// Completely random data
new card = MatrixCard();
// Save to database
// Send to client

var success = card.VerifyHash(challengeCount, seed, sessionKey, clientProof);
```

## Additional documentation

[The WoWDev wiki](https://wowdev.wiki/Login) has an article on the big picture of authentication.
[My blog](https://gtker.com) has articles that detail the implementation of this library.

## License

Licensed under either of

* Apache License, Version 2.0
  http://www.apache.org/licenses/LICENSE-2.0)
* MIT license
  http://opensource.org/licenses/MIT)

at your option.

## Contribution

Unless you explicitly state otherwise, any contribution intentionally submitted
for inclusion in the work by you, as defined in the Apache-2.0 license, shall be
dual licensed as above, without any additional terms or conditions.

using System;
using System.Collections.Generic;
using System.Numerics;
using System.Text;

namespace WowSrp.Internal
{
    internal static class Implementation
    {
        private const byte K = 3;

        internal static readonly byte[] PreCalculatedXorHash =
        {
            0xdd, 0x7b, 0xb0, 0x3a, 0x38, 0xac, 0x73, 0x11, 0x3, 0x98,
            0x7c, 0x5a, 0x50, 0x6f, 0xca, 0x96, 0x6c, 0x7b, 0xc2, 0xa7
        };

        private static readonly byte[] Zero = { 0, 0, 0, 0 };

        private static readonly BigInteger LargeSafePrime = Constants.LargeSafePrimeLittleEndian.ToBigInteger();
        private static readonly BigInteger GeneratorInt = Constants.Generator.ToBigInteger();
        private static readonly BigInteger KInt = K.ToBigInteger();

        public static byte[] CalculatePasswordVerifier(
            string username,
            string password,
            ReadOnlySpan<byte> salt)
        {
            Utils.AssertArrayLength(salt, Constants.KeyLength, nameof(salt));

            var x = CalculateX(username, password, salt);
            var xBigInt = x.ToBigInteger();

            var result = GeneratorInt.ModPow(xBigInt, LargeSafePrime);

            return result.ToPaddedArray(Constants.KeyLength);
        }

        public static byte[] CalculateX(string username, string password, ReadOnlySpan<byte> salt)
        {
            Utils.AssertArrayLength(salt, Constants.KeyLength, nameof(salt));
            username = Utils.ConvertUsernameOrPassword(username, nameof(username));
            password = Utils.ConvertUsernameOrPassword(password, nameof(password));

            var interim = username + ":" + password;
            var interimHashed = Utils.Sha1(Encoding.UTF8.GetBytes(interim));

            var data = Utils.ConcatArrays(salt, interimHashed);

            return Utils.Sha1(data);
        }

        public static byte[] CalculateServerPublicKey(ReadOnlySpan<byte> passwordVerifier,
            ReadOnlySpan<byte> serverPrivateKey)
        {
            Utils.AssertArrayLength(passwordVerifier, Constants.KeyLength, nameof(passwordVerifier));
            Utils.AssertArrayLength(serverPrivateKey, Constants.KeyLength, nameof(serverPrivateKey));

            var serverPrivateKeyInt = serverPrivateKey.ToBigInteger();

            var interim = (KInt * passwordVerifier.ToBigInteger() +
                           GeneratorInt.ModPow(serverPrivateKeyInt, LargeSafePrime)) % LargeSafePrime;

            return interim.ToPaddedArray(Constants.KeyLength);
        }

        public static byte[] CalculateUValue(ReadOnlySpan<byte> clientPublicKey,
            ReadOnlySpan<byte> serverPublicKey)
        {
            Utils.AssertArrayLength(clientPublicKey, Constants.KeyLength, nameof(clientPublicKey));
            Utils.AssertArrayLength(serverPublicKey, Constants.KeyLength, nameof(serverPublicKey));

            var data = Utils.ConcatArrays(clientPublicKey, serverPublicKey);
            return Utils.Sha1(data);
        }

        public static byte[] CalculateServerSKey(ReadOnlySpan<byte> clientPublicKey,
            ReadOnlySpan<byte> passwordVerifier, ReadOnlySpan<byte> u,
            ReadOnlySpan<byte> serverPrivateKey)
        {
            Utils.AssertArrayLength(clientPublicKey, Constants.KeyLength, nameof(clientPublicKey));
            Utils.AssertArrayLength(passwordVerifier, Constants.KeyLength, nameof(passwordVerifier));
            Utils.AssertArrayLength(u, Constants.ProofLength, nameof(u));
            Utils.AssertArrayLength(serverPrivateKey, Constants.KeyLength, nameof(serverPrivateKey));

            var clientPublicKeyInt = clientPublicKey.ToBigInteger();
            var passwordVerifierInt = passwordVerifier.ToBigInteger();
            var uInt = u.ToBigInteger();
            var serverPrivateKeyInt = serverPrivateKey.ToBigInteger();

            var interim =
                BigInteger.ModPow(clientPublicKeyInt * BigInteger.ModPow(passwordVerifierInt, uInt, LargeSafePrime),
                    serverPrivateKeyInt, LargeSafePrime);

            return interim.ToPaddedArray(Constants.KeyLength);
        }

        public static byte[] CalculateClientSKey(ReadOnlySpan<byte> clientPrivateKey,
            ReadOnlySpan<byte> serverPublicKey, ReadOnlySpan<byte> x, ReadOnlySpan<byte> u,
            ReadOnlySpan<byte> largeSafePrime, byte generator)
        {
            Utils.AssertArrayLength(clientPrivateKey, Constants.KeyLength, nameof(clientPrivateKey));
            Utils.AssertArrayLength(serverPublicKey, Constants.KeyLength, nameof(serverPublicKey));
            Utils.AssertArrayLength(u, Constants.ProofLength, nameof(u));
            Utils.AssertArrayLength(x, Constants.ProofLength, nameof(x));
            Utils.AssertArrayLength(largeSafePrime, Constants.KeyLength, nameof(largeSafePrime));

            var clientPrivateKeyInt = clientPrivateKey.ToBigInteger();
            var serverPublicKeyInt = serverPublicKey.ToBigInteger();
            var xInt = x.ToBigInteger();
            var uInt = u.ToBigInteger();
            var largeSafePrimeInt = largeSafePrime.ToBigInteger();
            var generatorInt = generator.ToBigInteger();

            var s = (serverPublicKeyInt - KInt * generatorInt.ModPow(xInt, largeSafePrimeInt)).ModPow(
                clientPrivateKeyInt + uInt * xInt,
                largeSafePrimeInt);

            return s.ToPaddedArray(Constants.KeyLength);
        }

        public static byte[] SplitSKey(ReadOnlySpan<byte> sKey)
        {
            Utils.AssertArrayLength(sKey, Constants.KeyLength, nameof(sKey));

            var offset = 0;

            while (sKey[offset] == 0)
            {
                offset += 2;
            }

            var newArray = new byte[sKey.Length - offset];
            for (var i = 0; i < newArray.Length; i++)
            {
                newArray[i] = sKey[i + offset];
            }

            return newArray;
        }

        public static byte[] ShaInterleave(ReadOnlySpan<byte> sKey)
        {
            Utils.AssertArrayLength(sKey, Constants.KeyLength, nameof(sKey));

            var splitSKey = SplitSKey(sKey);

            var e = new List<byte>();
            var f = new List<byte>();
            for (var i = 0; i < splitSKey.Length; i++)
            {
                if (i % 2 == 0)
                {
                    e.Add(splitSKey[i]);
                }
                else
                {
                    f.Add(splitSKey[i]);
                }
            }

            var g = Utils.Sha1(e.ToArray());
            var h = Utils.Sha1(f.ToArray());

            var sessionKey = new byte[Constants.SessionKeyLength];
            const int halfSessionKeyLength = 20;
            for (var i = 0; i < halfSessionKeyLength; i++)
            {
                sessionKey[i * 2] = g[i];
                sessionKey[i * 2 + 1] = h[i];
            }

            return sessionKey;
        }

        public static byte[] CalculateServerProof(ReadOnlySpan<byte> clientPublicKey,
            ReadOnlySpan<byte> clientProof, ReadOnlySpan<byte> sessionKey)
        {
            Utils.AssertArrayLength(clientPublicKey, Constants.KeyLength, nameof(clientPublicKey));
            Utils.AssertArrayLength(clientProof, Constants.ProofLength, nameof(clientProof));
            Utils.AssertArrayLength(sessionKey, Constants.SessionKeyLength, nameof(sessionKey));

            var full = Utils.ConcatArrays(clientPublicKey, clientProof, sessionKey);

            return Utils.Sha1(full);
        }

        public static byte[] CalculateClientProof(string username, ReadOnlySpan<byte> sessionKey,
            ReadOnlySpan<byte> clientPublicKey, ReadOnlySpan<byte> serverPublicKey, ReadOnlySpan<byte> salt,
            byte generator,
            byte[] largeSafePrime)
        {
            Utils.AssertArrayLength(clientPublicKey, Constants.KeyLength, nameof(clientPublicKey));
            Utils.AssertArrayLength(serverPublicKey, Constants.KeyLength, nameof(serverPublicKey));
            Utils.AssertArrayLength(salt, Constants.KeyLength, nameof(salt));
            Utils.AssertArrayLength(largeSafePrime, Constants.KeyLength, nameof(largeSafePrime));

            var xorHash = CalculateXorHash(generator, largeSafePrime);

            var hashedUsername = Utils.Sha1(Encoding.UTF8.GetBytes(username));

            var full = Utils.ConcatArrays(xorHash, hashedUsername, salt, clientPublicKey, serverPublicKey, sessionKey);

            return Utils.Sha1(full);
        }

        public static byte[] CalculateXorHash(byte generator, byte[] largeSafePrime)
        {
            Utils.AssertArrayLength(largeSafePrime, Constants.KeyLength, nameof(largeSafePrime));

            var hashedGenerator = Utils.Sha1(new[] { generator });
            var hashedLargeSafePrime = Utils.Sha1(largeSafePrime);

            for (var i = 0; i < Constants.ProofLength; i++)
            {
                // Reuse the hashedGenerator array in order to prevent alloc of
                // another array, and annoying casts where (byte) ^ (byte) = (int)
                hashedGenerator[i] ^= hashedLargeSafePrime[i];
            }

            return hashedGenerator;
        }

        public static byte[] CalculateClientPublicKey(ReadOnlySpan<byte> clientPrivateKey, byte generator,
            ReadOnlySpan<byte> largeSafePrime)
        {
            Utils.AssertArrayLength(clientPrivateKey, Constants.KeyLength, nameof(clientPrivateKey));
            Utils.AssertArrayLength(largeSafePrime, Constants.KeyLength, nameof(largeSafePrime));

            var generatorInt = generator.ToBigInteger();
            var clientPrivateKeyInt = clientPrivateKey.ToBigInteger();
            var largeSafePrimeInt = largeSafePrime.ToBigInteger();

            return generatorInt.ModPow(clientPrivateKeyInt, largeSafePrimeInt).ToPaddedArray(Constants.KeyLength);
        }

        public static byte[] CalculateReconnectProof(string username, ReadOnlySpan<byte> clientData,
            ReadOnlySpan<byte> serverData,
            ReadOnlySpan<byte> sessionKey)
        {
            Utils.AssertArrayLength(clientData, Constants.ReconnectDataLength, nameof(clientData));
            Utils.AssertArrayLength(serverData, Constants.ReconnectDataLength, nameof(serverData));
            Utils.AssertArrayLength(sessionKey, Constants.SessionKeyLength, nameof(sessionKey));

            var usernameArray = Encoding.UTF8.GetBytes(username);

            var concatArray = Utils.ConcatArrays(usernameArray, clientData, serverData, sessionKey);
            return Utils.Sha1(concatArray);
        }

        public static byte[] CalculateWorldServerProof(string username, uint clientSeed, uint serverSeed,
            ReadOnlySpan<byte> sessionKey)
        {
            var usernameArray = Encoding.UTF8.GetBytes(Utils.ConvertUsernameOrPassword(username, nameof(username)));
            var clientSeedArray = clientSeed.ToPaddedArray(4);
            var serverSeedArray = serverSeed.ToPaddedArray(4);

            var full = Utils.ConcatArrays(usernameArray, Zero, clientSeedArray, serverSeedArray, sessionKey);

            return Utils.Sha1(full);
        }

        public static byte[] CalculateServerSessionKey(ReadOnlySpan<byte> clientPublicKey,
            ReadOnlySpan<byte> serverPublicKey,
            ReadOnlySpan<byte> passwordVerifier,
            ReadOnlySpan<byte> serverPrivateKey)
        {
            Utils.AssertArrayLength(clientPublicKey, Constants.KeyLength, nameof(clientPublicKey));
            Utils.AssertArrayLength(serverPublicKey, Constants.KeyLength, nameof(serverPublicKey));
            Utils.AssertArrayLength(serverPrivateKey, Constants.KeyLength, nameof(serverPrivateKey));
            Utils.AssertArrayLength(passwordVerifier, Constants.KeyLength, nameof(passwordVerifier));

            var u = CalculateUValue(clientPublicKey, serverPublicKey);
            var sKey = CalculateServerSKey(clientPublicKey, passwordVerifier, u, serverPrivateKey);

            return ShaInterleave(sKey);
        }

        public static byte[] CalculateClientSessionKey(string username, string password,
            ReadOnlySpan<byte> serverPublicKey, ReadOnlySpan<byte> clientPrivateKey, byte generator,
            ReadOnlySpan<byte> largeSafePrime, ReadOnlySpan<byte> clientPublicKey,
            ReadOnlySpan<byte> salt)
        {
            Utils.AssertArrayLength(serverPublicKey, Constants.KeyLength, nameof(serverPublicKey));
            Utils.AssertArrayLength(clientPrivateKey, Constants.KeyLength, nameof(clientPrivateKey));
            Utils.AssertArrayLength(largeSafePrime, Constants.KeyLength, nameof(largeSafePrime));
            Utils.AssertArrayLength(clientPublicKey, Constants.KeyLength, nameof(clientPublicKey));
            Utils.AssertArrayLength(salt, Constants.KeyLength, nameof(salt));

            var x = CalculateX(username, password, salt);

            var u = CalculateUValue(clientPublicKey, serverPublicKey);
            var sKey = CalculateClientSKey(clientPrivateKey, serverPublicKey, x, u, largeSafePrime, generator);
            return ShaInterleave(sKey);
        }
    }
}
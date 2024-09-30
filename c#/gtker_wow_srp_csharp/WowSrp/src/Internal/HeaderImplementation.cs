using System;
using System.Security.Cryptography;

namespace WowSrp.Internal
{
    internal static class HeaderImplementation
    {
        // Variant is drop1024
        public const int DropAmount = 1024;

        // Used for Client (Encryption) to Server (Decryption)
        public static readonly byte[] S =
        {
            0xC2, 0xB3, 0x72, 0x3C, 0xC6, 0xAE, 0xD9, 0xB5, 0x34, 0x3C, 0x53, 0xEE, 0x2F, 0x43, 0x67, 0xCE
        };

        // Used for Server (Encryption) to Client (Decryption) messages
        public static readonly byte[] R =
        {
            0xCC, 0x98, 0xAE, 0x04, 0xE8, 0x97, 0xEA, 0xCA, 0x12, 0xDD, 0xC0, 0x93, 0x42, 0x91, 0x53, 0x57
        };

        private static readonly byte[] SKey =
        {
            0x38, 0xA7, 0x83, 0x15, 0xF8, 0x92, 0x25, 0x30, 0x71, 0x98, 0x67, 0xB1, 0x8C, 0x4, 0xE2,
            0xAA
        };

        public static byte[] CreateTbcKey(byte[] sessionKey) => CreateTbcWrathKey(sessionKey, SKey);

        public static byte[] CreateTbcWrathKey(byte[] sessionKey, byte[] key) =>
            new HMACSHA1(key).ComputeHash(sessionKey);

        public static void VanillaTbcDecrypt(Span<byte> data, ReadOnlySpan<byte> key, ref byte lastValue, ref int index)
        {
            for (var i = 0; i < data.Length; i++)
            {
                var encryptedValue = data[i];

                // Work around C# requiring casts for non assignment operators
                // This is just
                // unencrypted = (encrypted - last_value) ^ session_key[index]

                // Implementation requires underflow
                unchecked
                {
                    data[i] -= lastValue;
                }

                data[i] ^= key[index];

                lastValue = encryptedValue;

                index = (index + 1) % key.Length;
            }
        }

        public static void VanillaTbcEncrypt(Span<byte> data, ReadOnlySpan<byte> key, ref byte lastValue, ref int index)
        {
            for (var i = 0; i < data.Length; i++)
            {
                // Work around C# requiring casts for non assignment operators
                // This is just
                // encrypted = (unencrypted ^ session_key[index]) + last_value
                data[i] ^= key[index];

                // Implementation requires overflow
                unchecked
                {
                    data[i] += lastValue;
                }

                lastValue = data[i];

                index = (index + 1) % key.Length;
            }
        }
    }
}
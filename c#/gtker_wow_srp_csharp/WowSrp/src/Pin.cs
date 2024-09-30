using System;
using WowSrp.Internal;

namespace WowSrp
{
    /// <summary>
    ///     Hash sent by the client in
    ///     <a href="https://gtker.com/wow_messages/docs/cmd_auth_logon_proof_client.html">CMD_AUTH_LOGON_PROOF_Client</a>
    ///     .
    ///     The server must ask the client to send the PIN in
    ///     <a href="https://gtker.com/wow_messages/docs/cmd_auth_logon_challenge_server.html">CMD_AUTH_LOGON_CHALLENGE_Server</a>
    ///     and then verify it along with the <seealso cref="WowSrp.MatrixCard" />, and the general SRP6 implementation.
    ///     The server must calculate their own hash and compare it with what the client sent in order to validate that the
    ///     client entered the correct PIN.
    /// </summary>
    public static class Pin
    {
        /// <summary>
        ///     Minimum amount of PIN digits the game can handle.
        /// </summary>
        public const int MinimumLength = 4;

        /// <summary>
        ///     Maximum amount of PIN digits the game can handle.
        /// </summary>
        public const int MaximumLength = 10;

        /// <summary>
        ///     Highest digit allowed in <see cref="Pin" />.
        /// </summary>
        public const byte MaximumDigit = 9;

        /// <summary>
        ///     Lowest digit allowed in <see cref="Pin" />.
        /// </summary>
        public const byte MinimumDigit = 0;

        /// <summary>
        ///     Amount of bytes returned by <see cref="RandomPinSalt" /> and required by the salts for <see cref="CalculateHash" />
        ///     .
        /// </summary>
        public const int PinSaltSize = 16;

        /// <summary>
        ///     Convenience function to return a random value.
        /// </summary>
        public static uint RandomPinGridSeed() => Utils.RandomUint();

        /// <summary>
        ///     Convenience function to return random salt.
        /// </summary>
        public static byte[] RandomPinSalt() => Utils.RandomizedArray(PinSaltSize);

        /// <summary>
        ///     Hash sent by the client in
        ///     <a href="https://gtker.com/wow_messages/docs/cmd_auth_logon_proof_client.html">CMD_AUTH_LOGON_PROOF_Client</a>
        ///     .
        ///     The server must calculate their own hash and compare it with what the client sent in order to validate that the
        ///     client entered the correct PIN.
        /// </summary>
        /// <param name="pinGridSeed">
        ///     Random value sent by the server in
        ///     <a href="https://gtker.com/wow_messages/docs/cmd_auth_logon_challenge_server.html">CMD_AUTH_LOGON_CHALLENGE_Server</a>
        /// </param>
        /// <param name="serverSalt">
        ///     Raondom value sent by the server in
        ///     <a href="https://gtker.com/wow_messages/docs/cmd_auth_logon_challenge_server.html">CMD_AUTH_LOGON_CHALLENGE_Server</a>
        /// </param>
        /// <param name="clientSalt">
        ///     Random value sent by the client in
        ///     <a href="https://gtker.com/wow_messages/docs/cmd_auth_logon_proof_client.html">CMD_AUTH_LOGON_PROOF_Client</a>
        /// </param>
        /// <param name="pin">Represents a PIN of key presses the client performs. Every digit is a single <see cref="byte" />.</param>
        /// <exception cref="ArgumentException">
        ///     If the PIN is shorter than <see cref="MinimumLength" />, longer than
        ///     <see cref="MaximumLength" />, or any of the values are greater than <see cref="MaximumDigit" />.
        /// </exception>
        /// <returns>20 byte SHA1 hash.</returns>
        public static byte[] CalculateHash(byte[] pin, long pinGridSeed, ReadOnlySpan<byte> serverSalt,
            ReadOnlySpan<byte> clientSalt)
        {
            if (pin.Length < MinimumLength || pin.Length > MaximumLength)
            {
                throw new ArgumentException(
                    $"length of pin must be between [{MinimumLength};{MaximumLength}], is {pin.Length}", nameof(pin));
            }

            Utils.AssertArrayLength(serverSalt, PinSaltSize, nameof(serverSalt));
            Utils.AssertArrayLength(clientSalt, PinSaltSize, nameof(clientSalt));

            return PinImplementation.CalculateHash(pin, pinGridSeed, serverSalt, clientSalt);
        }
    }
}
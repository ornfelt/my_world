using System;
using WowSrp.Internal;

namespace WowSrp.Client
{
    /// <summary>
    ///     Final object of the client path.
    ///     Created from <see cref="SrpClientChallenge.VerifyServerProof" />.
    /// </summary>
    public readonly struct SrpClient
    {
        internal SrpClient(string username, byte[] sessionKey)
        {
            Username = username;
            SessionKey = sessionKey;
        }

        /// <summary>
        ///     Calculates the reconnect proof.
        /// </summary>
        public SrpClientReconnection CalculateReconnectValues(ReadOnlySpan<byte> serverChallengeData)
        {
            var clientChallengeData = Utils.RandomizedArray(Constants.ReconnectDataLength);

            var clientProof =
                Implementation.CalculateReconnectProof(Username, clientChallengeData, serverChallengeData, SessionKey);

            return new SrpClientReconnection(clientChallengeData, clientProof);
        }

        /// <summary>
        ///     Username of the client.
        /// </summary>
        public string Username { get; }

        /// <summary>
        ///     Generated session key.
        ///     Required for header encryption/decryption via <see cref="Header.IEncrypter" /> and <see cref="Header.IDecrypter" />
        ///     .
        /// </summary>
        public byte[] SessionKey { get; }
    }
}
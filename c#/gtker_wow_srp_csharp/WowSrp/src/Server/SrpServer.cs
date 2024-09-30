using System;
using System.Linq;
using WowSrp.Internal;

namespace WowSrp.Server
{
    /// <summary>
    ///     Third step of server authentication.
    ///     Created by <see cref="SrpProof.IntoServer" />.
    /// </summary>
    public readonly struct SrpServer
    {
        internal SrpServer(string username, byte[] sessionKey)
        {
            Utils.AssertArrayLength(sessionKey, Constants.SessionKeyLength, nameof(sessionKey));

            Username = username;
            SessionKey = sessionKey;
            ReconnectChallengeData = Utils.RandomizedArray(Constants.ReconnectDataLength);
        }

        /// <summary>
        ///     Verifies that the client knows the session key.
        /// </summary>
        public bool VerifyReconnectionAttempt(ReadOnlySpan<byte> clientData, ReadOnlySpan<byte> clientProof)
        {
            Utils.AssertArrayLength(clientData, Constants.ReconnectDataLength, nameof(clientData));
            Utils.AssertArrayLength(clientProof, Constants.ProofLength, nameof(clientProof));

            var serverProof =
                Implementation.CalculateReconnectProof(Username, clientData, ReconnectChallengeData, SessionKey);

            Utils.RandomBytes(ReconnectChallengeData);

            return serverProof.SequenceEqual(clientProof.ToArray());
        }

        /// <summary>
        ///     Username of the user.
        /// </summary>
        public string Username { get; }

        /// <summary>
        ///     Generated session key.
        ///     Required for header encryption/decryption via <see cref="Header.IEncrypter" /> and <see cref="Header.IDecrypter" />
        ///     .
        /// </summary>
        public byte[] SessionKey { get; }

        /// <summary>
        ///     Challenge data for reconnection.
        ///     Must be sent to the client.
        /// </summary>
        public byte[] ReconnectChallengeData { get; }
    }
}
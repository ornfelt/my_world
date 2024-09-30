using System;
using WowSrp.Internal;

namespace WowSrp.Server
{
    /// <summary>
    ///     Second step of server authentication.
    ///     Created by <see cref="SrpVerifier.IntoProof" />.
    /// </summary>
    public readonly struct SrpProof
    {
        internal SrpProof(string username, byte[] passwordVerifier, byte[] salt)
        {
            Username = username;
            PasswordVerifier = passwordVerifier;
            Salt = salt;

            ServerPrivateKey = Utils.RandomizedArray(Constants.KeyLength);

            ServerPublicKey = Implementation.CalculateServerPublicKey(PasswordVerifier, ServerPrivateKey);
        }

        /// <summary>
        ///     Verifies that the client knows the correct password.
        ///     Will return <code>null</code> if the proofs do not match.
        /// </summary>
        public (SrpServer, byte[])? IntoServer(ReadOnlySpan<byte> clientPublicKey, ReadOnlySpan<byte> clientProof)
        {
            var sessionKey =
                Implementation.CalculateServerSessionKey(clientPublicKey, ServerPublicKey, PasswordVerifier,
                    ServerPrivateKey);

            var clientProofCalculated = Implementation.CalculateClientProof(Username, sessionKey,
                clientPublicKey, ServerPublicKey, Salt, Constants.Generator,
                Constants.LargeSafePrimeLittleEndian);

            if (!clientProof.SequenceEqual(clientProofCalculated))
            {
                return null;
            }

            var serverProof = Implementation.CalculateServerProof(clientPublicKey, clientProof, sessionKey);

            return (new SrpServer(Username, sessionKey), serverProof);
        }

        private string Username { get; }

        /// <summary>
        ///     Salt of the user.
        ///     Same as <see cref="SrpVerifier.Salt" />.
        /// </summary>
        public byte[] Salt { get; }

        /// <summary>
        ///     Public key of the server.
        ///     Must be sent to the client.
        /// </summary>
        public byte[] ServerPublicKey { get; }

        private byte[] PasswordVerifier { get; }
        private byte[] ServerPrivateKey { get; }
    }
}
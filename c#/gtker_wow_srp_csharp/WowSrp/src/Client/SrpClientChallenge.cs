using System;
using WowSrp.Internal;

namespace WowSrp.Client
{
    /// <summary>
    ///     First step of client authentication.
    /// </summary>
    public readonly struct SrpClientChallenge
    {
        /// <summary>
        ///     First step of client authentication.
        /// </summary>
        public SrpClientChallenge(string username, string password, byte generator, byte[] largeSafePrime,
            ReadOnlySpan<byte> serverPublicKey,
            ReadOnlySpan<byte> salt)
        {
            Utils.AssertArrayLength(largeSafePrime, Constants.KeyLength, nameof(largeSafePrime));
            Utils.AssertArrayLength(serverPublicKey, Constants.KeyLength, nameof(serverPublicKey));
            Utils.AssertArrayLength(salt, Constants.KeyLength, nameof(salt));

            Username = Utils.ConvertUsernameOrPassword(username, nameof(username));
            password = Utils.ConvertUsernameOrPassword(password, nameof(password));

            var clientPrivateKey = Utils.RandomizedArray(Constants.KeyLength);

            ClientPublicKey = Implementation.CalculateClientPublicKey(clientPrivateKey, generator, largeSafePrime);

            SessionKey = Implementation.CalculateClientSessionKey(username, password, serverPublicKey, clientPrivateKey,
                generator, largeSafePrime, ClientPublicKey, salt);

            ClientProof = Implementation.CalculateClientProof(Username, SessionKey, ClientPublicKey, serverPublicKey,
                salt,
                generator, largeSafePrime);
        }

        /// <summary>
        ///     Username of the client.
        /// </summary>
        public string Username { get; }

        /// <summary>
        ///     Generated client proof. Must be sent to the server.
        /// </summary>
        public byte[] ClientProof { get; }

        /// <summary>
        ///     Client public key. Must be sent to the server.
        /// </summary>
        public byte[] ClientPublicKey { get; }


        /// <summary>
        ///     Verifies that the server has the same password as the client.
        /// </summary>
        public SrpClient? VerifyServerProof(ReadOnlySpan<byte> serverProof)
        {
            var calculatedServerProof = Implementation.CalculateServerProof(ClientPublicKey, ClientProof, SessionKey);

            if (!serverProof.SequenceEqual(calculatedServerProof))
            {
                return null;
            }

            return new SrpClient(Username, SessionKey);
        }

        private byte[] SessionKey { get; }
    }
}
using WowSrp.Internal;

namespace WowSrp
{
    /// <summary>
    ///     Proof sent by the client in
    ///     <a href="https://gtker.com/wow_messages/docs/cmsg_auth_session.html">CMSG_AUTH_SESSION</a>.
    ///     Must also be calculated by the server in order to verify that the client has authenticated with the login server.
    /// </summary>
    public static class WorldProof
    {
        /// <summary>
        ///     Convenience function for generating a random client or server seed.
        /// </summary>
        public static uint RandomSeed() => Utils.RandomUint();

        /// <summary>
        ///     Proof sent by the client in
        ///     <a href="https://gtker.com/wow_messages/docs/cmsg_auth_session.html">CMSG_AUTH_SESSION</a>.
        ///     Must also be calculated by the server in order to verify that the client has authenticated with the login server.
        /// </summary>
        public static byte[] CalculateProof(string username, uint clientSeed, uint serverSeed, byte[] sessionKey) =>
            Implementation.CalculateWorldServerProof(username, clientSeed, serverSeed, sessionKey);
    }
}
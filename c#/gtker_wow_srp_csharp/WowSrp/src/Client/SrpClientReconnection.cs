namespace WowSrp.Client
{
    /// <summary>
    ///     Data returned by <see cref="SrpClient.CalculateReconnectValues" />.
    /// </summary>
    public readonly struct SrpClientReconnection
    {
        /// <summary>
        ///     Challenge data that must be sent to the server.
        /// </summary>
        public byte[] ChallengeData { get; }

        /// <summary>
        ///     Client proof of reconnection.
        /// </summary>
        public byte[] ClientProof { get; }

        internal SrpClientReconnection(byte[] challengeData, byte[] clientProof)
        {
            ChallengeData = challengeData;
            ClientProof = clientProof;
        }
    }
}
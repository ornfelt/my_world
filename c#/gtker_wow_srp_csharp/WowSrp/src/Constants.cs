namespace WowSrp
{
    /// <summary>
    ///     Constants used by the library.
    /// </summary>
    public static class Constants
    {
        /// <summary>
        ///     Array length used for anything that isn't either the <see cref="SessionKeyLength">session key</see>,
        ///     <see cref="ReconnectDataLength">reconnect data</see>, or a <see cref="ProofLength" />.
        /// </summary>
        public const int KeyLength = 32;

        /// <summary>
        ///     Array length used for reconnect seed data.
        /// </summary>
        public const int ReconnectDataLength = 16;

        /// <summary>
        ///     Constant generator value. Must be sent to the server.
        /// </summary>
        public const byte Generator = 7;

        /// <summary>
        ///     Array length of the session key.
        /// </summary>
        public const int SessionKeyLength = 40;

        /// <summary>
        ///     Array length of proofs.
        /// </summary>
        public const int ProofLength = 20;

        internal const int ServerOpcodeLength = 2;
        internal const int ServerNormalSizeLength = 2;
        internal const int ServerNormalHeaderLength = ServerOpcodeLength + ServerNormalSizeLength;

        internal const int ServerWrathLargeSizeLength = 3;
        internal const int ServerWrathHeaderLength = ServerOpcodeLength + ServerWrathLargeSizeLength;

        internal const int ClientOpcodeLength = 4;
        internal const int ClientSizeLength = 2;

        internal const int ClientHeaderLength = 6;

        /// <summary>
        ///     Little endian array of the large safe prime used.
        ///     Must be sent to the client.
        /// </summary>
        public static readonly byte[] LargeSafePrimeLittleEndian =
        {
            0xb7, 0x9b, 0x3e, 0x2a, 0x87, 0x82, 0x3c, 0xab,
            0x8f, 0x5e, 0xbf, 0xbf, 0x8e, 0xb1, 0x01, 0x08,
            0x53, 0x50, 0x06, 0x29, 0x8b, 0x5b, 0xad, 0xbd,
            0x5b, 0x53, 0xe1, 0x89, 0x5e, 0x64, 0x4b, 0x89
        };
    }
}
using System;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using WowSrp.Internal;

namespace WowSrp.Header
{
    /// <summary>
    ///     Decrypter for Vanilla (1.0 through to 1.12).
    /// </summary>
    public class VanillaDecryption : IClientDecrypter, IServerDecrypter
    {
        private readonly byte[] _sessionKey;
        private int _index;
        private byte _lastValue;

        /// <summary>
        ///     Constructor.
        /// </summary>
        public VanillaDecryption(byte[] sessionKey)
        {
            Utils.AssertArrayLength(sessionKey, Constants.SessionKeyLength, nameof(sessionKey));
            _sessionKey = sessionKey;
        }

        /// <inheritdoc cref="IDecrypter.Decrypt" />
        public void Decrypt(Span<byte> data)
        {
            HeaderImplementation.VanillaTbcDecrypt(data, _sessionKey, ref _lastValue, ref _index);
        }

        /// <inheritdoc cref="IClientDecrypter.ReadClientHeader(Span&lt;byte&gt;)" />
        public HeaderData ReadClientHeader(Span<byte> span) => HeaderImplementations.ReadClientHeader(span, Decrypt);

        /// <inheritdoc cref="IClientDecrypter.ReadClientHeader(Stream)" />
        public HeaderData ReadClientHeader(Stream stream) => HeaderImplementations.ReadClientHeader(stream, Decrypt);

        /// <inheritdoc cref="IClientDecrypter.ReadClientHeaderAsync(Stream, CancellationToken)" />
        public Task<HeaderData> ReadClientHeaderAsync(Stream stream, CancellationToken cancellationToken = default) =>
            HeaderImplementations.ReadClientHeaderAsync(stream, Decrypt, cancellationToken);

        bool IServerDecrypter.IsWrath() => false;

        /// <inheritdoc cref="IServerDecrypter.ReadServerHeader(Span&lt;byte&gt;)" />
        public HeaderData ReadServerHeader(Span<byte> span) =>
            HeaderImplementations.ReadServerHeader(span, ((IServerDecrypter)this).IsWrath(), Decrypt);

        /// <inheritdoc cref="IServerDecrypter.ReadServerHeader(Stream)" />
        public HeaderData ReadServerHeader(Stream stream) => HeaderImplementations.ReadServerHeader(stream,
            ((IServerDecrypter)
                this).IsWrath(), Decrypt);

        /// <inheritdoc cref="IServerDecrypter.ReadServerHeaderAsync(Stream, CancellationToken)" />
        public async Task<HeaderData> ReadServerHeaderAsync(Stream stream,
            CancellationToken cancellationToken = default) =>
            await HeaderImplementations
                .ReadServerHeaderAsync(stream, ((IServerDecrypter)this).IsWrath(), Decrypt, cancellationToken)
                .ConfigureAwait(false);
    }
}
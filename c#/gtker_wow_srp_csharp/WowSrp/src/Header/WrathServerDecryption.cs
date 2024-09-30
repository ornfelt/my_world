using System;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using WowSrp.Internal;

namespace WowSrp.Header
{
    /// <summary>
    ///     Decrypter of wrath server headers.
    /// </summary>
    public class WrathServerDecryption : IServerDecrypter
    {
        private readonly Arc4 _arc4;

        /// <summary>
        ///     Constructor.
        /// </summary>
        public WrathServerDecryption(byte[] sessionKey)
        {
            _arc4 = new Arc4(HeaderImplementation.CreateTbcWrathKey(sessionKey, HeaderImplementation.R),
                HeaderImplementation.DropAmount);
        }

        /// <inheritdoc cref="IDecrypter.Decrypt" />
        public void Decrypt(Span<byte> data) => _arc4.ApplyKeyStream(data);

        bool IServerDecrypter.IsWrath() => true;


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
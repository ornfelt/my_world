using System;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using WowSrp.Internal;

namespace WowSrp.Header
{
    /// <summary>
    ///     Decrypter of Wrath client messages.
    /// </summary>
    public class WrathClientDecryption : IClientDecrypter
    {
        private readonly Arc4 _arc4;

        /// <summary>
        ///     Constructor.
        /// </summary>
        public WrathClientDecryption(byte[] sessionKey)
        {
            _arc4 = new Arc4(HeaderImplementation.CreateTbcWrathKey(sessionKey, HeaderImplementation.S),
                HeaderImplementation.DropAmount);
        }

        /// <inheritdoc cref="IDecrypter.Decrypt" />
        public void Decrypt(Span<byte> data) => _arc4.ApplyKeyStream(data);

        /// <inheritdoc cref="IClientDecrypter.ReadClientHeader(Span&lt;byte&gt;)" />
        public HeaderData ReadClientHeader(Span<byte> span) => HeaderImplementations.ReadClientHeader(span, Decrypt);

        /// <inheritdoc cref="IClientDecrypter.ReadClientHeader(Stream)" />
        public HeaderData ReadClientHeader(Stream stream) => HeaderImplementations.ReadClientHeader(stream, Decrypt);

        /// <inheritdoc cref="IClientDecrypter.ReadClientHeaderAsync(Stream, CancellationToken)" />
        public Task<HeaderData> ReadClientHeaderAsync(Stream stream, CancellationToken cancellationToken = default) =>
            HeaderImplementations.ReadClientHeaderAsync(stream, Decrypt, cancellationToken);
    }
}
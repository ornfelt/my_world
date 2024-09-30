using System;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using WowSrp.Internal;

namespace WowSrp.Header
{
    /// <summary>
    ///     Encrypter of client headers.
    /// </summary>
    public class WrathClientEncryption : IClientEncrypter
    {
        private readonly Arc4 _arc4;

        /// <summary>
        ///     Constructor.
        /// </summary>
        public WrathClientEncryption(byte[] sessionKey)
        {
            _arc4 = new Arc4(HeaderImplementation.CreateTbcWrathKey(sessionKey, HeaderImplementation.S),
                HeaderImplementation.DropAmount);
        }

        /// <inheritdoc cref="IEncrypter.Encrypt" />
        public void Encrypt(Span<byte> data) => _arc4.ApplyKeyStream(data);

        /// <inheritdoc cref="IClientEncrypter.CreateClientHeader" />
        public byte[] CreateClientHeader(uint size, uint opcode) =>
            HeaderImplementations.CreateClientHeader(size, opcode, Encrypt);

        /// <inheritdoc cref="IClientEncrypter.WriteClientHeader(Span&lt;byte&gt;, uint, uint)" />
        public void WriteClientHeader(Span<byte> span, uint size, uint opcode)
        {
            HeaderImplementations.WriteClientHeader(span, size, opcode, Encrypt);
        }

        /// <inheritdoc cref="IClientEncrypter.WriteClientHeader(byte[], uint, uint)" />
        public void WriteClientHeader(byte[] buffer, uint size, uint opcode)
        {
            HeaderImplementations.WriteClientHeader(buffer, size, opcode, Encrypt);
        }

        /// <inheritdoc cref="IClientEncrypter.WriteClientHeader(Stream, uint, uint)" />
        public void WriteClientHeader(Stream stream, uint size, uint opcode)
        {
            HeaderImplementations.WriteClientHeader(stream, size, opcode, Encrypt);
        }

        /// <inheritdoc cref="IClientEncrypter.WriteClientHeaderAsync(Stream, uint, uint, CancellationToken)" />
        public Task WriteClientHeaderAsync(Stream stream, uint size, uint opcode,
            CancellationToken cancellationToken = default) =>
            HeaderImplementations.WriteClientHeaderAsync(stream, size, opcode, Encrypt, cancellationToken);
    }
}
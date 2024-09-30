using System;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using WowSrp.Internal;

namespace WowSrp.Header
{
    /// <summary>
    ///     Encrypter of wrath server headers.
    /// </summary>
    public class WrathServerEncryption : IServerEncrypter
    {
        private readonly Arc4 _arc4;

        /// <summary>
        ///     Constructor.
        /// </summary>
        public WrathServerEncryption(byte[] sessionKey)
        {
            _arc4 = new Arc4(HeaderImplementation.CreateTbcWrathKey(sessionKey, HeaderImplementation.R),
                HeaderImplementation.DropAmount);
        }

        /// <inheritdoc cref="IEncrypter.Encrypt" />
        public void Encrypt(Span<byte> data) => _arc4.ApplyKeyStream(data);

        bool IServerEncrypter.IsWrath() => true;

        /// <inheritdoc cref="IServerEncrypter.CreateServerHeader" />
        public byte[] CreateServerHeader(uint size, uint opcode) =>
            HeaderImplementations.CreateServerHeader(size, opcode, ((IServerEncrypter)this).IsWrath(), Encrypt);

        /// <inheritdoc cref="IServerEncrypter.WriteServerHeader(Span&lt;byte&gt;, uint, uint)" />
        public void WriteServerHeader(Span<byte> span, uint size, uint opcode)
        {
            HeaderImplementations.WriteServerHeader(span, size, opcode, ((IServerEncrypter)this).IsWrath(), Encrypt);
        }

        /// <inheritdoc cref="IServerEncrypter.WriteServerHeader(byte[], uint, uint)" />
        public void WriteServerHeader(byte[] buffer, uint size, uint opcode)
        {
            HeaderImplementations.WriteServerHeader(buffer, size, opcode, ((IServerEncrypter)this).IsWrath(), Encrypt);
        }

        /// <inheritdoc cref="IServerEncrypter.WriteServerHeader(Stream, uint, uint)" />
        public void WriteServerHeader(Stream stream, uint size, uint opcode)
        {
            HeaderImplementations.WriteServerHeader(stream, size, opcode, ((IServerEncrypter)this).IsWrath(), Encrypt);
        }

        /// <inheritdoc cref="IServerEncrypter.WriteServerHeaderAsync(Stream, uint, uint, CancellationToken)" />
        public async Task WriteServerHeaderAsync(Stream stream, uint size, uint opcode,
            CancellationToken cancellationToken = default)
        {
            await HeaderImplementations.WriteServerHeaderAsync(stream, size, opcode, ((IServerEncrypter)this).IsWrath(),
                    Encrypt, cancellationToken)
                .ConfigureAwait(false);
        }
    }
}
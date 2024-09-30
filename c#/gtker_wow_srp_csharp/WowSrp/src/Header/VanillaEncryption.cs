using System;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using WowSrp.Internal;

namespace WowSrp.Header
{
    /// <summary>
    ///     Encrypter for Vanilla (1.0 through to 1.12).
    /// </summary>
    public class VanillaEncryption : IServerEncrypter, IClientEncrypter
    {
        private readonly byte[] _sessionKey;
        private int _index;
        private byte _lastValue;

        /// <summary>
        ///     Constructor.
        /// </summary>
        public VanillaEncryption(byte[] sessionKey)
        {
            Utils.AssertArrayLength(sessionKey, Constants.SessionKeyLength, nameof(sessionKey));
            _sessionKey = sessionKey;
        }

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

        /// <inheritdoc cref="IEncrypter.Encrypt" />
        public void Encrypt(Span<byte> data)
        {
            HeaderImplementation.VanillaTbcEncrypt(data, _sessionKey, ref _lastValue, ref _index);
        }

        bool IServerEncrypter.IsWrath() => false;

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
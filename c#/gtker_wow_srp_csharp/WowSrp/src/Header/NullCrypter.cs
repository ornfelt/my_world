using System;
using System.IO;
using System.Threading;
using System.Threading.Tasks;

namespace WowSrp.Header
{
    /// <summary>
    ///     Encrypter/Decrypter that doesn't do anything for Vanilla/TBC.
    ///     Used for reading the first messages sent by client and server.
    /// </summary>
    public class NullCrypter : IServerDecrypter, IServerEncrypter, IClientDecrypter, IClientEncrypter
    {
        /// <inheritdoc cref="IClientDecrypter.ReadClientHeader(Span&lt;byte&gt;)" />
        public HeaderData ReadClientHeader(Span<byte> span) => HeaderImplementations.ReadClientHeader(span, Decrypt);

        /// <inheritdoc cref="IClientDecrypter.ReadClientHeader(Stream)" />
        public HeaderData ReadClientHeader(Stream stream) => HeaderImplementations.ReadClientHeader(stream, Decrypt);

        /// <inheritdoc cref="IClientDecrypter.ReadClientHeaderAsync(Stream, CancellationToken)" />
        public Task<HeaderData> ReadClientHeaderAsync(Stream stream, CancellationToken cancellationToken = default) =>
            HeaderImplementations.ReadClientHeaderAsync(stream, Decrypt, cancellationToken);

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

        /// <inheritdoc cref="IDecrypter.Decrypt" />
        public void Decrypt(Span<byte> data)
        {
            // Deliberately do nothing
        }

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

        bool IServerEncrypter.IsWrath() => false;

        /// <inheritdoc cref="IEncrypter.Encrypt" />
        public void Encrypt(Span<byte> data)
        {
            // Deliberately do nothing
        }


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
using System;
using System.IO;
using System.Threading;
using System.Threading.Tasks;

namespace WowSrp.Header
{
    /// <summary>
    ///     A decrypter of client headers.
    ///     Would be used by the client.
    /// </summary>
    public interface IClientEncrypter : IEncrypter
    {
        /// <summary>
        ///     Creates a client header.
        /// </summary>
        byte[] CreateClientHeader(uint size, uint opcode);

        /// <summary>
        ///     Writes a client header.
        /// </summary>
        void WriteClientHeader(Span<byte> span, uint size, uint opcode);

        /// <summary>
        ///     Writes a client header.
        /// </summary>
        void WriteClientHeader(byte[] buffer, uint size, uint opcode);

        /// <summary>
        ///     Writes a client header.
        /// </summary>
        void WriteClientHeader(Stream stream, uint size, uint opcode);

        /// <summary>
        ///     Writes a client header.
        /// </summary>
        Task WriteClientHeaderAsync(Stream stream, uint size, uint opcode,
            CancellationToken cancellationToken = default);
    }
}
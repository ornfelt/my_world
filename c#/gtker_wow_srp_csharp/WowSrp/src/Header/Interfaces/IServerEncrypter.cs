using System;
using System.IO;
using System.Threading;
using System.Threading.Tasks;

namespace WowSrp.Header
{
    /// <summary>
    ///     Encrypter of server headers.
    ///     Would be used by servers.
    /// </summary>
    public interface IServerEncrypter : IEncrypter
    {
        internal bool IsWrath();

        /// <summary>
        ///     Create s server header.
        /// </summary>
        byte[] CreateServerHeader(uint size, uint opcode);

        /// <summary>
        ///     Writes a server header.
        /// </summary>
        void WriteServerHeader(Span<byte> span, uint size, uint opcode);

        /// <summary>
        ///     Writes a server header.
        /// </summary>
        void WriteServerHeader(byte[] buffer, uint size, uint opcode);

        /// <summary>
        ///     Writes a server header.
        /// </summary>
        void WriteServerHeader(Stream stream, uint size, uint opcode);

        /// <summary>
        ///     Writes a server header.
        /// </summary>
        Task WriteServerHeaderAsync(Stream stream, uint size, uint opcode,
            CancellationToken cancellationToken = default);
    }
}
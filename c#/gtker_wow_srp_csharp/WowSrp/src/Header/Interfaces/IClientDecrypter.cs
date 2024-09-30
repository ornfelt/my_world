using System;
using System.IO;
using System.Threading;
using System.Threading.Tasks;

namespace WowSrp.Header
{
    /// <summary>
    ///     A decrypter of client headers.
    ///     Would be used by the server.
    /// </summary>
    public interface IClientDecrypter : IDecrypter
    {
        /// <summary>
        ///     Reads a client header.
        /// </summary>
        public HeaderData ReadClientHeader(Span<byte> span) => HeaderImplementations.ReadClientHeader(span, Decrypt);

        /// <summary>
        ///     Reads a client header.
        /// </summary>
        public HeaderData ReadClientHeader(Stream stream) => HeaderImplementations.ReadClientHeader(stream, Decrypt);

        /// <summary>
        ///     Reads a client header.
        /// </summary>
        public Task<HeaderData> ReadClientHeaderAsync(Stream stream, CancellationToken cancellationToken = default) =>
            HeaderImplementations.ReadClientHeaderAsync(stream, Decrypt, cancellationToken);
    }
}
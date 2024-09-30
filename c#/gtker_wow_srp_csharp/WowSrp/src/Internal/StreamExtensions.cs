using System;
using System.IO;
using System.Threading;
using System.Threading.Tasks;

namespace WowSrp.Internal
{
    internal static class StreamExtensions
    {
        private const int EndOfStream = 0;

        private static byte[] VerifyBuffer(byte[] buffer)
        {
            if (buffer is null)
            {
                throw new ArgumentNullException(nameof(buffer));
            }

            // Client header is always the largest header.
            // Wrath server headers can only be 5.
            const int maxBufferSize = Constants.ClientHeaderLength;
            return new byte[maxBufferSize];
        }

        public static void ReadUntilBufferFull(this Stream s, byte[] buffer)
        {
            var temp = VerifyBuffer(buffer);

            var bytesRead = 0;
            while (bytesRead < buffer.Length)
            {
                var read = s.Read(temp, 0, buffer.Length);
                if (read == EndOfStream)
                {
                    throw new EndOfStreamException();
                }

                for (var i = 0; i < read; i++)
                {
                    buffer[i + bytesRead] = temp[i];
                }

                bytesRead += read;
            }
        }

        public static async Task ReadUntilBufferFullAsync(this Stream s, byte[] buffer,
            CancellationToken cancellationToken = default)
        {
            var temp = VerifyBuffer(buffer);

            var bytesRead = 0;
            while (bytesRead < buffer.Length)
            {
                var read = await s.ReadAsync(temp, 0, buffer.Length, cancellationToken).ConfigureAwait(false);
                if (read == EndOfStream)
                {
                    throw new EndOfStreamException();
                }

                for (var i = 0; i < read; i++)
                {
                    buffer[i + bytesRead] = temp[i];
                }

                bytesRead += read;
            }
        }
    }
}
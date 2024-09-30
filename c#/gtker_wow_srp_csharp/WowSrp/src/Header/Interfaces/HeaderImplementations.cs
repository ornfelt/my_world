using System;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using WowSrp.Internal;

namespace WowSrp.Header
{
    internal static class HeaderImplementations
    {
        public static byte[] CreateClientHeader(uint size, uint opcode, Encrypt encrypt)
        {
            var b = new byte[Constants.ClientHeaderLength];

            Utils.WriteBigEndian(size, b.AsSpan()[..Constants.ClientSizeLength]);
            Utils.WriteLittleEndian(opcode, b.AsSpan()[Constants.ClientSizeLength..]);

            encrypt(b);

            return b;
        }

        public static void WriteClientHeader(Span<byte> span, uint size, uint opcode, Encrypt encrypt)
        {
            var b = CreateClientHeader(size, opcode, encrypt);
            b.CopyTo(span);
        }

        public static void WriteClientHeader(byte[] buffer, uint size, uint opcode, Encrypt encrypt)
        {
            var b = CreateClientHeader(size, opcode, encrypt);
            b.CopyTo(buffer, 0);
        }

        public static void WriteClientHeader(Stream stream, uint size, uint opcode, Encrypt encrypt)
        {
            var b = CreateClientHeader(size, opcode, encrypt);
            stream.Write(b);
        }

        public static async Task WriteClientHeaderAsync(Stream stream, uint size, uint opcode, Encrypt encrypt,
            CancellationToken cancellationToken = default)
        {
            var b = CreateClientHeader(size, opcode, encrypt);
            await stream.WriteAsync(b, cancellationToken).ConfigureAwait(false);
        }

        public static HeaderData ReadClientHeader(Span<byte> span, Decrypt decrypt)
        {
            var newSpan = span[..Constants.ClientHeaderLength];
            decrypt(newSpan);
            return Utils.ReadSpans(newSpan[..Constants.ClientSizeLength],
                newSpan[Constants.ClientSizeLength..]);
        }

        public static HeaderData ReadClientHeader(Stream stream, Decrypt decrypt)
        {
            var header = new byte[Constants.ClientHeaderLength];
            stream.ReadUntilBufferFull(header);
            return ReadClientHeader(header, decrypt);
        }

        public static async Task<HeaderData> ReadClientHeaderAsync(Stream stream, Decrypt decrypt,
            CancellationToken cancellationToken = default)
        {
            var header = new byte[Constants.ClientHeaderLength];
            await stream.ReadUntilBufferFullAsync(header, cancellationToken).ConfigureAwait(false);
            return ReadClientHeader(header, decrypt);
        }

        private static HeaderData ReadUnencryptedHeader(Span<byte> span)
        {
            var serverSizeLength =
                span.Length == 5 ? Constants.ServerWrathLargeSizeLength : Constants.ServerNormalSizeLength;
            return Utils.ReadSpans(span[..serverSizeLength],
                span[serverSizeLength..]);
        }

        public static HeaderData ReadServerHeader(Span<byte> span, bool isWrath, Decrypt decrypt)
        {
            if (!isWrath)
            {
                var newBytes = span[..Constants.ServerNormalHeaderLength];
                decrypt(newBytes);

                return ReadUnencryptedHeader(newBytes);
            }

            decrypt(span[..1]);
            var serverSizeLength = Utils.ServerSizeFieldLength(span[0], isWrath);

            span[0] = Utils.ClearBigHeader(span[0]);
            var newSpan = span[..(serverSizeLength + Constants.ServerOpcodeLength)];
            decrypt(newSpan[1..]);

            return ReadUnencryptedHeader(newSpan);
        }

        public static HeaderData ReadServerHeader(Stream stream, bool isWrath, Decrypt decrypt)
        {
            if (!isWrath)
            {
                var buf = new byte[Constants.ServerNormalHeaderLength];
                stream.ReadUntilBufferFull(buf);
                return ReadServerHeader(buf, isWrath, decrypt);
            }

            var firstByte = new byte[1];

            stream.ReadUntilBufferFull(firstByte);
            decrypt(firstByte);
            var serverSizeLength = Utils.ServerSizeFieldLength(firstByte[0], isWrath);

            var remainingHeader = new byte[serverSizeLength + Constants.ServerOpcodeLength - 1];
            stream.ReadUntilBufferFull(remainingHeader);
            decrypt(remainingHeader);

            Span<byte> header = stackalloc byte[serverSizeLength + Constants.ServerOpcodeLength];

            header[0] = Utils.ClearBigHeader(firstByte[0]);
            remainingHeader.CopyTo(header[1..]);

            return ReadUnencryptedHeader(header);
        }

        public static async Task<HeaderData> ReadServerHeaderAsync(Stream stream, bool isWrath, Decrypt decrypt,
            CancellationToken cancellationToken = default)
        {
            if (!isWrath)
            {
                var buf = new byte[Constants.ServerNormalHeaderLength];
                await stream.ReadUntilBufferFullAsync(buf, cancellationToken).ConfigureAwait(false);
                return ReadServerHeader(buf, isWrath, decrypt);
            }

            var firstByte = new byte[1];

            await stream.ReadUntilBufferFullAsync(firstByte, cancellationToken).ConfigureAwait(false);
            decrypt(firstByte);
            var serverSizeLength = Utils.ServerSizeFieldLength(firstByte[0], isWrath);

            var remainingHeader = new byte[serverSizeLength + Constants.ServerOpcodeLength - 1];
            await stream.ReadUntilBufferFullAsync(remainingHeader, cancellationToken).ConfigureAwait(false);
            decrypt(remainingHeader);

            var header = new byte[serverSizeLength + Constants.ServerOpcodeLength];

            header[0] = Utils.ClearBigHeader(firstByte[0]);
            remainingHeader.CopyTo(header.AsSpan()[1..]);

            return ReadUnencryptedHeader(header);
        }

        public static byte[] CreateServerHeader(uint size, uint opcode, bool isWrath, Encrypt encrypt)
        {
            var serverSizeField = Utils.ServerSizeFieldSize(size, isWrath);
            var b = new byte[serverSizeField + Constants.ServerOpcodeLength];

            Utils.WriteBigEndian(size, b.AsSpan()[..serverSizeField]);
            if (isWrath && size > 0x7FFF)
            {
                b[0] = Utils.SetBigHeader(b[0]);
            }

            Utils.WriteLittleEndian(opcode, b.AsSpan()[serverSizeField..]);

            encrypt(b);

            return b;
        }

        public static void WriteServerHeader(Span<byte> span, uint size, uint opcode, bool isWrath, Encrypt encrypt)
        {
            var b = CreateServerHeader(size, opcode, isWrath, encrypt);
            b.CopyTo(span);
        }

        public static void WriteServerHeader(byte[] buffer, uint size, uint opcode, bool isWrath, Encrypt encrypt)
        {
            var b = CreateServerHeader(size, opcode, isWrath, encrypt);
            b.CopyTo(buffer, 0);
        }

        public static void WriteServerHeader(Stream stream, uint size, uint opcode, bool isWrath, Encrypt encrypt)
        {
            var b = CreateServerHeader(size, opcode, isWrath, encrypt);
            stream.Write(b);
        }

        public static async Task WriteServerHeaderAsync(Stream stream, uint size, uint opcode, bool isWrath,
            Encrypt encrypt,
            CancellationToken cancellationToken = default)
        {
            var b = CreateServerHeader(size, opcode, isWrath, encrypt);
            await stream.WriteAsync(b, cancellationToken).ConfigureAwait(false);
        }

        internal delegate void Encrypt(Span<byte> data);

        internal delegate void Decrypt(Span<byte> data);
    }
}
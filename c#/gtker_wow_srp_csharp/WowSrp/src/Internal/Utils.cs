using System;
using System.Security.Cryptography;
using WowSrp.Header;

namespace WowSrp.Internal
{
    internal static class Utils
    {
        public static void AssertArrayLength(ReadOnlySpan<byte> array, int expected,
            string variableName)
        {
            if (array.Length != expected)
            {
                throw new ArgumentException(
                    $"invalid length of {array.Length}. Only arrays of length {expected} is valid.", variableName);
            }
        }

        public static string ConvertUsernameOrPassword(string s, string argumentName)
        {
            if (s.Length > 16)
            {
                throw new ArgumentException($"length is '{s.Length}', but can not be longer than '16' characters",
                    argumentName);
            }

            return s.ToUpper();
        }

        public static T[] TrimArray<T>(T[] array, int length)
        {
            var newArray = new T[length];

            var newLength = Math.Min(array.Length, length);

            for (var i = 0; i < newLength; i++)
            {
                newArray[i] = array[i];
            }

            return newArray;
        }

        public static byte[] Md5(byte[] buf) => MD5.Create().ComputeHash(buf);

        public static byte[] Sha1(byte[] buf) => SHA1.Create().ComputeHash(buf);

        public static uint RandomUint()
        {
            Span<byte> buf = stackalloc byte[4];
            RandomBytes(buf);
            return Convert.ToUInt32(ReadLittleEndian(buf));
        }

        public static void RandomBytes(Span<byte> buf)
        {
            new Random().NextBytes(buf);
        }

        public static byte[] RandomizedArray(int length)
        {
            var a = new byte[length];
            RandomBytes(a);
            return a;
        }

        public static uint ReadLittleEndian(ReadOnlySpan<byte> span)
        {
            uint value = 0;
            for (var i = 0; i < span.Length; i++)
            {
                value |= (uint)span[i] << (i * 8);
            }

            return value;
        }

        public static void WriteLittleEndian(ulong value, Span<byte> span)
        {
            for (var i = 0; i < span.Length; i++)
            {
                unchecked
                {
                    span[i] = (byte)(value >> (i * 8));
                }
            }
        }

        public static void WriteLittleEndian(int value, Span<byte> span)
        {
            for (var i = 0; i < span.Length; i++)
            {
                unchecked
                {
                    span[i] = (byte)(value >> (i * 8));
                }
            }
        }

        public static uint ReadBigEndian(ReadOnlySpan<byte> span)
        {
            uint value = 0;
            for (var i = 0; i < span.Length; i++)
            {
                var opposite = span.Length - i - 1;
                value |= (uint)span[i] << (opposite * 8);
            }

            return value;
        }

        public static void WriteBigEndian(uint value, Span<byte> span)
        {
            for (var i = 0; i < span.Length; i++)
            {
                unchecked
                {
                    var opposite = span.Length - i - 1;
                    span[i] = (byte)(value >> (opposite * 8));
                }
            }
        }

        public static HeaderData ReadSpans(ReadOnlySpan<byte> size, ReadOnlySpan<byte> opcode) =>
            new HeaderData(ReadBigEndian(size), ReadLittleEndian(opcode));

        public static bool HasBigHeader(byte b) => (b & 0x80) != 0;

        public static byte ClearBigHeader(byte b) => (byte)(b & 0x7F);

        public static byte SetBigHeader(byte b) => (byte)(b | 0x80);

        public static int ServerSizeFieldLength(byte span, bool isWrath)
        {
            if (isWrath && HasBigHeader(span))
            {
                return Constants.ServerWrathLargeSizeLength;
            }

            return Constants.ServerNormalSizeLength;
        }

        public static int ServerSizeFieldSize(uint size, bool isWrath)
        {
            if (isWrath && size > 0x7FFF)
            {
                return Constants.ServerWrathLargeSizeLength;
            }

            return Constants.ServerNormalSizeLength;
        }


        public static byte[] ConcatArrays(ReadOnlySpan<byte> a, ReadOnlySpan<byte> b)
        {
            var array = new byte[a.Length + b.Length];

            a.CopyTo(array);
            b.CopyTo(array.AsSpan(a.Length));

            return array;
        }

        public static byte[] ConcatArrays(ReadOnlySpan<byte> a, ReadOnlySpan<byte> b, ReadOnlySpan<byte> c)
        {
            var array = new byte[a.Length + b.Length + c.Length];

            a.CopyTo(array);
            b.CopyTo(array.AsSpan(a.Length));
            c.CopyTo(array.AsSpan(a.Length + b.Length));

            return array;
        }

        public static byte[] ConcatArrays(ReadOnlySpan<byte> a, ReadOnlySpan<byte> b, ReadOnlySpan<byte> c,
            ReadOnlySpan<byte> d)
        {
            var array = new byte[a.Length + b.Length + c.Length + d.Length];

            a.CopyTo(array);
            b.CopyTo(array.AsSpan(a.Length));
            c.CopyTo(array.AsSpan(a.Length + b.Length));
            d.CopyTo(array.AsSpan(a.Length + b.Length + c.Length));

            return array;
        }

        public static byte[] ConcatArrays(ReadOnlySpan<byte> a, ReadOnlySpan<byte> b, ReadOnlySpan<byte> c,
            ReadOnlySpan<byte> d, ReadOnlySpan<byte> e)
        {
            var array = new byte[a.Length + b.Length + c.Length + d.Length + e.Length];

            a.CopyTo(array);
            b.CopyTo(array.AsSpan(a.Length));
            c.CopyTo(array.AsSpan(a.Length + b.Length));
            d.CopyTo(array.AsSpan(a.Length + b.Length + c.Length));
            e.CopyTo(array.AsSpan(a.Length + b.Length + c.Length + d.Length));

            return array;
        }

        public static byte[] ConcatArrays(ReadOnlySpan<byte> a, ReadOnlySpan<byte> b, ReadOnlySpan<byte> c,
            ReadOnlySpan<byte> d, ReadOnlySpan<byte> e, ReadOnlySpan<byte> f)
        {
            var array = new byte[a.Length + b.Length + c.Length + d.Length + e.Length + f.Length];

            a.CopyTo(array);
            b.CopyTo(array.AsSpan(a.Length));
            c.CopyTo(array.AsSpan(a.Length + b.Length));
            d.CopyTo(array.AsSpan(a.Length + b.Length + c.Length));
            e.CopyTo(array.AsSpan(a.Length + b.Length + c.Length + d.Length));
            f.CopyTo(array.AsSpan(a.Length + b.Length + c.Length + d.Length + e.Length));

            return array;
        }
    }
}
using System;
using System.Numerics;

namespace WowSrp.Internal
{
    internal static class Extensions
    {
        public static BigInteger ToBigInteger(this byte v) => new BigInteger(v);

        public static BigInteger ToBigInteger(this byte[] buffer) => new BigInteger(buffer, true);

        public static BigInteger ToBigInteger(this ReadOnlySpan<byte> buffer) => new BigInteger(buffer, true);

        public static BigInteger ModPow(this BigInteger b, BigInteger exponent, BigInteger modulus)
        {
            var s = BigInteger.ModPow(b, exponent, modulus);

            if (s < 0)
            {
                s += modulus;
            }

            return s;
        }


        public static byte[] ToPaddedArray(this BigInteger b, int length) => Utils.TrimArray(b.ToByteArray(), length);

        public static byte[] ToPaddedArray(this uint b, int length) =>
            Utils.TrimArray(BitConverter.GetBytes(b), length);
    }
}
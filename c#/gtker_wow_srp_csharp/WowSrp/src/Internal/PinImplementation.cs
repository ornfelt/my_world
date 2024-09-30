using System;
using System.Linq;

namespace WowSrp.Internal
{
    internal static class PinImplementation
    {
        public static byte[] RemapPinGrid(long pinGridSeed)
        {
            byte[] grid = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            byte[] remappedGrid = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

            for (var i = 10; i > 0; i--)
            {
                var remainder = pinGridSeed % i;
                pinGridSeed /= i;
                var remappedIndex = 10 - i;
                remappedGrid[remappedIndex] = grid[remainder];

                var copySize = i - remainder - 1;

                for (var j = 0; j < copySize; j++)
                {
                    grid[remainder + j] = grid[remainder + j + 1];
                }
            }

            return remappedGrid;
        }

        public static void RandomizedGrid(byte[] bytes, byte[] remappedPinGrid)
        {
            foreach (var (i, b) in bytes.Select((b, i) => (i, b)))
            {
                bytes[i] = (byte)remappedPinGrid.TakeWhile(c => b != c).Count();
            }
        }

        public static byte[] CalculateHash(byte[] pin, long pinGridSeed, ReadOnlySpan<byte> serverSalt,
            ReadOnlySpan<byte> clientSalt)
        {
            var remappedPinGrid = RemapPinGrid(pinGridSeed);

            RandomizedGrid(pin, remappedPinGrid);

            // Convert digits to ASCII
            for (var i = 0; i < pin.Length; i++)
            {
                pin[i] += 0x30;
            }

            var full = Utils.ConcatArrays(serverSalt, pin);
            var firstHash = Utils.Sha1(full);
            var full2 = Utils.ConcatArrays(clientSalt, firstHash);
            return Utils.Sha1(full2);
        }
    }
}
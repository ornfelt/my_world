using System;

namespace WowSrp.Internal
{
    internal static class MatrixCardImplementation
    {
        public static void FillMatrixCard(byte[] buf)
        {
            for (var i = 0; i < buf.Length; i++)
            {
                buf[i] = Convert.ToByte(new Random().Next(0, 9));
            }
        }

        public static uint[] GenerateCoordinates(byte height, byte width, byte challengeCount, ulong seed)
        {
            VerifyArguments(width, height, challengeCount);

            var matrixIndices = new uint[width * height];
            for (uint i = 0; i < width * height; i++)
            {
                matrixIndices[i] = i;
            }

            var coordinates = new uint[challengeCount];

            for (var i = 0; i < coordinates.Length; i++)
            {
                var count = (ulong)(matrixIndices.Length - i);
                var index = seed % count;

                coordinates[i] = matrixIndices[index];

                for (var j = index; j < count - 1; j++)
                {
                    matrixIndices[j] = matrixIndices[j + 1];
                }

                seed /= count;
            }

            return coordinates;
        }

        public static (byte, byte) GetCoordinate(byte challengeCount, ReadOnlySpan<uint> coordinates, byte width,
            byte height, byte round)
        {
            VerifyArguments(width, height, challengeCount);

            var coordinate = coordinates[round];

            var x = coordinate % width;
            var y = coordinate / width;

            return ((byte)x, (byte)y);
        }

        public static void VerifyArguments(byte width, byte height, byte challengeCount)
        {
            if (height == 0)
            {
                throw new ArgumentException("height can not be 0", nameof(height));
            }

            if (width == 0)
            {
                throw new ArgumentException("height can not be 0", nameof(width));
            }

            if (height * width < challengeCount)
            {
                throw new ArgumentException("height * width must be greater than challengeCount",
                    nameof(challengeCount));
            }
        }
    }
}
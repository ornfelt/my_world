using System;
using WowSrp.Internal;

namespace WowSrp
{
    /// <summary>
    ///     Represents the physical matrix card data.
    ///     Can be either randomly generated or loaded from the database.
    /// </summary>
    public class MatrixCard
    {
        /// <summary>
        ///     Default digit count used by the simple constructors.
        /// </summary>
        public const byte DefaultDigitCount = 3;

        /// <summary>
        ///     Default width used by the simple constructors.
        ///     Same as <see cref="MatrixCardVerifier.DefaultWidth" />
        /// </summary>
        public const byte DefaultWidth = MatrixCardVerifier.DefaultWidth;

        /// <summary>
        ///     Default height used by the simple constructors.
        ///     Same as <see cref="MatrixCardVerifier.DefaultHeight" />
        /// </summary>
        public const byte DefaultHeight = MatrixCardVerifier.DefaultHeight;

        /// <summary>
        ///     The matrix card as a raw array.
        ///     Save this to the database.
        /// </summary>
        public readonly byte[] Data;

        /// <summary>
        ///     Digit count provided.
        /// </summary>
        public readonly byte DigitCount;

        /// <summary>
        ///     Height of the matrix card.
        /// </summary>
        public readonly byte Height;

        /// <summary>
        ///     Width of the matrix card.
        /// </summary>
        public readonly byte Width;

        /// <summary>
        ///     Creates a new Matrix Card with randomized data and the default parameters
        /// </summary>
        public MatrixCard() : this(DefaultDigitCount, DefaultHeight, DefaultWidth)
        {
        }

        /// <summary>
        ///     Creates a new Matrix Card with randomized data.
        /// </summary>
        public MatrixCard(byte digitCount, byte height, byte width) : this(digitCount, height, width,
            new byte[height * width])
        {
            MatrixCardImplementation.FillMatrixCard(Data);
        }

        /// <summary>
        ///     Creates a new Matrix Card with specific data.
        /// </summary>
        public MatrixCard(byte digitCount, byte height, byte width, byte[] data)
        {
            if (digitCount == 0)
            {
                throw new ArgumentException("digitCount can not be 0", nameof(digitCount));
            }

            if (height == 0)
            {
                throw new ArgumentException("height can not be 0", nameof(height));
            }

            if (width == 0)
            {
                throw new ArgumentException("width can not be 0", nameof(width));
            }

            if (height * width != data.Length)
            {
                throw new ArgumentException(
                    $"data.Length {data.Length} must be same as height {height} * width {width}", nameof(data));
            }

            DigitCount = digitCount;
            Height = height;
            Width = width;
            Data = data;
        }

        /// <summary>
        ///     Returns an array where every element contains the number in that field as a byte array.
        /// </summary>
        public byte[][] IndividualDigits()
        {
            var buf = new byte[Height * Width][];

            for (var i = 0; i < buf.Length; i++)
            {
                buf[i] = new byte[DigitCount];
                Data.AsSpan(i * DigitCount, DigitCount).CopyTo(buf[i]);
            }

            return buf;
        }

        /// <summary>
        ///     Returns the digits at <code>(x, y)</code>.
        /// </summary>
        public ReadOnlySpan<byte> GetNumberAtCoordinates(byte x, byte y)
        {
            var start = x * y;
            return new ReadOnlySpan<byte>(Data, start, DigitCount);
        }

        /// <summary>
        ///     Returns true if the <paramref name="clientProof" /> matches the generated proof.
        /// </summary>
        public bool VerifyHash(byte challengeCount, ulong seed, ReadOnlySpan<byte> sessionKey,
            ReadOnlySpan<byte> clientProof)
        {
            MatrixCardImplementation.VerifyArguments(Width, Height, challengeCount);

            var verifier = new MatrixCardVerifier(Height, Width, challengeCount, seed, sessionKey);

            for (byte round = 0; round < challengeCount; round++)
            {
                var (x, y) = verifier.GetCoordinate(round);
                var digits = GetNumberAtCoordinates(x, y);
                verifier.EnterMultipleDigits(digits);
            }

            var hash = verifier.CalculateHash();

            return hash == clientProof;
        }
    }
}
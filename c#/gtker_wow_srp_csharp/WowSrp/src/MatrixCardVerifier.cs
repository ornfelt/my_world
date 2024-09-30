using System;
using System.Security.Cryptography;
using WowSrp.Internal;

namespace WowSrp
{
    /// <summary>
    ///     Used to verify a <see cref="MatrixCard" /> if manual entry is required, such as in GUI applications.
    /// </summary>
    public class MatrixCardVerifier
    {
        /// <summary>
        ///     Default height used by the simple constructor.
        ///     Same as <see cref="MatrixCard.DefaultHeight" />
        /// </summary>
        public const byte DefaultHeight = 26;

        /// <summary>
        ///     Default width used by the simple constructor.
        ///     Same as <see cref="MatrixCard.DefaultWidth" />
        /// </summary>
        public const byte DefaultWidth = 26;

        /// <summary>
        ///     Default challenge count used by the simple constructor.
        /// </summary>
        public const byte DefaultChallengeCount = 2;

        private readonly Arc4 _arc4;
        private readonly byte _challengeCount;
        private readonly uint[] _coordinates;
        private readonly byte _height;
        private readonly HMACSHA1 _hmac;
        private readonly byte _width;

        /// <summary>
        ///     Simple constructor. Uses the default values.
        /// </summary>
        public MatrixCardVerifier(ulong seed, ReadOnlySpan<byte> sessionKey) : this(DefaultHeight, DefaultWidth,
            DefaultChallengeCount, seed, sessionKey)
        {
        }

        /// <summary>
        ///     Explicit constructor. Uses custom values.
        /// </summary>
        public MatrixCardVerifier(byte height, byte width, byte challengeCount, ulong seed,
            ReadOnlySpan<byte> sessionKey)
        {
            Utils.AssertArrayLength(sessionKey, Constants.SessionKeyLength, nameof(sessionKey));
            MatrixCardImplementation.VerifyArguments(width, height, challengeCount);

            _challengeCount = challengeCount;
            _height = height;
            _width = width;

            _coordinates = MatrixCardImplementation.GenerateCoordinates(_height, _width, _challengeCount, seed);

            var seedArray = new byte[8];
            Utils.WriteLittleEndian(seed, seedArray);
            var md5Pre = Utils.ConcatArrays(seedArray, sessionKey);

            var md5 = Utils.Md5(md5Pre);

            _arc4 = new Arc4(md5, 0);

            _hmac = new HMACSHA1(md5);
        }

        /// <summary>
        ///     Returns the <code>(x, y)</code> for a given <paramref name="round" />.
        /// </summary>
        public (byte, byte) GetCoordinate(byte round)
        {
            if (round > _challengeCount)
            {
                throw new ArgumentException($"round '{round}' is greater than challengeCount '{_challengeCount}'",
                    nameof(round));
            }

            return MatrixCardImplementation.GetCoordinate(_challengeCount, _coordinates, _width, _height, round);
        }

        /// <summary>
        ///     Enters a single digit into the algorithm.
        /// </summary>
        public void EnterDigit(byte value)
        {
            var b = new byte[1];
            b[0] = value;
            _arc4.ApplyKeyStream(b);
            _hmac.TransformBlock(b, 0, b.Length, b, 0);
        }

        /// <summary>
        ///     Enters multiple digits into the algorithm. This is just a foreach around <see cref="EnterDigit" />.
        /// </summary>
        public void EnterMultipleDigits(ReadOnlySpan<byte> values)
        {
            foreach (var value in values)
            {
                EnterDigit(value);
            }
        }

        /// <summary>
        ///     Finalizes the algorithm and returns the hash.
        /// </summary>
        public byte[] CalculateHash()
        {
            var b = new byte[1];
            _hmac.TransformFinalBlock(b, 0, 0);
            return _hmac.Hash;
        }
    }
}
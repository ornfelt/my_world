using WowSrp;
using WowSrp.Internal;

namespace Tests;

public class MatrixCard
{
    [Test]
    public void GenerateCoordinates()
    {
        var contents = File.ReadAllLines("./tests/matrix_card/coordinates_regression.txt");
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');

            var height = byte.Parse(split[0]);
            var width = byte.Parse(split[1]);
            var challengeCount = byte.Parse(split[2]);
            var seed = ulong.Parse(split[3]);
            var expected = new uint[challengeCount];
            for (var i = 0; i < challengeCount; i++)
            {
                expected[i] = uint.Parse(split[4 + i]);
            }

            var actual = MatrixCardImplementation.GenerateCoordinates(height, width, challengeCount, seed);

            Assert.That(actual, Is.EqualTo(expected));
        }
    }

    [Test]
    public void GetCoordinate()
    {
        var contents = File.ReadAllLines("./tests/matrix_card/get_coordinate_regression.txt");
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');

            var height = byte.Parse(split[0]);
            var width = byte.Parse(split[1]);
            var seed = ulong.Parse(split[2]);
            var challengeCount = byte.Parse(split[3]);
            _ = TestUtils.StringToByteArray(split[4]);
            var expected = new (byte, byte)[challengeCount];
            for (var i = 0; i < challengeCount; i++)
            {
                var x = byte.Parse(split[5 + i * 2]);
                var y = byte.Parse(split[5 + i * 2 + 1]);
                expected[i] = (x, y);
            }

            var coordinates = MatrixCardImplementation.GenerateCoordinates(height, width, challengeCount, seed);

            for (var round = 0; round < expected.Length; round++)
            {
                var (actualX, actualY) =
                    MatrixCardImplementation.GetCoordinate(challengeCount, coordinates, width, height, (byte)round);

                Assert.That(actualX, Is.EqualTo(expected[round].Item1));
                Assert.That(actualY, Is.EqualTo(expected[round].Item2));
            }
        }
    }

    [Test]
    public void ProofRegression()
    {
        var contents = File.ReadAllLines("./tests/matrix_card/proof_regression.txt");
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');

            var height = byte.Parse(split[0]);
            var width = byte.Parse(split[1]);
            var seed = ulong.Parse(split[2]);
            var sessionKey = TestUtils.StringToByteArray(split[3]);
            var expected = TestUtils.StringToByteArray(split[4]);
            var challengeCount = byte.Parse(split[5]);
            var challenges = new byte[challengeCount];
            for (var i = 0; i < challengeCount; i++)
            {
                challenges[i] = byte.Parse(split[6 + i]);
            }

            var verifier = new MatrixCardVerifier(width, height, challengeCount, seed, sessionKey);

            for (var round = 0; round < challengeCount; round++)
            {
                verifier.EnterDigit(challenges[round]);
            }

            var actual = verifier.CalculateHash();

            Assert.That(actual, Is.EqualTo(expected));
        }
    }
}
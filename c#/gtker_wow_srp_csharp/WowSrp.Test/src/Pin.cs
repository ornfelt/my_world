using WowSrp;
using WowSrp.Internal;

namespace Tests;

public class PinTest
{
    [Test]
    public void RemapPinGrid()
    {
        var contents = File.ReadAllLines("./tests/pin/remap_regression.txt");
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');

            var pinGridSeed = long.Parse(split[0]);
            var expected = TestUtils.StringToByteArray(split[1]);

            var actual = PinImplementation.RemapPinGrid(pinGridSeed);

            Assert.That(actual, Is.EqualTo(expected));
        }
    }

    [Test]
    public void RandomizeGrid()
    {
        var contents = File.ReadAllLines("./tests/pin/convert_regression.txt");
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');

            var bytes = TestUtils.StringToByteArrayNoPadding(split[0]);
            var remappedPinGrid = TestUtils.StringToByteArrayNoPadding(split[1]);
            var expected = TestUtils.StringToByteArrayNoPadding(split[2]);

            PinImplementation.RandomizedGrid(bytes, remappedPinGrid);

            Assert.That(bytes, Is.EqualTo(expected));
        }
    }

    [Test]
    public void CalculateHash()
    {
        var contents = File.ReadAllLines("./tests/pin/regression.txt");
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');

            var pin = TestUtils.StringToByteArrayNoPadding(split[0]);
            var pinGridSeed = long.Parse(split[1]);
            var serverSalt = TestUtils.StringToByteArray(split[2]);
            var clientSalt = TestUtils.StringToByteArray(split[3]);
            var expected = TestUtils.StringToByteArray(split[4]);

            var actual = Pin.CalculateHash(pin, pinGridSeed, serverSalt, clientSalt);

            Assert.That(actual, Is.EqualTo(expected));
        }
    }
}
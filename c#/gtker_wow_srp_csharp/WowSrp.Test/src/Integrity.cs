using WowSrp;

namespace Tests;

public class IntegrityTest
{
    [Test]
    public void Generic()
    {
        var contents = File.ReadAllLines("./tests/integrity/generic_regression.txt");
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');

            var allFiles = TestUtils.StringToByteArray(split[0]);
            var checksumSalt = TestUtils.StringToByteArray(split[1]);
            var clientPublicKey = TestUtils.StringToByteArray(split[2]);
            var expected = TestUtils.StringToByteArray(split[3]);

            var actual = Integrity.GenericCheck(allFiles, checksumSalt, clientPublicKey);

            Assert.That(actual, Is.EqualTo(expected));
        }
    }

    [Test]
    public void Windows()
    {
        var contents = File.ReadAllLines("./tests/integrity/windows_regression.txt");
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');

            var wowExe = TestUtils.StringToByteArray(split[0]);
            var fmodDll = TestUtils.StringToByteArray(split[1]);
            var ijl15Dll = TestUtils.StringToByteArray(split[2]);
            var dbghelpDll = TestUtils.StringToByteArray(split[3]);
            var unicowsDll = TestUtils.StringToByteArray(split[4]);
            var checksumSalt = TestUtils.StringToByteArray(split[5]);
            var clientPublicKey = TestUtils.StringToByteArray(split[6]);
            var expected = TestUtils.StringToByteArray(split[7]);

            var actual = Integrity.WindowsCheck(wowExe, fmodDll, ijl15Dll, dbghelpDll, unicowsDll, checksumSalt,
                clientPublicKey);

            Assert.That(actual, Is.EqualTo(expected));
        }
    }

    [Test]
    public void Mac()
    {
        var contents = File.ReadAllLines("./tests/integrity/windows_regression.txt");
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');

            var worldOfWarcraft = TestUtils.StringToByteArray(split[0]);
            var infoPlist = TestUtils.StringToByteArray(split[1]);
            var objectsXib = TestUtils.StringToByteArray(split[2]);
            var wowIcns = TestUtils.StringToByteArray(split[3]);
            var pkgInfo = TestUtils.StringToByteArray(split[4]);
            var checksumSalt = TestUtils.StringToByteArray(split[5]);
            var clientPublicKey = TestUtils.StringToByteArray(split[6]);
            var expected = TestUtils.StringToByteArray(split[7]);

            var actual = Integrity.MacCheck(worldOfWarcraft, infoPlist, objectsXib, wowIcns, pkgInfo, checksumSalt,
                clientPublicKey);

            Assert.That(actual, Is.EqualTo(expected));
        }
    }

    [Test]
    public void Reconnect()
    {
        var contents = File.ReadAllLines("./tests/integrity/reconnect_regression.txt");
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');

            var checksumSalt = TestUtils.StringToByteArray(split[0]);
            var expected = TestUtils.StringToByteArray(split[1]);

            var actual = Integrity.ReconnectCheck(checksumSalt);

            Assert.That(actual, Is.EqualTo(expected));
        }
    }
}
using WowSrp.Header;

namespace Tests;

public class EncryptionTest
{
    [Test]
    public void Vanilla()
    {
        var contents = File.ReadAllLines("./tests/encryption/calculate_encrypt_values.txt");
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');

            var sessionKey = TestUtils.StringToByteArray(split[0]);
            var data = TestUtils.StringToByteArray(split[1]);
            var expected = TestUtils.StringToByteArray(split[2]);

            var encrypt =
                new VanillaEncryption(sessionKey);
            ((IEncrypter)encrypt).Encrypt(data);

            Assert.That(data, Is.EqualTo(expected));
        }
    }
}
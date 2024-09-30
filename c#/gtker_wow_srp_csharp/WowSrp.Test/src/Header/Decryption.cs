using WowSrp.Header;

namespace Tests;

public class DecryptionTest
{
    [Test]
    public void Vanilla()
    {
        var contents = File.ReadAllLines("./tests/encryption/calculate_decrypt_values.txt");
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');

            var sessionKey = TestUtils.StringToByteArray(split[0]);
            var data = TestUtils.StringToByteArray(split[1]);
            var expected = TestUtils.StringToByteArray(split[2]);

            var decrypt =
                new VanillaDecryption(sessionKey);
            ((IDecrypter)decrypt).Decrypt(data);

            Assert.That(data, Is.EqualTo(expected));
        }
    }
}
using WowSrp.Header;
using WowSrp.Internal;

namespace Tests;

public class HeaderImplementation
{
    [Test]
    public void CreateTbcKey()
    {
        var contents = File.ReadAllLines("./tests/encryption/create_tbc_key.txt");
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');

            var sessionKey = TestUtils.StringToByteArray(split[0]);
            var expected = TestUtils.StringToByteArray(split[1]);

            var actual =
                WowSrp.Internal.HeaderImplementation.CreateTbcKey(sessionKey);

            Assert.That(actual, Is.EqualTo(expected));
        }
    }


    [Test]
    public void CreateWrathKey()
    {
        var contents = File.ReadAllLines("./tests/encryption/create_wrath_hmac_key.txt");
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');

            var sessionKey = TestUtils.StringToByteArray(split[0]);
            var key = TestUtils.StringToByteArray(split[1]);
            var expected = TestUtils.StringToByteArray(split[2]);

            var actual =
                WowSrp.Internal.HeaderImplementation.CreateTbcWrathKey(sessionKey, key);

            Assert.That(actual, Is.EqualTo(expected));
        }
    }

    [Test]
    public void Wrath()
    {
        var contents = File.ReadAllLines("./tests/encryption/calculate_wrath_encrypt_values.txt");
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');

            var sessionKey = TestUtils.StringToByteArray(split[0]);
            var data = TestUtils.StringToByteArray(split[1]);
            var originalData = data.Clone();
            var expectedClient = TestUtils.StringToByteArray(split[2]);
            var expectedServer = TestUtils.StringToByteArray(split[3]);

            var serverEnc = new WrathServerEncryption(sessionKey);
            ((IEncrypter)serverEnc).Encrypt(data);
            Assert.That(data, Is.EqualTo(expectedServer));

            var serverDec = new WrathServerDecryption(sessionKey);
            ((IDecrypter)serverDec).Decrypt(data);
            Assert.That(data, Is.EqualTo(originalData));


            var clientEnc = new WrathClientEncryption(sessionKey);
            ((IEncrypter)clientEnc).Encrypt(data);
            Assert.That(data, Is.EqualTo(expectedClient));

            var clientDec = new WrathClientDecryption(sessionKey);
            ((IDecrypter)clientDec).Decrypt(data);
            Assert.That(data, Is.EqualTo(originalData));
        }
    }

    [Test]
    public void Arc4()
    {
        // https://datatracker.ietf.org/doc/html/rfc6229
        byte[] key = [1, 2, 3, 4, 5];
        byte[] data =
        [
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0
        ];
        byte[] expected =
        [
            0xb2,
            0x39,
            0x63,
            0x05,
            0xf0,
            0x3d,
            0xc0,
            0x27,
            0xcc,
            0xc3,
            0x52,
            0x4a,
            0x0a,
            0x11,
            0x18,
            0xa8,
            0x69,
            0x82,
            0x94,
            0x4f,
            0x18,
            0xfc,
            0x82,
            0xd5,
            0x89,
            0xc4,
            0x03,
            0xa4,
            0x7a,
            0x0d,
            0x09,
            0x19
        ];

        var rc = new Arc4(key, 0);
        rc.ApplyKeyStream(data);

        Assert.That(data, Is.EqualTo(expected));

        key =
        [
            0x01,
            0x02,
            0x03,
            0x04,
            0x05,
            0x06,
            0x07,
            0x08,
            0x09,
            0x0a,
            0x0b,
            0x0c,
            0x0d,
            0x0e,
            0x0f,
            0x10,
            0x11,
            0x12,
            0x13,
            0x14,
            0x15,
            0x16,
            0x17,
            0x18,
            0x19,
            0x1a,
            0x1b,
            0x1c,
            0x1d,
            0x1e,
            0x1f,
            0x20
        ];
        data =
        [
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0
        ];
        expected =
        [
            0xea,
            0xa6,
            0xbd,
            0x25,
            0x88,
            0x0b,
            0xf9,
            0x3d,
            0x3f,
            0x5d,
            0x1e,
            0x4c,
            0xa2,
            0x61,
            0x1d,
            0x91,
            0xcf,
            0xa4,
            0x5c,
            0x9f,
            0x7e,
            0x71,
            0x4b,
            0x54,
            0xbd,
            0xfa,
            0x80,
            0x02,
            0x7c,
            0xb1,
            0x43,
            0x80
        ];

        rc = new Arc4(key, 0);
        rc.ApplyKeyStream(data);
        Assert.That(data, Is.EqualTo(expected));
    }
}
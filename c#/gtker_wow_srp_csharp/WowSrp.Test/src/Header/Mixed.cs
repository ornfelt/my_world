using WowSrp.Header;

namespace Tests;

public class Mixed
{
    private static readonly byte[] SessionKey =
    [
        99,
        131,
        9,
        219,
        107,
        35,
        248,
        24,
        247,
        161,
        213,
        174,
        25,
        135,
        70,
        253,
        173,
        103,
        149,
        186,
        85,
        162,
        130,
        144,
        129,
        83,
        118,
        179,
        93,
        82,
        160,
        128,
        165,
        215,
        35,
        125,
        224,
        8,
        156,
        140
    ];

    private readonly byte[] SessionKeyReg =
    [
        0x2E,
        0xFE,
        0xE7,
        0xB0,
        0xC1,
        0x77,
        0xEB,
        0xBD,
        0xFF,
        0x66,
        0x76,
        0xC5,
        0x6E,
        0xFC,
        0x23,
        0x39,
        0xBE,
        0x9C,
        0xAD,
        0x14,
        0xBF,
        0x8B,
        0x54,
        0xBB,
        0x5A,
        0x86,
        0xFB,
        0xF8,
        0x1F,
        0x6D,
        0x42,
        0x4A,
        0xA2,
        0x3C,
        0xC9,
        0xA3,
        0x14,
        0x9F,
        0xB1,
        0x75
    ];

    [Test]
    public async Task VanillaExtraMethods()
    {
        var enc = new VanillaEncryption(SessionKey);
        var dec = new VanillaDecryption(SessionKey);

        await ServerToClient(enc, dec);
        await ClientToServer(enc, dec);
    }

    [Test]
    public async Task TbcExtraMethods()
    {
        var enc = new TbcEncryption(SessionKey);
        var dec = new TbcDecryption(SessionKey);

        await ServerToClient(enc, dec);
        await ClientToServer(enc, dec);
    }

    [Test]
    public async Task WrathExtraMethods()
    {
        var serverEnc = new WrathServerEncryption(SessionKey);
        var serverDec = new WrathServerDecryption(SessionKey);
        var clientEnc = new WrathClientEncryption(SessionKey);
        var clientDec = new WrathClientDecryption(SessionKey);

        await ServerToClient(serverEnc, serverDec);
        await ClientToServer(clientEnc, clientDec);
    }

    private static async Task ServerToClient(IServerEncrypter s, IServerDecrypter c)
    {
        var buf = new byte[6];

        s.WriteServerHeader(buf, 0x1020, 0x3040);

        var d = c.ReadServerHeader(buf);
        Assert.Multiple(() =>
        {
            Assert.That(d.Size, Is.EqualTo(0x1020));
            Assert.That(d.Opcode, Is.EqualTo(0x3040));
        });

        s.WriteServerHeader(buf.AsSpan(), 0x1525, 0x3545);

        d = c.ReadServerHeader(buf.AsSpan());
        Assert.Multiple(() =>
        {
            Assert.That(d.Size, Is.EqualTo(0x1525));
            Assert.That(d.Opcode, Is.EqualTo(0x3545));
        });

        var mem = new MemoryStream(buf);
        s.WriteServerHeader(mem, 0x1121, 0x3141);

        mem = new MemoryStream(buf);
        d = c.ReadServerHeader(mem);
        Assert.Multiple(() =>
        {
            Assert.That(d.Size, Is.EqualTo(0x1121));
            Assert.That(d.Opcode, Is.EqualTo(0x3141));
        });

        mem = new MemoryStream(buf);
        await s.WriteServerHeaderAsync(mem, 0x1121, 0x3141);

        mem = new MemoryStream(buf);
        d = await c.ReadServerHeaderAsync(mem);
        Assert.Multiple(() =>
        {
            Assert.That(d.Size, Is.EqualTo(0x1121));
            Assert.That(d.Opcode, Is.EqualTo(0x3141));
        });
    }

    private static async Task ClientToServer(IClientEncrypter s, IClientDecrypter c)
    {
        var buf = new byte[6];

        s.WriteClientHeader(buf, 0x1020, 0x3040);

        var d = c.ReadClientHeader(buf);
        Assert.Multiple(() =>
        {
            Assert.That(d.Size, Is.EqualTo(0x1020));
            Assert.That(d.Opcode, Is.EqualTo(0x3040));
        });

        s.WriteClientHeader(buf.AsSpan(), 0x1525, 0x3545);

        d = c.ReadClientHeader(buf.AsSpan());
        Assert.Multiple(() =>
        {
            Assert.That(d.Size, Is.EqualTo(0x1525));
            Assert.That(d.Opcode, Is.EqualTo(0x3545));
        });

        var mem = new MemoryStream(buf);
        s.WriteClientHeader(mem, 0x1121, 0x3141);

        mem = new MemoryStream(buf);
        d = c.ReadClientHeader(mem);
        Assert.Multiple(() =>
        {
            Assert.That(d.Size, Is.EqualTo(0x1121));
            Assert.That(d.Opcode, Is.EqualTo(0x3141));
        });

        mem = new MemoryStream(buf);
        await s.WriteClientHeaderAsync(mem, 0x1121, 0x3141);

        mem = new MemoryStream(buf);
        d = await c.ReadClientHeaderAsync(mem);
        Assert.Multiple(() =>
        {
            Assert.That(d.Size, Is.EqualTo(0x1121));
            Assert.That(d.Opcode, Is.EqualTo(0x3141));
        });
    }

    [Test]
    public void TbcRegression()
    {
        var contents = File.ReadAllLines("./tests/encryption/calculate_tbc_encrypt_values.txt");
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');

            var sessionKey = TestUtils.StringToByteArray(split[0]);
            var data = TestUtils.StringToByteArray(split[1]);
            var originalData = (byte[])data.Clone();
            var expected = TestUtils.StringToByteArray(split[2]);

            var encrypt = new TbcEncryption(sessionKey);

            encrypt.Encrypt(data);
            Assert.That(data, Is.EqualTo(expected));

            var decrypt =
                new TbcDecryption(sessionKey);
            decrypt.Decrypt(data);

            Assert.That(data, Is.EqualTo(originalData));
        }
    }

    private async Task ClientHelper(string path, Func<byte[], (IClientDecrypter, IClientEncrypter)> create)
    {
        var contents = File.ReadAllLines(path);
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');

            var data = TestUtils.StringToByteArray(split[0]);
            var originalData = data.Clone();
            var size = uint.Parse(split[1]);
            var opcode = uint.Parse(split[2]);

            var (decrypt, encrypt) = create(SessionKeyReg);
            var header = decrypt.ReadClientHeader(data);

            Assert.Multiple(() =>
            {
                Assert.That(header.Size, Is.EqualTo(size));
                Assert.That(header.Opcode, Is.EqualTo(opcode));
            });
            encrypt.WriteClientHeader(data, size, opcode);

            Assert.That(data, Is.EqualTo(originalData));

            (decrypt, encrypt) = create(SessionKeyReg);
            header = decrypt.ReadClientHeader(new MemoryStream(data));

            Assert.Multiple(() =>
            {
                Assert.That(header.Size, Is.EqualTo(size));
                Assert.That(header.Opcode, Is.EqualTo(opcode));
            });
            encrypt.WriteClientHeader(new MemoryStream(data), size, opcode);

            Assert.That(data, Is.EqualTo(originalData));

            (decrypt, encrypt) = create(SessionKeyReg);
            header = await decrypt.ReadClientHeaderAsync(new MemoryStream(data));

            Assert.Multiple(() =>
            {
                Assert.That(header.Size, Is.EqualTo(size));
                Assert.That(header.Opcode, Is.EqualTo(opcode));
            });
            await encrypt.WriteClientHeaderAsync(new MemoryStream(data), size, opcode);

            Assert.That(data, Is.EqualTo(originalData));
        }
    }

    private async Task ServerHelper(string path, Func<byte[], (IServerDecrypter, IServerEncrypter)> create)
    {
        var contents = File.ReadAllLines(path);
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');

            var data = TestUtils.StringToByteArray(split[0]);
            var originalData = data.Clone();
            var size = uint.Parse(split[1]);
            var opcode = uint.Parse(split[2]);
            var (decrypt, encrypt) = create(SessionKeyReg);

            var header = decrypt.ReadServerHeader(data);

            Assert.Multiple(() =>
            {
                Assert.That(header.Size, Is.EqualTo(size));
                Assert.That(header.Opcode, Is.EqualTo(opcode));
            });
            encrypt.WriteServerHeader(data, size, opcode);

            Assert.That(data, Is.EqualTo(originalData));

            (decrypt, encrypt) = create(SessionKeyReg);
            header = decrypt.ReadServerHeader(new MemoryStream(data));

            Assert.Multiple(() =>
            {
                Assert.That(header.Size, Is.EqualTo(size));
                Assert.That(header.Opcode, Is.EqualTo(opcode));
            });
            encrypt.WriteServerHeader(new MemoryStream(data), size, opcode);

            Assert.That(data, Is.EqualTo(originalData));

            (decrypt, encrypt) = create(SessionKeyReg);
            header = await decrypt.ReadServerHeaderAsync(new MemoryStream(data));

            Assert.Multiple(() =>
            {
                Assert.That(header.Size, Is.EqualTo(size));
                Assert.That(header.Opcode, Is.EqualTo(opcode));
            });
            await encrypt.WriteServerHeaderAsync(new MemoryStream(data), size, opcode);

            Assert.That(data, Is.EqualTo(originalData));
        }
    }

    [Test]
    public async Task VanillaServer()
    {
        await ServerHelper("./tests/encryption/vanilla_server.txt",
            sessionKey => (new VanillaDecryption(sessionKey), new VanillaEncryption(sessionKey)));
    }

    [Test]
    public async Task VanillaClient()
    {
        await ClientHelper("./tests/encryption/vanilla_client.txt",
            sessionKey => (new VanillaDecryption(sessionKey), new VanillaEncryption(sessionKey)));
    }

    [Test]
    public async Task TbcServer()
    {
        await ServerHelper("./tests/encryption/tbc_server.txt",
            sessionKey => (new TbcDecryption(sessionKey), new TbcEncryption(sessionKey)));
    }

    [Test]
    public async Task TbcClient()
    {
        await ClientHelper("./tests/encryption/tbc_client.txt",
            sessionKey => (new TbcDecryption(sessionKey), new TbcEncryption(sessionKey)));
    }

    [Test]
    public async Task WrathServer()
    {
        await ServerHelper("./tests/encryption/wrath_server.txt",
            sessionKey => (new WrathServerDecryption(sessionKey), new WrathServerEncryption(sessionKey)));
    }

    [Test]
    public async Task WrathClient()
    {
        await ClientHelper("./tests/encryption/wrath_client.txt",
            sessionKey => (new WrathClientDecryption(sessionKey), new WrathClientEncryption(sessionKey)));
    }
}
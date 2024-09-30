using WowSrp;
using WowSrp.Internal;

namespace Tests;

public class ImplementationTest
{
    [Test]
    public void CalculateXSaltValues()
    {
        var contents = File.ReadAllLines("./tests/srp6_internal/calculate_x_salt_values.txt");
        const string username = "USERNAME123";
        const string password = "PASSWORD123";
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');
            var salt = TestUtils.StringToByteArray(split[0]);
            var expected = TestUtils.StringToByteArray(split[1]);
            var actual = Implementation.CalculateX(username, password, salt);

            Assert.That(actual, Is.EqualTo(expected));
        }
    }

    [Test]
    public void CalculateXValues()
    {
        var contents = File.ReadAllLines("./tests/srp6_internal/calculate_x_values.txt");
        var salt = TestUtils.StringToByteArray("CAC94AF32D817BA64B13F18FDEDEF92AD4ED7EF7AB0E19E9F2AE13C828AEAF57");
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');
            var username = split[0];
            var password = split[1];
            var expected = TestUtils.StringToByteArray(split[2]);
            var actual = Implementation.CalculateX(username, password, salt);

            Assert.That(actual, Is.EqualTo(expected));
        }
    }

    [Test]
    public void CalculatePasswordVerifier()
    {
        var contents = File.ReadAllLines("./tests/srp6_internal/calculate_v_values.txt");
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');
            var username = split[0];
            var password = split[1];
            var salt = TestUtils.StringToByteArray(split[2]);
            var expected = TestUtils.StringToByteArray(split[3]);
            var actual = Implementation.CalculatePasswordVerifier(username, password, salt);

            Assert.That(actual, Is.EqualTo(expected));
        }
    }

    [Test]
    public void CalculateServerPublicKey()
    {
        var contents = File.ReadAllLines("./tests/srp6_internal/calculate_B_values.txt");
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');
            var passwordVerifier = TestUtils.StringToByteArray(split[0]);
            var serverPrivateKey = TestUtils.StringToByteArray(split[1]);
            var expected = TestUtils.StringToByteArray(split[2]);
            var actual = Implementation.CalculateServerPublicKey(passwordVerifier, serverPrivateKey);

            Assert.That(actual, Is.EqualTo(expected));
        }
    }

    [Test]
    public void CalculateUValue()
    {
        var contents = File.ReadAllLines("./tests/srp6_internal/calculate_u_values.txt");
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');
            var clientPublicKey = TestUtils.StringToByteArray(split[0]);
            var serverPublicKey = TestUtils.StringToByteArray(split[1]);
            var expected = TestUtils.StringToByteArray(split[2]);
            var actual = Implementation.CalculateUValue(clientPublicKey, serverPublicKey);

            Assert.That(actual, Is.EqualTo(expected));
        }
    }

    [Test]
    public void CalculateServerSKey()
    {
        var contents = File.ReadAllLines("./tests/srp6_internal/calculate_S_values.txt");
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');

            var clientPublicKey = TestUtils.StringToByteArray(split[0]);
            var passwordVerifier = TestUtils.StringToByteArray(split[1]);
            var u = TestUtils.StringToByteArray(split[2]);
            var serverPrivateKey = TestUtils.StringToByteArray(split[3]);
            var expected = TestUtils.StringToByteArray(split[4]);
            var actual = Implementation.CalculateServerSKey(clientPublicKey, passwordVerifier, u, serverPrivateKey);

            Assert.That(actual, Is.EqualTo(expected));
        }
    }

    [Test]
    public void CalculateClientSKey()
    {
        var contents = File.ReadAllLines("./tests/srp6_internal/calculate_client_S_values.txt");
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');

            var serverPublicKey = TestUtils.StringToByteArray(split[0]);
            var clientPrivateKey = TestUtils.StringToByteArray(split[1]);
            var x = TestUtils.StringToByteArray(split[2]);
            var u = TestUtils.StringToByteArray(split[3]);
            var expected = TestUtils.StringToByteArray(split[4]);
            var actual = Implementation.CalculateClientSKey(clientPrivateKey, serverPublicKey, x, u,
                Constants.LargeSafePrimeLittleEndian, Constants.Generator);

            Assert.That(actual, Is.EqualTo(expected));
        }
    }

    [Test]
    public void SplitSKey()
    {
        var contents = File.ReadAllLines("./tests/srp6_internal/calculate_split_s_key.txt");
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');

            var sKey = TestUtils.StringToByteArray(split[0]);
            var expected = TestUtils.StringToByteArray(split[1]);

            var actual = Implementation.SplitSKey(sKey);

            Assert.That(actual, Is.EqualTo(expected));
        }
    }

    [Test]
    public void ShaInterleave()
    {
        var contents = File.ReadAllLines("./tests/srp6_internal/calculate_interleaved_values.txt");
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');

            var sKey = TestUtils.StringToByteArray(split[0]);
            var expected = TestUtils.StringToByteArray(split[1]);

            var actual = Implementation.ShaInterleave(sKey);

            Assert.That(actual, Is.EqualTo(expected));
        }
    }

    [Test]
    public void CalculateServerProof()
    {
        var contents = File.ReadAllLines("./tests/srp6_internal/calculate_M2_values.txt");
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');

            var clientPublicKey = TestUtils.StringToByteArray(split[0]);
            var clientProof = TestUtils.StringToByteArray(split[1]);
            var sessionKey = TestUtils.StringToByteArray(split[2]);
            var expected = TestUtils.StringToByteArray(split[3]);

            var actual = Implementation.CalculateServerProof(clientPublicKey, clientProof, sessionKey);

            Assert.That(actual, Is.EqualTo(expected));
        }
    }

    [Test]
    public void CalculateClientProof()
    {
        var contents = File.ReadAllLines("./tests/srp6_internal/calculate_M1_values.txt");
        Assert.That(contents, Is.Not.Empty);


        foreach (var line in contents)
        {
            var split = line.Split(' ');

            var username = split[0];
            var sessionKey = TestUtils.StringToByteArray(split[1]);
            var clientPublicKey = TestUtils.StringToByteArray(split[2]);
            var serverPublicKey = TestUtils.StringToByteArray(split[3]);
            var salt = TestUtils.StringToByteArray(split[4]);
            var expected = TestUtils.StringToByteArray(split[5]);

            var actual =
                Implementation.CalculateClientProof(username, sessionKey, clientPublicKey, serverPublicKey, salt,
                    Constants.Generator, Constants.LargeSafePrimeLittleEndian);

            Assert.That(actual, Is.EqualTo(expected));
        }
    }

    [Test]
    public void CalculateXorHash()
    {
        var contents = File.ReadAllLines("./tests/srp6_internal/calculate_xor_hash.txt");
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');

            var generator = byte.Parse(split[0]);
            var largeSafePrime = TestUtils.StringToByteArray(split[1]);
            var expected = TestUtils.StringToByteArray(split[2]);

            var actual =
                Implementation.CalculateXorHash(generator, largeSafePrime);

            Assert.That(actual, Is.EqualTo(expected));
        }
    }

    [Test]
    public void PreCalculatedXorHashMatches()
    {
        var calculated =
            Implementation.CalculateXorHash(Constants.Generator, Constants.LargeSafePrimeLittleEndian);
        Assert.That(Implementation.PreCalculatedXorHash, Is.EqualTo(calculated));
    }

    [Test]
    public void CalculateClientPublicKey()
    {
        var contents = File.ReadAllLines("./tests/srp6_internal/calculate_A_values.txt");
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');

            var clientPrivateKey = TestUtils.StringToByteArray(split[0]);
            var expected = TestUtils.StringToByteArray(split[1]);

            var actual =
                Implementation.CalculateClientPublicKey(clientPrivateKey, Constants.Generator,
                    Constants.LargeSafePrimeLittleEndian);

            Assert.That(actual, Is.EqualTo(expected));
        }
    }

    [Test]
    public void CalculateReconnectProof()
    {
        var contents = File.ReadAllLines("./tests/srp6_internal/calculate_reconnection_values.txt");
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');

            var username = split[0];
            var clientData = TestUtils.StringToByteArray(split[1]);
            var serverData = TestUtils.StringToByteArray(split[2]);
            var sessionKey = TestUtils.StringToByteArray(split[3]);
            var expected = TestUtils.StringToByteArray(split[4]);

            var actual =
                Implementation.CalculateReconnectProof(username, clientData, serverData, sessionKey);

            Assert.That(actual, Is.EqualTo(expected));
        }
    }

    [Test]
    public void CalculateWorldServerProof()
    {
        var contents = File.ReadAllLines("./tests/srp6_internal/calculate_world_server_proof.txt");
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');

            var username = split[0];
            var sessionKey = TestUtils.StringToByteArray(split[1]);
            var serverSeed = uint.Parse(split[2]);
            var clientSeed = uint.Parse(split[3]);
            var expected = TestUtils.StringToByteArray(split[4]);

            var actual =
                Implementation.CalculateWorldServerProof(username, clientSeed, serverSeed, sessionKey);

            Assert.That(actual, Is.EqualTo(expected));
        }
    }

    [Test]
    public void CalculateServerSessionKey()
    {
        var contents = File.ReadAllLines("./tests/srp6_internal/calculate_server_session_key.txt");
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');

            var clientPublicKey = TestUtils.StringToByteArray(split[0]);
            var passwordVerifier = TestUtils.StringToByteArray(split[1]);
            var serverPrivateKey = TestUtils.StringToByteArray(split[2]);
            var expected = TestUtils.StringToByteArray(split[3]);

            var serverPublicKey = Implementation.CalculateServerPublicKey(passwordVerifier, serverPrivateKey);

            var actual =
                Implementation.CalculateServerSessionKey(clientPublicKey, serverPublicKey, passwordVerifier,
                    serverPrivateKey);

            Assert.That(actual, Is.EqualTo(expected));
        }
    }

    [Test]
    public void CalculateClientSessionKey()
    {
        var contents = File.ReadAllLines("./tests/srp6_internal/calculate_client_session_key.txt");
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');

            var username = split[0];
            var password = split[1];
            var serverPublicKey = TestUtils.StringToByteArray(split[2]);
            var clientPrivateKey = TestUtils.StringToByteArray(split[3]);
            var generator = byte.Parse(split[4]);
            var largeSafePrime = TestUtils.StringToByteArray(split[5]);
            var clientPublicKey = TestUtils.StringToByteArray(split[6]);
            var salt = TestUtils.StringToByteArray(split[7]);
            var expected = TestUtils.StringToByteArray(split[8]);

            var actual =
                Implementation.CalculateClientSessionKey(username, password, serverPublicKey, clientPrivateKey,
                    generator,
                    largeSafePrime, clientPublicKey, salt);

            Assert.That(actual, Is.EqualTo(expected));
        }
    }

    [Test]
    public void WorldProofTest()
    {
        var contents = File.ReadAllLines("./tests/encryption/calculate_world_server_proof.txt");
        Assert.That(contents, Is.Not.Empty);

        foreach (var line in contents)
        {
            var split = line.Split(' ');

            var username = split[0];
            var sessionKey = TestUtils.StringToByteArray(split[1]);
            var serverSeed = uint.Parse(split[2]);
            var clientSeed = uint.Parse(split[3]);
            var expected = TestUtils.StringToByteArray(split[4]);

            var actual =
                WorldProof.CalculateProof(username, clientSeed, serverSeed, sessionKey);

            Assert.That(actual, Is.EqualTo(expected));
        }
    }
}
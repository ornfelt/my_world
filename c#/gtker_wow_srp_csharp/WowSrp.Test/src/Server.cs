using WowSrp;
using WowSrp.Client;
using WowSrp.Server;

namespace Tests;

public class Server
{
    private const string Username = "abc";
    private const string Password = "sdf";

    [Test]
    public void Basic()
    {
        var verifier = new SrpVerifier(Username, Password);

        var proof = verifier.IntoProof();
        var challenge = new SrpClientChallenge(Username, Password, Constants.Generator,
            Constants.LargeSafePrimeLittleEndian, proof.ServerPublicKey, proof.Salt);

        var (server, serverProof) = proof.IntoServer(challenge.ClientPublicKey, challenge.ClientProof)!.Value;

        var client = challenge.VerifyServerProof(serverProof)!.Value;
        Assert.That(server.SessionKey, Is.EqualTo(client.SessionKey));

        var serverReconnectChallengeData = new byte[Constants.ReconnectDataLength];
        server.ReconnectChallengeData.CopyTo(serverReconnectChallengeData, 0);

        var clientReconnect = client.CalculateReconnectValues(server.ReconnectChallengeData);
        var success = server.VerifyReconnectionAttempt(clientReconnect.ChallengeData, clientReconnect.ClientProof);

        Assert.Multiple(() =>
        {
            Assert.That(success, Is.EqualTo(true));
            Assert.That(serverReconnectChallengeData, Is.Not.EqualTo(server.ReconnectChallengeData));
        });
    }

    [Test]
    public void IncorrectUsage()
    {
        var verifier = new SrpVerifier(Username, Password);

        Assert.That(verifier.Username, Is.EqualTo("ABC"));

        var verifier2 = new SrpVerifier(verifier.Username, verifier.PasswordVerifier, verifier.Salt);
        Assert.Multiple(() =>
        {
            Assert.That(verifier2.Username, Is.EqualTo(verifier.Username));
            Assert.That(verifier2.Salt, Is.EqualTo(verifier.Salt));
            Assert.That(verifier2.PasswordVerifier, Is.EqualTo(verifier.PasswordVerifier));
        });

        Assert.Multiple(() =>
        {
            Assert.Throws<ArgumentException>(() => { _ = new SrpVerifier("AAAAAAAAAAAAAAAAA", "a"); });
            Assert.Throws<ArgumentException>(() => { _ = new SrpVerifier("a", "AAAAAAAAAAAAAAAAA"); });
        });

        Assert.Multiple(() =>
        {
            var wrongArray = new byte[1];
            var correctArray = new byte[32];
            Assert.Throws<ArgumentException>(() =>
            {
                _ = new SrpVerifier("AAAAAAAAAAAAAAAAA", correctArray, correctArray);
            });
            Assert.Throws<ArgumentException>(() => { _ = new SrpVerifier("A", correctArray, wrongArray); });
            Assert.Throws<ArgumentException>(() => { _ = new SrpVerifier("A", wrongArray, correctArray); });
        });
    }
}
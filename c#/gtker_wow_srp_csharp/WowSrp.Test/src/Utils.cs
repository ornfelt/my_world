namespace Tests;

public class Utils
{
    [Test]
    public void Endianness()
    {
        const int testValue = 0x2010;

        var a = WowSrp.Internal.Utils.ReadLittleEndian([0x10, 0x20]);
        Assert.That(a, Is.EqualTo(testValue));
        byte[] array = [0, 0];
        WowSrp.Internal.Utils.WriteLittleEndian(testValue, array);
        Assert.That(array, Is.EqualTo((byte[])[0x10, 0x20]));

        var c = WowSrp.Internal.Utils.ReadBigEndian([0x20, 0x10]);
        Assert.That(c, Is.EqualTo(testValue));
        WowSrp.Internal.Utils.WriteBigEndian(testValue, array);
        Assert.That(array, Is.EqualTo((byte[])[0x20, 0x10]));

        a = WowSrp.Internal.Utils.ReadLittleEndian([0x10, 0x20, 0x00]);
        Assert.That(a, Is.EqualTo(testValue));
        array = [0, 0, 0];
        WowSrp.Internal.Utils.WriteLittleEndian(testValue, array);
        Assert.That(array, Is.EqualTo((byte[])[0x10, 0x20, 0x00]));

        c = WowSrp.Internal.Utils.ReadBigEndian([0x00, 0x20, 0x10]);
        Assert.That(c, Is.EqualTo(testValue));
        WowSrp.Internal.Utils.WriteBigEndian(testValue, array);
        Assert.That(array, Is.EqualTo((byte[])[0x00, 0x20, 0x10]));
    }
}
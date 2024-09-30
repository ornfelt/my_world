package wow.sniffer.io;

import org.junit.Test;

import static org.junit.Assert.assertEquals;

public class PacketDataReaderTest {

    @Test
    public void revertLong() {
        assertEquals(PacketDataReader.revertLong(0x0513B67300000000L), 0x0000000073B61305L);
    }

    @Test
    public void revertShort() {
        assertEquals(PacketDataReader.revertShort((short) 0x0513), (short)0x1305);
    }
}
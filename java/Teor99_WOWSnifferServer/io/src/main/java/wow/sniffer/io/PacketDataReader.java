package wow.sniffer.io;

import java.io.DataInputStream;
import java.io.IOException;
import java.io.InputStream;

public class PacketDataReader implements AutoCloseable {
    private static final char[] HEX_ARRAY = "0123456789ABCDEF".toCharArray();
    private final DataInputStream dis;

    public PacketDataReader(InputStream inputStream) {
        this.dis = new DataInputStream(inputStream);
    }

    public int readIntE() throws IOException {
        return revertInt(dis.readInt());
    }

    public static int revertInt(int i) {
        int b1 = (i & 0xFF);
        int b2 = (i & 0xFF00);
        int b3 = (i & 0xFF0000);
        int b4 = (i & 0xFF000000);

        b1 <<= 24;
        b2 <<= 8;
        b3 >>= 8;
        b4 >>= 24;

        b1 &= 0xFF000000;
        b2 &= 0xFF0000;
        b3 &= 0xFF00;
        b4 &= 0xFF;

        return b1 | b2 | b3 | b4;
    }

    public byte readByte() throws IOException {
        return dis.readByte();
    }

    public String bytesToHexString() throws IOException {
        byte[] bytes = dis.readAllBytes();
        char[] hexChars = new char[bytes.length * 2];
        for (int j = 0; j < bytes.length; j++) {
            int v = bytes[j] & 0xFF;
            hexChars[j * 2] = HEX_ARRAY[v >>> 4];
            hexChars[j * 2 + 1] = HEX_ARRAY[v & 0x0F];
        }
        return new String(hexChars);
    }

    public short readShortE() throws IOException {
        return revertShort(dis.readShort());
    }

    public static short revertShort(short s) {
        short b1 = (short) (s & 0xFF);
        short b2 = (short) (s & 0xFF00);

        b1 <<= 8;
        b2 >>= 8;

        b1 &= 0xFF00L;
        b2 &= 0xFFL;

        return (short) (b1 | b2);
    }

    public long readLongE() throws IOException {
        return revertLong(dis.readLong());
    }

    public static long revertLong(long l) {
        long b1 = (l & 0xFFL);
        long b2 = (l & 0xFF00L);
        long b3 = (l & 0xFF0000L);
        long b4 = (l & 0xFF000000L);
        long b5 = (l & 0xFF00000000L);
        long b6 = (l & 0xFF0000000000L);
        long b7 = (l & 0xFF000000000000L);
        long b8 = (l & 0xFF00000000000000L);

        b1 <<= 56;
        b2 <<= 40;
        b3 <<= 24;
        b4 <<= 8;
        b5 >>= 8;
        b6 >>= 24;
        b7 >>= 40;
        b8 >>= 56;

        b1 &= 0xFF00000000000000L;
        b2 &= 0xFF000000000000L;
        b3 &= 0xFF0000000000L;
        b4 &= 0xFF00000000L;
        b5 &= 0xFF000000L;
        b6 &= 0xFF0000L;
        b7 &= 0xFF00L;
        b8 &= 0xFFL;

        return b1 | b2 | b3 | b4 | b5 | b6 | b7 | b8;
    }

    public void skip(int i) throws IOException {
        dis.skip(i);
    }

    @Override
    public void close() throws IOException {
        dis.close();
    }

    public String readCString() throws IOException {
        StringBuilder sb = new StringBuilder();

        while (true) {
            byte b = dis.readByte();
            if (b == 0) break;
            sb.append((char) b);
        }

        return sb.toString();
    }
}

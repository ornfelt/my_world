package wow.sniffer.net;

import wow.sniffer.io.PacketDataReader;

import java.io.IOException;
import java.io.InputStream;
import java.util.Date;

public class PacketReader implements AutoCloseable{
    private final PacketDataReader dataStream;

    public PacketReader(InputStream inputStream) {
        this.dataStream = new PacketDataReader(inputStream);
    }

    public Packet readPacket() throws IOException {
        Opcode opcode = Opcode.getOpcodeByValue(dataStream.readIntE());
        int packetSize = dataStream.readIntE();
        Date timestamp = new Date((long) dataStream.readIntE() * 1000);
        Direction direction = Direction.getDirectionByValue(dataStream.readByte());
        byte[] packetData = new byte[]{};

        if (packetSize > 0) {
            packetData = new byte[packetSize];
            for (int i = 0; i < packetData.length; i++) {
                packetData[i] = dataStream.readByte();
            }
        }

        return new Packet(opcode, packetSize, timestamp, direction, packetData);
    }

    public void close() throws IOException {
        dataStream.close();
    }
}

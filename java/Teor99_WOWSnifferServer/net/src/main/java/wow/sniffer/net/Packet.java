package wow.sniffer.net;

import wow.sniffer.io.PacketDataReader;

import java.io.ByteArrayInputStream;
import java.text.SimpleDateFormat;
import java.util.Date;

public class Packet {

    private final Direction direction;
    private final Opcode opcode;
    private final int size;
    private final Date timestamp;
    private final byte[] data;

    public Packet(Opcode opcode, int size, Date timestamp, Direction direction, byte[] data) {
        this.opcode = opcode;
        this.size = size;
        this.timestamp = timestamp;
        this.direction = direction;
        this.data = data;
    }

    @Override
    public String toString() {

        return direction.toString() +
                ": " + opcode.toString() +
                " Length: " + size +
                " Time: " + new SimpleDateFormat("MM/dd/yyyy HH:mm:ss.SSS").format(timestamp);
    }

    public Opcode getOpcode() {
        return opcode;
    }

    public int getSize() {
        return size;
    }

    public Date getTimestamp() {
        return timestamp;
    }

    public Direction getDirection() {
        return direction;
    }

    public byte[] getData() {
        return data;
    }

    public PacketDataReader getPacketDataReader() {
        return new PacketDataReader(new ByteArrayInputStream(data));
    }
}

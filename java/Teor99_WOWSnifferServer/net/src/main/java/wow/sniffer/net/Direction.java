package wow.sniffer.net;

public enum Direction {
    Unknown(-1),
    ClientToServer(0),
    ServerToClient(1),
    BNClientToServer(2),
    BNServerToClient(3),
    Bidirectional(4);

    Direction(int code) {
        this.value = code;
    }

    private final int value;

    public int getValue() {
        return value;
    }

    public static Direction getDirectionByValue(int code) {
        for (Direction value : values()) {
            if (value.value == code) {
                return value;
            }
        }

        return Unknown;
    }

    @Override
    public String toString() {
        return name();
    }
}

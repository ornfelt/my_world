package wow.sniffer.game;

public enum AuctionFaction {
    ALLIANCE,
    HORDE;

    public static AuctionFaction getFactionByCode(int code) {
        switch (code) {
            case 1:
            case 2:
                return ALLIANCE;
            case 5:
                return HORDE;
        }

        throw new IllegalArgumentException("Unknown auction faction id: " + code);
    }
}

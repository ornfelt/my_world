package wow.sniffer.game.mail;

public enum MailType {
    NORMAL,
    AUCTION,
    CREATURE,
    GAMEOBJECT,
    ITEM;

    public static MailType getMailTypeByCode(byte code) {
        switch(code) {
            case 0: return NORMAL;
            case 2: return AUCTION;
            case 3: return CREATURE;
            case 4: return GAMEOBJECT;
            case 5: return ITEM;
        }

        throw new IllegalArgumentException("Unknown mail code: " + code);
    }
}

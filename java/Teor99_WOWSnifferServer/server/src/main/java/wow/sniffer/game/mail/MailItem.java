package wow.sniffer.game.mail;

public class MailItem {
    private final int guid;
    private final int id;
    private final int count;

    public MailItem(int guid, int id, int count) {
        this.guid = guid;
        this.id = id;
        this.count = count;
    }

    public int getGuid() {
        return guid;
    }

    public int getId() {
        return id;
    }

    public int getCount() {
        return count;
    }
}

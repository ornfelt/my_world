package wow.sniffer.game.mail;

import java.util.List;

public class Mail {
    private final int id;
    private final short size;
    private final MailType mailType;
    private final long playerGUID;
    private final int entry;
    private final int cod;
    private final int packageId;
    private final int stationery;
    private final int money;
    private final int flags;
    private final int time;
    private final int templateId;
    private final String subject;
    private final String body;
    private final List<MailItem> attachedItems;

    public Mail(int id, short size, MailType mailType, long playerGUID, int entry, int cod, int packageId, int stationery, int money, int flags, int time, int templateId, String subject, String body, List<MailItem> attachedItems) {
        this.id = id;
        this.size = size;
        this.mailType = mailType;
        this.playerGUID = playerGUID;
        this.entry = entry;
        this.cod = cod;
        this.packageId = packageId;
        this.stationery = stationery;
        this.money = money;
        this.flags = flags;
        this.time = time;
        this.templateId = templateId;
        this.subject = subject;
        this.body = body;
        this.attachedItems = attachedItems;
    }

    public int getId() {
        return id;
    }

    public short getSize() {
        return size;
    }

    public MailType getMailType() {
        return mailType;
    }

    public long getPlayerGUID() {
        return playerGUID;
    }

    public int getEntry() {
        return entry;
    }

    public int getCod() {
        return cod;
    }

    public int getPackageId() {
        return packageId;
    }

    public int getStationery() {
        return stationery;
    }

    public int getMoney() {
        return money;
    }

    public int getFlags() {
        return flags;
    }

    public int getTime() {
        return time;
    }

    public int getTemplateId() {
        return templateId;
    }

    public String getSubject() {
        return subject;
    }

    public String getBody() {
        return body;
    }

    public List<MailItem> getAttachedItems() {
        return attachedItems;
    }
}

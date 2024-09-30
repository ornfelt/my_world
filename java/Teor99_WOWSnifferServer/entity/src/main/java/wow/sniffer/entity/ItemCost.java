package wow.sniffer.entity;

import javax.persistence.Column;
import javax.persistence.EmbeddedId;
import javax.persistence.Entity;
import java.util.Date;

@Entity
public class ItemCost {

    @EmbeddedId
    private ItemSource itemSource;

    private Integer price;

    @Column(columnDefinition = "TIMESTAMP")
    private Date lastScan;

    public ItemCost(ItemSource itemSource, Integer price, Date lastScan) {
        this.itemSource = itemSource;
        this.price = price;
        this.lastScan = lastScan;
    }

    public ItemCost() {
    }

    public Integer getId() {
        return itemSource.getId();
    }

    public String getSource() {
        return itemSource.getSource();
    }

    public Integer getPrice() {
        return price;
    }

    public void setPrice(Integer price) {
        this.price = price;
    }

    public Date getLastScan() {
        return lastScan;
    }

    public void setLastScan(Date lastScan) {
        this.lastScan = lastScan;
    }
}

package wow.sniffer.entity;

import javax.persistence.EmbeddedId;
import javax.persistence.Entity;

@Entity
public class ItemForSale {
    @EmbeddedId
    private ItemForSaleId itemForSaleId;

    private Integer count;

    private Integer minBuyout;

    public ItemForSale() {
    }

    public ItemForSale(ItemForSaleId itemForSaleId, Integer count, Integer minBuyout) {
        this.itemForSaleId = itemForSaleId;
        this.count = count;
        this.minBuyout = minBuyout;
    }

    public ItemForSaleId getItemForSaleId() {
        return itemForSaleId;
    }

    public void setItemForSaleId(ItemForSaleId itemForSaleId) {
        this.itemForSaleId = itemForSaleId;
    }

    public Integer getCount() {
        return count;
    }

    public void setCount(Integer count) {
        this.count = count;
    }

    public Integer getMinBuyout() {
        return minBuyout;
    }

    public void setMinBuyout(Integer minBuyout) {
        this.minBuyout = minBuyout;
    }
}

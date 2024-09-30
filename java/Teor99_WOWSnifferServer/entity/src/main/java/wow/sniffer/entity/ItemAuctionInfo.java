package wow.sniffer.entity;

import javax.persistence.Column;
import javax.persistence.Embeddable;
import java.util.Date;

@Embeddable
public class ItemAuctionInfo {

    @Column(columnDefinition = "TIMESTAMP")
    private Date timestamp;

    private Integer totalCount;
    private Integer auctionCount;
    private Integer minBuyout;

    public ItemAuctionInfo() {
    }

    public Date getTimestamp() {
        return timestamp;
    }

    public void setTimestamp(Date timestamp) {
        this.timestamp = timestamp;
    }

    public Integer getTotalCount() {
        return totalCount;
    }

    public void setTotalCount(Integer totalCount) {
        this.totalCount = totalCount;
    }

    public Integer getAuctionCount() {
        return auctionCount;
    }

    public void setAuctionCount(Integer auctionCount) {
        this.auctionCount = auctionCount;
    }

    public Integer getMinBuyout() {
        return minBuyout;
    }

    public void setMinBuyout(Integer minBuyout) {
        this.minBuyout = minBuyout;
    }
}

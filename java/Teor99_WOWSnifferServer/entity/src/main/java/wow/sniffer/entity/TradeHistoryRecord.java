package wow.sniffer.entity;

import com.sun.istack.NotNull;

import javax.persistence.*;
import java.util.Date;

@Entity
public class TradeHistoryRecord {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Integer recordId;

    @NotNull
    private Integer itemId;
    @NotNull
    @Column(columnDefinition = "TIMESTAMP")
    private Date timestamp;

    @NotNull
    @Column(length = 16)
    private String action;

    @NotNull
    private Integer count;
    @NotNull
    private Integer cost;

    public TradeHistoryRecord() {
    }

    public TradeHistoryRecord(Integer itemId, Date timestamp, String action, Integer count, Integer cost) {
        this.itemId = itemId;
        this.timestamp = timestamp;
        this.action = action;
        this.count = count;
        this.cost = cost;
    }

    public Integer getItemId() {
        return itemId;
    }

    public Date getTimestamp() {
        return timestamp;
    }

    public String getAction() {
        return action;
    }

    public Integer getCount() {
        return count;
    }

    public Integer getCost() {
        return cost;
    }
}

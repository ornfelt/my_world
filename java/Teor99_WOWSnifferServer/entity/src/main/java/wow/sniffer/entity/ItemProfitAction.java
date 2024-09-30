package wow.sniffer.entity;

import com.sun.istack.NotNull;

import javax.persistence.*;
import java.util.Date;

@Entity
@Table(indexes = {@Index(name = "item_id_index1", columnList = "id")})
public class ItemProfitAction {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Integer recordId;
    @NotNull
    private Integer id;
    @Column(length = 64)
    private String action;
    @Column(name = "a")
    private Integer allianceMinBuyout;
    @Column(name = "h")
    private Integer hordeMinBuyout;
    private Integer profit;

    @Column(columnDefinition = "TEXT")
    private String comment;

    @Column(columnDefinition = "TIMESTAMP")
    private Date recordTimestamp;

    public ItemProfitAction() {
    }

    public ItemProfitAction(Integer id, String action, Integer allianceMinBuyout, Integer hordeMinBuyout, Integer profit, String comment, Date recordTimestamp) {
        this.id = id;
        this.action = action;
        this.allianceMinBuyout = allianceMinBuyout;
        this.hordeMinBuyout = hordeMinBuyout;
        this.profit = profit;
        this.comment = comment;
        this.recordTimestamp = recordTimestamp;
    }
}

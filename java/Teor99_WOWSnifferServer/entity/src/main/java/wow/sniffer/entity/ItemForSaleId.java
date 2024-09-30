package wow.sniffer.entity;

import javax.persistence.Embeddable;
import javax.persistence.ManyToOne;
import java.io.Serializable;

@Embeddable
public class ItemForSaleId implements Serializable {
    @ManyToOne
    private GameCharacter gameCharacter;
    private Integer item_id;
    private String source;

    public ItemForSaleId(GameCharacter gameCharacter, Integer item_id, String source) {
        this.gameCharacter = gameCharacter;
        this.item_id = item_id;
        this.source = source;
    }

    public ItemForSaleId() {
    }

    public GameCharacter getGameCharacter() {
        return gameCharacter;
    }

    public void setGameCharacter(GameCharacter gameCharacter) {
        this.gameCharacter = gameCharacter;
    }

    public Integer getItem_id() {
        return item_id;
    }

    public void setItem_id(Integer item_id) {
        this.item_id = item_id;
    }

    public String getSource() {
        return source;
    }

    public void setSource(String source) {
        this.source = source;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        ItemForSaleId that = (ItemForSaleId) o;

        if (!gameCharacter.equals(that.gameCharacter)) return false;
        if (!item_id.equals(that.item_id)) return false;
        return source.equals(that.source);
    }

    @Override
    public int hashCode() {
        int result = gameCharacter.hashCode();
        result = 31 * result + item_id.hashCode();
        result = 31 * result + source.hashCode();
        return result;
    }
}

package wow.sniffer.entity;

import javax.persistence.Embeddable;
import java.io.Serializable;

@Embeddable
public class ItemSource implements Serializable {
    private Integer id;
    private String source;

    public ItemSource() {
    }

    public ItemSource(Integer id, String source) {
        this.id = id;
        this.source = source;
    }

    public Integer getId() {
        return id;
    }

    public void setId(Integer id) {
        this.id = id;
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

        ItemSource that = (ItemSource) o;

        if (!id.equals(that.id)) return false;
        return source.equals(that.source);
    }

    @Override
    public int hashCode() {
        int result = id.hashCode();
        result = 31 * result + source.hashCode();
        return result;
    }
}

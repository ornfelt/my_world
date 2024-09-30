package wow.sniffer.entity;

import javax.persistence.*;

@Entity
public class Component {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Integer componentId;

    @ManyToOne
    @JoinColumn(name = "spell_id")
    Spell spell;

    @ManyToOne
    @JoinColumn(name = "item_id")
    Item item;

    @Column(nullable = false)
    private Integer count;

    public Component() {
    }

    public Component(Spell spell, Item item, Integer count) {
        this.spell = spell;
        this.item = item;
        this.count = count;
    }

    public Spell getSpell() {
        return spell;
    }

    public void setSpell(Spell spell) {
        this.spell = spell;
    }

    public Item getItem() {
        return item;
    }

    public void setItem(Item item) {
        this.item = item;
    }

    public Integer getCount() {
        return count;
    }

    public void setCount(Integer count) {
        this.count = count;
    }

    @Override
    public String toString() {
        return count + " x (" + item.getItemId() + ") " + getItem().getName();
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        Component component = (Component) o;

        if (!item.equals(component.item)) return false;
        return count.equals(component.count);
    }

    @Override
    public int hashCode() {
        int result = item.hashCode();
        result = 31 * result + count.hashCode();
        return result;
    }
}

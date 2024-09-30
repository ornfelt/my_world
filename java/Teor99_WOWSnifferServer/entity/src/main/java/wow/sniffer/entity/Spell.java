package wow.sniffer.entity;

import javax.persistence.*;
import java.util.List;
import java.util.Set;

@Entity
public class Spell {

    @Id
    private Integer spellId;

    @Column(nullable = false)
    private String name;

    @OneToOne
    @JoinColumn(name = "craft_item_id")
    private Item craftItem;

    @Column(columnDefinition = "TINYINT(1) DEFAULT 0", nullable = false)
    private boolean autoUpdate;

    @Column(nullable = false)
    private Integer craftItemCount;

    @Column(nullable = false)
    private Long castTime;

    @Column(nullable = false)
    private Long cooldownTime;

    @Column(nullable = false)
    private Integer level;

    @OneToMany(mappedBy = "spell")
    private Set<Component> components;

    @ManyToMany
    @JoinTable(
            name = "spell_need_subspell",
            joinColumns = @JoinColumn(name = "spell_id"),
            inverseJoinColumns = @JoinColumn(name = "subspell_id"))
    private List<Spell> subSpellSet;

    @ManyToMany
    @JoinTable(
            name = "spell_has_alt_spell",
            joinColumns = @JoinColumn(name = "spell_id"),
            inverseJoinColumns = @JoinColumn(name = "alt_spell_id"))
    private Set<Spell> altSpellSet;

    public Spell() {
    }

    public Spell(Integer spellId, String name, Set<Component> components) {
        this.spellId = spellId;
        this.name = name;
        this.components = components;
    }

    public Spell(Integer spellId, String name, Item craftItem, boolean autoUpdate, Integer craftItemCount, Long castTime, Long cooldownTime, Integer level, Set<Component> components, List<Spell> subSpellSet, Set<Spell> altSpellSet) {
        this.spellId = spellId;
        this.name = name;
        this.craftItem = craftItem;
        this.autoUpdate = autoUpdate;
        this.craftItemCount = craftItemCount;
        this.castTime = castTime;
        this.cooldownTime = cooldownTime;
        this.level = level;
        this.components = components;
        this.subSpellSet = subSpellSet;
        this.altSpellSet = altSpellSet;
    }

    public Set<Spell> getAltSpellSet() {
        return altSpellSet;
    }

    public void setAltSpellSet(Set<Spell> altSpellSet) {
        this.altSpellSet = altSpellSet;
    }

    public Integer getSpellId() {
        return spellId;
    }

    public void setSpellId(Integer spellId) {
        this.spellId = spellId;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public Item getCraftItem() {
        return craftItem;
    }

    public void setCraftItem(Item craftItem) {
        this.craftItem = craftItem;
    }

    public boolean isAutoUpdate() {
        return autoUpdate;
    }

    public void setAutoUpdate(boolean autoUpdate) {
        this.autoUpdate = autoUpdate;
    }

    public Integer getCraftItemCount() {
        return craftItemCount;
    }

    public void setCraftItemCount(Integer craftItemCount) {
        this.craftItemCount = craftItemCount;
    }

    public Long getCastTime() {
        return castTime;
    }

    public void setCastTime(Long castTime) {
        this.castTime = castTime;
    }

    public Long getCooldownTime() {
        return cooldownTime;
    }

    public void setCooldownTime(Long cooldownTime) {
        this.cooldownTime = cooldownTime;
    }

    public Integer getLevel() {
        return level;
    }

    public void setLevel(Integer level) {
        this.level = level;
    }

    public Set<Component> getComponents() {
        return components;
    }

    public void setComponents(Set<Component> components) {
        this.components = components;
    }

    public List<Spell> getSubSpellSet() {
        return subSpellSet;
    }

    public void setSubSpellSet(List<Spell> subSpellSet) {
        this.subSpellSet = subSpellSet;
    }

    @Override
    public String toString() {
        return spellId + " - " + name;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        Spell spell = (Spell) o;

        if (!name.equals(spell.name)) return false;
        if (!craftItem.equals(spell.craftItem)) return false;
        if (!craftItemCount.equals(spell.craftItemCount)) return false;
        if (!castTime.equals(spell.castTime)) return false;
        if (!cooldownTime.equals(spell.cooldownTime)) return false;
        if (!level.equals(spell.level)) return false;
        return components.equals(spell.components);
    }

    @Override
    public int hashCode() {
        int result = name.hashCode();
        result = 31 * result + craftItem.hashCode();
        result = 31 * result + craftItemCount.hashCode();
        result = 31 * result + castTime.hashCode();
        result = 31 * result + cooldownTime.hashCode();
        result = 31 * result + level.hashCode();
        result = 31 * result + components.hashCode();
        return result;
    }
}


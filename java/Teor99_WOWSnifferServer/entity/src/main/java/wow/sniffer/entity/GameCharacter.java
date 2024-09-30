package wow.sniffer.entity;

import javax.persistence.*;
import java.util.List;

@Entity
public class GameCharacter {
    @Id
    private Long charId;
    private String name;

    @ManyToMany
    @JoinTable(
            name = "game_character_spell_set",
            joinColumns = @JoinColumn(name = "char_id"),
            inverseJoinColumns = @JoinColumn(name = "spell_id"))
    private List<Spell> spellSet;

    public GameCharacter() {
    }

    public GameCharacter(Long charId, String name) {
        this.charId = charId;
        this.name = name;
    }

    @Override
    public String toString() {
        return "guid: " + Long.toHexString(charId) + " name: " + name;
    }

    public long getCharId() {
        return charId;
    }

    public String getName() {
        return name;
    }

    public List<Spell> getSpellList() {
        return spellSet;
    }

    public void setSpellSet(List<Spell> spellSet) {
        this.spellSet = spellSet;
    }
}

package wow.sniffer;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.CommandLineRunner;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import wow.sniffer.dao.SpellDAO;
import wow.sniffer.entity.Component;
import wow.sniffer.entity.Spell;

import java.time.Duration;
import java.time.Instant;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.stream.Collectors;

@SpringBootApplication
public class AltSpellSearcher implements CommandLineRunner {

    private final Logger log = LoggerFactory.getLogger(AltSpellSearcher.class);

    @Autowired
    private SpellDAO spellDAO;

    private List<Spell> origSpells;

    @Override
    public void run(String... args) {
        log.info("Load spells from DB");
        Instant before = Instant.now();

        origSpells = spellDAO.findAllSpells();

        log.info("Start search alternative spells");
//        for (Spell spell : origSpellSet.stream().filter(spell -> spell.getSpellId().equals(29654)).collect(Collectors.toSet())) {
        for (Spell spell : origSpells) {
            processAltSpells(spell);
        }

        log.info("All operations time: " + Duration.between(before, Instant.now()));
    }

    private void processAltSpells(Spell spell) {
        log.info(spell.toString());
        Set<Spell> foundedAltSpellSet = new HashSet<>();
        int sizeBefore;

        do {
            sizeBefore = foundedAltSpellSet.size();
            int hashCode = spell.hashCode();
            processTempSpellSet(spell, foundedAltSpellSet);
            if (hashCode != spell.hashCode()) throw new IllegalArgumentException(spell + " spell has been modified in alt spells search (not allowed modify orig spell)");
        } while (foundedAltSpellSet.size() != sizeBefore);

        if (foundedAltSpellSet.isEmpty()) return;

        if (foundedAltSpellSet.size() > 900) throw new IllegalArgumentException("tempSpellSet.size() > 900");

        int subSpellId = spell.getSpellId() * 1000;
        for (Spell subSpell : foundedAltSpellSet) {
            subSpell.setSpellId(subSpellId++);
        }
        spell.setAltSpellSet(foundedAltSpellSet);
        log.info("found alt spells: " + foundedAltSpellSet.size());

        spellDAO.saveNewSpell(spell);
    }

    private void processTempSpellSet(Spell rootSpell, Set<Spell> tempSpellSet) {
        if (tempSpellSet.size() == 0) {
            tempSpellSet.addAll(findAltSpellsForSpell(rootSpell));
        } else {
            Set<Spell> spellForAdd = new HashSet<>();
            for (Spell spell : tempSpellSet) {
                spellForAdd.addAll(findAltSpellsForSpell(spell));
            }
            tempSpellSet.addAll(spellForAdd);
        }
    }

    private Set<Spell> findAltSpellsForSpell(Spell spell) {
        Set<Spell> resultSet = new HashSet<>();

        if (spell.getSubSpellSet().size() == 3) return resultSet;

        for (Component component : spell.getComponents()) {
            for (Spell subSpell : getCraftSpellsForItem(component.getItem().getItemId())) {
                // skip if sub spell already exist in set
                if (spell.getSubSpellSet().stream().anyMatch(spell1 -> spell1.getSpellId().equals(subSpell.getSpellId()))) {
                    continue;
                }

                resultSet.add(getNewSpellForComponent(spell, component, subSpell));
            }
        }

        return resultSet;
    }

    private Spell getNewSpellForComponent(Spell spell, Component ignoredComp, Spell subSpell) {
        Spell newSpell = new Spell();
        newSpell.setSpellId(spell.getSpellId());
        newSpell.setName(spell.getName());
        newSpell.setCraftItem(spell.getCraftItem());
        newSpell.setAutoUpdate(false);
        newSpell.setLevel(spell.getLevel());

        int multiplier = ignoredComp.getCount() / subSpell.getCraftItemCount();

        int craftCountMultiplier = 1;
        int oldComponentMultiplier = 1;
        int newComponentMultiplier = 1;

        if (multiplier == 0) {
            craftCountMultiplier = subSpell.getCraftItemCount();
            oldComponentMultiplier = subSpell.getCraftItemCount();
        } else if (multiplier > 1) {
            newComponentMultiplier = multiplier;
        }

        // cast time
        newSpell.setCastTime(spell.getCastTime() + subSpell.getCastTime() * newComponentMultiplier);
        // cooldown time
        newSpell.setCooldownTime(spell.getCooldownTime() + subSpell.getCooldownTime() * newComponentMultiplier);
        // craft count
        newSpell.setCraftItemCount(spell.getCraftItemCount() * craftCountMultiplier);

        // components
        // old
        Set<Component> componentSet = new HashSet<>();
        for (Component sc : spell.getComponents()) {
            if (sc == ignoredComp) continue;
            componentSet.add(new Component(newSpell, sc.getItem(), sc.getCount() * oldComponentMultiplier));
        }

        // new
        for (Component ssc : subSpell.getComponents()) {
            componentSet.add(new Component(newSpell, ssc.getItem(), ssc.getCount() * newComponentMultiplier));
        }
        newSpell.setComponents(componentSet);

        // subspells
        newSpell.setSubSpellSet(spell.getSubSpellSet() == null ? new ArrayList<>() : new ArrayList<>(spell.getSubSpellSet()));
        newSpell.getSubSpellSet().add(0, subSpell);

        return newSpell;
    }

    private Set<Spell> getCraftSpellsForItem(Integer itemId) {
        return origSpells.stream()
                .filter(spell -> spell.getCraftItem().getItemId().equals(itemId))
                .collect(Collectors.toSet());
    }

    public static void main(String[] args) {
        SpringApplication.run(AltSpellSearcher.class, args);
    }
}

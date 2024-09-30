package wow.sniffer.dao;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Repository;
import wow.sniffer.entity.Component;
import wow.sniffer.entity.Spell;

import javax.persistence.EntityManager;
import javax.persistence.PersistenceContext;
import javax.transaction.Transactional;
import java.util.List;
import java.util.Set;

@Repository
public class SpellDAO {
    private final Logger log = LoggerFactory.getLogger(SpellDAO.class);

    @PersistenceContext
    private EntityManager entityManager;

    @Transactional
    public List<Spell> findAllSpells() {
        List<Spell> all = entityManager.createQuery("SELECT s FROM Spell s", Spell.class).getResultList();

        all.forEach(spell -> {
            spell.getComponents().forEach(Component::getItem);
            spell.getSubSpellSet().size();
        });

        return all;
    }

    @Transactional
    public void saveNewSpell(Spell spell) {
        for (Spell altSpell : spell.getAltSpellSet()) {
            Set<Component> components = altSpell.getComponents();
            altSpell.setComponents(null);
            entityManager.persist(altSpell);
            for (Component component : components) {
                entityManager.persist(component);
            }
            altSpell.setComponents(components);
            entityManager.merge(altSpell);
        }
        entityManager.merge(spell);
    }
}

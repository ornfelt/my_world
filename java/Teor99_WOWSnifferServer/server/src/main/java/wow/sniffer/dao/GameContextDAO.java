package wow.sniffer.dao;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Repository;
import wow.sniffer.entity.*;

import javax.persistence.EntityManager;
import javax.persistence.PersistenceContext;
import javax.transaction.Transactional;
import java.util.List;

@Repository
public class GameContextDAO {

    private final Logger log = LoggerFactory.getLogger(GameContextDAO.class);

    @PersistenceContext
    private EntityManager entityManager;

    @Transactional
    public void saveTradeHistoryRecord(TradeHistoryRecord tradeHistoryRecord) {
        entityManager.persist(tradeHistoryRecord);
    }

    @Transactional
    public void deleteOldItemCostRecords() {
        entityManager.createNativeQuery("DELETE FROM item_cost WHERE last_scan IS NOT NULL AND TIMESTAMPDIFF(MINUTE, last_scan, now()) >= 60").executeUpdate();
    }

    @Transactional
    public void deleteOldItemProfitActionRecords() {
        entityManager.createNativeQuery("DELETE FROM item_profit_action WHERE TIMESTAMPDIFF(MINUTE, record_timestamp, now()) >= 60").executeUpdate();
    }

    @Transactional
    public void saveItemHistoryList(List<ItemHistory> itemHistoryList) {
        itemHistoryList.forEach(entityManager::persist);
    }

    @Transactional
    public void saveItemCostList(List<ItemCost> itemCostList) {
        itemCostList.forEach(entityManager::merge);
    }

    @Transactional
    public void updateGameCharacterList(List<GameCharacter> charList) {
        charList.forEach(gameCharacter -> {
            GameCharacter gc = entityManager.find(GameCharacter.class, gameCharacter.getCharId());
            if (gc == null) {
                entityManager.persist(gameCharacter);
            }
        });
    }

    @Transactional
    public void updateGameCharacterSpellIdList(Long playerGUID, List<Integer> spellIdList) {
        GameCharacter player = entityManager.find(GameCharacter.class, playerGUID);
        player.getSpellList().clear();
        spellIdList.forEach(spellId -> {
            Spell spell = entityManager.find(Spell.class, spellId);
            if (spell != null) {
                player.getSpellList().add(spell);
            }
        });

        log.info(player.toString() + " update spell count: " + player.getSpellList().size());
    }

    @Transactional
    public void deleteAllItemProfitActionRecords() {
        entityManager.createNativeQuery("TRUNCATE TABLE item_profit_action").executeUpdate();
    }

    public List<ItemCost> getAllItemCostList() {
        return entityManager.createQuery("SELECT ic FROM ItemCost ic", ItemCost.class).getResultList();
    }

    @Transactional
    public void updateItemProfitActionList(List<ItemProfitAction> itemProfitActions) {
        itemProfitActions.forEach(entityManager::merge);
    }

    @Transactional
    public List<Spell> getAutoUpdateCraftSpells() {
        List<Spell> resultList = entityManager.createQuery("SELECT s FROM Spell s WHERE autoUpdate = TRUE", Spell.class).getResultList();
        resultList.forEach(spell -> {
            spell.getComponents().forEach(Component::getItem);
            spell.getSubSpellSet().forEach(Spell::getSpellId);
            spell.getAltSpellSet().forEach(spell1 -> {
                spell1.getComponents().forEach(Component::getItem);
                spell1.getSubSpellSet().forEach(Spell::getSpellId);
            });
        });

        return resultList;
    }

    public GameCharacter getGameCharacterById(Long playerGUID) {
        return entityManager.find(GameCharacter.class, playerGUID);
    }

    @Transactional
    public void updateItemForSaleList(List<ItemForSale> items) {
        if (items.isEmpty()) return;

        long charId = items.get(0)
                .getItemForSaleId()
                .getGameCharacter()
                .getCharId();

        entityManager.createNativeQuery("DELETE FROM item_for_sale WHERE game_character_char_id = :char_id")
                .setParameter("char_id", charId)
                .executeUpdate();

        items.forEach(entityManager::merge);
    }
}

package wow.sniffer;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.annotation.Scope;
import wow.sniffer.dao.GameContextDAO;
import wow.sniffer.entity.*;
import wow.sniffer.game.AuctionFaction;
import wow.sniffer.game.AuctionRecord;
import wow.sniffer.game.GameContext;
import wow.sniffer.game.mail.Mail;
import wow.sniffer.game.mail.MailItem;
import wow.sniffer.game.mail.MailType;
import wow.sniffer.io.PacketDataReader;
import wow.sniffer.net.Direction;
import wow.sniffer.net.Packet;

import java.io.IOException;
import java.time.Duration;
import java.time.Instant;
import java.util.*;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.atomic.AtomicReference;
import java.util.stream.Collectors;

@org.springframework.stereotype.Component
@Scope("prototype")
public class PacketHandler extends Thread {

    private final Logger log = LoggerFactory.getLogger(PacketHandler.class);
    private BlockingQueue<Packet> queue;
    private final GameContext gameContext;

    @Autowired
    private GameContextDAO gameContextDAO;

    public PacketHandler() {
        gameContext = new GameContext();
    }

    @Override
    public void run() {
        log.info("Begin game data processing");

        try {
            while (!isInterrupted()) {
                Packet packet = queue.take();
                handlePacket(packet);
            }
        } catch (InterruptedException ignored) {
        }
        log.info("End of game data processing");
    }

    private void handlePacket(Packet packet) {

        try {
            switch (packet.getOpcode()) {
                case CMSG_AUTH_SESSION:
                    cmsgAuthSession(packet);
                    break;
                case SMSG_ENUM_CHARACTERS_RESULT:
                    smsgEnumCharactersResult(packet);
                    break;
                case CMSG_PLAYER_LOGIN:
                    cmsgPlayerLogin(packet);
                    break;
                case SMSG_SEND_KNOWN_SPELLS:
                    smsgSendKnownSpells(packet);
                    break;
                case CMSG_AUCTION_LIST_ITEMS:
                    cmsgAuctionListItems(packet);
                    break;
                case SMSG_AUCTION_LIST_RESULT:
                    smsgAuctionListResult(packet);
                    break;
                case MSG_AUCTION_HELLO:
                    if (packet.getDirection() == Direction.ServerToClient) smsgAuctionHello(packet);
                    break;
                case SMSG_MAIL_LIST_RESULT:
                    smsgMailListResult(packet);
                    break;
                case SMSG_MAIL_COMMAND_RESULT:
                    smsgMailCommandResult(packet);
                    break;
                case SMSG_AUCTION_LIST_OWNER_ITEMS_RESULT:
                    smsgAuctionListOwnerItemsResult(packet);
                    break;
            }
        } catch (Exception e) {
            log.error(packet.toString());
            log.error("Error while packet handling ", e);
        }
    }

    private void smsgAuctionListOwnerItemsResult(Packet packet) throws IOException {
        try (PacketDataReader packetData = packet.getPacketDataReader()) {
            int count = packetData.readIntE();
            List<AuctionRecord> auctionOwnerItemRecords = new ArrayList<>();
            for (int i = 0; i < count; i++) {
                AuctionRecord auctionRecord = readAuctionRecord(packetData);
                auctionOwnerItemRecords.add(auctionRecord);
            }

            List<ItemForSale> items = new ArrayList<>();
            Map<Integer, List<AuctionRecord>> map = auctionOwnerItemRecords.stream()
                    .collect(Collectors.groupingBy(AuctionRecord::getId));

            map.forEach((itemId, auctionRecords) -> {
                AtomicReference<Integer> minBuyoutPerItem = new AtomicReference<>(Integer.MAX_VALUE);
                auctionRecords.stream()
                        .min(Comparator.comparing(AuctionRecord::getBuyoutPerItem))
                        .ifPresent(auctionRecord -> minBuyoutPerItem.set(auctionRecord.getBuyoutPerItem()));

                int itemCount = auctionRecords.stream().mapToInt(AuctionRecord::getCount).sum();
                GameCharacter gameCharacter = gameContextDAO.getGameCharacterById(gameContext.getPlayerGUID());
                String source = null;
                if (gameContext.getAuctionFaction() == AuctionFaction.ALLIANCE) {
                    source = "alliance_auction";
                } else if (gameContext.getAuctionFaction() == AuctionFaction.HORDE) {
                    source = "horde_auction";
                }

                items.add(new ItemForSale(new ItemForSaleId(gameCharacter, itemId, source), itemCount, minBuyoutPerItem.get()));

//                log.info("itemId: " + itemId + " itemCount: " + itemCount + " minBuyoutPerItem: " + minBuyoutPerItem);
            });

            gameContextDAO.updateItemForSaleList(items);
        }
    }

    private void smsgMailCommandResult(Packet packet) throws IOException {
        try (PacketDataReader packetData = packet.getPacketDataReader()) {

            int mailId = packetData.readIntE();
            int actionCode = packetData.readIntE();
            int errorCode = packetData.readIntE();

            if (actionCode == 4 && errorCode == 0) {
                Mail mailForRemove = gameContext.getMailList().stream()
                        .filter(mail -> mail.getId() == mailId)
                        .collect(Collectors.toList())
                        .stream()
                        .findFirst()
                        .orElseThrow(() -> new IllegalArgumentException("Not found mailId(" + mailId + ") in mail list"));

                if (mailForRemove.getMailType() == MailType.AUCTION) {
                    String[] subjectArray = mailForRemove.getSubject().split(":");
                    String[] bodyArray = mailForRemove.getBody().split(":");
                    int itemId = Integer.parseInt(subjectArray[0]);
                    int action = Integer.parseInt(subjectArray[2]);
                    int count = Integer.parseInt(subjectArray[4]);
                    int cost = Integer.parseInt(bodyArray[2]) / count;
                    String actionString = null;
                    if (action == 1) {
                        actionString = "buy";
                    } else if (action == 2) {
                        actionString = "sell";
                    }

                    if (actionString != null) {
                        gameContextDAO.saveTradeHistoryRecord(new TradeHistoryRecord(itemId, packet.getTimestamp(), actionString, count, cost));
                    }

                    gameContext.getMailList().remove(mailForRemove);
                }
            }
        }
    }

    private void smsgMailListResult(Packet packet) throws IOException {
        try (PacketDataReader packetData = packet.getPacketDataReader()) {

            List<Mail> mailList = new ArrayList<>();
            int totalMailCount = packetData.readIntE();
            byte mailCount = packetData.readByte();
            for (int i = 0; i < mailCount; i++) {
                short msgSize = packetData.readShortE();
                int mailId = packetData.readIntE();
                MailType mailType = MailType.getMailTypeByCode(packetData.readByte());
                long playerGUID = 0;
                int entry = 0;
                switch (mailType) {
                    case NORMAL:
                        playerGUID = packetData.readLongE();
                        break;
                    case CREATURE:
                    case GAMEOBJECT:
                    case ITEM:
                    case AUCTION:
                        entry = packetData.readIntE();
                        break;
                }
                int cod = packetData.readIntE();
                int packageId = packetData.readIntE();
                int stationery = packetData.readIntE();
                int money = packetData.readIntE();
                int flags = packetData.readIntE();
                int time = packetData.readIntE();
                int templateId = packetData.readIntE();
                String subject = packetData.readCString();
                String body = packetData.readCString().trim();

                int packageItemCount = packetData.readByte();
                List<MailItem> attachedItems = new ArrayList<>();
                for (int j = 0; j < packageItemCount; j++) {
                    byte itemIndex = packetData.readByte();
                    int guid = packetData.readIntE();
                    int itemId = packetData.readIntE();
                    for (int k = 0; k < 7; k++) {
                        packetData.skip(12);
                    }
                    packetData.skip(8);
                    int itemCount = packetData.readIntE();
                    packetData.skip(13);
                    attachedItems.add(new MailItem(guid, itemId, itemCount));
                }
                mailList.add(new Mail(mailId, msgSize, mailType, playerGUID, entry, cod, packageId, stationery, money, flags,
                        time, templateId, subject, body, attachedItems));
            }

            gameContext.setMailList(mailList);
        }
    }

    private void cmsgAuctionListItems(Packet packet) throws IOException {
        try (PacketDataReader packetData = packet.getPacketDataReader()) {
            packetData.skip(8);
            int savedAuctionRecords = packetData.readIntE();
            if (savedAuctionRecords == 0) {
                gameContext.getAuctionRecords().clear();
            }
        }
    }

    private void smsgAuctionHello(Packet packet) throws IOException {
        try (PacketDataReader packetData = packet.getPacketDataReader()) {

            packetData.skip(8);
            int ahId = packetData.readIntE();

            gameContext.setAuctionFaction(AuctionFaction.getFactionByCode(ahId));

            log.info("Set auction faction as: " + gameContext.getAuctionFaction().name());

            gameContextDAO.deleteOldItemCostRecords();
            gameContextDAO.deleteOldItemProfitActionRecords();
        }
    }

    private void smsgAuctionListResult(Packet packet) throws IOException {
        try (PacketDataReader packetData = packet.getPacketDataReader()) {

            Instant start = Instant.now();
            int count = packetData.readIntE();
            if (count == 0) return;

            for (int i = 0; i < count; i++) {
                AuctionRecord auctionRecord = readAuctionRecord(packetData);
                gameContext.getAuctionRecords().add(auctionRecord);
            }

            int totalItemCount = packetData.readIntE();
            if (totalItemCount == 0) {
                throw new IllegalArgumentException("Auction list result total count 0, but count: " + count);
            }
            packetData.skip(4);

            boolean isFullScan = totalItemCount > 1000;

            if (totalItemCount == gameContext.getAuctionRecords().size()) {
                List<ItemCost> itemCostList = getItemStatListFromAuctionRecordsList(gameContext.getAuctionRecords());

                // save item price history if fullscan
                if (isFullScan) {
                    List<ItemHistory> itemHistoryList = new ArrayList<>();
                    for (ItemCost itemCost : itemCostList) {
                        itemHistoryList.add(new ItemHistory(itemCost.getId(), itemCost.getPrice(), packet.getTimestamp()));
                    }
                    gameContextDAO.saveItemHistoryList(itemHistoryList);
                }

                // save items prices
                gameContextDAO.saveItemCostList(itemCostList);
                gameContextDAO.deleteOldItemCostRecords();

                // clear profit
                gameContextDAO.deleteAllItemProfitActionRecords();

                // get item prices for profit calc
                List<ItemCost> allItemCostList = gameContextDAO.getAllItemCostList();

                // calc resell profit
                List<ItemProfitAction> itemProfitActions = calculateResellProfit(allItemCostList);
                gameContextDAO.updateItemProfitActionList(itemProfitActions);

                // calc craft profit
                List<Spell> craftSpells = gameContextDAO.getAutoUpdateCraftSpells();
                gameContextDAO.updateItemProfitActionList(calculateProfitListFromCraftSpellList(craftSpells, allItemCostList));

                Instant finish = Instant.now();

                log.info("Auction response package processing took: " + Duration.between(start, finish).toSeconds() + " seconds, updated item count: " + itemCostList.size());
            }
        }
    }

    private AuctionRecord readAuctionRecord(PacketDataReader packetData) throws IOException {
        packetData.skip(4);
        int id = packetData.readIntE();
        packetData.skip(92);
        int itemCount = packetData.readIntE();
        packetData.skip(24);
        int buyout = packetData.readIntE();
        packetData.skip(16);
        return new AuctionRecord(id, new Date(), itemCount, buyout);
    }

    private List<ItemProfitAction> calculateProfitListFromCraftSpellList(List<Spell> craftSpells, List<ItemCost> itemCostList) {

        List<ItemProfitAction> resultList = new ArrayList<>();

        for (Spell spell : craftSpells) {
            resultList.addAll(calculateProfitListFromCraftSpell(spell, itemCostList));

            for (Spell altSpell : spell.getAltSpellSet()) {
                resultList.addAll(calculateProfitListFromCraftSpell(altSpell, itemCostList));
            }
        }

        return resultList;
    }

    private List<ItemProfitAction> calculateProfitListFromCraftSpell(Spell spell, List<ItemCost> itemCostList) {
        List<ItemProfitAction> resultList = new ArrayList<>();

        if (spell.getCooldownTime() != 0) {
            return resultList;
        }

        // check all component have cost source
        boolean allComponentsCostExist = true;
        for (Component component : spell.getComponents()) {
            if (itemCostList.stream().noneMatch(itemCost -> itemCost.getId().equals(component.getItem().getItemId()))) {
                allComponentsCostExist = false;
                break;
            }
        }

        // if craft item or component not have cost source - skip spell
        if (!allComponentsCostExist || itemCostList.stream().noneMatch(itemCost -> itemCost.getId().equals(spell.getCraftItem().getItemId()))) {
            return resultList;
        }

        List<ItemCost> craftItemCostSourceList = itemCostList.stream()
                .filter(itemCost -> itemCost.getId().equals(spell.getCraftItem().getItemId()))
                .sorted(Comparator.comparing(ItemCost::getPrice, Comparator.reverseOrder()))
                .collect(Collectors.toList());

        ItemCost allianceItemCost = craftItemCostSourceList.stream()
                .filter(itemCost -> itemCost.getSource().equals("alliance_auction"))
                .findFirst()
                .orElse(null);

        ItemCost hordeItemCost = craftItemCostSourceList.stream()
                .filter(itemCost -> itemCost.getSource().equals("horde_auction"))
                .findFirst()
                .orElse(null);

        Integer allianceMinBuyout = allianceItemCost != null ? allianceItemCost.getPrice() : null;
        Integer hordeMinBuyout = hordeItemCost != null ? hordeItemCost.getPrice() : null;

        if (allianceItemCost != null) {
            resultList.add(getItemProfitAction(spell, "CRAFT_SELL_TO_ALLIANCE", itemCostList, craftItemCostSourceList, allianceItemCost, allianceMinBuyout, hordeMinBuyout));
        }

        if (hordeItemCost != null) {
            resultList.add(getItemProfitAction(spell, "CRAFT_SELL_TO_HORDE", itemCostList, craftItemCostSourceList, hordeItemCost, allianceMinBuyout, hordeMinBuyout));
        }

        return resultList;
    }

    private ItemProfitAction getItemProfitAction(Spell spell, String actionMessage, List<ItemCost> itemCostList, List<ItemCost> craftItemCostSourceList, ItemCost sourceCost, Integer allianceMinBuyout, Integer hordeMinBuyout) {
        StringBuilder spellInfo = new StringBuilder();
        spellInfo.append("spell: (")
                .append(spell.getSpellId())
                .append(") ")
                .append(spell.getName())
                .append("\n");

        StringBuilder sellTo = new StringBuilder();
        sellTo.append("can sell to:\n");
        for (ItemCost cost : craftItemCostSourceList) {
            sellTo.append("\t")
                    .append(priceToString(cost.getPrice()))
                    .append(" ")
                    .append(cost.getSource())
                    .append("\n");
        }

        int craftCost = 0;

        StringBuilder compInfo = new StringBuilder();
        compInfo.append("components:\n");
        for (Component comp : spell.getComponents()) {

            List<ItemCost> compItemCostSourceList = itemCostList.stream()
                    .filter(itemCost -> itemCost.getId().equals(comp.getItem().getItemId()))
                    .sorted(Comparator.comparing(ItemCost::getPrice))
                    .collect(Collectors.toList());

            craftCost += compItemCostSourceList.get(0).getPrice() * comp.getCount();
            compInfo.append(comp.getCount())
                    .append(" x (")
                    .append(comp.getItem().getItemId())
                    .append(") ")
                    .append(comp.getItem().getName())
                    .append("\n");
            for (ItemCost cost : compItemCostSourceList) {
                compInfo.append("\t")
                        .append(priceToString(cost.getPrice()))
                        .append(" ")
                        .append(cost.getSource())
                        .append("\n");
            }
        }
        // craft cost per item
        craftCost /= spell.getCraftItemCount();
        // profit per item
        int profit = sourceCost.getPrice() - craftCost;

        spellInfo.append("craft cost: ")
                .append(priceToString(craftCost))
                .append("\n")
                .append("profit: ")
                .append(priceToString(profit))
                .append("\n");

        if (!spell.getSubSpellSet().isEmpty()) {
            spellInfo.append("subspells:\n");
            for (Spell subSpell : spell.getSubSpellSet()) {
                spellInfo.append("\t")
                        .append(subSpell.getName())
                        .append("\n");
            }
        }

        String comment = spellInfo.toString() + "\n" + sellTo.toString() + "\n" + compInfo.toString();
        return new ItemProfitAction(spell.getCraftItem().getItemId(), actionMessage, allianceMinBuyout, hordeMinBuyout, profit, comment, new Date());
    }

    private List<ItemProfitAction> calculateResellProfit(List<ItemCost> itemsForCalc) {
        List<ItemProfitAction> resultList = new ArrayList<>();

        Map<Integer, List<ItemCost>> collect = itemsForCalc.stream().collect(Collectors.groupingBy(ItemCost::getId));

        for (Map.Entry<Integer, List<ItemCost>> entry : collect.entrySet()) {
            List<ItemCost> itemCostList = entry.getValue();

            // ignore vendor items
            if (itemCostList.stream().anyMatch(itemCost -> itemCost.getSource().equals("vendor"))) continue;

            itemCostList = itemCostList.stream().filter(itemCost -> !itemCost.getSource().equals("vendor")).collect(Collectors.toList());

            if (itemCostList.isEmpty()) continue;

            if (itemCostList.size() == 1) {
                resultList.addAll(getEmptyAuctionItems(itemCostList));
            } else {
                for (int i = 0; i < itemCostList.size(); i++) {
                    for (int j = i + 1; j < itemCostList.size(); j++) {
                        resultList.addAll(calcProfitFromPair(itemCostList.get(i), itemCostList.get(j)));
                    }
                }
            }
        }

        return resultList;
    }

    private List<ItemProfitAction> getEmptyAuctionItems(List<ItemCost> items) {
        List<ItemProfitAction> resultList = new ArrayList<>();
        ItemCost allianceItemCost = items.stream().filter(itemCost -> itemCost.getSource().equals("alliance_auction")).findFirst().orElse(null);
        ItemCost hordeItemCost = items.stream().filter(itemCost -> itemCost.getSource().equals("horde_auction")).findFirst().orElse(null);

        if (allianceItemCost == null && hordeItemCost == null) {
            throw new IllegalArgumentException("Item has all auctions null item cost");
        }

        String action = allianceItemCost == null ? "EMPTY_ALLIANCE_AUCTION" : "EMPTY_HORDE_AUCTION";

        Integer allianceMinBuyout = allianceItemCost != null ? allianceItemCost.getPrice() : null;
        Integer hordeMinBuyout = hordeItemCost != null ? hordeItemCost.getPrice() : null;

        resultList.add(new ItemProfitAction(items.get(0).getId(), action, allianceMinBuyout, hordeMinBuyout, null, null, new Date()));

        return resultList;
    }

    private List<ItemProfitAction> calcProfitFromPair(ItemCost firstItemCost, ItemCost secondItemCost) {
        List<ItemProfitAction> resultList = new ArrayList<>();
        List<ItemCost> items = new ArrayList<>();
        items.add(firstItemCost);
        items.add(secondItemCost);
        items.sort(Comparator.comparing(ItemCost::getPrice));

        ItemCost minItemCost = items.get(0);
        ItemCost maxItemCost = items.get(1);

        int profit = maxItemCost.getPrice() - minItemCost.getPrice();
        if (profit >= 0) {
            ItemCost allianceItemCost = items.stream().filter(itemCost -> itemCost.getSource().equals("alliance_auction")).findFirst().orElse(null);
            ItemCost hordeItemCost = items.stream().filter(itemCost -> itemCost.getSource().equals("horde_auction")).findFirst().orElse(null);

            Integer allianceMinBuyout = allianceItemCost != null ? allianceItemCost.getPrice() : null;
            Integer hordeMinBuyout = hordeItemCost != null ? hordeItemCost.getPrice() : null;

            String action = "RESELL_" + minItemCost.getSource().toUpperCase() + "_TO_" + maxItemCost.getSource().toUpperCase();
            String comment = minItemCost.getSource() + " " + priceToString(minItemCost.getPrice()) +
                    " -> " + maxItemCost.getSource() + " " + priceToString(maxItemCost.getPrice());

            resultList.add(new ItemProfitAction(minItemCost.getId(), action, allianceMinBuyout, hordeMinBuyout, profit, comment, new Date()));
        }

        return resultList;
    }

    public static String priceToString(Integer price) {
        int cooper = price % 100;
        int silver = (price - cooper) % 10000 / 100;
        int gold = (price - silver - cooper) / 10000;
        StringBuilder sb = new StringBuilder();
        if (gold != 0) {
            sb.append(gold).append("g");
        }
        if (silver != 0) {
            sb.append(silver).append("s");
        }
        if (cooper != 0) {
            sb.append(cooper).append("c");
        }

        return sb.toString();
    }


    private List<ItemCost> getItemStatListFromAuctionRecordsList(List<AuctionRecord> auctionRecords) {
        List<ItemCost> resultList = new ArrayList<>();
        if (auctionRecords.isEmpty()) return resultList;

        auctionRecords.sort(Comparator.comparing(AuctionRecord::getId));

        String source = null;
        if (gameContext.getAuctionFaction() == AuctionFaction.ALLIANCE) {
            source = "alliance_auction";
        } else if (gameContext.getAuctionFaction() == AuctionFaction.HORDE) {
            source = "horde_auction";
        }

        Map<Integer, List<AuctionRecord>> collect = auctionRecords.stream().collect(Collectors.groupingBy(AuctionRecord::getId));

        for (Map.Entry<Integer, List<AuctionRecord>> entry : collect.entrySet()) {
            List<AuctionRecord> groupedAuctionRecords = entry.getValue();
            AuctionRecord auctionRecord = groupedAuctionRecords.stream()
                    .filter(record -> record.getBuyoutPerItem() != 0)
                    .min(Comparator.comparing(AuctionRecord::getBuyoutPerItem)).orElse(null);

            if (auctionRecord != null) {
                resultList.add(new ItemCost(new ItemSource(auctionRecord.getId(), source), auctionRecord.getBuyoutPerItem(), new Date()));
            }
        }

        return resultList;
    }

    private void smsgSendKnownSpells(Packet packet) throws IOException {
        try (PacketDataReader packetData = packet.getPacketDataReader()) {

            List<Integer> spellIdList = new ArrayList<>();
            packetData.skip(1);
            short count = packetData.readShortE();
            log.info("Known spells: " + count);

            for (int i = 0; i < count; i++) {
                int spellId = packetData.readIntE();
                packetData.skip(2);
                spellIdList.add(spellId);
            }

            gameContextDAO.updateGameCharacterSpellIdList(gameContext.getPlayerGUID(), spellIdList);
        }
    }

    private void cmsgPlayerLogin(Packet packet) throws IOException {
        try (PacketDataReader packetData = packet.getPacketDataReader()) {
            gameContext.setPlayerGUID(packetData.readLongE());
        }
    }

    private void smsgEnumCharactersResult(Packet packet) throws IOException {
        try (PacketDataReader packetData = packet.getPacketDataReader()) {

            log.info("Login chamber character list:");
            List<GameCharacter> charList = new ArrayList<>();
            byte count = packetData.readByte();
            for (int i = 0; i < count; i++) {
                long guid = packetData.readLongE();
                String charName = packetData.readCString();
                byte raceCode = packetData.readByte();
                byte classCode = packetData.readByte();
                packetData.skip(6);
                byte level = packetData.readByte();
                packetData.skip(252);
                GameCharacter gameCharacter = new GameCharacter(guid, charName);
                log.info(gameCharacter.toString());
                charList.add(gameCharacter);
            }

            gameContextDAO.updateGameCharacterList(charList);
        }
    }

    private void cmsgAuthSession(Packet packet) throws IOException {
        try (PacketDataReader packetData = packet.getPacketDataReader()) {
            packetData.skip(8);
            gameContext.setAccountName(packetData.readCString());
            log.info("Client auth request with account: " + gameContext.getAccountName());
        }
    }

    public BlockingQueue<Packet> getQueue() {
        return queue;
    }

    public void setQueue(BlockingQueue<Packet> queue) {
        this.queue = queue;
    }


}

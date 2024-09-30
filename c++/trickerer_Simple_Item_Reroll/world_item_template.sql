DELETE FROM `item_template` WHERE `entry` = 39352;
INSERT INTO `item_template`
(`entry`,`class`,`name`,`displayid`,`Quality`,`BuyPrice`,`SellPrice`,`ItemLevel`,`delay`,`bonding`,`description`,`Material`,`ScriptName`)
VALUES
('39352','7','Mystic Oil','45064','3','500','125','1','0','0','Drop on an item with randomized stats to reroll.','3','item_randomstat_reroller');

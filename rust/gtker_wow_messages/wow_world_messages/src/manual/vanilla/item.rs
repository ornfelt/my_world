use crate::vanilla::{
    ItemSpells, ItemStat, ItemStatType, SMSG_ITEM_QUERY_SINGLE_RESPONSE_found,
    SMSG_ITEM_NAME_QUERY_RESPONSE, SMSG_ITEM_QUERY_SINGLE_RESPONSE,
};
use wow_world_base::vanilla::{Item, Level};

/// Convert an [`Item`] to a [`SMSG_ITEM_QUERY_SINGLE_RESPONSE`].
///
/// The message is just a tedious listing of [`Item`] fields with no
/// potential deviations so it has been upstreamed.
pub fn item_to_query_response(item: &Item) -> SMSG_ITEM_QUERY_SINGLE_RESPONSE {
    item.into()
}

/// Convert an [`Item`] to a [`SMSG_ITEM_NAME_QUERY_RESPONSE`].
///
/// The message is just a listing of [`Item`] fields with no
/// potential deviations so it has been upstreamed.
pub fn item_to_name_query_response(item: &Item) -> SMSG_ITEM_NAME_QUERY_RESPONSE {
    item.into()
}

impl From<&Item> for SMSG_ITEM_QUERY_SINGLE_RESPONSE {
    fn from(v: &Item) -> Self {
        Self {
            item: v.entry(),
            found: Some(SMSG_ITEM_QUERY_SINGLE_RESPONSE_found {
                class_and_sub_class: v.class_and_sub_class(),
                name1: v.name().to_string(),
                name2: "".to_string(),
                name3: "".to_string(),
                name4: "".to_string(),
                display_id: v.display_id(),
                quality: v.quality(),
                flags: v.flags(),
                buy_price: v.buy_price(),
                sell_price: v.sell_price(),
                inventory_type: v.inventory_type(),
                allowed_class: v.allowed_class(),
                allowed_race: v.allowed_race(),
                item_level: Level::new(v.item_level() as u8),
                required_level: Level::new(v.required_level() as u8),
                required_skill: v.required_skill(),
                required_skill_rank: v.required_skill_rank() as u32,
                required_spell: v.required_spell() as u32,
                required_honor_rank: v.required_honor_rank() as u32,
                required_city_rank: v.required_city_rank() as u32,
                required_faction: v.required_faction(),
                required_faction_rank: v.required_reputation_rank() as u32,
                max_count: v.max_count() as u32,
                stackable: v.stackable() as u32,
                container_slots: v.container_slots() as u32,
                stats: [
                    ItemStat {
                        stat_type: ItemStatType::Mana,
                        value: v.mana(),
                    },
                    ItemStat {
                        stat_type: ItemStatType::Health,
                        value: v.health(),
                    },
                    ItemStat {
                        stat_type: ItemStatType::Strength,
                        value: v.strength(),
                    },
                    ItemStat {
                        stat_type: ItemStatType::Agility,
                        value: v.agility(),
                    },
                    ItemStat {
                        stat_type: ItemStatType::Stamina,
                        value: v.stamina(),
                    },
                    ItemStat {
                        stat_type: ItemStatType::Intellect,
                        value: v.intellect(),
                    },
                    ItemStat {
                        stat_type: ItemStatType::Spirit,
                        value: v.spirit(),
                    },
                    ItemStat {
                        stat_type: ItemStatType::Mana,
                        value: 0,
                    },
                    ItemStat {
                        stat_type: ItemStatType::Mana,
                        value: 0,
                    },
                    ItemStat {
                        stat_type: ItemStatType::Mana,
                        value: 0,
                    },
                ],
                damages: v.damages_array(),
                armor: v.armor(),
                holy_resistance: v.holy_res(),
                fire_resistance: v.fire_res(),
                nature_resistance: v.nature_res(),
                frost_resistance: v.frost_res(),
                shadow_resistance: v.shadow_res(),
                arcane_resistance: v.arcane_res(),
                delay: v.delay() as u32,
                ammo_type: v.ammo_type() as u32,
                ranged_range_modification: v.ranged_mod_range(),
                spells: [
                    ItemSpells {
                        spell: v.spells_array()[0].spell as u32,
                        spell_trigger: v.spells_array()[0].spell_trigger,
                        spell_charges: v.spells_array()[0].spell_charges,
                        spell_cooldown: v.spells_array()[0].spell_cooldown,
                        spell_category: v.spells_array()[0].spell_category as u32,
                        spell_category_cooldown: v.spells_array()[0].spell_category_cooldown,
                    },
                    ItemSpells {
                        spell: v.spells_array()[1].spell as u32,
                        spell_trigger: v.spells_array()[1].spell_trigger,
                        spell_charges: v.spells_array()[1].spell_charges,
                        spell_cooldown: v.spells_array()[1].spell_cooldown,
                        spell_category: v.spells_array()[1].spell_category as u32,
                        spell_category_cooldown: v.spells_array()[1].spell_category_cooldown,
                    },
                    ItemSpells {
                        spell: v.spells_array()[2].spell as u32,
                        spell_trigger: v.spells_array()[2].spell_trigger,
                        spell_charges: v.spells_array()[2].spell_charges,
                        spell_cooldown: v.spells_array()[2].spell_cooldown,
                        spell_category: v.spells_array()[2].spell_category as u32,
                        spell_category_cooldown: v.spells_array()[2].spell_category_cooldown,
                    },
                    ItemSpells {
                        spell: v.spells_array()[3].spell as u32,
                        spell_trigger: v.spells_array()[3].spell_trigger,
                        spell_charges: v.spells_array()[3].spell_charges,
                        spell_cooldown: v.spells_array()[3].spell_cooldown,
                        spell_category: v.spells_array()[3].spell_category as u32,
                        spell_category_cooldown: v.spells_array()[3].spell_category_cooldown,
                    },
                    ItemSpells {
                        spell: v.spells_array()[4].spell as u32,
                        spell_trigger: v.spells_array()[4].spell_trigger,
                        spell_charges: v.spells_array()[4].spell_charges,
                        spell_cooldown: v.spells_array()[4].spell_cooldown,
                        spell_category: v.spells_array()[4].spell_category as u32,
                        spell_category_cooldown: v.spells_array()[4].spell_category_cooldown,
                    },
                ],
                bonding: v.bonding(),
                description: v.description().to_string(),
                page_text: v.page_text() as u32,
                language: v.language(),
                page_text_material: v.page_text_material(),
                start_quest: v.start_quest() as u32,
                lock_id: v.lock_id() as u32,
                material: v.material() as u32,
                sheathe_type: v.sheathe_type(),
                random_property: v.random_property() as u32,
                block: v.block() as u32,
                item_set: v.item_set(),
                max_durability: v.max_durability() as u32,
                area: v.area(),
                map: v.map(),
                bag_family: v.bag_family(),
            }),
        }
    }
}

impl From<Item> for SMSG_ITEM_QUERY_SINGLE_RESPONSE {
    fn from(v: Item) -> Self {
        (&v).into()
    }
}

impl From<&Item> for SMSG_ITEM_NAME_QUERY_RESPONSE {
    fn from(v: &Item) -> Self {
        Self {
            item: v.entry(),
            item_name: v.name().to_string(),
        }
    }
}

impl From<Item> for SMSG_ITEM_NAME_QUERY_RESPONSE {
    fn from(v: Item) -> Self {
        (&v).into()
    }
}

use wow_world_base::tbc::{
    AllowedClass, AllowedRace, BagFamily, Bonding, Faction, Gold, InventoryType, 
    Item, ItemClassAndSubClass, ItemDamageType, ItemFlag, ItemQuality, ItemSet, 
    ItemSocket, ItemStat, Language, PageTextMaterial, SheatheType, Skill, Spells,
};

const EMPTY_ITEMSOCKET:&[ItemSocket]=&[];
const EMPTY_ITEMDAMAGETYPE:&[ItemDamageType]=&[];
const EMPTY_ITEMSTAT:&[ItemStat]=&[];
const EMPTY_SPELLS:&[Spells]=&[];
pub const fn n(
    entry: u16,
    class_and_sub_class: ItemClassAndSubClass,
    sound_override_sub_class: i8,
    name: &'static str,
    display_id: u16,
    quality: ItemQuality,
    flags: u32,
    buy_count: i16,
    buy_price: u32,
    sell_price: u32,
    inventory_type: InventoryType,
    allowed_class: u32,
    allowed_race: u32,
    item_level: i16,
    required_level: i8,
    required_skill: Skill,
    required_skill_rank: i16,
    required_faction: Faction,
    required_reputation_rank: i8,
    max_count: i32,
    stackable: i16,
    container_slots: i8,
    armor: i16,
    fire_res: i16,
    nature_res: i16,
    frost_res: i16,
    shadow_res: i16,
    arcane_res: i16,
    delay: i16,
    ammo_type: i8,
    ranged_mod_range: f32,
    bonding: Bonding,
    description: &'static str,
    page_text: i16,
    language: Language,
    page_text_material: PageTextMaterial,
    start_quest: i16,
    material: i8,
    sheathe_type: SheatheType,
    random_property: i16,
    random_suffix: i16,
    block: i16,
    item_set: ItemSet,
    max_durability: i16,
    bag_family: u32,
    socket_bonus: i16,
    gem_properties: i16,
    required_disenchant_skill: i16,
    armor_damage_modifier: f32,
    disenchant_id: i8,
    food_type: i8,
    duration: i32,
    extra_flags: i8,
    sockets: &'static [ItemSocket],
    damages: &'static [ItemDamageType],
    stats: &'static [ItemStat],
    spells: &'static [Spells],
) -> Item {
    Item::new(
        entry,
        class_and_sub_class,
        sound_override_sub_class,
        name,
        display_id,
        quality,
        ItemFlag::new(flags),
        buy_count,
        Gold::new(buy_price),
        Gold::new(sell_price),
        inventory_type,
        AllowedClass::new(allowed_class),
        AllowedRace::new(allowed_race),
        item_level,
        required_level,
        required_skill,
        required_skill_rank,
        required_faction,
        required_reputation_rank,
        max_count,
        stackable,
        container_slots,
        armor,
        fire_res,
        nature_res,
        frost_res,
        shadow_res,
        arcane_res,
        delay,
        ammo_type,
        ranged_mod_range,
        bonding,
        description,
        page_text,
        language,
        page_text_material,
        start_quest,
        material,
        sheathe_type,
        random_property,
        random_suffix,
        block,
        item_set,
        max_durability,
        BagFamily::new(bag_family),
        socket_bonus,
        gem_properties,
        required_disenchant_skill,
        armor_damage_modifier,
        disenchant_id,
        food_type,
        duration,
        extra_flags,
        sockets,
        damages,
        stats,
        spells,
    )
}
pub const fn na(
    entry: u16,
    class_and_sub_class: ItemClassAndSubClass,
    sound_override_sub_class: i8,
    name: &'static str,
    display_id: u16,
    quality: ItemQuality,
    flags: u32,
    buy_count: i16,
    buy_price: u32,
    sell_price: u32,
    inventory_type: InventoryType,
    allowed_class: u32,
    allowed_race: u32,
    item_level: i16,
    required_level: i8,
    required_skill: Skill,
    required_skill_rank: i16,
    required_faction: Faction,
    required_reputation_rank: i8,
    max_count: i32,
    stackable: i16,
    container_slots: i8,
    armor: i16,
    fire_res: i16,
    nature_res: i16,
    frost_res: i16,
    shadow_res: i16,
    arcane_res: i16,
    delay: i16,
    ammo_type: i8,
    ranged_mod_range: f32,
    bonding: Bonding,
    description: &'static str,
    page_text: i16,
    language: Language,
    page_text_material: PageTextMaterial,
    start_quest: i16,
    material: i8,
    sheathe_type: SheatheType,
    random_property: i16,
    random_suffix: i16,
    block: i16,
    item_set: ItemSet,
    max_durability: i16,
    bag_family: u32,
    socket_bonus: i16,
    gem_properties: i16,
    required_disenchant_skill: i16,
    armor_damage_modifier: f32,
    disenchant_id: i8,
    food_type: i8,
    duration: i32,
    extra_flags: i8,
    sockets: &'static [ItemSocket],
    stats: &'static [ItemStat],
    spells: &'static [Spells],
) -> Item {
    Item::new(
        entry,
        class_and_sub_class,
        sound_override_sub_class,
        name,
        display_id,
        quality,
        ItemFlag::new(flags),
        buy_count,
        Gold::new(buy_price),
        Gold::new(sell_price),
        inventory_type,
        AllowedClass::new(allowed_class),
        AllowedRace::new(allowed_race),
        item_level,
        required_level,
        required_skill,
        required_skill_rank,
        required_faction,
        required_reputation_rank,
        max_count,
        stackable,
        container_slots,
        armor,
        fire_res,
        nature_res,
        frost_res,
        shadow_res,
        arcane_res,
        delay,
        ammo_type,
        ranged_mod_range,
        bonding,
        description,
        page_text,
        language,
        page_text_material,
        start_quest,
        material,
        sheathe_type,
        random_property,
        random_suffix,
        block,
        item_set,
        max_durability,
        BagFamily::new(bag_family),
        socket_bonus,
        gem_properties,
        required_disenchant_skill,
        armor_damage_modifier,
        disenchant_id,
        food_type,
        duration,
        extra_flags,
        sockets,
        EMPTY_ITEMDAMAGETYPE,
        stats,
        spells,
    )
}
pub const fn nab(
    entry: u16,
    class_and_sub_class: ItemClassAndSubClass,
    sound_override_sub_class: i8,
    name: &'static str,
    display_id: u16,
    quality: ItemQuality,
    flags: u32,
    buy_count: i16,
    buy_price: u32,
    sell_price: u32,
    inventory_type: InventoryType,
    allowed_class: u32,
    allowed_race: u32,
    item_level: i16,
    required_level: i8,
    required_skill: Skill,
    required_skill_rank: i16,
    required_faction: Faction,
    required_reputation_rank: i8,
    max_count: i32,
    stackable: i16,
    container_slots: i8,
    armor: i16,
    fire_res: i16,
    nature_res: i16,
    frost_res: i16,
    shadow_res: i16,
    arcane_res: i16,
    delay: i16,
    ammo_type: i8,
    ranged_mod_range: f32,
    bonding: Bonding,
    description: &'static str,
    page_text: i16,
    language: Language,
    page_text_material: PageTextMaterial,
    start_quest: i16,
    material: i8,
    sheathe_type: SheatheType,
    random_property: i16,
    random_suffix: i16,
    block: i16,
    item_set: ItemSet,
    max_durability: i16,
    bag_family: u32,
    socket_bonus: i16,
    gem_properties: i16,
    required_disenchant_skill: i16,
    armor_damage_modifier: f32,
    disenchant_id: i8,
    food_type: i8,
    duration: i32,
    extra_flags: i8,
    sockets: &'static [ItemSocket],
    stats: &'static [ItemStat],
) -> Item {
    Item::new(
        entry,
        class_and_sub_class,
        sound_override_sub_class,
        name,
        display_id,
        quality,
        ItemFlag::new(flags),
        buy_count,
        Gold::new(buy_price),
        Gold::new(sell_price),
        inventory_type,
        AllowedClass::new(allowed_class),
        AllowedRace::new(allowed_race),
        item_level,
        required_level,
        required_skill,
        required_skill_rank,
        required_faction,
        required_reputation_rank,
        max_count,
        stackable,
        container_slots,
        armor,
        fire_res,
        nature_res,
        frost_res,
        shadow_res,
        arcane_res,
        delay,
        ammo_type,
        ranged_mod_range,
        bonding,
        description,
        page_text,
        language,
        page_text_material,
        start_quest,
        material,
        sheathe_type,
        random_property,
        random_suffix,
        block,
        item_set,
        max_durability,
        BagFamily::new(bag_family),
        socket_bonus,
        gem_properties,
        required_disenchant_skill,
        armor_damage_modifier,
        disenchant_id,
        food_type,
        duration,
        extra_flags,
        sockets,
        EMPTY_ITEMDAMAGETYPE,
        stats,
        EMPTY_SPELLS,
    )
}
pub const fn nac(
    entry: u16,
    class_and_sub_class: ItemClassAndSubClass,
    sound_override_sub_class: i8,
    name: &'static str,
    display_id: u16,
    quality: ItemQuality,
    flags: u32,
    buy_count: i16,
    buy_price: u32,
    sell_price: u32,
    inventory_type: InventoryType,
    allowed_class: u32,
    allowed_race: u32,
    item_level: i16,
    required_level: i8,
    required_skill: Skill,
    required_skill_rank: i16,
    required_faction: Faction,
    required_reputation_rank: i8,
    max_count: i32,
    stackable: i16,
    container_slots: i8,
    armor: i16,
    fire_res: i16,
    nature_res: i16,
    frost_res: i16,
    shadow_res: i16,
    arcane_res: i16,
    delay: i16,
    ammo_type: i8,
    ranged_mod_range: f32,
    bonding: Bonding,
    description: &'static str,
    page_text: i16,
    language: Language,
    page_text_material: PageTextMaterial,
    start_quest: i16,
    material: i8,
    sheathe_type: SheatheType,
    random_property: i16,
    random_suffix: i16,
    block: i16,
    item_set: ItemSet,
    max_durability: i16,
    bag_family: u32,
    socket_bonus: i16,
    gem_properties: i16,
    required_disenchant_skill: i16,
    armor_damage_modifier: f32,
    disenchant_id: i8,
    food_type: i8,
    duration: i32,
    extra_flags: i8,
    stats: &'static [ItemStat],
    spells: &'static [Spells],
) -> Item {
    Item::new(
        entry,
        class_and_sub_class,
        sound_override_sub_class,
        name,
        display_id,
        quality,
        ItemFlag::new(flags),
        buy_count,
        Gold::new(buy_price),
        Gold::new(sell_price),
        inventory_type,
        AllowedClass::new(allowed_class),
        AllowedRace::new(allowed_race),
        item_level,
        required_level,
        required_skill,
        required_skill_rank,
        required_faction,
        required_reputation_rank,
        max_count,
        stackable,
        container_slots,
        armor,
        fire_res,
        nature_res,
        frost_res,
        shadow_res,
        arcane_res,
        delay,
        ammo_type,
        ranged_mod_range,
        bonding,
        description,
        page_text,
        language,
        page_text_material,
        start_quest,
        material,
        sheathe_type,
        random_property,
        random_suffix,
        block,
        item_set,
        max_durability,
        BagFamily::new(bag_family),
        socket_bonus,
        gem_properties,
        required_disenchant_skill,
        armor_damage_modifier,
        disenchant_id,
        food_type,
        duration,
        extra_flags,
        EMPTY_ITEMSOCKET,
        EMPTY_ITEMDAMAGETYPE,
        stats,
        spells,
    )
}
pub const fn nacb(
    entry: u16,
    class_and_sub_class: ItemClassAndSubClass,
    sound_override_sub_class: i8,
    name: &'static str,
    display_id: u16,
    quality: ItemQuality,
    flags: u32,
    buy_count: i16,
    buy_price: u32,
    sell_price: u32,
    inventory_type: InventoryType,
    allowed_class: u32,
    allowed_race: u32,
    item_level: i16,
    required_level: i8,
    required_skill: Skill,
    required_skill_rank: i16,
    required_faction: Faction,
    required_reputation_rank: i8,
    max_count: i32,
    stackable: i16,
    container_slots: i8,
    armor: i16,
    fire_res: i16,
    nature_res: i16,
    frost_res: i16,
    shadow_res: i16,
    arcane_res: i16,
    delay: i16,
    ammo_type: i8,
    ranged_mod_range: f32,
    bonding: Bonding,
    description: &'static str,
    page_text: i16,
    language: Language,
    page_text_material: PageTextMaterial,
    start_quest: i16,
    material: i8,
    sheathe_type: SheatheType,
    random_property: i16,
    random_suffix: i16,
    block: i16,
    item_set: ItemSet,
    max_durability: i16,
    bag_family: u32,
    socket_bonus: i16,
    gem_properties: i16,
    required_disenchant_skill: i16,
    armor_damage_modifier: f32,
    disenchant_id: i8,
    food_type: i8,
    duration: i32,
    extra_flags: i8,
    stats: &'static [ItemStat],
) -> Item {
    Item::new(
        entry,
        class_and_sub_class,
        sound_override_sub_class,
        name,
        display_id,
        quality,
        ItemFlag::new(flags),
        buy_count,
        Gold::new(buy_price),
        Gold::new(sell_price),
        inventory_type,
        AllowedClass::new(allowed_class),
        AllowedRace::new(allowed_race),
        item_level,
        required_level,
        required_skill,
        required_skill_rank,
        required_faction,
        required_reputation_rank,
        max_count,
        stackable,
        container_slots,
        armor,
        fire_res,
        nature_res,
        frost_res,
        shadow_res,
        arcane_res,
        delay,
        ammo_type,
        ranged_mod_range,
        bonding,
        description,
        page_text,
        language,
        page_text_material,
        start_quest,
        material,
        sheathe_type,
        random_property,
        random_suffix,
        block,
        item_set,
        max_durability,
        BagFamily::new(bag_family),
        socket_bonus,
        gem_properties,
        required_disenchant_skill,
        armor_damage_modifier,
        disenchant_id,
        food_type,
        duration,
        extra_flags,
        EMPTY_ITEMSOCKET,
        EMPTY_ITEMDAMAGETYPE,
        stats,
        EMPTY_SPELLS,
    )
}
pub const fn nacd(
    entry: u16,
    class_and_sub_class: ItemClassAndSubClass,
    sound_override_sub_class: i8,
    name: &'static str,
    display_id: u16,
    quality: ItemQuality,
    flags: u32,
    buy_count: i16,
    buy_price: u32,
    sell_price: u32,
    inventory_type: InventoryType,
    allowed_class: u32,
    allowed_race: u32,
    item_level: i16,
    required_level: i8,
    required_skill: Skill,
    required_skill_rank: i16,
    required_faction: Faction,
    required_reputation_rank: i8,
    max_count: i32,
    stackable: i16,
    container_slots: i8,
    armor: i16,
    fire_res: i16,
    nature_res: i16,
    frost_res: i16,
    shadow_res: i16,
    arcane_res: i16,
    delay: i16,
    ammo_type: i8,
    ranged_mod_range: f32,
    bonding: Bonding,
    description: &'static str,
    page_text: i16,
    language: Language,
    page_text_material: PageTextMaterial,
    start_quest: i16,
    material: i8,
    sheathe_type: SheatheType,
    random_property: i16,
    random_suffix: i16,
    block: i16,
    item_set: ItemSet,
    max_durability: i16,
    bag_family: u32,
    socket_bonus: i16,
    gem_properties: i16,
    required_disenchant_skill: i16,
    armor_damage_modifier: f32,
    disenchant_id: i8,
    food_type: i8,
    duration: i32,
    extra_flags: i8,
    spells: &'static [Spells],
) -> Item {
    Item::new(
        entry,
        class_and_sub_class,
        sound_override_sub_class,
        name,
        display_id,
        quality,
        ItemFlag::new(flags),
        buy_count,
        Gold::new(buy_price),
        Gold::new(sell_price),
        inventory_type,
        AllowedClass::new(allowed_class),
        AllowedRace::new(allowed_race),
        item_level,
        required_level,
        required_skill,
        required_skill_rank,
        required_faction,
        required_reputation_rank,
        max_count,
        stackable,
        container_slots,
        armor,
        fire_res,
        nature_res,
        frost_res,
        shadow_res,
        arcane_res,
        delay,
        ammo_type,
        ranged_mod_range,
        bonding,
        description,
        page_text,
        language,
        page_text_material,
        start_quest,
        material,
        sheathe_type,
        random_property,
        random_suffix,
        block,
        item_set,
        max_durability,
        BagFamily::new(bag_family),
        socket_bonus,
        gem_properties,
        required_disenchant_skill,
        armor_damage_modifier,
        disenchant_id,
        food_type,
        duration,
        extra_flags,
        EMPTY_ITEMSOCKET,
        EMPTY_ITEMDAMAGETYPE,
        EMPTY_ITEMSTAT,
        spells,
    )
}
pub const fn nacdb(
    entry: u16,
    class_and_sub_class: ItemClassAndSubClass,
    sound_override_sub_class: i8,
    name: &'static str,
    display_id: u16,
    quality: ItemQuality,
    flags: u32,
    buy_count: i16,
    buy_price: u32,
    sell_price: u32,
    inventory_type: InventoryType,
    allowed_class: u32,
    allowed_race: u32,
    item_level: i16,
    required_level: i8,
    required_skill: Skill,
    required_skill_rank: i16,
    required_faction: Faction,
    required_reputation_rank: i8,
    max_count: i32,
    stackable: i16,
    container_slots: i8,
    armor: i16,
    fire_res: i16,
    nature_res: i16,
    frost_res: i16,
    shadow_res: i16,
    arcane_res: i16,
    delay: i16,
    ammo_type: i8,
    ranged_mod_range: f32,
    bonding: Bonding,
    description: &'static str,
    page_text: i16,
    language: Language,
    page_text_material: PageTextMaterial,
    start_quest: i16,
    material: i8,
    sheathe_type: SheatheType,
    random_property: i16,
    random_suffix: i16,
    block: i16,
    item_set: ItemSet,
    max_durability: i16,
    bag_family: u32,
    socket_bonus: i16,
    gem_properties: i16,
    required_disenchant_skill: i16,
    armor_damage_modifier: f32,
    disenchant_id: i8,
    food_type: i8,
    duration: i32,
    extra_flags: i8,
) -> Item {
    Item::new(
        entry,
        class_and_sub_class,
        sound_override_sub_class,
        name,
        display_id,
        quality,
        ItemFlag::new(flags),
        buy_count,
        Gold::new(buy_price),
        Gold::new(sell_price),
        inventory_type,
        AllowedClass::new(allowed_class),
        AllowedRace::new(allowed_race),
        item_level,
        required_level,
        required_skill,
        required_skill_rank,
        required_faction,
        required_reputation_rank,
        max_count,
        stackable,
        container_slots,
        armor,
        fire_res,
        nature_res,
        frost_res,
        shadow_res,
        arcane_res,
        delay,
        ammo_type,
        ranged_mod_range,
        bonding,
        description,
        page_text,
        language,
        page_text_material,
        start_quest,
        material,
        sheathe_type,
        random_property,
        random_suffix,
        block,
        item_set,
        max_durability,
        BagFamily::new(bag_family),
        socket_bonus,
        gem_properties,
        required_disenchant_skill,
        armor_damage_modifier,
        disenchant_id,
        food_type,
        duration,
        extra_flags,
        EMPTY_ITEMSOCKET,
        EMPTY_ITEMDAMAGETYPE,
        EMPTY_ITEMSTAT,
        EMPTY_SPELLS,
    )
}
pub const fn nad(
    entry: u16,
    class_and_sub_class: ItemClassAndSubClass,
    sound_override_sub_class: i8,
    name: &'static str,
    display_id: u16,
    quality: ItemQuality,
    flags: u32,
    buy_count: i16,
    buy_price: u32,
    sell_price: u32,
    inventory_type: InventoryType,
    allowed_class: u32,
    allowed_race: u32,
    item_level: i16,
    required_level: i8,
    required_skill: Skill,
    required_skill_rank: i16,
    required_faction: Faction,
    required_reputation_rank: i8,
    max_count: i32,
    stackable: i16,
    container_slots: i8,
    armor: i16,
    fire_res: i16,
    nature_res: i16,
    frost_res: i16,
    shadow_res: i16,
    arcane_res: i16,
    delay: i16,
    ammo_type: i8,
    ranged_mod_range: f32,
    bonding: Bonding,
    description: &'static str,
    page_text: i16,
    language: Language,
    page_text_material: PageTextMaterial,
    start_quest: i16,
    material: i8,
    sheathe_type: SheatheType,
    random_property: i16,
    random_suffix: i16,
    block: i16,
    item_set: ItemSet,
    max_durability: i16,
    bag_family: u32,
    socket_bonus: i16,
    gem_properties: i16,
    required_disenchant_skill: i16,
    armor_damage_modifier: f32,
    disenchant_id: i8,
    food_type: i8,
    duration: i32,
    extra_flags: i8,
    sockets: &'static [ItemSocket],
    spells: &'static [Spells],
) -> Item {
    Item::new(
        entry,
        class_and_sub_class,
        sound_override_sub_class,
        name,
        display_id,
        quality,
        ItemFlag::new(flags),
        buy_count,
        Gold::new(buy_price),
        Gold::new(sell_price),
        inventory_type,
        AllowedClass::new(allowed_class),
        AllowedRace::new(allowed_race),
        item_level,
        required_level,
        required_skill,
        required_skill_rank,
        required_faction,
        required_reputation_rank,
        max_count,
        stackable,
        container_slots,
        armor,
        fire_res,
        nature_res,
        frost_res,
        shadow_res,
        arcane_res,
        delay,
        ammo_type,
        ranged_mod_range,
        bonding,
        description,
        page_text,
        language,
        page_text_material,
        start_quest,
        material,
        sheathe_type,
        random_property,
        random_suffix,
        block,
        item_set,
        max_durability,
        BagFamily::new(bag_family),
        socket_bonus,
        gem_properties,
        required_disenchant_skill,
        armor_damage_modifier,
        disenchant_id,
        food_type,
        duration,
        extra_flags,
        sockets,
        EMPTY_ITEMDAMAGETYPE,
        EMPTY_ITEMSTAT,
        spells,
    )
}
pub const fn nb(
    entry: u16,
    class_and_sub_class: ItemClassAndSubClass,
    sound_override_sub_class: i8,
    name: &'static str,
    display_id: u16,
    quality: ItemQuality,
    flags: u32,
    buy_count: i16,
    buy_price: u32,
    sell_price: u32,
    inventory_type: InventoryType,
    allowed_class: u32,
    allowed_race: u32,
    item_level: i16,
    required_level: i8,
    required_skill: Skill,
    required_skill_rank: i16,
    required_faction: Faction,
    required_reputation_rank: i8,
    max_count: i32,
    stackable: i16,
    container_slots: i8,
    armor: i16,
    fire_res: i16,
    nature_res: i16,
    frost_res: i16,
    shadow_res: i16,
    arcane_res: i16,
    delay: i16,
    ammo_type: i8,
    ranged_mod_range: f32,
    bonding: Bonding,
    description: &'static str,
    page_text: i16,
    language: Language,
    page_text_material: PageTextMaterial,
    start_quest: i16,
    material: i8,
    sheathe_type: SheatheType,
    random_property: i16,
    random_suffix: i16,
    block: i16,
    item_set: ItemSet,
    max_durability: i16,
    bag_family: u32,
    socket_bonus: i16,
    gem_properties: i16,
    required_disenchant_skill: i16,
    armor_damage_modifier: f32,
    disenchant_id: i8,
    food_type: i8,
    duration: i32,
    extra_flags: i8,
    sockets: &'static [ItemSocket],
    damages: &'static [ItemDamageType],
    stats: &'static [ItemStat],
) -> Item {
    Item::new(
        entry,
        class_and_sub_class,
        sound_override_sub_class,
        name,
        display_id,
        quality,
        ItemFlag::new(flags),
        buy_count,
        Gold::new(buy_price),
        Gold::new(sell_price),
        inventory_type,
        AllowedClass::new(allowed_class),
        AllowedRace::new(allowed_race),
        item_level,
        required_level,
        required_skill,
        required_skill_rank,
        required_faction,
        required_reputation_rank,
        max_count,
        stackable,
        container_slots,
        armor,
        fire_res,
        nature_res,
        frost_res,
        shadow_res,
        arcane_res,
        delay,
        ammo_type,
        ranged_mod_range,
        bonding,
        description,
        page_text,
        language,
        page_text_material,
        start_quest,
        material,
        sheathe_type,
        random_property,
        random_suffix,
        block,
        item_set,
        max_durability,
        BagFamily::new(bag_family),
        socket_bonus,
        gem_properties,
        required_disenchant_skill,
        armor_damage_modifier,
        disenchant_id,
        food_type,
        duration,
        extra_flags,
        sockets,
        damages,
        stats,
        EMPTY_SPELLS,
    )
}
pub const fn nc(
    entry: u16,
    class_and_sub_class: ItemClassAndSubClass,
    sound_override_sub_class: i8,
    name: &'static str,
    display_id: u16,
    quality: ItemQuality,
    flags: u32,
    buy_count: i16,
    buy_price: u32,
    sell_price: u32,
    inventory_type: InventoryType,
    allowed_class: u32,
    allowed_race: u32,
    item_level: i16,
    required_level: i8,
    required_skill: Skill,
    required_skill_rank: i16,
    required_faction: Faction,
    required_reputation_rank: i8,
    max_count: i32,
    stackable: i16,
    container_slots: i8,
    armor: i16,
    fire_res: i16,
    nature_res: i16,
    frost_res: i16,
    shadow_res: i16,
    arcane_res: i16,
    delay: i16,
    ammo_type: i8,
    ranged_mod_range: f32,
    bonding: Bonding,
    description: &'static str,
    page_text: i16,
    language: Language,
    page_text_material: PageTextMaterial,
    start_quest: i16,
    material: i8,
    sheathe_type: SheatheType,
    random_property: i16,
    random_suffix: i16,
    block: i16,
    item_set: ItemSet,
    max_durability: i16,
    bag_family: u32,
    socket_bonus: i16,
    gem_properties: i16,
    required_disenchant_skill: i16,
    armor_damage_modifier: f32,
    disenchant_id: i8,
    food_type: i8,
    duration: i32,
    extra_flags: i8,
    damages: &'static [ItemDamageType],
    stats: &'static [ItemStat],
    spells: &'static [Spells],
) -> Item {
    Item::new(
        entry,
        class_and_sub_class,
        sound_override_sub_class,
        name,
        display_id,
        quality,
        ItemFlag::new(flags),
        buy_count,
        Gold::new(buy_price),
        Gold::new(sell_price),
        inventory_type,
        AllowedClass::new(allowed_class),
        AllowedRace::new(allowed_race),
        item_level,
        required_level,
        required_skill,
        required_skill_rank,
        required_faction,
        required_reputation_rank,
        max_count,
        stackable,
        container_slots,
        armor,
        fire_res,
        nature_res,
        frost_res,
        shadow_res,
        arcane_res,
        delay,
        ammo_type,
        ranged_mod_range,
        bonding,
        description,
        page_text,
        language,
        page_text_material,
        start_quest,
        material,
        sheathe_type,
        random_property,
        random_suffix,
        block,
        item_set,
        max_durability,
        BagFamily::new(bag_family),
        socket_bonus,
        gem_properties,
        required_disenchant_skill,
        armor_damage_modifier,
        disenchant_id,
        food_type,
        duration,
        extra_flags,
        EMPTY_ITEMSOCKET,
        damages,
        stats,
        spells,
    )
}
pub const fn ncb(
    entry: u16,
    class_and_sub_class: ItemClassAndSubClass,
    sound_override_sub_class: i8,
    name: &'static str,
    display_id: u16,
    quality: ItemQuality,
    flags: u32,
    buy_count: i16,
    buy_price: u32,
    sell_price: u32,
    inventory_type: InventoryType,
    allowed_class: u32,
    allowed_race: u32,
    item_level: i16,
    required_level: i8,
    required_skill: Skill,
    required_skill_rank: i16,
    required_faction: Faction,
    required_reputation_rank: i8,
    max_count: i32,
    stackable: i16,
    container_slots: i8,
    armor: i16,
    fire_res: i16,
    nature_res: i16,
    frost_res: i16,
    shadow_res: i16,
    arcane_res: i16,
    delay: i16,
    ammo_type: i8,
    ranged_mod_range: f32,
    bonding: Bonding,
    description: &'static str,
    page_text: i16,
    language: Language,
    page_text_material: PageTextMaterial,
    start_quest: i16,
    material: i8,
    sheathe_type: SheatheType,
    random_property: i16,
    random_suffix: i16,
    block: i16,
    item_set: ItemSet,
    max_durability: i16,
    bag_family: u32,
    socket_bonus: i16,
    gem_properties: i16,
    required_disenchant_skill: i16,
    armor_damage_modifier: f32,
    disenchant_id: i8,
    food_type: i8,
    duration: i32,
    extra_flags: i8,
    damages: &'static [ItemDamageType],
    stats: &'static [ItemStat],
) -> Item {
    Item::new(
        entry,
        class_and_sub_class,
        sound_override_sub_class,
        name,
        display_id,
        quality,
        ItemFlag::new(flags),
        buy_count,
        Gold::new(buy_price),
        Gold::new(sell_price),
        inventory_type,
        AllowedClass::new(allowed_class),
        AllowedRace::new(allowed_race),
        item_level,
        required_level,
        required_skill,
        required_skill_rank,
        required_faction,
        required_reputation_rank,
        max_count,
        stackable,
        container_slots,
        armor,
        fire_res,
        nature_res,
        frost_res,
        shadow_res,
        arcane_res,
        delay,
        ammo_type,
        ranged_mod_range,
        bonding,
        description,
        page_text,
        language,
        page_text_material,
        start_quest,
        material,
        sheathe_type,
        random_property,
        random_suffix,
        block,
        item_set,
        max_durability,
        BagFamily::new(bag_family),
        socket_bonus,
        gem_properties,
        required_disenchant_skill,
        armor_damage_modifier,
        disenchant_id,
        food_type,
        duration,
        extra_flags,
        EMPTY_ITEMSOCKET,
        damages,
        stats,
        EMPTY_SPELLS,
    )
}
pub const fn ncd(
    entry: u16,
    class_and_sub_class: ItemClassAndSubClass,
    sound_override_sub_class: i8,
    name: &'static str,
    display_id: u16,
    quality: ItemQuality,
    flags: u32,
    buy_count: i16,
    buy_price: u32,
    sell_price: u32,
    inventory_type: InventoryType,
    allowed_class: u32,
    allowed_race: u32,
    item_level: i16,
    required_level: i8,
    required_skill: Skill,
    required_skill_rank: i16,
    required_faction: Faction,
    required_reputation_rank: i8,
    max_count: i32,
    stackable: i16,
    container_slots: i8,
    armor: i16,
    fire_res: i16,
    nature_res: i16,
    frost_res: i16,
    shadow_res: i16,
    arcane_res: i16,
    delay: i16,
    ammo_type: i8,
    ranged_mod_range: f32,
    bonding: Bonding,
    description: &'static str,
    page_text: i16,
    language: Language,
    page_text_material: PageTextMaterial,
    start_quest: i16,
    material: i8,
    sheathe_type: SheatheType,
    random_property: i16,
    random_suffix: i16,
    block: i16,
    item_set: ItemSet,
    max_durability: i16,
    bag_family: u32,
    socket_bonus: i16,
    gem_properties: i16,
    required_disenchant_skill: i16,
    armor_damage_modifier: f32,
    disenchant_id: i8,
    food_type: i8,
    duration: i32,
    extra_flags: i8,
    damages: &'static [ItemDamageType],
    spells: &'static [Spells],
) -> Item {
    Item::new(
        entry,
        class_and_sub_class,
        sound_override_sub_class,
        name,
        display_id,
        quality,
        ItemFlag::new(flags),
        buy_count,
        Gold::new(buy_price),
        Gold::new(sell_price),
        inventory_type,
        AllowedClass::new(allowed_class),
        AllowedRace::new(allowed_race),
        item_level,
        required_level,
        required_skill,
        required_skill_rank,
        required_faction,
        required_reputation_rank,
        max_count,
        stackable,
        container_slots,
        armor,
        fire_res,
        nature_res,
        frost_res,
        shadow_res,
        arcane_res,
        delay,
        ammo_type,
        ranged_mod_range,
        bonding,
        description,
        page_text,
        language,
        page_text_material,
        start_quest,
        material,
        sheathe_type,
        random_property,
        random_suffix,
        block,
        item_set,
        max_durability,
        BagFamily::new(bag_family),
        socket_bonus,
        gem_properties,
        required_disenchant_skill,
        armor_damage_modifier,
        disenchant_id,
        food_type,
        duration,
        extra_flags,
        EMPTY_ITEMSOCKET,
        damages,
        EMPTY_ITEMSTAT,
        spells,
    )
}
pub const fn ncdb(
    entry: u16,
    class_and_sub_class: ItemClassAndSubClass,
    sound_override_sub_class: i8,
    name: &'static str,
    display_id: u16,
    quality: ItemQuality,
    flags: u32,
    buy_count: i16,
    buy_price: u32,
    sell_price: u32,
    inventory_type: InventoryType,
    allowed_class: u32,
    allowed_race: u32,
    item_level: i16,
    required_level: i8,
    required_skill: Skill,
    required_skill_rank: i16,
    required_faction: Faction,
    required_reputation_rank: i8,
    max_count: i32,
    stackable: i16,
    container_slots: i8,
    armor: i16,
    fire_res: i16,
    nature_res: i16,
    frost_res: i16,
    shadow_res: i16,
    arcane_res: i16,
    delay: i16,
    ammo_type: i8,
    ranged_mod_range: f32,
    bonding: Bonding,
    description: &'static str,
    page_text: i16,
    language: Language,
    page_text_material: PageTextMaterial,
    start_quest: i16,
    material: i8,
    sheathe_type: SheatheType,
    random_property: i16,
    random_suffix: i16,
    block: i16,
    item_set: ItemSet,
    max_durability: i16,
    bag_family: u32,
    socket_bonus: i16,
    gem_properties: i16,
    required_disenchant_skill: i16,
    armor_damage_modifier: f32,
    disenchant_id: i8,
    food_type: i8,
    duration: i32,
    extra_flags: i8,
    damages: &'static [ItemDamageType],
) -> Item {
    Item::new(
        entry,
        class_and_sub_class,
        sound_override_sub_class,
        name,
        display_id,
        quality,
        ItemFlag::new(flags),
        buy_count,
        Gold::new(buy_price),
        Gold::new(sell_price),
        inventory_type,
        AllowedClass::new(allowed_class),
        AllowedRace::new(allowed_race),
        item_level,
        required_level,
        required_skill,
        required_skill_rank,
        required_faction,
        required_reputation_rank,
        max_count,
        stackable,
        container_slots,
        armor,
        fire_res,
        nature_res,
        frost_res,
        shadow_res,
        arcane_res,
        delay,
        ammo_type,
        ranged_mod_range,
        bonding,
        description,
        page_text,
        language,
        page_text_material,
        start_quest,
        material,
        sheathe_type,
        random_property,
        random_suffix,
        block,
        item_set,
        max_durability,
        BagFamily::new(bag_family),
        socket_bonus,
        gem_properties,
        required_disenchant_skill,
        armor_damage_modifier,
        disenchant_id,
        food_type,
        duration,
        extra_flags,
        EMPTY_ITEMSOCKET,
        damages,
        EMPTY_ITEMSTAT,
        EMPTY_SPELLS,
    )
}
pub const fn nd(
    entry: u16,
    class_and_sub_class: ItemClassAndSubClass,
    sound_override_sub_class: i8,
    name: &'static str,
    display_id: u16,
    quality: ItemQuality,
    flags: u32,
    buy_count: i16,
    buy_price: u32,
    sell_price: u32,
    inventory_type: InventoryType,
    allowed_class: u32,
    allowed_race: u32,
    item_level: i16,
    required_level: i8,
    required_skill: Skill,
    required_skill_rank: i16,
    required_faction: Faction,
    required_reputation_rank: i8,
    max_count: i32,
    stackable: i16,
    container_slots: i8,
    armor: i16,
    fire_res: i16,
    nature_res: i16,
    frost_res: i16,
    shadow_res: i16,
    arcane_res: i16,
    delay: i16,
    ammo_type: i8,
    ranged_mod_range: f32,
    bonding: Bonding,
    description: &'static str,
    page_text: i16,
    language: Language,
    page_text_material: PageTextMaterial,
    start_quest: i16,
    material: i8,
    sheathe_type: SheatheType,
    random_property: i16,
    random_suffix: i16,
    block: i16,
    item_set: ItemSet,
    max_durability: i16,
    bag_family: u32,
    socket_bonus: i16,
    gem_properties: i16,
    required_disenchant_skill: i16,
    armor_damage_modifier: f32,
    disenchant_id: i8,
    food_type: i8,
    duration: i32,
    extra_flags: i8,
    sockets: &'static [ItemSocket],
    damages: &'static [ItemDamageType],
    spells: &'static [Spells],
) -> Item {
    Item::new(
        entry,
        class_and_sub_class,
        sound_override_sub_class,
        name,
        display_id,
        quality,
        ItemFlag::new(flags),
        buy_count,
        Gold::new(buy_price),
        Gold::new(sell_price),
        inventory_type,
        AllowedClass::new(allowed_class),
        AllowedRace::new(allowed_race),
        item_level,
        required_level,
        required_skill,
        required_skill_rank,
        required_faction,
        required_reputation_rank,
        max_count,
        stackable,
        container_slots,
        armor,
        fire_res,
        nature_res,
        frost_res,
        shadow_res,
        arcane_res,
        delay,
        ammo_type,
        ranged_mod_range,
        bonding,
        description,
        page_text,
        language,
        page_text_material,
        start_quest,
        material,
        sheathe_type,
        random_property,
        random_suffix,
        block,
        item_set,
        max_durability,
        BagFamily::new(bag_family),
        socket_bonus,
        gem_properties,
        required_disenchant_skill,
        armor_damage_modifier,
        disenchant_id,
        food_type,
        duration,
        extra_flags,
        sockets,
        damages,
        EMPTY_ITEMSTAT,
        spells,
    )
}
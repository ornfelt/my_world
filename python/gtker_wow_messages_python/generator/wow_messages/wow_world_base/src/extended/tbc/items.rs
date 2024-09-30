use crate::tbc::{tbc_race_class_match, InventoryType, RaceClass};

tbc_race_class_match!(starter_items, &'static [StarterItem], RaceClass);

pub struct StarterItem {
    pub item: u32,
    pub ty: InventoryType,
    pub amount: u8,
}

impl StarterItem {
    pub(crate) const fn multi(item: u32, ty: InventoryType, amount: u8) -> Self {
        Self { item, ty, amount }
    }

    pub(crate) const fn single(item: u32, ty: InventoryType) -> Self {
        Self {
            item,
            ty,
            amount: 1,
        }
    }
}

const HUMAN_WARRIOR: &[StarterItem] = &[
    StarterItem::single(38, InventoryType::Body), // Recruit's Shirt
    StarterItem::single(39, InventoryType::Legs), // Recruit's Pants
    StarterItem::single(40, InventoryType::Feet), // Recruit's Boots
    StarterItem::single(25, InventoryType::WeaponMainHand), // Worn Shortsword
    StarterItem::single(2362, InventoryType::Shield), // Worn Wooden Shield
    StarterItem::multi(117, InventoryType::NonEquip, 5), // Tough Jerky
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
];
const HUMAN_PALADIN: &[StarterItem] = &[
    StarterItem::single(45, InventoryType::Body), // Squire's Shirt
    StarterItem::single(43, InventoryType::Feet), // Squire's Boots
    StarterItem::single(44, InventoryType::Legs), // Squire's Pants
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
    StarterItem::single(2361, InventoryType::TwoHandedWeapon), // Battleworn Hammer
    StarterItem::multi(159, InventoryType::NonEquip, 5), // Refreshing Spring Water
    StarterItem::multi(2070, InventoryType::NonEquip, 5), // Darnassian Bleu
];
const HUMAN_ROGUE: &[StarterItem] = &[
    StarterItem::single(49, InventoryType::Body), // Footpad's Shirt
    StarterItem::single(47, InventoryType::Feet), // Footpad's Shoes
    StarterItem::single(48, InventoryType::Legs), // Footpad's Pants
    StarterItem::single(28979, InventoryType::Thrown), // Light Throwing Knife
    StarterItem::single(2092, InventoryType::Weapon), // Worn Dagger
    StarterItem::multi(2070, InventoryType::NonEquip, 5), // Darnassian Bleu
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
];
const HUMAN_PRIEST: &[StarterItem] = &[
    StarterItem::single(6098, InventoryType::Robe), // Neophyte's Robe
    StarterItem::single(52, InventoryType::Legs),   // Neophyte's Pants
    StarterItem::single(53, InventoryType::Body),   // Neophyte's Shirt
    StarterItem::single(51, InventoryType::Feet),   // Neophyte's Boots
    StarterItem::single(36, InventoryType::WeaponMainHand), // Worn Mace
    StarterItem::multi(159, InventoryType::NonEquip, 5), // Refreshing Spring Water
    StarterItem::multi(2070, InventoryType::NonEquip, 5), // Darnassian Bleu
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
];
const HUMAN_MAGE: &[StarterItem] = &[
    StarterItem::single(56, InventoryType::Robe), // Apprentice's Robe
    StarterItem::single(1395, InventoryType::Legs), // Apprentice's Pants
    StarterItem::single(55, InventoryType::Feet), // Apprentice's Boots
    StarterItem::single(35, InventoryType::TwoHandedWeapon), // Bent Staff
    StarterItem::single(6096, InventoryType::Body), // Apprentice's Shirt
    StarterItem::multi(2070, InventoryType::NonEquip, 5), // Darnassian Bleu
    StarterItem::multi(159, InventoryType::NonEquip, 5), // Refreshing Spring Water
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
];
const HUMAN_WARLOCK: &[StarterItem] = &[
    StarterItem::single(57, InventoryType::Robe), // Acolyte's Robe
    StarterItem::single(6097, InventoryType::Body), // Acolyte's Shirt
    StarterItem::single(1396, InventoryType::Legs), // Acolyte's Pants
    StarterItem::single(2092, InventoryType::Weapon), // Worn Dagger
    StarterItem::single(59, InventoryType::Feet), // Acolyte's Shoes
    StarterItem::multi(4604, InventoryType::NonEquip, 5), // Forest Mushroom Cap
    StarterItem::multi(159, InventoryType::NonEquip, 5), // Refreshing Spring Water
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
];
const ORC_WARRIOR: &[StarterItem] = &[
    StarterItem::single(6125, InventoryType::Body), // Brawler's Harness
    StarterItem::single(139, InventoryType::Legs),  // Brawler's Pants
    StarterItem::single(140, InventoryType::Feet),  // Brawler's Boots
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
    StarterItem::single(12282, InventoryType::TwoHandedWeapon), // Worn Battleaxe
    StarterItem::multi(117, InventoryType::NonEquip, 5), // Tough Jerky
];
const ORC_HUNTER: &[StarterItem] = &[
    StarterItem::single(127, InventoryType::Body), // Trapper's Shirt
    StarterItem::single(6126, InventoryType::Legs), // Trapper's Pants
    StarterItem::single(6127, InventoryType::Feet), // Trapper's Boots
    StarterItem::multi(159, InventoryType::NonEquip, 5), // Refreshing Spring Water
    StarterItem::single(37, InventoryType::WeaponMainHand), // Worn Axe
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
    StarterItem::single(2101, InventoryType::Bag), // Light Quiver
    StarterItem::single(2504, InventoryType::Ranged), // Worn Shortbow
    StarterItem::multi(117, InventoryType::NonEquip, 5), // Tough Jerky
    StarterItem::multi(2512, InventoryType::Ammo, 200), // Rough Arrow
];
const ORC_ROGUE: &[StarterItem] = &[
    StarterItem::single(2105, InventoryType::Body), // Thug Shirt
    StarterItem::single(120, InventoryType::Legs),  // Thug Pants
    StarterItem::single(121, InventoryType::Feet),  // Thug Boots
    StarterItem::single(25861, InventoryType::Thrown), // Crude Throwing Axe
    StarterItem::single(2092, InventoryType::Weapon), // Worn Dagger
    StarterItem::multi(117, InventoryType::NonEquip, 5), // Tough Jerky
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
];
const ORC_SHAMAN: &[StarterItem] = &[
    StarterItem::single(154, InventoryType::Body), // Primitive Mantle
    StarterItem::single(153, InventoryType::Legs), // Primitive Kilt
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
    StarterItem::single(36, InventoryType::WeaponMainHand), // Worn Mace
    StarterItem::multi(117, InventoryType::NonEquip, 5), // Tough Jerky
    StarterItem::multi(159, InventoryType::NonEquip, 5), // Refreshing Spring Water
];
const ORC_WARLOCK: &[StarterItem] = &[
    StarterItem::single(6129, InventoryType::Robe), // Acolyte's Robe
    StarterItem::single(1396, InventoryType::Legs), // Acolyte's Pants
    StarterItem::single(59, InventoryType::Feet),   // Acolyte's Shoes
    StarterItem::single(2092, InventoryType::Weapon), // Worn Dagger
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
    StarterItem::multi(117, InventoryType::NonEquip, 5), // Tough Jerky
    StarterItem::multi(159, InventoryType::NonEquip, 5), // Refreshing Spring Water
];
const DWARF_WARRIOR: &[StarterItem] = &[
    StarterItem::single(38, InventoryType::Body), // Recruit's Shirt
    StarterItem::single(39, InventoryType::Legs), // Recruit's Pants
    StarterItem::single(40, InventoryType::Feet), // Recruit's Boots
    StarterItem::single(12282, InventoryType::TwoHandedWeapon), // Worn Battleaxe
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
    StarterItem::multi(117, InventoryType::NonEquip, 5), // Tough Jerky
];
const DWARF_PALADIN: &[StarterItem] = &[
    StarterItem::single(6117, InventoryType::Body), // Squire's Shirt
    StarterItem::single(6118, InventoryType::Legs), // Squire's Pants
    StarterItem::single(43, InventoryType::Feet),   // Squire's Boots
    StarterItem::single(2361, InventoryType::TwoHandedWeapon), // Battleworn Hammer
    StarterItem::multi(4540, InventoryType::NonEquip, 5), // Tough Hunk of Bread
    StarterItem::multi(159, InventoryType::NonEquip, 5), // Refreshing Spring Water
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
];
const DWARF_HUNTER: &[StarterItem] = &[
    StarterItem::single(148, InventoryType::Body), // Rugged Trapper's Shirt
    StarterItem::single(147, InventoryType::Legs), // Rugged Trapper's Pants
    StarterItem::single(129, InventoryType::Feet), // Rugged Trapper's Boots
    StarterItem::single(37, InventoryType::WeaponMainHand), // Worn Axe
    StarterItem::multi(159, InventoryType::NonEquip, 5), // Refreshing Spring Water
    StarterItem::single(2102, InventoryType::Bag), // Small Ammo Pouch
    StarterItem::single(2508, InventoryType::RangedRight), // Old Blunderbuss
    StarterItem::multi(2516, InventoryType::Ammo, 200), // Light Shot
    StarterItem::multi(117, InventoryType::NonEquip, 5), // Tough Jerky
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
];
const DWARF_ROGUE: &[StarterItem] = &[
    StarterItem::single(2092, InventoryType::Weapon), // Worn Dagger
    StarterItem::single(49, InventoryType::Body),     // Footpad's Shirt
    StarterItem::single(48, InventoryType::Legs),     // Footpad's Pants
    StarterItem::single(47, InventoryType::Feet),     // Footpad's Shoes
    StarterItem::multi(4540, InventoryType::NonEquip, 5), // Tough Hunk of Bread
    StarterItem::single(25861, InventoryType::Thrown), // Crude Throwing Axe
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
];
const DWARF_PRIEST: &[StarterItem] = &[
    StarterItem::single(6098, InventoryType::Robe), // Neophyte's Robe
    StarterItem::single(53, InventoryType::Body),   // Neophyte's Shirt
    StarterItem::single(52, InventoryType::Legs),   // Neophyte's Pants
    StarterItem::single(51, InventoryType::Feet),   // Neophyte's Boots
    StarterItem::single(36, InventoryType::WeaponMainHand), // Worn Mace
    StarterItem::multi(159, InventoryType::NonEquip, 5), // Refreshing Spring Water
    StarterItem::multi(4540, InventoryType::NonEquip, 5), // Tough Hunk of Bread
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
];
const NIGHT_ELF_WARRIOR: &[StarterItem] = &[
    StarterItem::single(25, InventoryType::WeaponMainHand), // Worn Shortsword
    StarterItem::single(6120, InventoryType::Body),         // Recruit's Shirt
    StarterItem::single(6121, InventoryType::Legs),         // Recruit's Pants
    StarterItem::single(6122, InventoryType::Feet),         // Recruit's Boots
    StarterItem::single(2362, InventoryType::Shield),       // Worn Wooden Shield
    StarterItem::multi(117, InventoryType::NonEquip, 5),    // Tough Jerky
    StarterItem::single(6948, InventoryType::NonEquip),     // Hearthstone
];
const NIGHT_ELF_HUNTER: &[StarterItem] = &[
    StarterItem::single(2092, InventoryType::Weapon), // Worn Dagger
    StarterItem::single(148, InventoryType::Body),    // Rugged Trapper's Shirt
    StarterItem::single(147, InventoryType::Legs),    // Rugged Trapper's Pants
    StarterItem::single(129, InventoryType::Feet),    // Rugged Trapper's Boots
    StarterItem::multi(159, InventoryType::NonEquip, 5), // Refreshing Spring Water
    StarterItem::single(2504, InventoryType::Ranged), // Worn Shortbow
    StarterItem::single(2101, InventoryType::Bag),    // Light Quiver
    StarterItem::multi(2512, InventoryType::Ammo, 200), // Rough Arrow
    StarterItem::multi(117, InventoryType::NonEquip, 5), // Tough Jerky
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
];
const NIGHT_ELF_ROGUE: &[StarterItem] = &[
    StarterItem::single(2092, InventoryType::Weapon), // Worn Dagger
    StarterItem::single(49, InventoryType::Body),     // Footpad's Shirt
    StarterItem::single(48, InventoryType::Legs),     // Footpad's Pants
    StarterItem::single(47, InventoryType::Feet),     // Footpad's Shoes
    StarterItem::multi(4540, InventoryType::NonEquip, 5), // Tough Hunk of Bread
    StarterItem::single(28979, InventoryType::Thrown), // Light Throwing Knife
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
];
const NIGHT_ELF_PRIEST: &[StarterItem] = &[
    StarterItem::single(36, InventoryType::WeaponMainHand), // Worn Mace
    StarterItem::single(6119, InventoryType::Robe),         // Neophyte's Robe
    StarterItem::single(52, InventoryType::Legs),           // Neophyte's Pants
    StarterItem::single(51, InventoryType::Feet),           // Neophyte's Boots
    StarterItem::single(53, InventoryType::Body),           // Neophyte's Shirt
    StarterItem::multi(2070, InventoryType::NonEquip, 5),   // Darnassian Bleu
    StarterItem::multi(159, InventoryType::NonEquip, 5),    // Refreshing Spring Water
    StarterItem::single(6948, InventoryType::NonEquip),     // Hearthstone
];
const NIGHT_ELF_DRUID: &[StarterItem] = &[
    StarterItem::single(3661, InventoryType::TwoHandedWeapon), // Handcrafted Staff
    StarterItem::single(6123, InventoryType::Robe),            // Novice's Robe
    StarterItem::single(6124, InventoryType::Legs),            // Novice's Pants
    StarterItem::multi(159, InventoryType::NonEquip, 5),       // Refreshing Spring Water
    StarterItem::multi(4536, InventoryType::NonEquip, 5),      // Shiny Red Apple
    StarterItem::single(6948, InventoryType::NonEquip),        // Hearthstone
];
const UNDEAD_WARRIOR: &[StarterItem] = &[
    StarterItem::single(6125, InventoryType::Body), // Brawler's Harness
    StarterItem::single(139, InventoryType::Legs),  // Brawler's Pants
    StarterItem::single(140, InventoryType::Feet),  // Brawler's Boots
    StarterItem::single(25, InventoryType::WeaponMainHand), // Worn Shortsword
    StarterItem::single(2362, InventoryType::Shield), // Worn Wooden Shield
    StarterItem::multi(4604, InventoryType::NonEquip, 5), // Forest Mushroom Cap
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
];
const UNDEAD_ROGUE: &[StarterItem] = &[
    StarterItem::single(2092, InventoryType::Weapon), // Worn Dagger
    StarterItem::single(2105, InventoryType::Body),   // Thug Shirt
    StarterItem::single(120, InventoryType::Legs),    // Thug Pants
    StarterItem::single(121, InventoryType::Feet),    // Thug Boots
    StarterItem::multi(4604, InventoryType::NonEquip, 5), // Forest Mushroom Cap
    StarterItem::single(25861, InventoryType::Thrown), // Crude Throwing Axe
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
];
const UNDEAD_PRIEST: &[StarterItem] = &[
    StarterItem::single(6144, InventoryType::Robe), // Neophyte's Robe
    StarterItem::single(53, InventoryType::Body),   // Neophyte's Shirt
    StarterItem::single(52, InventoryType::Legs),   // Neophyte's Pants
    StarterItem::single(36, InventoryType::WeaponMainHand), // Worn Mace
    StarterItem::multi(4604, InventoryType::NonEquip, 5), // Forest Mushroom Cap
    StarterItem::multi(159, InventoryType::NonEquip, 5), // Refreshing Spring Water
    StarterItem::single(51, InventoryType::Feet),   // Neophyte's Boots
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
];
const UNDEAD_MAGE: &[StarterItem] = &[
    StarterItem::single(6140, InventoryType::Robe), // Apprentice's Robe
    StarterItem::single(1395, InventoryType::Legs), // Apprentice's Pants
    StarterItem::single(6096, InventoryType::Body), // Apprentice's Shirt
    StarterItem::single(35, InventoryType::TwoHandedWeapon), // Bent Staff
    StarterItem::single(55, InventoryType::Feet),   // Apprentice's Boots
    StarterItem::multi(4604, InventoryType::NonEquip, 5), // Forest Mushroom Cap
    StarterItem::multi(159, InventoryType::NonEquip, 5), // Refreshing Spring Water
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
];
const UNDEAD_WARLOCK: &[StarterItem] = &[
    StarterItem::single(6129, InventoryType::Robe), // Acolyte's Robe
    StarterItem::single(1396, InventoryType::Legs), // Acolyte's Pants
    StarterItem::single(59, InventoryType::Feet),   // Acolyte's Shoes
    StarterItem::single(2092, InventoryType::Weapon), // Worn Dagger
    StarterItem::multi(4604, InventoryType::NonEquip, 5), // Forest Mushroom Cap
    StarterItem::multi(159, InventoryType::NonEquip, 5), // Refreshing Spring Water
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
];
const TAUREN_WARRIOR: &[StarterItem] = &[
    StarterItem::single(6125, InventoryType::Body), // Brawler's Harness
    StarterItem::single(139, InventoryType::Legs),  // Brawler's Pants
    StarterItem::single(2361, InventoryType::TwoHandedWeapon), // Battleworn Hammer
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
    StarterItem::multi(4540, InventoryType::NonEquip, 5), // Tough Hunk of Bread
];
const TAUREN_HUNTER: &[StarterItem] = &[
    StarterItem::single(127, InventoryType::Body), // Trapper's Shirt
    StarterItem::single(6126, InventoryType::Legs), // Trapper's Pants
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
    StarterItem::single(37, InventoryType::WeaponMainHand), // Worn Axe
    StarterItem::multi(159, InventoryType::NonEquip, 5), // Refreshing Spring Water
    StarterItem::single(2508, InventoryType::RangedRight), // Old Blunderbuss
    StarterItem::single(2102, InventoryType::Bag), // Small Ammo Pouch
    StarterItem::multi(2516, InventoryType::Ammo, 200), // Light Shot
    StarterItem::multi(117, InventoryType::NonEquip, 5), // Tough Jerky
];
const TAUREN_SHAMAN: &[StarterItem] = &[
    StarterItem::single(154, InventoryType::Body), // Primitive Mantle
    StarterItem::single(153, InventoryType::Legs), // Primitive Kilt
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
    StarterItem::single(36, InventoryType::WeaponMainHand), // Worn Mace
    StarterItem::multi(4604, InventoryType::NonEquip, 5), // Forest Mushroom Cap
    StarterItem::multi(159, InventoryType::NonEquip, 5), // Refreshing Spring Water
];
const TAUREN_DRUID: &[StarterItem] = &[
    StarterItem::single(35, InventoryType::TwoHandedWeapon), // Bent Staff
    StarterItem::single(6139, InventoryType::Robe),          // Novice's Robe
    StarterItem::single(6124, InventoryType::Legs),          // Novice's Pants
    StarterItem::multi(159, InventoryType::NonEquip, 5),     // Refreshing Spring Water
    StarterItem::multi(4536, InventoryType::NonEquip, 5),    // Shiny Red Apple
    StarterItem::single(6948, InventoryType::NonEquip),      // Hearthstone
];
const GNOME_WARRIOR: &[StarterItem] = &[
    StarterItem::single(38, InventoryType::Body), // Recruit's Shirt
    StarterItem::single(39, InventoryType::Legs), // Recruit's Pants
    StarterItem::single(40, InventoryType::Feet), // Recruit's Boots
    StarterItem::single(25, InventoryType::WeaponMainHand), // Worn Shortsword
    StarterItem::single(2362, InventoryType::Shield), // Worn Wooden Shield
    StarterItem::multi(117, InventoryType::NonEquip, 5), // Tough Jerky
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
];
const GNOME_ROGUE: &[StarterItem] = &[
    StarterItem::single(49, InventoryType::Body), // Footpad's Shirt
    StarterItem::single(48, InventoryType::Legs), // Footpad's Pants
    StarterItem::single(47, InventoryType::Feet), // Footpad's Shoes
    StarterItem::single(2092, InventoryType::Weapon), // Worn Dagger
    StarterItem::single(28979, InventoryType::Thrown), // Light Throwing Knife
    StarterItem::multi(117, InventoryType::NonEquip, 5), // Tough Jerky
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
];
const GNOME_MAGE: &[StarterItem] = &[
    StarterItem::single(56, InventoryType::Robe), // Apprentice's Robe
    StarterItem::single(1395, InventoryType::Legs), // Apprentice's Pants
    StarterItem::single(6096, InventoryType::Body), // Apprentice's Shirt
    StarterItem::single(55, InventoryType::Feet), // Apprentice's Boots
    StarterItem::single(35, InventoryType::TwoHandedWeapon), // Bent Staff
    StarterItem::multi(4536, InventoryType::NonEquip, 5), // Shiny Red Apple
    StarterItem::multi(159, InventoryType::NonEquip, 5), // Refreshing Spring Water
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
];
const GNOME_WARLOCK: &[StarterItem] = &[
    StarterItem::single(57, InventoryType::Robe), // Acolyte's Robe
    StarterItem::single(6097, InventoryType::Body), // Acolyte's Shirt
    StarterItem::single(1396, InventoryType::Legs), // Acolyte's Pants
    StarterItem::single(59, InventoryType::Feet), // Acolyte's Shoes
    StarterItem::single(2092, InventoryType::Weapon), // Worn Dagger
    StarterItem::multi(159, InventoryType::NonEquip, 5), // Refreshing Spring Water
    StarterItem::multi(4604, InventoryType::NonEquip, 5), // Forest Mushroom Cap
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
];
const TROLL_WARRIOR: &[StarterItem] = &[
    StarterItem::single(37, InventoryType::WeaponMainHand), // Worn Axe
    StarterItem::multi(117, InventoryType::NonEquip, 5),    // Tough Jerky
    StarterItem::single(6125, InventoryType::Body),         // Brawler's Harness
    StarterItem::single(139, InventoryType::Legs),          // Brawler's Pants
    StarterItem::single(2362, InventoryType::Shield),       // Worn Wooden Shield
    StarterItem::single(25861, InventoryType::Thrown),      // Crude Throwing Axe
    StarterItem::single(6948, InventoryType::NonEquip),     // Hearthstone
];
const TROLL_HUNTER: &[StarterItem] = &[
    StarterItem::single(37, InventoryType::WeaponMainHand), // Worn Axe
    StarterItem::multi(4604, InventoryType::NonEquip, 5),   // Forest Mushroom Cap
    StarterItem::single(2101, InventoryType::Bag),          // Light Quiver
    StarterItem::multi(2512, InventoryType::Ammo, 200),     // Rough Arrow
    StarterItem::single(2504, InventoryType::Ranged),       // Worn Shortbow
    StarterItem::single(6126, InventoryType::Legs),         // Trapper's Pants
    StarterItem::single(127, InventoryType::Body),          // Trapper's Shirt
    StarterItem::multi(159, InventoryType::NonEquip, 5),    // Refreshing Spring Water
    StarterItem::single(6948, InventoryType::NonEquip),     // Hearthstone
];
const TROLL_ROGUE: &[StarterItem] = &[
    StarterItem::single(2092, InventoryType::Weapon), // Worn Dagger
    StarterItem::multi(117, InventoryType::NonEquip, 5), // Tough Jerky
    StarterItem::single(6136, InventoryType::Body),   // Thug Shirt
    StarterItem::single(6137, InventoryType::Legs),   // Thug Pants
    StarterItem::single(6138, InventoryType::Feet),   // Thug Boots
    StarterItem::single(25861, InventoryType::Thrown), // Crude Throwing Axe
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
];
const TROLL_PRIEST: &[StarterItem] = &[
    StarterItem::single(36, InventoryType::WeaponMainHand), // Worn Mace
    StarterItem::multi(4540, InventoryType::NonEquip, 5),   // Tough Hunk of Bread
    StarterItem::multi(159, InventoryType::NonEquip, 5),    // Refreshing Spring Water
    StarterItem::single(6144, InventoryType::Robe),         // Neophyte's Robe
    StarterItem::single(53, InventoryType::Body),           // Neophyte's Shirt
    StarterItem::single(52, InventoryType::Legs),           // Neophyte's Pants
    StarterItem::single(6948, InventoryType::NonEquip),     // Hearthstone
];
const TROLL_SHAMAN: &[StarterItem] = &[
    StarterItem::single(36, InventoryType::WeaponMainHand), // Worn Mace
    StarterItem::multi(117, InventoryType::NonEquip, 5),    // Tough Jerky
    StarterItem::multi(159, InventoryType::NonEquip, 5),    // Refreshing Spring Water
    StarterItem::single(6134, InventoryType::Body),         // Primitive Mantle
    StarterItem::single(6135, InventoryType::Legs),         // Primitive Kilt
    StarterItem::single(6948, InventoryType::NonEquip),     // Hearthstone
];
const TROLL_MAGE: &[StarterItem] = &[
    StarterItem::single(6140, InventoryType::Robe), // Apprentice's Robe
    StarterItem::single(1395, InventoryType::Legs), // Apprentice's Pants
    StarterItem::single(6096, InventoryType::Body), // Apprentice's Shirt
    StarterItem::single(55, InventoryType::Feet),   // Apprentice's Boots
    StarterItem::single(35, InventoryType::TwoHandedWeapon), // Bent Staff
    StarterItem::multi(117, InventoryType::NonEquip, 5), // Tough Jerky
    StarterItem::multi(159, InventoryType::NonEquip, 5), // Refreshing Spring Water
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
];
const BLOOD_ELF_PALADIN: &[StarterItem] = &[
    StarterItem::single(24143, InventoryType::Body), // Initiate's Shirt
    StarterItem::single(24145, InventoryType::Legs), // Initiate's Pants
    StarterItem::single(24146, InventoryType::Feet), // Initiate's Boots
    StarterItem::single(23346, InventoryType::TwoHandedWeapon), // Battleworn Claymore
    StarterItem::single(20857, InventoryType::NonEquip), // Honey Bread
    StarterItem::multi(159, InventoryType::NonEquip, 5), // Refreshing Spring Water
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
];
const BLOOD_ELF_HUNTER: &[StarterItem] = &[
    StarterItem::single(20901, InventoryType::Body), // Warder's Shirt
    StarterItem::single(20899, InventoryType::Legs), // Warder's Pants
    StarterItem::single(20900, InventoryType::Feet), // Warder's Boots
    StarterItem::multi(159, InventoryType::NonEquip, 5), // Refreshing Spring Water
    StarterItem::single(20982, InventoryType::Weapon), // Sharp Dagger
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
    StarterItem::single(2101, InventoryType::Bag),   // Light Quiver
    StarterItem::single(20980, InventoryType::Ranged), // Warder's Shortbow
    StarterItem::single(20857, InventoryType::NonEquip), // Honey Bread
    StarterItem::multi(2512, InventoryType::Ammo, 200), // Rough Arrow
];
const BLOOD_ELF_ROGUE: &[StarterItem] = &[
    StarterItem::single(20982, InventoryType::Weapon), // Sharp Dagger
    StarterItem::single(20897, InventoryType::Body),   // Lookout's Tunic
    StarterItem::single(20896, InventoryType::Legs),   // Lookout's Pants
    StarterItem::single(20898, InventoryType::Feet),   // Lookout's Shoes
    StarterItem::single(20857, InventoryType::NonEquip), // Honey Bread
    StarterItem::single(25861, InventoryType::Thrown), // Crude Throwing Axe
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
];
const BLOOD_ELF_PRIEST: &[StarterItem] = &[
    StarterItem::single(20891, InventoryType::Robe), // Neophyte's Robe
    StarterItem::single(53, InventoryType::Body),    // Neophyte's Shirt
    StarterItem::single(52, InventoryType::Legs),    // Neophyte's Pants
    StarterItem::single(20981, InventoryType::WeaponMainHand), // Neophyte's Mace
    StarterItem::single(20857, InventoryType::NonEquip), // Honey Bread
    StarterItem::multi(159, InventoryType::NonEquip, 5), // Refreshing Spring Water
    StarterItem::single(51, InventoryType::Feet),    // Neophyte's Boots
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
];
const BLOOD_ELF_MAGE: &[StarterItem] = &[
    StarterItem::single(20893, InventoryType::Robe), // Apprentice's Robe
    StarterItem::single(20894, InventoryType::Legs), // Apprentice's Pants
    StarterItem::single(6096, InventoryType::Body),  // Apprentice's Shirt
    StarterItem::single(35, InventoryType::TwoHandedWeapon), // Bent Staff
    StarterItem::single(20895, InventoryType::Feet), // Apprentice's Boots
    StarterItem::single(20857, InventoryType::NonEquip), // Honey Bread
    StarterItem::multi(159, InventoryType::NonEquip, 5), // Refreshing Spring Water
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
];
const BLOOD_ELF_WARLOCK: &[StarterItem] = &[
    StarterItem::single(20892, InventoryType::Robe), // Acolyte's Robe
    StarterItem::single(1396, InventoryType::Legs),  // Acolyte's Pants
    StarterItem::single(59, InventoryType::Feet),    // Acolyte's Shoes
    StarterItem::single(20983, InventoryType::Weapon), // Acolyte's Dagger
    StarterItem::single(6097, InventoryType::Body),  // Acolyte's Shirt
    StarterItem::single(20857, InventoryType::NonEquip), // Honey Bread
    StarterItem::multi(159, InventoryType::NonEquip, 5), // Refreshing Spring Water
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
];
const DRAENEI_WARRIOR: &[StarterItem] = &[
    StarterItem::single(23473, InventoryType::Body), // Recruit's Shirt
    StarterItem::single(23474, InventoryType::Legs), // Recruit's Pants
    StarterItem::single(23475, InventoryType::Feet), // Recruit's Boots
    StarterItem::single(23346, InventoryType::TwoHandedWeapon), // Battleworn Claymore
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
    StarterItem::multi(4540, InventoryType::NonEquip, 5), // Tough Hunk of Bread
];
const DRAENEI_PALADIN: &[StarterItem] = &[
    StarterItem::single(23476, InventoryType::Body), // Squire's Shirt
    StarterItem::single(23477, InventoryType::Legs), // Squire's Pants
    StarterItem::single(2361, InventoryType::TwoHandedWeapon), // Battleworn Hammer
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
    StarterItem::multi(4540, InventoryType::NonEquip, 5), // Tough Hunk of Bread
    StarterItem::multi(159, InventoryType::NonEquip, 5), // Refreshing Spring Water
];
const DRAENEI_HUNTER: &[StarterItem] = &[
    StarterItem::single(23345, InventoryType::Body), // Scout's Shirt
    StarterItem::single(23344, InventoryType::Legs), // Scout's Pants
    StarterItem::single(25, InventoryType::WeaponMainHand), // Worn Shortsword
    StarterItem::single(23348, InventoryType::Feet), // Scout's Boots
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
    StarterItem::multi(4540, InventoryType::NonEquip, 5), // Tough Hunk of Bread
    StarterItem::multi(159, InventoryType::NonEquip, 5), // Refreshing Spring Water
    StarterItem::single(23347, InventoryType::RangedRight), // Weathered Crossbow
    StarterItem::single(2101, InventoryType::Bag),   // Light Quiver
    StarterItem::multi(2512, InventoryType::Ammo, 200), // Rough Arrow
];
const DRAENEI_PRIEST: &[StarterItem] = &[
    StarterItem::single(1396, InventoryType::Legs), // Acolyte's Pants
    StarterItem::single(6097, InventoryType::Body), // Acolyte's Shirt
    StarterItem::single(36, InventoryType::WeaponMainHand), // Worn Mace
    StarterItem::single(23322, InventoryType::Robe), // Acolyte's Robe
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
    StarterItem::multi(4540, InventoryType::NonEquip, 5), // Tough Hunk of Bread
    StarterItem::multi(159, InventoryType::NonEquip, 5), // Refreshing Spring Water
];
const DRAENEI_SHAMAN: &[StarterItem] = &[
    StarterItem::single(23345, InventoryType::Body), // Scout's Shirt
    StarterItem::single(23344, InventoryType::Legs), // Scout's Pants
    StarterItem::single(23348, InventoryType::Feet), // Scout's Boots
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
    StarterItem::single(36, InventoryType::WeaponMainHand), // Worn Mace
    StarterItem::multi(4540, InventoryType::NonEquip, 5), // Tough Hunk of Bread
    StarterItem::multi(159, InventoryType::NonEquip, 5), // Refreshing Spring Water
];
const DRAENEI_MAGE: &[StarterItem] = &[
    StarterItem::single(23478, InventoryType::Legs), // Recruit's Pants
    StarterItem::single(23479, InventoryType::Robe), // Recruit's Robe
    StarterItem::single(35, InventoryType::TwoHandedWeapon), // Bent Staff
    StarterItem::single(23473, InventoryType::Body), // Recruit's Shirt
    StarterItem::single(6948, InventoryType::NonEquip), // Hearthstone
    StarterItem::multi(4540, InventoryType::NonEquip, 5), // Tough Hunk of Bread
    StarterItem::multi(159, InventoryType::NonEquip, 5), // Refreshing Spring Water
];

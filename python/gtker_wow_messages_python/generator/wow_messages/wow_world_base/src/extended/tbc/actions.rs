use crate::extended::shared::Action;
use crate::extended::tbc::tbc_race_class_match;

tbc_race_class_match!(starter_actions, &'static [Action], crate::tbc::RaceClass);

// AUTOGENERATED_START
const HUMAN_WARRIOR: &[Action] = &[
    Action::new(72, 6603, 0),
    Action::new(73, 78, 0),
    Action::new(83, 117, 128),
];
const HUMAN_PALADIN: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 20154, 0),
    Action::new(2, 635, 0),
    Action::new(10, 159, 128),
    Action::new(11, 2070, 128),
];
const HUMAN_ROGUE: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 1752, 0),
    Action::new(2, 2098, 0),
    Action::new(3, 2764, 0),
    Action::new(11, 2070, 128),
];
const HUMAN_PRIEST: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 585, 0),
    Action::new(2, 2050, 0),
    Action::new(10, 159, 128),
    Action::new(11, 2070, 128),
];
const HUMAN_MAGE: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 133, 0),
    Action::new(2, 168, 0),
    Action::new(10, 159, 128),
    Action::new(11, 2070, 128),
];
const HUMAN_WARLOCK: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 686, 0),
    Action::new(2, 687, 0),
    Action::new(10, 159, 128),
    Action::new(11, 4604, 128),
];
const ORC_WARRIOR: &[Action] = &[
    Action::new(72, 6603, 0),
    Action::new(73, 78, 0),
    Action::new(83, 117, 128),
];
const ORC_HUNTER: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 2973, 0),
    Action::new(2, 75, 0),
    Action::new(10, 159, 128),
    Action::new(11, 117, 128),
];
const ORC_ROGUE: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 1752, 0),
    Action::new(2, 2098, 0),
    Action::new(11, 117, 128),
];
const ORC_SHAMAN: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 403, 0),
    Action::new(2, 331, 0),
    Action::new(10, 159, 128),
    Action::new(11, 117, 128),
];
const ORC_WARLOCK: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 686, 0),
    Action::new(2, 687, 0),
    Action::new(10, 159, 128),
    Action::new(11, 117, 128),
];
const DWARF_WARRIOR: &[Action] = &[
    Action::new(72, 6603, 0),
    Action::new(73, 78, 0),
    Action::new(83, 117, 128),
];
const DWARF_PALADIN: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 20154, 0),
    Action::new(2, 635, 0),
    Action::new(10, 159, 128),
    Action::new(11, 4540, 128),
];
const DWARF_HUNTER: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 2973, 0),
    Action::new(2, 75, 0),
    Action::new(10, 159, 128),
    Action::new(11, 117, 128),
];
const DWARF_ROGUE: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 1752, 0),
    Action::new(2, 2098, 0),
    Action::new(3, 2764, 0),
    Action::new(11, 4540, 128),
];
const DWARF_PRIEST: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 585, 0),
    Action::new(2, 2050, 0),
    Action::new(10, 159, 128),
    Action::new(11, 4540, 128),
];
const NIGHT_ELF_WARRIOR: &[Action] = &[
    Action::new(72, 6603, 0),
    Action::new(73, 78, 0),
    Action::new(74, 20580, 0),
    Action::new(83, 117, 128),
];
const NIGHT_ELF_HUNTER: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 2973, 0),
    Action::new(2, 75, 0),
    Action::new(3, 20580, 0),
    Action::new(10, 159, 128),
    Action::new(11, 117, 128),
];
const NIGHT_ELF_ROGUE: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 1752, 0),
    Action::new(2, 2098, 0),
    Action::new(3, 2764, 0),
    Action::new(4, 20580, 0),
    Action::new(11, 4540, 128),
];
const NIGHT_ELF_PRIEST: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 585, 0),
    Action::new(2, 2050, 0),
    Action::new(3, 20580, 0),
    Action::new(10, 159, 128),
    Action::new(11, 2070, 128),
];
const NIGHT_ELF_DRUID: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 5176, 0),
    Action::new(2, 5185, 0),
    Action::new(3, 20580, 0),
    Action::new(10, 159, 128),
    Action::new(11, 4536, 128),
];
const UNDEAD_WARRIOR: &[Action] = &[
    Action::new(72, 6603, 0),
    Action::new(73, 78, 0),
    Action::new(83, 4604, 128),
];
const UNDEAD_ROGUE: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 1752, 0),
    Action::new(2, 2098, 0),
    Action::new(3, 2764, 0),
    Action::new(11, 4604, 128),
];
const UNDEAD_PRIEST: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 585, 0),
    Action::new(2, 2050, 0),
    Action::new(10, 159, 128),
    Action::new(11, 4604, 128),
];
const UNDEAD_MAGE: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 133, 0),
    Action::new(2, 168, 0),
    Action::new(10, 159, 128),
    Action::new(11, 4604, 128),
];
const UNDEAD_WARLOCK: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 686, 0),
    Action::new(2, 687, 0),
    Action::new(10, 159, 128),
    Action::new(11, 4604, 128),
];
const TAUREN_WARRIOR: &[Action] = &[
    Action::new(72, 6603, 0),
    Action::new(73, 78, 0),
    Action::new(74, 20549, 0),
    Action::new(83, 4540, 128),
];
const TAUREN_HUNTER: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 2973, 0),
    Action::new(2, 75, 0),
    Action::new(3, 20549, 0),
    Action::new(10, 159, 128),
    Action::new(11, 117, 128),
];
const TAUREN_SHAMAN: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 403, 0),
    Action::new(2, 331, 0),
    Action::new(3, 20549, 0),
    Action::new(10, 159, 128),
    Action::new(11, 4604, 128),
];
const TAUREN_DRUID: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 5176, 0),
    Action::new(2, 5185, 0),
    Action::new(3, 20549, 0),
    Action::new(10, 159, 128),
    Action::new(11, 4536, 128),
];
const GNOME_WARRIOR: &[Action] = &[
    Action::new(72, 6603, 0),
    Action::new(73, 78, 0),
    Action::new(83, 117, 128),
];
const GNOME_ROGUE: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 1752, 0),
    Action::new(2, 2098, 0),
    Action::new(3, 2764, 0),
    Action::new(11, 117, 128),
];
const GNOME_MAGE: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 133, 0),
    Action::new(2, 168, 0),
    Action::new(10, 159, 128),
    Action::new(11, 4536, 128),
];
const GNOME_WARLOCK: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 686, 0),
    Action::new(2, 687, 0),
    Action::new(10, 159, 128),
    Action::new(11, 4604, 128),
];
const TROLL_WARRIOR: &[Action] = &[
    Action::new(72, 6603, 0),
    Action::new(73, 78, 0),
    Action::new(74, 2764, 0),
    Action::new(83, 117, 128),
];
const TROLL_HUNTER: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 2973, 0),
    Action::new(2, 75, 0),
    Action::new(10, 159, 128),
    Action::new(11, 4604, 128),
];
const TROLL_ROGUE: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 1752, 0),
    Action::new(2, 2098, 0),
    Action::new(3, 2764, 0),
    Action::new(11, 117, 128),
];
const TROLL_PRIEST: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 585, 0),
    Action::new(2, 2050, 0),
    Action::new(10, 159, 128),
    Action::new(11, 4540, 128),
];
const TROLL_SHAMAN: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 403, 0),
    Action::new(2, 331, 0),
    Action::new(10, 159, 128),
    Action::new(11, 117, 128),
];
const TROLL_MAGE: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 133, 0),
    Action::new(2, 168, 0),
    Action::new(10, 159, 128),
    Action::new(11, 117, 128),
];
const BLOOD_ELF_PALADIN: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 20154, 0),
    Action::new(2, 635, 0),
    Action::new(3, 28734, 0),
    Action::new(4, 28730, 0),
    Action::new(10, 159, 128),
    Action::new(11, 20857, 128),
];
const BLOOD_ELF_HUNTER: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 2973, 0),
    Action::new(2, 75, 0),
    Action::new(3, 28734, 0),
    Action::new(4, 28730, 0),
    Action::new(10, 159, 128),
    Action::new(11, 20857, 128),
];
const BLOOD_ELF_ROGUE: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 1752, 0),
    Action::new(2, 2098, 0),
    Action::new(3, 2764, 0),
    Action::new(4, 28734, 0),
    Action::new(5, 25046, 0),
    Action::new(11, 20857, 128),
];
const BLOOD_ELF_PRIEST: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 585, 0),
    Action::new(2, 2050, 0),
    Action::new(3, 28734, 0),
    Action::new(4, 28730, 0),
    Action::new(10, 159, 128),
    Action::new(11, 20857, 128),
];
const BLOOD_ELF_MAGE: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 133, 0),
    Action::new(2, 168, 0),
    Action::new(3, 28734, 0),
    Action::new(4, 28730, 0),
    Action::new(10, 159, 128),
    Action::new(11, 20857, 128),
];
const BLOOD_ELF_WARLOCK: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 686, 0),
    Action::new(2, 687, 0),
    Action::new(3, 28734, 0),
    Action::new(4, 28730, 0),
    Action::new(10, 159, 128),
    Action::new(11, 20857, 128),
];
const DRAENEI_WARRIOR: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(72, 6603, 0),
    Action::new(73, 78, 0),
    Action::new(74, 28880, 0),
    Action::new(83, 4540, 128),
    Action::new(84, 6603, 0),
    Action::new(96, 6603, 0),
    Action::new(108, 6603, 0),
];
const DRAENEI_PALADIN: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 20154, 0),
    Action::new(2, 635, 0),
    Action::new(3, 28880, 0),
    Action::new(10, 159, 128),
    Action::new(11, 4540, 128),
    Action::new(83, 4540, 128),
];
const DRAENEI_HUNTER: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 2973, 0),
    Action::new(2, 75, 0),
    Action::new(3, 28880, 0),
    Action::new(10, 159, 128),
    Action::new(11, 4540, 128),
    Action::new(72, 6603, 0),
    Action::new(73, 2973, 0),
    Action::new(74, 75, 0),
    Action::new(82, 159, 128),
    Action::new(83, 4540, 128),
];
const DRAENEI_PRIEST: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 585, 0),
    Action::new(2, 2050, 0),
    Action::new(3, 28880, 0),
    Action::new(10, 159, 128),
    Action::new(11, 4540, 128),
    Action::new(83, 4540, 128),
];
const DRAENEI_SHAMAN: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 403, 0),
    Action::new(2, 331, 0),
    Action::new(3, 28880, 0),
    Action::new(10, 159, 128),
    Action::new(11, 4540, 128),
];
const DRAENEI_MAGE: &[Action] = &[
    Action::new(0, 6603, 0),
    Action::new(1, 133, 0),
    Action::new(2, 168, 0),
    Action::new(3, 28880, 0),
    Action::new(10, 159, 128),
    Action::new(11, 4540, 128),
    Action::new(83, 4540, 128),
];
// AUTOGENERATED_END
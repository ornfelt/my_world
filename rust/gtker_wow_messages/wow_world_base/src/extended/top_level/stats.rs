pub const fn health_bonus_from_stamina(stamina: u8) -> u16 {
    let base_stamina = if stamina < 20 { stamina as u16 } else { 20 };
    let additional_stamina = stamina as u16 - base_stamina;

    base_stamina + (additional_stamina * 10)
}

pub const fn calculate_health(base_health: u16, stamina: u8) -> u16 {
    base_health + health_bonus_from_stamina(stamina)
}

pub const fn mana_bonus_from_intellect(intellect: u8) -> u16 {
    let base_intellect = if intellect < 20 { intellect as u16 } else { 20 };
    let additional_intellect = intellect as u16 - base_intellect;

    base_intellect + (additional_intellect * 15)
}

pub const fn calculate_mana(base_mana: u16, intellect: u8) -> u16 {
    base_mana + mana_bonus_from_intellect(intellect)
}

#[derive(Debug, Copy, Clone, Ord, PartialOrd, Eq, PartialEq, Hash)]
pub struct BaseStats {
    pub strength: u8,
    pub agility: u8,
    pub stamina: u8,
    pub intellect: u8,
    pub spirit: u8,
    pub health: u16,
    pub mana: u16,
}

impl BaseStats {
    pub const fn new(
        strength: u8,
        agility: u8,
        stamina: u8,
        intellect: u8,
        spirit: u8,
        health: u16,
        mana: u16,
    ) -> Self {
        Self {
            strength,
            agility,
            stamina,
            intellect,
            spirit,
            health,
            mana,
        }
    }
}

/// Auto generated from the original `wowm` in file [`wow_message_parser/wowm/world/queries/smsg_item_query_single_response.wowm:31`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/queries/smsg_item_query_single_response.wowm#L31):
/// ```text
/// enum SpellTriggerType : u8 {
///     ON_USE = 0;
///     ON_EQUIP = 1;
///     CHANCE_ON_HIT = 2;
/// }
/// ```
#[derive(Debug, PartialEq, Eq, Hash, Ord, PartialOrd, Copy, Clone)]
#[cfg_attr(feature = "serde", derive(serde::Deserialize, serde::Serialize))]
pub enum SpellTriggerType {
    OnUse,
    OnEquip,
    ChanceOnHit,
}

impl SpellTriggerType {
    pub const fn as_int(&self) -> u8 {
        match self {
            Self::OnUse => 0x0,
            Self::OnEquip => 0x1,
            Self::ChanceOnHit => 0x2,
        }
    }

    pub const fn variants() -> [Self; 3] {
        [
            Self::OnUse,
            Self::OnEquip,
            Self::ChanceOnHit,
        ]
    }

    pub const fn from_int(value: u8) -> Result<Self, crate::errors::EnumError> {
        match value {
            0 => Ok(Self::OnUse),
            1 => Ok(Self::OnEquip),
            2 => Ok(Self::ChanceOnHit),
            v => Err(crate::errors::EnumError::new(NAME, v as i128),)
        }
    }
}

#[cfg(feature = "print-testcase")]
impl SpellTriggerType {
    pub const fn as_test_case_value(&self) -> &'static str {
        match self {
            Self::OnUse => "ON_USE",
            Self::OnEquip => "ON_EQUIP",
            Self::ChanceOnHit => "CHANCE_ON_HIT",
        }
    }

}

const NAME: &str = "SpellTriggerType";

impl Default for SpellTriggerType {
    fn default() -> Self {
        Self::OnUse
    }
}

impl std::fmt::Display for SpellTriggerType {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::OnUse => f.write_str("OnUse"),
            Self::OnEquip => f.write_str("OnEquip"),
            Self::ChanceOnHit => f.write_str("ChanceOnHit"),
        }
    }
}

impl TryFrom<u8> for SpellTriggerType {
    type Error = crate::errors::EnumError;
    fn try_from(value: u8) -> Result<Self, Self::Error> {
        Self::from_int(value)
    }
}

impl TryFrom<u16> for SpellTriggerType {
    type Error = crate::errors::EnumError;
    fn try_from(value: u16) -> Result<Self, Self::Error> {
        TryInto::<u8>::try_into(value)
            .map_err(|_| crate::errors::EnumError::new(NAME, value.into()))?
            .try_into()
    }
}

impl TryFrom<u32> for SpellTriggerType {
    type Error = crate::errors::EnumError;
    fn try_from(value: u32) -> Result<Self, Self::Error> {
        TryInto::<u8>::try_into(value)
            .map_err(|_| crate::errors::EnumError::new(NAME, value.into()))?
            .try_into()
    }
}

impl TryFrom<u64> for SpellTriggerType {
    type Error = crate::errors::EnumError;
    fn try_from(value: u64) -> Result<Self, Self::Error> {
        TryInto::<u8>::try_into(value)
            .map_err(|_| crate::errors::EnumError::new(NAME, value.into()))?
            .try_into()
    }
}

impl TryFrom<i8> for SpellTriggerType {
    type Error = crate::errors::EnumError;
    fn try_from(value: i8) -> Result<Self, Self::Error> {
        let v = u8::from_le_bytes(value.to_le_bytes());
        Self::from_int(v)
    }
}

impl TryFrom<i16> for SpellTriggerType {
    type Error = crate::errors::EnumError;
    fn try_from(value: i16) -> Result<Self, Self::Error> {
        TryInto::<u8>::try_into(value)
            .map_err(|_| crate::errors::EnumError::new(NAME, value.into()))?
            .try_into()
    }
}

impl TryFrom<i32> for SpellTriggerType {
    type Error = crate::errors::EnumError;
    fn try_from(value: i32) -> Result<Self, Self::Error> {
        TryInto::<u8>::try_into(value)
            .map_err(|_| crate::errors::EnumError::new(NAME, value.into()))?
            .try_into()
    }
}

impl TryFrom<i64> for SpellTriggerType {
    type Error = crate::errors::EnumError;
    fn try_from(value: i64) -> Result<Self, Self::Error> {
        TryInto::<u8>::try_into(value)
            .map_err(|_| crate::errors::EnumError::new(NAME, value.into()))?
            .try_into()
    }
}

impl TryFrom<usize> for SpellTriggerType {
    type Error = crate::errors::EnumError;
    fn try_from(value: usize) -> Result<Self, Self::Error> {
        TryInto::<u8>::try_into(value)
            .map_err(|_| crate::errors::EnumError::new(NAME, value as i128))?
            .try_into()
    }
}

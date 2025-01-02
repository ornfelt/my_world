use crate::inner::shared::gender_vanilla_tbc_wrath::Gender;

/// Enum containing only Male and Female.
///
/// Player characters must be either male or female for e.g. display ids
/// and can not legally choose anything else through the client.
#[derive(Debug, Clone, Copy, Ord, PartialOrd, Eq, PartialEq, Hash)]
#[cfg_attr(feature = "serde", derive(serde::Deserialize, serde::Serialize))]
pub enum PlayerGender {
    Male,
    Female,
}

impl TryFrom<Gender> for PlayerGender {
    type Error = Gender;

    fn try_from(value: Gender) -> Result<Self, Self::Error> {
        Ok(match value {
            Gender::Male => Self::Male,
            Gender::Female => Self::Female,
            gender => return Err(gender),
        })
    }
}

impl From<PlayerGender> for Gender {
    fn from(v: PlayerGender) -> Self {
        match v {
            PlayerGender::Male => Self::Male,
            PlayerGender::Female => Self::Female,
        }
    }
}

impl Default for PlayerGender {
    fn default() -> Self {
        Self::Male
    }
}

impl std::fmt::Display for PlayerGender {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.write_str(match self {
            PlayerGender::Male => "Male",
            PlayerGender::Female => "Female",
        })
    }
}

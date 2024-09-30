use crate::{
    DbcTable, Indexable,
};
use crate::header::{
    DbcHeader, HEADER_SIZE, parse_header,
};
use crate::wrath_tables::light_params::LightParamsKey;
use crate::wrath_tables::sound_ambience::SoundAmbienceKey;
use crate::wrath_tables::zone_music::ZoneMusicKey;
use std::io::Write;

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct ScreenEffect {
    pub rows: Vec<ScreenEffectRow>,
}

impl DbcTable for ScreenEffect {
    type Row = ScreenEffectRow;

    const FILENAME: &'static str = "ScreenEffect.dbc";

    fn rows(&self) -> &[Self::Row] { &self.rows }
    fn rows_mut(&mut self) -> &mut [Self::Row] { &mut self.rows }

    fn read(b: &mut impl std::io::Read) -> Result<Self, crate::DbcError> {
        let mut header = [0_u8; HEADER_SIZE];
        b.read_exact(&mut header)?;
        let header = parse_header(&header)?;

        if header.record_size != 40 {
            return Err(crate::DbcError::InvalidHeader(
                crate::InvalidHeaderError::RecordSize {
                    expected: 40,
                    actual: header.record_size,
                },
            ));
        }

        if header.field_count != 10 {
            return Err(crate::DbcError::InvalidHeader(
                crate::InvalidHeaderError::FieldCount {
                    expected: 10,
                    actual: header.field_count,
                },
            ));
        }

        let mut r = vec![0_u8; (header.record_count * header.record_size) as usize];
        b.read_exact(&mut r)?;
        let mut string_block = vec![0_u8; header.string_block_size as usize];
        b.read_exact(&mut string_block)?;

        let mut rows = Vec::with_capacity(header.record_count as usize);

        for mut chunk in r.chunks(header.record_size as usize) {
            let chunk = &mut chunk;

            // id: primary_key (ScreenEffect) int32
            let id = ScreenEffectKey::new(crate::util::read_i32_le(chunk)?);

            // name: string_ref
            let name = {
                let s = crate::util::get_string_as_vec(chunk, &string_block)?;
                String::from_utf8(s)?
            };

            // effect: int32
            let effect = crate::util::read_i32_le(chunk)?;

            // param: int32[4]
            let param = crate::util::read_array_i32::<4>(chunk)?;

            // light_params_id: foreign_key (LightParams) int32
            let light_params_id = LightParamsKey::new(crate::util::read_i32_le(chunk)?.into());

            // sound_ambience_id: foreign_key (SoundAmbience) int32
            let sound_ambience_id = SoundAmbienceKey::new(crate::util::read_i32_le(chunk)?.into());

            // zone_music_id: foreign_key (ZoneMusic) int32
            let zone_music_id = ZoneMusicKey::new(crate::util::read_i32_le(chunk)?.into());


            rows.push(ScreenEffectRow {
                id,
                name,
                effect,
                param,
                light_params_id,
                sound_ambience_id,
                zone_music_id,
            });
        }

        Ok(ScreenEffect { rows, })
    }

    fn write(&self, b: &mut impl Write) -> Result<(), std::io::Error> {
        let header = DbcHeader {
            record_count: self.rows.len() as u32,
            field_count: 10,
            record_size: 40,
            string_block_size: self.string_block_size(),
        };

        b.write_all(&header.write_header())?;

        let mut string_index = 1;
        for row in &self.rows {
            // id: primary_key (ScreenEffect) int32
            b.write_all(&row.id.id.to_le_bytes())?;

            // name: string_ref
            if !row.name.is_empty() {
                b.write_all(&(string_index as u32).to_le_bytes())?;
                string_index += row.name.len() + 1;
            }
            else {
                b.write_all(&(0_u32).to_le_bytes())?;
            }

            // effect: int32
            b.write_all(&row.effect.to_le_bytes())?;

            // param: int32[4]
            for i in row.param {
                b.write_all(&i.to_le_bytes())?;
            }


            // light_params_id: foreign_key (LightParams) int32
            b.write_all(&(row.light_params_id.id as i32).to_le_bytes())?;

            // sound_ambience_id: foreign_key (SoundAmbience) int32
            b.write_all(&(row.sound_ambience_id.id as i32).to_le_bytes())?;

            // zone_music_id: foreign_key (ZoneMusic) int32
            b.write_all(&(row.zone_music_id.id as i32).to_le_bytes())?;

        }

        self.write_string_block(b)?;

        Ok(())
    }

}

impl Indexable for ScreenEffect {
    type PrimaryKey = ScreenEffectKey;
    fn get(&self, key: impl TryInto<Self::PrimaryKey>) -> Option<&Self::Row> {
        let key = key.try_into().ok()?;
        self.rows.iter().find(|a| a.id.id == key.id)
    }

    fn get_mut(&mut self, key: impl TryInto<Self::PrimaryKey>) -> Option<&mut Self::Row> {
        let key = key.try_into().ok()?;
        self.rows.iter_mut().find(|a| a.id.id == key.id)
    }
}

impl ScreenEffect {
    fn write_string_block(&self, b: &mut impl Write) -> Result<(), std::io::Error> {
        b.write_all(&[0])?;

        for row in &self.rows {
            if !row.name.is_empty() { b.write_all(row.name.as_bytes())?; b.write_all(&[0])?; };
        }

        Ok(())
    }

    fn string_block_size(&self) -> u32 {
        let mut sum = 1;
        for row in &self.rows {
            if !row.name.is_empty() { sum += row.name.len() + 1; };
        }

        sum as u32
    }

}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Ord, PartialOrd, Hash, Default)]
pub struct ScreenEffectKey {
    pub id: i32
}

impl ScreenEffectKey {
    pub const fn new(id: i32) -> Self {
        Self { id }
    }

}

impl From<u8> for ScreenEffectKey {
    fn from(v: u8) -> Self {
        Self::new(v.into())
    }
}

impl From<u16> for ScreenEffectKey {
    fn from(v: u16) -> Self {
        Self::new(v.into())
    }
}

impl From<i8> for ScreenEffectKey {
    fn from(v: i8) -> Self {
        Self::new(v.into())
    }
}

impl From<i16> for ScreenEffectKey {
    fn from(v: i16) -> Self {
        Self::new(v.into())
    }
}

impl From<i32> for ScreenEffectKey {
    fn from(v: i32) -> Self {
        Self::new(v)
    }
}

impl TryFrom<u32> for ScreenEffectKey {
    type Error = u32;
    fn try_from(v: u32) -> Result<Self, Self::Error> {
        Ok(TryInto::<i32>::try_into(v).ok().ok_or(v)?.into())
    }
}

impl TryFrom<usize> for ScreenEffectKey {
    type Error = usize;
    fn try_from(v: usize) -> Result<Self, Self::Error> {
        Ok(TryInto::<i32>::try_into(v).ok().ok_or(v)?.into())
    }
}

impl TryFrom<u64> for ScreenEffectKey {
    type Error = u64;
    fn try_from(v: u64) -> Result<Self, Self::Error> {
        Ok(TryInto::<i32>::try_into(v).ok().ok_or(v)?.into())
    }
}

impl TryFrom<i64> for ScreenEffectKey {
    type Error = i64;
    fn try_from(v: i64) -> Result<Self, Self::Error> {
        Ok(TryInto::<i32>::try_into(v).ok().ok_or(v)?.into())
    }
}

impl TryFrom<isize> for ScreenEffectKey {
    type Error = isize;
    fn try_from(v: isize) -> Result<Self, Self::Error> {
        Ok(TryInto::<i32>::try_into(v).ok().ok_or(v)?.into())
    }
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct ScreenEffectRow {
    pub id: ScreenEffectKey,
    pub name: String,
    pub effect: i32,
    pub param: [i32; 4],
    pub light_params_id: LightParamsKey,
    pub sound_ambience_id: SoundAmbienceKey,
    pub zone_music_id: ZoneMusicKey,
}


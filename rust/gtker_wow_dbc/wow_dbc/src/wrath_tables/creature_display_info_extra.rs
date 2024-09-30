use crate::{
    DbcTable, Indexable,
};
use crate::header::{
    DbcHeader, HEADER_SIZE, parse_header,
};
use crate::wrath_tables::chr_races::ChrRacesKey;
use std::io::Write;

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct CreatureDisplayInfoExtra {
    pub rows: Vec<CreatureDisplayInfoExtraRow>,
}

impl DbcTable for CreatureDisplayInfoExtra {
    type Row = CreatureDisplayInfoExtraRow;

    const FILENAME: &'static str = "CreatureDisplayInfoExtra.dbc";

    fn rows(&self) -> &[Self::Row] { &self.rows }
    fn rows_mut(&mut self) -> &mut [Self::Row] { &mut self.rows }

    fn read(b: &mut impl std::io::Read) -> Result<Self, crate::DbcError> {
        let mut header = [0_u8; HEADER_SIZE];
        b.read_exact(&mut header)?;
        let header = parse_header(&header)?;

        if header.record_size != 84 {
            return Err(crate::DbcError::InvalidHeader(
                crate::InvalidHeaderError::RecordSize {
                    expected: 84,
                    actual: header.record_size,
                },
            ));
        }

        if header.field_count != 21 {
            return Err(crate::DbcError::InvalidHeader(
                crate::InvalidHeaderError::FieldCount {
                    expected: 21,
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

            // id: primary_key (CreatureDisplayInfoExtra) int32
            let id = CreatureDisplayInfoExtraKey::new(crate::util::read_i32_le(chunk)?);

            // display_race_id: foreign_key (ChrRaces) int32
            let display_race_id = ChrRacesKey::new(crate::util::read_i32_le(chunk)?.into());

            // display_sex_id: int32
            let display_sex_id = crate::util::read_i32_le(chunk)?;

            // skin_id: int32
            let skin_id = crate::util::read_i32_le(chunk)?;

            // face_id: int32
            let face_id = crate::util::read_i32_le(chunk)?;

            // hair_style_id: int32
            let hair_style_id = crate::util::read_i32_le(chunk)?;

            // hair_color_id: int32
            let hair_color_id = crate::util::read_i32_le(chunk)?;

            // facial_hair_id: int32
            let facial_hair_id = crate::util::read_i32_le(chunk)?;

            // n_p_c_item_display: int32[11]
            let n_p_c_item_display = crate::util::read_array_i32::<11>(chunk)?;

            // flags: int32
            let flags = crate::util::read_i32_le(chunk)?;

            // bake_name: string_ref
            let bake_name = {
                let s = crate::util::get_string_as_vec(chunk, &string_block)?;
                String::from_utf8(s)?
            };


            rows.push(CreatureDisplayInfoExtraRow {
                id,
                display_race_id,
                display_sex_id,
                skin_id,
                face_id,
                hair_style_id,
                hair_color_id,
                facial_hair_id,
                n_p_c_item_display,
                flags,
                bake_name,
            });
        }

        Ok(CreatureDisplayInfoExtra { rows, })
    }

    fn write(&self, b: &mut impl Write) -> Result<(), std::io::Error> {
        let header = DbcHeader {
            record_count: self.rows.len() as u32,
            field_count: 21,
            record_size: 84,
            string_block_size: self.string_block_size(),
        };

        b.write_all(&header.write_header())?;

        let mut string_index = 1;
        for row in &self.rows {
            // id: primary_key (CreatureDisplayInfoExtra) int32
            b.write_all(&row.id.id.to_le_bytes())?;

            // display_race_id: foreign_key (ChrRaces) int32
            b.write_all(&(row.display_race_id.id as i32).to_le_bytes())?;

            // display_sex_id: int32
            b.write_all(&row.display_sex_id.to_le_bytes())?;

            // skin_id: int32
            b.write_all(&row.skin_id.to_le_bytes())?;

            // face_id: int32
            b.write_all(&row.face_id.to_le_bytes())?;

            // hair_style_id: int32
            b.write_all(&row.hair_style_id.to_le_bytes())?;

            // hair_color_id: int32
            b.write_all(&row.hair_color_id.to_le_bytes())?;

            // facial_hair_id: int32
            b.write_all(&row.facial_hair_id.to_le_bytes())?;

            // n_p_c_item_display: int32[11]
            for i in row.n_p_c_item_display {
                b.write_all(&i.to_le_bytes())?;
            }


            // flags: int32
            b.write_all(&row.flags.to_le_bytes())?;

            // bake_name: string_ref
            if !row.bake_name.is_empty() {
                b.write_all(&(string_index as u32).to_le_bytes())?;
                string_index += row.bake_name.len() + 1;
            }
            else {
                b.write_all(&(0_u32).to_le_bytes())?;
            }

        }

        self.write_string_block(b)?;

        Ok(())
    }

}

impl Indexable for CreatureDisplayInfoExtra {
    type PrimaryKey = CreatureDisplayInfoExtraKey;
    fn get(&self, key: impl TryInto<Self::PrimaryKey>) -> Option<&Self::Row> {
        let key = key.try_into().ok()?;
        self.rows.iter().find(|a| a.id.id == key.id)
    }

    fn get_mut(&mut self, key: impl TryInto<Self::PrimaryKey>) -> Option<&mut Self::Row> {
        let key = key.try_into().ok()?;
        self.rows.iter_mut().find(|a| a.id.id == key.id)
    }
}

impl CreatureDisplayInfoExtra {
    fn write_string_block(&self, b: &mut impl Write) -> Result<(), std::io::Error> {
        b.write_all(&[0])?;

        for row in &self.rows {
            if !row.bake_name.is_empty() { b.write_all(row.bake_name.as_bytes())?; b.write_all(&[0])?; };
        }

        Ok(())
    }

    fn string_block_size(&self) -> u32 {
        let mut sum = 1;
        for row in &self.rows {
            if !row.bake_name.is_empty() { sum += row.bake_name.len() + 1; };
        }

        sum as u32
    }

}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Ord, PartialOrd, Hash, Default)]
pub struct CreatureDisplayInfoExtraKey {
    pub id: i32
}

impl CreatureDisplayInfoExtraKey {
    pub const fn new(id: i32) -> Self {
        Self { id }
    }

}

impl From<u8> for CreatureDisplayInfoExtraKey {
    fn from(v: u8) -> Self {
        Self::new(v.into())
    }
}

impl From<u16> for CreatureDisplayInfoExtraKey {
    fn from(v: u16) -> Self {
        Self::new(v.into())
    }
}

impl From<i8> for CreatureDisplayInfoExtraKey {
    fn from(v: i8) -> Self {
        Self::new(v.into())
    }
}

impl From<i16> for CreatureDisplayInfoExtraKey {
    fn from(v: i16) -> Self {
        Self::new(v.into())
    }
}

impl From<i32> for CreatureDisplayInfoExtraKey {
    fn from(v: i32) -> Self {
        Self::new(v)
    }
}

impl TryFrom<u32> for CreatureDisplayInfoExtraKey {
    type Error = u32;
    fn try_from(v: u32) -> Result<Self, Self::Error> {
        Ok(TryInto::<i32>::try_into(v).ok().ok_or(v)?.into())
    }
}

impl TryFrom<usize> for CreatureDisplayInfoExtraKey {
    type Error = usize;
    fn try_from(v: usize) -> Result<Self, Self::Error> {
        Ok(TryInto::<i32>::try_into(v).ok().ok_or(v)?.into())
    }
}

impl TryFrom<u64> for CreatureDisplayInfoExtraKey {
    type Error = u64;
    fn try_from(v: u64) -> Result<Self, Self::Error> {
        Ok(TryInto::<i32>::try_into(v).ok().ok_or(v)?.into())
    }
}

impl TryFrom<i64> for CreatureDisplayInfoExtraKey {
    type Error = i64;
    fn try_from(v: i64) -> Result<Self, Self::Error> {
        Ok(TryInto::<i32>::try_into(v).ok().ok_or(v)?.into())
    }
}

impl TryFrom<isize> for CreatureDisplayInfoExtraKey {
    type Error = isize;
    fn try_from(v: isize) -> Result<Self, Self::Error> {
        Ok(TryInto::<i32>::try_into(v).ok().ok_or(v)?.into())
    }
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct CreatureDisplayInfoExtraRow {
    pub id: CreatureDisplayInfoExtraKey,
    pub display_race_id: ChrRacesKey,
    pub display_sex_id: i32,
    pub skin_id: i32,
    pub face_id: i32,
    pub hair_style_id: i32,
    pub hair_color_id: i32,
    pub facial_hair_id: i32,
    pub n_p_c_item_display: [i32; 11],
    pub flags: i32,
    pub bake_name: String,
}


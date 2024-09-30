use crate::{
    DbcTable, Indexable,
};
use crate::header::{
    DbcHeader, HEADER_SIZE, parse_header,
};
use std::io::Write;

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct CreatureSpellData {
    pub rows: Vec<CreatureSpellDataRow>,
}

impl DbcTable for CreatureSpellData {
    type Row = CreatureSpellDataRow;

    const FILENAME: &'static str = "CreatureSpellData.dbc";

    fn rows(&self) -> &[Self::Row] { &self.rows }
    fn rows_mut(&mut self) -> &mut [Self::Row] { &mut self.rows }

    fn read(b: &mut impl std::io::Read) -> Result<Self, crate::DbcError> {
        let mut header = [0_u8; HEADER_SIZE];
        b.read_exact(&mut header)?;
        let header = parse_header(&header)?;

        if header.record_size != 36 {
            return Err(crate::DbcError::InvalidHeader(
                crate::InvalidHeaderError::RecordSize {
                    expected: 36,
                    actual: header.record_size,
                },
            ));
        }

        if header.field_count != 9 {
            return Err(crate::DbcError::InvalidHeader(
                crate::InvalidHeaderError::FieldCount {
                    expected: 9,
                    actual: header.field_count,
                },
            ));
        }

        let mut r = vec![0_u8; (header.record_count * header.record_size) as usize];
        b.read_exact(&mut r)?;

        let mut rows = Vec::with_capacity(header.record_count as usize);

        for mut chunk in r.chunks(header.record_size as usize) {
            let chunk = &mut chunk;

            // id: primary_key (CreatureSpellData) uint32
            let id = CreatureSpellDataKey::new(crate::util::read_u32_le(chunk)?);

            // spell: uint32[4]
            let spell = crate::util::read_array_u32::<4>(chunk)?;

            // cooldown_time_1: int32[4]
            let cooldown_time_1 = crate::util::read_array_i32::<4>(chunk)?;


            rows.push(CreatureSpellDataRow {
                id,
                spell,
                cooldown_time_1,
            });
        }

        Ok(CreatureSpellData { rows, })
    }

    fn write(&self, b: &mut impl Write) -> Result<(), std::io::Error> {
        let header = DbcHeader {
            record_count: self.rows.len() as u32,
            field_count: 9,
            record_size: 36,
            string_block_size: 1,
        };

        b.write_all(&header.write_header())?;

        for row in &self.rows {
            // id: primary_key (CreatureSpellData) uint32
            b.write_all(&row.id.id.to_le_bytes())?;

            // spell: uint32[4]
            for i in row.spell {
                b.write_all(&i.to_le_bytes())?;
            }


            // cooldown_time_1: int32[4]
            for i in row.cooldown_time_1 {
                b.write_all(&i.to_le_bytes())?;
            }


        }

        b.write_all(&[0_u8])?;

        Ok(())
    }

}

impl Indexable for CreatureSpellData {
    type PrimaryKey = CreatureSpellDataKey;
    fn get(&self, key: impl TryInto<Self::PrimaryKey>) -> Option<&Self::Row> {
        let key = key.try_into().ok()?;
        self.rows.iter().find(|a| a.id.id == key.id)
    }

    fn get_mut(&mut self, key: impl TryInto<Self::PrimaryKey>) -> Option<&mut Self::Row> {
        let key = key.try_into().ok()?;
        self.rows.iter_mut().find(|a| a.id.id == key.id)
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Ord, PartialOrd, Hash, Default)]
pub struct CreatureSpellDataKey {
    pub id: u32
}

impl CreatureSpellDataKey {
    pub const fn new(id: u32) -> Self {
        Self { id }
    }

}

impl From<u8> for CreatureSpellDataKey {
    fn from(v: u8) -> Self {
        Self::new(v.into())
    }
}

impl From<u16> for CreatureSpellDataKey {
    fn from(v: u16) -> Self {
        Self::new(v.into())
    }
}

impl From<u32> for CreatureSpellDataKey {
    fn from(v: u32) -> Self {
        Self::new(v)
    }
}

impl TryFrom<u64> for CreatureSpellDataKey {
    type Error = u64;
    fn try_from(v: u64) -> Result<Self, Self::Error> {
        Ok(TryInto::<u32>::try_into(v).ok().ok_or(v)?.into())
    }
}

impl TryFrom<usize> for CreatureSpellDataKey {
    type Error = usize;
    fn try_from(v: usize) -> Result<Self, Self::Error> {
        Ok(TryInto::<u32>::try_into(v).ok().ok_or(v)?.into())
    }
}

impl TryFrom<i8> for CreatureSpellDataKey {
    type Error = i8;
    fn try_from(v: i8) -> Result<Self, Self::Error> {
        Ok(TryInto::<u32>::try_into(v).ok().ok_or(v)?.into())
    }
}

impl TryFrom<i16> for CreatureSpellDataKey {
    type Error = i16;
    fn try_from(v: i16) -> Result<Self, Self::Error> {
        Ok(TryInto::<u32>::try_into(v).ok().ok_or(v)?.into())
    }
}

impl TryFrom<i32> for CreatureSpellDataKey {
    type Error = i32;
    fn try_from(v: i32) -> Result<Self, Self::Error> {
        Ok(TryInto::<u32>::try_into(v).ok().ok_or(v)?.into())
    }
}

impl TryFrom<i64> for CreatureSpellDataKey {
    type Error = i64;
    fn try_from(v: i64) -> Result<Self, Self::Error> {
        Ok(TryInto::<u32>::try_into(v).ok().ok_or(v)?.into())
    }
}

impl TryFrom<isize> for CreatureSpellDataKey {
    type Error = isize;
    fn try_from(v: isize) -> Result<Self, Self::Error> {
        Ok(TryInto::<u32>::try_into(v).ok().ok_or(v)?.into())
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct CreatureSpellDataRow {
    pub id: CreatureSpellDataKey,
    pub spell: [u32; 4],
    pub cooldown_time_1: [i32; 4],
}


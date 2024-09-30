use crate::{
    DbcTable, Indexable,
};
use crate::header::{
    DbcHeader, HEADER_SIZE, parse_header,
};
use std::io::Write;

#[derive(Debug, Clone, PartialEq, PartialOrd)]
pub struct CameraShakes {
    pub rows: Vec<CameraShakesRow>,
}

impl DbcTable for CameraShakes {
    type Row = CameraShakesRow;

    const FILENAME: &'static str = "CameraShakes.dbc";

    fn rows(&self) -> &[Self::Row] { &self.rows }
    fn rows_mut(&mut self) -> &mut [Self::Row] { &mut self.rows }

    fn read(b: &mut impl std::io::Read) -> Result<Self, crate::DbcError> {
        let mut header = [0_u8; HEADER_SIZE];
        b.read_exact(&mut header)?;
        let header = parse_header(&header)?;

        if header.record_size != 32 {
            return Err(crate::DbcError::InvalidHeader(
                crate::InvalidHeaderError::RecordSize {
                    expected: 32,
                    actual: header.record_size,
                },
            ));
        }

        if header.field_count != 8 {
            return Err(crate::DbcError::InvalidHeader(
                crate::InvalidHeaderError::FieldCount {
                    expected: 8,
                    actual: header.field_count,
                },
            ));
        }

        let mut r = vec![0_u8; (header.record_count * header.record_size) as usize];
        b.read_exact(&mut r)?;

        let mut rows = Vec::with_capacity(header.record_count as usize);

        for mut chunk in r.chunks(header.record_size as usize) {
            let chunk = &mut chunk;

            // id: primary_key (CameraShakes) uint32
            let id = CameraShakesKey::new(crate::util::read_u32_le(chunk)?);

            // shake_type: int32
            let shake_type = crate::util::read_i32_le(chunk)?;

            // direction: int32
            let direction = crate::util::read_i32_le(chunk)?;

            // amplitude: float
            let amplitude = crate::util::read_f32_le(chunk)?;

            // frequency: float
            let frequency = crate::util::read_f32_le(chunk)?;

            // duration: float
            let duration = crate::util::read_f32_le(chunk)?;

            // phase: float
            let phase = crate::util::read_f32_le(chunk)?;

            // coefficient: float
            let coefficient = crate::util::read_f32_le(chunk)?;


            rows.push(CameraShakesRow {
                id,
                shake_type,
                direction,
                amplitude,
                frequency,
                duration,
                phase,
                coefficient,
            });
        }

        Ok(CameraShakes { rows, })
    }

    fn write(&self, b: &mut impl Write) -> Result<(), std::io::Error> {
        let header = DbcHeader {
            record_count: self.rows.len() as u32,
            field_count: 8,
            record_size: 32,
            string_block_size: 1,
        };

        b.write_all(&header.write_header())?;

        for row in &self.rows {
            // id: primary_key (CameraShakes) uint32
            b.write_all(&row.id.id.to_le_bytes())?;

            // shake_type: int32
            b.write_all(&row.shake_type.to_le_bytes())?;

            // direction: int32
            b.write_all(&row.direction.to_le_bytes())?;

            // amplitude: float
            b.write_all(&row.amplitude.to_le_bytes())?;

            // frequency: float
            b.write_all(&row.frequency.to_le_bytes())?;

            // duration: float
            b.write_all(&row.duration.to_le_bytes())?;

            // phase: float
            b.write_all(&row.phase.to_le_bytes())?;

            // coefficient: float
            b.write_all(&row.coefficient.to_le_bytes())?;

        }

        b.write_all(&[0_u8])?;

        Ok(())
    }

}

impl Indexable for CameraShakes {
    type PrimaryKey = CameraShakesKey;
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
pub struct CameraShakesKey {
    pub id: u32
}

impl CameraShakesKey {
    pub const fn new(id: u32) -> Self {
        Self { id }
    }

}

impl From<u8> for CameraShakesKey {
    fn from(v: u8) -> Self {
        Self::new(v.into())
    }
}

impl From<u16> for CameraShakesKey {
    fn from(v: u16) -> Self {
        Self::new(v.into())
    }
}

impl From<u32> for CameraShakesKey {
    fn from(v: u32) -> Self {
        Self::new(v)
    }
}

impl TryFrom<u64> for CameraShakesKey {
    type Error = u64;
    fn try_from(v: u64) -> Result<Self, Self::Error> {
        Ok(TryInto::<u32>::try_into(v).ok().ok_or(v)?.into())
    }
}

impl TryFrom<usize> for CameraShakesKey {
    type Error = usize;
    fn try_from(v: usize) -> Result<Self, Self::Error> {
        Ok(TryInto::<u32>::try_into(v).ok().ok_or(v)?.into())
    }
}

impl TryFrom<i8> for CameraShakesKey {
    type Error = i8;
    fn try_from(v: i8) -> Result<Self, Self::Error> {
        Ok(TryInto::<u32>::try_into(v).ok().ok_or(v)?.into())
    }
}

impl TryFrom<i16> for CameraShakesKey {
    type Error = i16;
    fn try_from(v: i16) -> Result<Self, Self::Error> {
        Ok(TryInto::<u32>::try_into(v).ok().ok_or(v)?.into())
    }
}

impl TryFrom<i32> for CameraShakesKey {
    type Error = i32;
    fn try_from(v: i32) -> Result<Self, Self::Error> {
        Ok(TryInto::<u32>::try_into(v).ok().ok_or(v)?.into())
    }
}

impl TryFrom<i64> for CameraShakesKey {
    type Error = i64;
    fn try_from(v: i64) -> Result<Self, Self::Error> {
        Ok(TryInto::<u32>::try_into(v).ok().ok_or(v)?.into())
    }
}

impl TryFrom<isize> for CameraShakesKey {
    type Error = isize;
    fn try_from(v: isize) -> Result<Self, Self::Error> {
        Ok(TryInto::<u32>::try_into(v).ok().ok_or(v)?.into())
    }
}

#[derive(Debug, Clone, Copy, PartialEq, PartialOrd)]
pub struct CameraShakesRow {
    pub id: CameraShakesKey,
    pub shake_type: i32,
    pub direction: i32,
    pub amplitude: f32,
    pub frequency: f32,
    pub duration: f32,
    pub phase: f32,
    pub coefficient: f32,
}


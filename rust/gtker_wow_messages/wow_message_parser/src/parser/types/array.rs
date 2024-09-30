use crate::parser::types::sizes::{
    Sizes, GUID_SIZE, PACKED_GUID_MAX_SIZE, PACKED_GUID_MIN_SIZE, SPELL_SIZE,
};
use crate::parser::types::struct_member::StructMemberDefinition;
use crate::parser::types::IntegerType;
use crate::{Container, CSTRING_LARGEST_ALLOWED, CSTRING_SMALLEST_ALLOWED};

#[derive(Debug, Eq, PartialEq, Ord, PartialOrd, Clone)]
pub(crate) enum ArrayType {
    Integer(IntegerType),
    Struct(Box<Container>),
    CString,
    Guid,
    PackedGuid,
    Spell,
}

impl ArrayType {
    pub(crate) fn rust_str(&self) -> String {
        match &self {
            ArrayType::Integer(i) => i.rust_str().to_string(),
            ArrayType::Struct(c) => c.name().to_string(),
            ArrayType::CString => "String".to_string(),
            ArrayType::Guid => "Guid".to_string(),
            ArrayType::PackedGuid => "Guid".to_string(),
            ArrayType::Spell => "u32".to_string(),
        }
    }

    pub(crate) fn str(&self) -> String {
        match self {
            ArrayType::Integer(i) => i.str().to_string(),
            ArrayType::Struct(i) => i.name().to_string(),
            ArrayType::CString => "CString".to_string(),
            ArrayType::Guid => "Guid".to_string(),
            ArrayType::PackedGuid => "PackedGuid".to_string(),
            ArrayType::Spell => "Spell".to_string(),
        }
    }

    pub(crate) fn sizes(&self) -> Sizes {
        let mut s = Sizes::new();

        match self {
            ArrayType::Integer(i) => {
                s.inc_both(i.size().into());
            }
            ArrayType::Struct(c) => {
                s += c.sizes();
            }
            ArrayType::CString => {
                s.inc(
                    CSTRING_SMALLEST_ALLOWED.into(),
                    CSTRING_LARGEST_ALLOWED.into(),
                );
            }
            ArrayType::Guid => {
                s.inc_both(GUID_SIZE.into());
            }
            ArrayType::PackedGuid => {
                s.inc(PACKED_GUID_MIN_SIZE.into(), PACKED_GUID_MAX_SIZE.into());
            }
            ArrayType::Spell => {
                s.inc_both(SPELL_SIZE.into());
            }
        }

        s
    }
}

#[derive(Debug, Eq, PartialEq, PartialOrd, Ord, Clone)]
pub(crate) enum ArraySize {
    Fixed(i128),
    Variable(Box<StructMemberDefinition>),
    Endless,
}

impl ArraySize {
    pub(crate) fn str(&self) -> String {
        match self {
            ArraySize::Fixed(i) => i.to_string(),
            ArraySize::Variable(m) => m.name().to_string(),
            ArraySize::Endless => "-".to_string(),
        }
    }
}

#[derive(Debug, Eq, PartialEq, Ord, PartialOrd, Clone)]
pub(crate) struct Array {
    inner: ArrayType,
    size: ArraySize,
    compressed: bool,
}

impl Array {
    pub fn new(inner: ArrayType, size: ArraySize, compressed: bool) -> Self {
        Self {
            inner,
            size,
            compressed,
        }
    }

    pub(crate) fn ty(&self) -> &ArrayType {
        &self.inner
    }

    pub(crate) fn size(&self) -> ArraySize {
        self.size.clone()
    }

    pub(crate) fn fixed_size(&self) -> i128 {
        match self.size() {
            ArraySize::Fixed(s) => s,
            ArraySize::Variable(_) | ArraySize::Endless => panic!(),
        }
    }

    pub(crate) fn str(&self) -> String {
        format!("{}[{}]", self.inner.str(), self.size.str())
    }

    pub(crate) fn compressed(&self) -> bool {
        self.compressed
    }

    pub(crate) fn is_endless(&self) -> bool {
        matches!(self.size(), ArraySize::Endless)
    }
    pub(crate) fn rust_str(&self) -> String {
        match &self.size {
            ArraySize::Fixed(i) => format!("[{}; {}]", self.inner.rust_str(), i),
            ArraySize::Variable(_) | ArraySize::Endless => {
                format!("Vec<{}>", self.inner.rust_str())
            }
        }
    }

    pub(crate) fn rust_str_inner(&self) -> String {
        self.inner.rust_str()
    }

    pub(crate) fn is_byte_array(&self) -> bool {
        matches!(self.ty(), ArrayType::Integer(IntegerType::U8))
    }

    pub(crate) fn is_constant_sized_u8_array(&self) -> bool {
        match &self.size() {
            ArraySize::Fixed(_) => matches!(&self.ty(), ArrayType::Integer(IntegerType::U8)),
            ArraySize::Variable(_) => false,
            ArraySize::Endless => false,
        }
    }

    pub(crate) fn inner_type_is_copy(&self) -> bool {
        match self.ty() {
            ArrayType::Spell | ArrayType::PackedGuid | ArrayType::Integer(_) | ArrayType::Guid => {
                true
            }
            ArrayType::Struct(c) => c.is_constant_sized(),

            ArrayType::CString => false,
        }
    }

    pub(crate) fn is_constant(&self) -> bool {
        self.ty().sizes().is_constant().is_some()
            && match self.size() {
                ArraySize::Fixed(_) => true,
                ArraySize::Variable(_) | ArraySize::Endless => false,
            }
    }
}

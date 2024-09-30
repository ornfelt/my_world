pub enum VisibleItemIndex {
    Index0,
    Index1,
    Index2,
    Index3,
    Index4,
    Index5,
    Index6,
    Index7,
    Index8,
    Index9,
    Index10,
    Index11,
    Index12,
    Index13,
    Index14,
    Index15,
    Index16,
    Index17,
    Index18,
}

impl VisibleItemIndex {
    pub(crate) const fn offset(&self) -> u16 {
        344 + self.index()
    }

    pub(crate) const fn index(&self) -> u16 {
        match self {
            Self::Index0 => 0,
            Self::Index1 => 12,
            Self::Index2 => 24,
            Self::Index3 => 36,
            Self::Index4 => 48,
            Self::Index5 => 60,
            Self::Index6 => 72,
            Self::Index7 => 84,
            Self::Index8 => 96,
            Self::Index9 => 108,
            Self::Index10 => 120,
            Self::Index11 => 132,
            Self::Index12 => 144,
            Self::Index13 => 156,
            Self::Index14 => 168,
            Self::Index15 => 180,
            Self::Index16 => 192,
            Self::Index17 => 204,
            Self::Index18 => 216,
        }
    }

    pub(crate) const fn first(&self) -> u16 {
        344 + self.index()
    }

    pub(crate) const fn last(&self) -> u16 {
        self.first() + 12
    }

}

impl Default for VisibleItemIndex {
    fn default() -> Self {
        Self::Index0
    }
}

pub(crate) const fn visibleitem_try_from_inner(value: u16) -> Option<VisibleItemIndex> {
    Some(match value {
        0 => VisibleItemIndex::Index0,
        1 => VisibleItemIndex::Index1,
        2 => VisibleItemIndex::Index2,
        3 => VisibleItemIndex::Index3,
        4 => VisibleItemIndex::Index4,
        5 => VisibleItemIndex::Index5,
        6 => VisibleItemIndex::Index6,
        7 => VisibleItemIndex::Index7,
        8 => VisibleItemIndex::Index8,
        9 => VisibleItemIndex::Index9,
        10 => VisibleItemIndex::Index10,
        11 => VisibleItemIndex::Index11,
        12 => VisibleItemIndex::Index12,
        13 => VisibleItemIndex::Index13,
        14 => VisibleItemIndex::Index14,
        15 => VisibleItemIndex::Index15,
        16 => VisibleItemIndex::Index16,
        17 => VisibleItemIndex::Index17,
        18 => VisibleItemIndex::Index18,
        _ => return None,
    })
}

impl TryFrom<u8> for VisibleItemIndex {
    type Error = u8;

    fn try_from(value: u8) -> Result<Self, Self::Error> {
        visibleitem_try_from_inner(value.into()).ok_or(value)
    }
}

impl TryFrom<u16> for VisibleItemIndex {
    type Error = u16;

    fn try_from(value: u16) -> Result<Self, Self::Error> {
        visibleitem_try_from_inner(value).ok_or(value)
    }
}

impl TryFrom<u32> for VisibleItemIndex {
    type Error = u32;

    fn try_from(value: u32) -> Result<Self, Self::Error> {
        visibleitem_try_from_inner(value.try_into().ok().ok_or(value)?).ok_or(value)
    }
}

impl TryFrom<u64> for VisibleItemIndex {
    type Error = u64;

    fn try_from(value: u64) -> Result<Self, Self::Error> {
        visibleitem_try_from_inner(value.try_into().ok().ok_or(value)?).ok_or(value)
    }
}

impl TryFrom<i8> for VisibleItemIndex {
    type Error = i8;

    fn try_from(value: i8) -> Result<Self, Self::Error> {
        visibleitem_try_from_inner(value.try_into().ok().ok_or(value)?).ok_or(value)
    }
}

impl TryFrom<i16> for VisibleItemIndex {
    type Error = i16;

    fn try_from(value: i16) -> Result<Self, Self::Error> {
        visibleitem_try_from_inner(value.try_into().ok().ok_or(value)?).ok_or(value)
    }
}

impl TryFrom<i32> for VisibleItemIndex {
    type Error = i32;

    fn try_from(value: i32) -> Result<Self, Self::Error> {
        visibleitem_try_from_inner(value.try_into().ok().ok_or(value)?).ok_or(value)
    }
}

impl TryFrom<i64> for VisibleItemIndex {
    type Error = i64;

    fn try_from(value: i64) -> Result<Self, Self::Error> {
        visibleitem_try_from_inner(value.try_into().ok().ok_or(value)?).ok_or(value)
    }
}

impl TryFrom<usize> for VisibleItemIndex {
    type Error = usize;

    fn try_from(value: usize) -> Result<Self, Self::Error> {
        visibleitem_try_from_inner(value.try_into().ok().ok_or(value)?).ok_or(value)
    }
}

pub enum SkillInfoIndex {
    Index0,
    Index1,
    Index2,
    Index3,
    Index4,
    Index5,
    Index6,
    Index7,
    Index8,
    Index9,
    Index10,
    Index11,
    Index12,
    Index13,
    Index14,
    Index15,
    Index16,
    Index17,
    Index18,
    Index19,
    Index20,
    Index21,
    Index22,
    Index23,
    Index24,
    Index25,
    Index26,
    Index27,
    Index28,
    Index29,
    Index30,
    Index31,
    Index32,
    Index33,
    Index34,
    Index35,
    Index36,
    Index37,
    Index38,
    Index39,
    Index40,
    Index41,
    Index42,
    Index43,
    Index44,
    Index45,
    Index46,
    Index47,
    Index48,
    Index49,
    Index50,
    Index51,
    Index52,
    Index53,
    Index54,
    Index55,
    Index56,
    Index57,
    Index58,
    Index59,
    Index60,
    Index61,
    Index62,
    Index63,
    Index64,
    Index65,
    Index66,
    Index67,
    Index68,
    Index69,
    Index70,
    Index71,
    Index72,
    Index73,
    Index74,
    Index75,
    Index76,
    Index77,
    Index78,
    Index79,
    Index80,
    Index81,
    Index82,
    Index83,
    Index84,
    Index85,
    Index86,
    Index87,
    Index88,
    Index89,
    Index90,
    Index91,
    Index92,
    Index93,
    Index94,
    Index95,
    Index96,
    Index97,
    Index98,
    Index99,
    Index100,
    Index101,
    Index102,
    Index103,
    Index104,
    Index105,
    Index106,
    Index107,
    Index108,
    Index109,
    Index110,
    Index111,
    Index112,
    Index113,
    Index114,
    Index115,
    Index116,
    Index117,
    Index118,
    Index119,
    Index120,
    Index121,
    Index122,
    Index123,
    Index124,
    Index125,
    Index126,
    Index127,
}

impl SkillInfoIndex {
    pub(crate) const fn offset(&self) -> u16 {
        928 + self.index()
    }

    pub(crate) const fn index(&self) -> u16 {
        match self {
            Self::Index0 => 0,
            Self::Index1 => 3,
            Self::Index2 => 6,
            Self::Index3 => 9,
            Self::Index4 => 12,
            Self::Index5 => 15,
            Self::Index6 => 18,
            Self::Index7 => 21,
            Self::Index8 => 24,
            Self::Index9 => 27,
            Self::Index10 => 30,
            Self::Index11 => 33,
            Self::Index12 => 36,
            Self::Index13 => 39,
            Self::Index14 => 42,
            Self::Index15 => 45,
            Self::Index16 => 48,
            Self::Index17 => 51,
            Self::Index18 => 54,
            Self::Index19 => 57,
            Self::Index20 => 60,
            Self::Index21 => 63,
            Self::Index22 => 66,
            Self::Index23 => 69,
            Self::Index24 => 72,
            Self::Index25 => 75,
            Self::Index26 => 78,
            Self::Index27 => 81,
            Self::Index28 => 84,
            Self::Index29 => 87,
            Self::Index30 => 90,
            Self::Index31 => 93,
            Self::Index32 => 96,
            Self::Index33 => 99,
            Self::Index34 => 102,
            Self::Index35 => 105,
            Self::Index36 => 108,
            Self::Index37 => 111,
            Self::Index38 => 114,
            Self::Index39 => 117,
            Self::Index40 => 120,
            Self::Index41 => 123,
            Self::Index42 => 126,
            Self::Index43 => 129,
            Self::Index44 => 132,
            Self::Index45 => 135,
            Self::Index46 => 138,
            Self::Index47 => 141,
            Self::Index48 => 144,
            Self::Index49 => 147,
            Self::Index50 => 150,
            Self::Index51 => 153,
            Self::Index52 => 156,
            Self::Index53 => 159,
            Self::Index54 => 162,
            Self::Index55 => 165,
            Self::Index56 => 168,
            Self::Index57 => 171,
            Self::Index58 => 174,
            Self::Index59 => 177,
            Self::Index60 => 180,
            Self::Index61 => 183,
            Self::Index62 => 186,
            Self::Index63 => 189,
            Self::Index64 => 192,
            Self::Index65 => 195,
            Self::Index66 => 198,
            Self::Index67 => 201,
            Self::Index68 => 204,
            Self::Index69 => 207,
            Self::Index70 => 210,
            Self::Index71 => 213,
            Self::Index72 => 216,
            Self::Index73 => 219,
            Self::Index74 => 222,
            Self::Index75 => 225,
            Self::Index76 => 228,
            Self::Index77 => 231,
            Self::Index78 => 234,
            Self::Index79 => 237,
            Self::Index80 => 240,
            Self::Index81 => 243,
            Self::Index82 => 246,
            Self::Index83 => 249,
            Self::Index84 => 252,
            Self::Index85 => 255,
            Self::Index86 => 258,
            Self::Index87 => 261,
            Self::Index88 => 264,
            Self::Index89 => 267,
            Self::Index90 => 270,
            Self::Index91 => 273,
            Self::Index92 => 276,
            Self::Index93 => 279,
            Self::Index94 => 282,
            Self::Index95 => 285,
            Self::Index96 => 288,
            Self::Index97 => 291,
            Self::Index98 => 294,
            Self::Index99 => 297,
            Self::Index100 => 300,
            Self::Index101 => 303,
            Self::Index102 => 306,
            Self::Index103 => 309,
            Self::Index104 => 312,
            Self::Index105 => 315,
            Self::Index106 => 318,
            Self::Index107 => 321,
            Self::Index108 => 324,
            Self::Index109 => 327,
            Self::Index110 => 330,
            Self::Index111 => 333,
            Self::Index112 => 336,
            Self::Index113 => 339,
            Self::Index114 => 342,
            Self::Index115 => 345,
            Self::Index116 => 348,
            Self::Index117 => 351,
            Self::Index118 => 354,
            Self::Index119 => 357,
            Self::Index120 => 360,
            Self::Index121 => 363,
            Self::Index122 => 366,
            Self::Index123 => 369,
            Self::Index124 => 372,
            Self::Index125 => 375,
            Self::Index126 => 378,
            Self::Index127 => 381,
        }
    }

    pub(crate) const fn first(&self) -> u16 {
        928 + self.index()
    }

    pub(crate) const fn last(&self) -> u16 {
        self.first() + 3
    }

}

impl Default for SkillInfoIndex {
    fn default() -> Self {
        Self::Index0
    }
}

pub(crate) const fn skillinfo_try_from_inner(value: u16) -> Option<SkillInfoIndex> {
    Some(match value {
        0 => SkillInfoIndex::Index0,
        1 => SkillInfoIndex::Index1,
        2 => SkillInfoIndex::Index2,
        3 => SkillInfoIndex::Index3,
        4 => SkillInfoIndex::Index4,
        5 => SkillInfoIndex::Index5,
        6 => SkillInfoIndex::Index6,
        7 => SkillInfoIndex::Index7,
        8 => SkillInfoIndex::Index8,
        9 => SkillInfoIndex::Index9,
        10 => SkillInfoIndex::Index10,
        11 => SkillInfoIndex::Index11,
        12 => SkillInfoIndex::Index12,
        13 => SkillInfoIndex::Index13,
        14 => SkillInfoIndex::Index14,
        15 => SkillInfoIndex::Index15,
        16 => SkillInfoIndex::Index16,
        17 => SkillInfoIndex::Index17,
        18 => SkillInfoIndex::Index18,
        19 => SkillInfoIndex::Index19,
        20 => SkillInfoIndex::Index20,
        21 => SkillInfoIndex::Index21,
        22 => SkillInfoIndex::Index22,
        23 => SkillInfoIndex::Index23,
        24 => SkillInfoIndex::Index24,
        25 => SkillInfoIndex::Index25,
        26 => SkillInfoIndex::Index26,
        27 => SkillInfoIndex::Index27,
        28 => SkillInfoIndex::Index28,
        29 => SkillInfoIndex::Index29,
        30 => SkillInfoIndex::Index30,
        31 => SkillInfoIndex::Index31,
        32 => SkillInfoIndex::Index32,
        33 => SkillInfoIndex::Index33,
        34 => SkillInfoIndex::Index34,
        35 => SkillInfoIndex::Index35,
        36 => SkillInfoIndex::Index36,
        37 => SkillInfoIndex::Index37,
        38 => SkillInfoIndex::Index38,
        39 => SkillInfoIndex::Index39,
        40 => SkillInfoIndex::Index40,
        41 => SkillInfoIndex::Index41,
        42 => SkillInfoIndex::Index42,
        43 => SkillInfoIndex::Index43,
        44 => SkillInfoIndex::Index44,
        45 => SkillInfoIndex::Index45,
        46 => SkillInfoIndex::Index46,
        47 => SkillInfoIndex::Index47,
        48 => SkillInfoIndex::Index48,
        49 => SkillInfoIndex::Index49,
        50 => SkillInfoIndex::Index50,
        51 => SkillInfoIndex::Index51,
        52 => SkillInfoIndex::Index52,
        53 => SkillInfoIndex::Index53,
        54 => SkillInfoIndex::Index54,
        55 => SkillInfoIndex::Index55,
        56 => SkillInfoIndex::Index56,
        57 => SkillInfoIndex::Index57,
        58 => SkillInfoIndex::Index58,
        59 => SkillInfoIndex::Index59,
        60 => SkillInfoIndex::Index60,
        61 => SkillInfoIndex::Index61,
        62 => SkillInfoIndex::Index62,
        63 => SkillInfoIndex::Index63,
        64 => SkillInfoIndex::Index64,
        65 => SkillInfoIndex::Index65,
        66 => SkillInfoIndex::Index66,
        67 => SkillInfoIndex::Index67,
        68 => SkillInfoIndex::Index68,
        69 => SkillInfoIndex::Index69,
        70 => SkillInfoIndex::Index70,
        71 => SkillInfoIndex::Index71,
        72 => SkillInfoIndex::Index72,
        73 => SkillInfoIndex::Index73,
        74 => SkillInfoIndex::Index74,
        75 => SkillInfoIndex::Index75,
        76 => SkillInfoIndex::Index76,
        77 => SkillInfoIndex::Index77,
        78 => SkillInfoIndex::Index78,
        79 => SkillInfoIndex::Index79,
        80 => SkillInfoIndex::Index80,
        81 => SkillInfoIndex::Index81,
        82 => SkillInfoIndex::Index82,
        83 => SkillInfoIndex::Index83,
        84 => SkillInfoIndex::Index84,
        85 => SkillInfoIndex::Index85,
        86 => SkillInfoIndex::Index86,
        87 => SkillInfoIndex::Index87,
        88 => SkillInfoIndex::Index88,
        89 => SkillInfoIndex::Index89,
        90 => SkillInfoIndex::Index90,
        91 => SkillInfoIndex::Index91,
        92 => SkillInfoIndex::Index92,
        93 => SkillInfoIndex::Index93,
        94 => SkillInfoIndex::Index94,
        95 => SkillInfoIndex::Index95,
        96 => SkillInfoIndex::Index96,
        97 => SkillInfoIndex::Index97,
        98 => SkillInfoIndex::Index98,
        99 => SkillInfoIndex::Index99,
        100 => SkillInfoIndex::Index100,
        101 => SkillInfoIndex::Index101,
        102 => SkillInfoIndex::Index102,
        103 => SkillInfoIndex::Index103,
        104 => SkillInfoIndex::Index104,
        105 => SkillInfoIndex::Index105,
        106 => SkillInfoIndex::Index106,
        107 => SkillInfoIndex::Index107,
        108 => SkillInfoIndex::Index108,
        109 => SkillInfoIndex::Index109,
        110 => SkillInfoIndex::Index110,
        111 => SkillInfoIndex::Index111,
        112 => SkillInfoIndex::Index112,
        113 => SkillInfoIndex::Index113,
        114 => SkillInfoIndex::Index114,
        115 => SkillInfoIndex::Index115,
        116 => SkillInfoIndex::Index116,
        117 => SkillInfoIndex::Index117,
        118 => SkillInfoIndex::Index118,
        119 => SkillInfoIndex::Index119,
        120 => SkillInfoIndex::Index120,
        121 => SkillInfoIndex::Index121,
        122 => SkillInfoIndex::Index122,
        123 => SkillInfoIndex::Index123,
        124 => SkillInfoIndex::Index124,
        125 => SkillInfoIndex::Index125,
        126 => SkillInfoIndex::Index126,
        127 => SkillInfoIndex::Index127,
        _ => return None,
    })
}

impl TryFrom<u8> for SkillInfoIndex {
    type Error = u8;

    fn try_from(value: u8) -> Result<Self, Self::Error> {
        skillinfo_try_from_inner(value.into()).ok_or(value)
    }
}

impl TryFrom<u16> for SkillInfoIndex {
    type Error = u16;

    fn try_from(value: u16) -> Result<Self, Self::Error> {
        skillinfo_try_from_inner(value).ok_or(value)
    }
}

impl TryFrom<u32> for SkillInfoIndex {
    type Error = u32;

    fn try_from(value: u32) -> Result<Self, Self::Error> {
        skillinfo_try_from_inner(value.try_into().ok().ok_or(value)?).ok_or(value)
    }
}

impl TryFrom<u64> for SkillInfoIndex {
    type Error = u64;

    fn try_from(value: u64) -> Result<Self, Self::Error> {
        skillinfo_try_from_inner(value.try_into().ok().ok_or(value)?).ok_or(value)
    }
}

impl TryFrom<i8> for SkillInfoIndex {
    type Error = i8;

    fn try_from(value: i8) -> Result<Self, Self::Error> {
        skillinfo_try_from_inner(value.try_into().ok().ok_or(value)?).ok_or(value)
    }
}

impl TryFrom<i16> for SkillInfoIndex {
    type Error = i16;

    fn try_from(value: i16) -> Result<Self, Self::Error> {
        skillinfo_try_from_inner(value.try_into().ok().ok_or(value)?).ok_or(value)
    }
}

impl TryFrom<i32> for SkillInfoIndex {
    type Error = i32;

    fn try_from(value: i32) -> Result<Self, Self::Error> {
        skillinfo_try_from_inner(value.try_into().ok().ok_or(value)?).ok_or(value)
    }
}

impl TryFrom<i64> for SkillInfoIndex {
    type Error = i64;

    fn try_from(value: i64) -> Result<Self, Self::Error> {
        skillinfo_try_from_inner(value.try_into().ok().ok_or(value)?).ok_or(value)
    }
}

impl TryFrom<usize> for SkillInfoIndex {
    type Error = usize;

    fn try_from(value: usize) -> Result<Self, Self::Error> {
        skillinfo_try_from_inner(value.try_into().ok().ok_or(value)?).ok_or(value)
    }
}


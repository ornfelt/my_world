use std::io::{Read, Write};

use crate::wrath::ResyncRune;

/// Auto generated from the original `wowm` in file [`wow_message_parser/wowm/world/spell/smsg_resync_runes.wowm:8`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/spell/smsg_resync_runes.wowm#L8):
/// ```text
/// smsg SMSG_RESYNC_RUNES = 0x0487 {
///     u32 amount_of_runes;
///     ResyncRune[amount_of_runes] runes;
/// }
/// ```
#[derive(Debug, Clone, PartialEq, Eq, Hash, PartialOrd, Ord, Default)]
pub struct SMSG_RESYNC_RUNES {
    pub runes: Vec<ResyncRune>,
}

impl crate::private::Sealed for SMSG_RESYNC_RUNES {}
impl SMSG_RESYNC_RUNES {
    fn read_inner(mut r: &mut &[u8], body_size: u32) -> Result<Self, crate::errors::ParseErrorKind> {
        if !(4..=16777215).contains(&body_size) {
            return Err(crate::errors::ParseErrorKind::InvalidSize);
        }

        // amount_of_runes: u32
        let amount_of_runes = crate::util::read_u32_le(&mut r)?;

        // runes: ResyncRune[amount_of_runes]
        let runes = {
            let mut runes = Vec::with_capacity(amount_of_runes as usize);

            let allocation_size = u64::from(amount_of_runes) * 2;
            if allocation_size > crate::errors::MAX_ALLOCATION_SIZE_WRATH {
                return Err(crate::errors::ParseErrorKind::AllocationTooLargeError(allocation_size));
            }

            for _ in 0..amount_of_runes {
                runes.push(ResyncRune::read(&mut r)?);
            }
            runes
        };

        Ok(Self {
            runes,
        })
    }

}

impl crate::Message for SMSG_RESYNC_RUNES {
    const OPCODE: u32 = 0x0487;

    #[cfg(feature = "print-testcase")]
    fn message_name(&self) -> &'static str {
        "SMSG_RESYNC_RUNES"
    }

    #[cfg(feature = "print-testcase")]
    fn to_test_case_string(&self) -> Option<String> {
        use std::fmt::Write;
        use crate::traits::Message;

        let mut s = String::new();

        writeln!(s, "test SMSG_RESYNC_RUNES {{").unwrap();
        // Members
        writeln!(s, "    amount_of_runes = {};", self.runes.len()).unwrap();
        writeln!(s, "    runes = [").unwrap();
        for v in self.runes.as_slice() {
            writeln!(s, "        {{").unwrap();
            // Members
            writeln!(s, "            current_rune = {};", v.current_rune).unwrap();
            writeln!(s, "            rune_cooldown = {};", v.rune_cooldown).unwrap();

            writeln!(s, "        }},").unwrap();
        }
        writeln!(s, "    ];").unwrap();

        writeln!(s, "}} [").unwrap();

        let [a, b] = (u16::try_from(self.size() + 2).unwrap()).to_be_bytes();
        writeln!(s, "    {a:#04X}, {b:#04X}, /* size */").unwrap();
        let [a, b] = 1159_u16.to_le_bytes();
        writeln!(s, "    {a:#04X}, {b:#04X}, /* opcode */").unwrap();
        let mut bytes: Vec<u8> = Vec::new();
        self.write_into_vec(&mut bytes).unwrap();
        let mut bytes = bytes.into_iter();

        crate::util::write_bytes(&mut s, &mut bytes, 4, "amount_of_runes", "    ");
        if !self.runes.is_empty() {
            writeln!(s, "    /* runes: ResyncRune[amount_of_runes] start */").unwrap();
            for (i, v) in self.runes.iter().enumerate() {
                writeln!(s, "    /* runes: ResyncRune[amount_of_runes] {i} start */").unwrap();
                crate::util::write_bytes(&mut s, &mut bytes, 1, "current_rune", "        ");
                crate::util::write_bytes(&mut s, &mut bytes, 1, "rune_cooldown", "        ");
                writeln!(s, "    /* runes: ResyncRune[amount_of_runes] {i} end */").unwrap();
            }
            writeln!(s, "    /* runes: ResyncRune[amount_of_runes] end */").unwrap();
        }


        writeln!(s, "] {{").unwrap();
        writeln!(s, "    versions = \"{}\";", std::env::var("WOWM_TEST_CASE_WORLD_VERSION").unwrap_or("3.3.5".to_string())).unwrap();
        writeln!(s, "}}\n").unwrap();

        Some(s)
    }

    fn size_without_header(&self) -> u32 {
        self.size() as u32
    }

    fn write_into_vec(&self, mut w: impl Write) -> Result<(), std::io::Error> {
        // amount_of_runes: u32
        w.write_all(&(self.runes.len() as u32).to_le_bytes())?;

        // runes: ResyncRune[amount_of_runes]
        for i in self.runes.iter() {
            i.write_into_vec(&mut w)?;
        }

        Ok(())
    }

    fn read_body<S: crate::private::Sealed>(r: &mut &[u8], body_size: u32) -> Result<Self, crate::errors::ParseError> {
        Self::read_inner(r, body_size).map_err(|a| crate::errors::ParseError::new(1159, "SMSG_RESYNC_RUNES", body_size, a))
    }

}

#[cfg(feature = "wrath")]
impl crate::wrath::ServerMessage for SMSG_RESYNC_RUNES {}

impl SMSG_RESYNC_RUNES {
    pub(crate) fn size(&self) -> usize {
        4 // amount_of_runes: u32
        + self.runes.len() * 2 // runes: ResyncRune[amount_of_runes]
    }
}

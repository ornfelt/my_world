use crate::MessageError;

pub enum ClientOpcodes {
    SessionKeyAnswer {
        name: String,
        session_key: Option<[u8; 40]>,
    },
    RegisterRealmReply {
        realm_id: Option<u8>,
    },
    AddUserReply {
        name: String,
        success: bool,
    },
    RemoveUserReply {
        name: String,
        success: bool,
    },
    ModifyUserReply {
        name: String,
        success: bool,
    },
}

impl ClientOpcodes {
    const SESSION_KEY_ANSWER_OPCODE: u8 = 1;
    const REGISTER_REALM_REPLY_OPCODE: u8 = 5;
    const ADD_USER_REPLY_OPCODE: u8 = 7;
    const REMOVE_USER_REPLY_OPCODE: u8 = 9;
    const MODIFY_USER_REPLY_OPCODE: u8 = 11;

    const fn opcode(&self) -> u8 {
        match self {
            ClientOpcodes::SessionKeyAnswer { .. } => Self::SESSION_KEY_ANSWER_OPCODE,
            ClientOpcodes::RegisterRealmReply { .. } => Self::REGISTER_REALM_REPLY_OPCODE,
            ClientOpcodes::AddUserReply { .. } => Self::ADD_USER_REPLY_OPCODE,
            ClientOpcodes::RemoveUserReply { .. } => Self::REMOVE_USER_REPLY_OPCODE,
            ClientOpcodes::ModifyUserReply { .. } => Self::MODIFY_USER_REPLY_OPCODE,
        }
    }

    #[cfg(feature = "sync")]
    pub fn read<R: std::io::Read>(mut r: R) -> Result<Self, MessageError> {
        let mut opcode = [0_u8; 1];
        r.read_exact(&mut opcode)?;

        Ok(match opcode[0] {
            Self::SESSION_KEY_ANSWER_OPCODE => {
                let name = crate::read_string(&mut r)?;

                let session_key = if crate::read_bool(&mut r)? {
                    let mut session_key = [0_u8; 40];
                    r.read_exact(&mut session_key)?;

                    Some(session_key)
                } else {
                    None
                };

                Self::SessionKeyAnswer { name, session_key }
            }
            Self::REGISTER_REALM_REPLY_OPCODE => {
                let success = crate::read_bool(&mut r)?;
                let realm_id = if success {
                    Some(crate::read_u8(&mut r)?)
                } else {
                    None
                };

                Self::RegisterRealmReply { realm_id }
            }
            Self::REMOVE_USER_REPLY_OPCODE
            | Self::MODIFY_USER_REPLY_OPCODE
            | Self::ADD_USER_REPLY_OPCODE => {
                let name = crate::read_string(&mut r)?;

                let success = crate::read_bool(&mut r)?;

                Self::AddUserReply { name, success }
            }
            v => return Err(MessageError::InvalidOpcode(v)),
        })
    }

    pub fn write<W: std::io::Write>(&mut self, mut w: W) -> std::io::Result<()> {
        crate::write_u8(&mut w, self.opcode())?;

        match self {
            ClientOpcodes::SessionKeyAnswer { name, session_key } => {
                crate::write_string(&mut w, &name)?;

                if let Some(session_key) = session_key {
                    crate::write_bool(&mut w, true)?;

                    w.write_all(session_key)?;
                } else {
                    crate::write_bool(&mut w, false)?;
                }
            }
            ClientOpcodes::RegisterRealmReply { realm_id } => {
                if let Some(realm_id) = realm_id {
                    crate::write_bool(&mut w, true)?;

                    crate::write_u8(&mut w, *realm_id)?;
                } else {
                    crate::write_bool(&mut w, false)?;
                }
            }
            ClientOpcodes::ModifyUserReply { name, success }
            | ClientOpcodes::RemoveUserReply { name, success }
            | ClientOpcodes::AddUserReply { name, success } => {
                crate::write_string(&mut w, name)?;

                crate::write_bool(&mut w, *success)?;
            }
        }

        Ok(())
    }

    #[cfg(feature = "tokio")]
    pub async fn tokio_read<R: tokio::io::AsyncReadExt + Unpin>(
        mut r: R,
    ) -> Result<Self, MessageError> {
        let mut opcode = [0_u8; 1];
        r.read_exact(&mut opcode).await?;

        Ok(match opcode[0] {
            Self::SESSION_KEY_ANSWER_OPCODE => {
                let name = crate::read_string_tokio(&mut r).await?;

                let session_key = if crate::read_bool_tokio(&mut r).await? {
                    let mut session_key = [0_u8; 40];
                    r.read_exact(&mut session_key).await?;

                    Some(session_key)
                } else {
                    None
                };

                Self::SessionKeyAnswer { name, session_key }
            }
            Self::REGISTER_REALM_REPLY_OPCODE => {
                let success = crate::read_bool_tokio(&mut r).await?;
                let realm_id = if success {
                    Some(crate::read_u8_tokio(&mut r).await?)
                } else {
                    None
                };

                Self::RegisterRealmReply { realm_id }
            }
            Self::MODIFY_USER_REPLY_OPCODE
            | Self::REMOVE_USER_REPLY_OPCODE
            | Self::ADD_USER_REPLY_OPCODE => {
                let name = crate::read_string_tokio(&mut r).await?;

                let success = crate::read_bool_tokio(&mut r).await?;

                Self::AddUserReply { name, success }
            }
            v => return Err(MessageError::InvalidOpcode(v)),
        })
    }

    #[cfg(feature = "tokio")]
    pub async fn tokio_write<W: tokio::io::AsyncWriteExt + Unpin>(
        &mut self,
        mut w: W,
    ) -> std::io::Result<()> {
        let mut v = Vec::new();
        self.write(&mut v)?;
        w.write_all(&v).await
    }
}

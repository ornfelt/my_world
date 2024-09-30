use crate::error::MessageError;

pub enum ServerOpcodes {
    RequestSessionKey {
        name: String,
    },
    RegisterRealm {
        name: String,
        address: String,
        population: f32,
        locked: bool,
        flags: u8,
        category: u8,
        realm_type: u8,
        version_major: u8,
        version_minor: u8,
        version_patch: u8,
        version_build: u16,
    },
    AddUser {
        name: String,
        password: String,
    },
    RemoveUser {
        name: String,
    },
    ModifyUser {
        name: String,
    },
}

impl ServerOpcodes {
    const REQUEST_SESSION_KEY_OPCODE: u8 = 0;
    const REGISTER_REALM_OPCODE: u8 = 4;
    const ADD_USER_OPCODE: u8 = 6;
    const REMOVE_USER_OPCODE: u8 = 8;
    const MODIFY_USER_OPCODE: u8 = 10;

    const fn opcode(&self) -> u8 {
        match self {
            ServerOpcodes::RequestSessionKey { .. } => Self::REQUEST_SESSION_KEY_OPCODE,
            ServerOpcodes::RegisterRealm { .. } => Self::REGISTER_REALM_OPCODE,
            ServerOpcodes::AddUser { .. } => Self::ADD_USER_OPCODE,
            ServerOpcodes::RemoveUser { .. } => Self::REMOVE_USER_OPCODE,
            ServerOpcodes::ModifyUser { .. } => Self::MODIFY_USER_OPCODE,
        }
    }

    #[cfg(feature = "sync")]
    pub fn read<R: std::io::Read>(mut r: R) -> Result<Self, MessageError> {
        let mut opcode = [0_u8; 1];
        r.read_exact(&mut opcode)?;

        Ok(match opcode[0] {
            Self::REQUEST_SESSION_KEY_OPCODE => {
                let name = crate::read_string(r)?;

                Self::RequestSessionKey { name }
            }
            Self::REGISTER_REALM_OPCODE => {
                let name = crate::read_string(&mut r)?;

                let address = crate::read_string(&mut r)?;

                let population = crate::read_f32(&mut r)?;

                let locked = crate::read_bool(&mut r)?;

                let flags = crate::read_u8(&mut r)?;

                let category = crate::read_u8(&mut r)?;

                let realm_type = crate::read_u8(&mut r)?;

                let version_major = crate::read_u8(&mut r)?;
                let version_minor = crate::read_u8(&mut r)?;
                let version_patch = crate::read_u8(&mut r)?;
                let version_build = crate::read_u16(&mut r)?;

                Self::RegisterRealm {
                    name,
                    address,
                    population,
                    locked,
                    flags,
                    category,
                    realm_type,
                    version_major,
                    version_minor,
                    version_patch,
                    version_build,
                }
            }
            Self::ADD_USER_OPCODE => {
                let name = crate::read_string(&mut r)?;
                let password = crate::read_string(&mut r)?;

                Self::AddUser { name, password }
            }
            Self::REMOVE_USER_OPCODE => {
                let name = crate::read_string(&mut r)?;

                Self::RemoveUser { name }
            }
            Self::MODIFY_USER_OPCODE => {
                let name = crate::read_string(&mut r)?;

                Self::ModifyUser { name }
            }
            v => return Err(MessageError::InvalidOpcode(v)),
        })
    }

    pub fn write<W: std::io::Write>(&mut self, mut w: W) -> std::io::Result<()> {
        crate::write_u8(&mut w, self.opcode())?;

        match self {
            ServerOpcodes::RequestSessionKey { name } => {
                crate::write_string(&mut w, &name)?;
            }
            ServerOpcodes::RegisterRealm {
                name,
                address,
                population,
                locked,
                flags,
                category,
                realm_type,
                version_major,
                version_minor,
                version_patch,
                version_build,
            } => {
                crate::write_string(&mut w, name)?;

                crate::write_string(&mut w, address)?;

                crate::write_f32(&mut w, *population)?;

                crate::write_bool(&mut w, *locked)?;

                crate::write_u8(&mut w, *flags)?;

                crate::write_u8(&mut w, *category)?;

                crate::write_u8(&mut w, *realm_type)?;

                crate::write_u8(&mut w, *version_major)?;
                crate::write_u8(&mut w, *version_minor)?;
                crate::write_u8(&mut w, *version_patch)?;
                crate::write_u16(&mut w, *version_build)?;
            }
            ServerOpcodes::AddUser { name, password } => {
                crate::write_string(&mut w, name)?;

                crate::write_string(&mut w, password)?;
            }
            ServerOpcodes::RemoveUser { name } => {
                crate::write_string(&mut w, name)?;
            }
            ServerOpcodes::ModifyUser { name } => {
                crate::write_string(&mut w, name)?;
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
            Self::REQUEST_SESSION_KEY_OPCODE => {
                let name = crate::read_string_tokio(r).await?;

                Self::RequestSessionKey { name }
            }
            Self::REGISTER_REALM_OPCODE => {
                let name = crate::read_string_tokio(&mut r).await?;

                let address = crate::read_string_tokio(&mut r).await?;

                let population = crate::read_f32_tokio(&mut r).await?;

                let locked = crate::read_bool_tokio(&mut r).await?;

                let flags = crate::read_u8_tokio(&mut r).await?;

                let category = crate::read_u8_tokio(&mut r).await?;

                let realm_type = crate::read_u8_tokio(&mut r).await?;

                let version_major = crate::read_u8_tokio(&mut r).await?;
                let version_minor = crate::read_u8_tokio(&mut r).await?;
                let version_patch = crate::read_u8_tokio(&mut r).await?;
                let version_build = crate::read_u16_tokio(&mut r).await?;

                Self::RegisterRealm {
                    name,
                    address,
                    population,
                    locked,
                    flags,
                    category,
                    realm_type,
                    version_major,
                    version_minor,
                    version_patch,
                    version_build,
                }
            }
            Self::ADD_USER_OPCODE => {
                let name = crate::read_string_tokio(&mut r).await?;
                let password = crate::read_string_tokio(&mut r).await?;

                Self::AddUser { name, password }
            }
            Self::REMOVE_USER_OPCODE => {
                let name = crate::read_string_tokio(&mut r).await?;

                Self::RemoveUser { name }
            }
            Self::MODIFY_USER_OPCODE => {
                let name = crate::read_string_tokio(&mut r).await?;

                Self::ModifyUser { name }
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

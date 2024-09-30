use std::future::Future;
use warthog_lib::{
    CMD_AUTH_LOGON_CHALLENGE_Client, CredentialProvider, Credentials, MatrixCard,
    MatrixCardOptions, MatrixCardVerifier, NormalizedString, PinCode, SrpVerifier,
};

#[derive(Debug, Copy, Clone)]
pub(crate) struct ProviderImpl {
    use_pin: bool,
    use_matrix_card: bool,
}

impl ProviderImpl {
    pub fn new(use_pin: bool, use_matrix_card: bool) -> Self {
        Self {
            use_pin,
            use_matrix_card,
        }
    }
}

impl CredentialProvider for ProviderImpl {
    fn get_user(
        &mut self,
        message: &CMD_AUTH_LOGON_CHALLENGE_Client,
    ) -> impl Future<Output = Option<Credentials>> + Send {
        let v = SrpVerifier::from_username_and_password(
            NormalizedString::new(&message.account_name).unwrap(),
            NormalizedString::new(&message.account_name).unwrap(),
        );

        let matrix_card = if message.version.supports_matrix_card() && self.use_matrix_card {
            Some(MatrixCardOptions {
                matrix_card: MatrixCard::from_data(vec![
                    0;
                    MatrixCard::DEFAULT_DIGIT_COUNT as usize
                        * MatrixCard::DEFAULT_HEIGHT as usize
                        * MatrixCard::DEFAULT_WIDTH as usize
                ]),
                challenge_count: MatrixCardVerifier::DEFAULT_CHALLENGE_COUNT,
            })
        } else {
            None
        };

        let pin = if message.version.supports_pin() && self.use_pin {
            Some(PinCode::from_u64(1234).unwrap())
        } else {
            None
        };

        async move {
            Some(Credentials {
                password_verifier: *v.password_verifier(),
                salt: *v.salt(),
                pin,
                matrix_card,
            })
        }
    }

    fn add_user(
        &mut self,
        _username: &str,
        _password: &str,
    ) -> impl Future<Output = Option<()>> + Send {
        async move { Some(()) }
    }

    fn remove_user(&mut self, _username: &str) -> impl Future<Output = bool> + Send {
        async move { true }
    }

    fn modify_user(&mut self, _username: &str) -> impl Future<Output = bool> + Send {
        async move { true }
    }
}

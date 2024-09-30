use crate::key::{Proof, SessionKey};
use crate::normalized_string::NormalizedString;
use crate::PROOF_LENGTH;
use sha1::{Digest, Sha1};

// This function is the same between Vanilla, TBC and Wrath
pub(crate) fn calculate_world_server_proof(
    username: &NormalizedString,
    session_key: &SessionKey,
    server_seed: u32,
    client_seed: u32,
) -> Proof {
    let server_proof: [u8; PROOF_LENGTH as usize] = Sha1::new()
        .chain_update(username.as_ref())
        .chain_update(0_u32.to_le_bytes())
        .chain_update(client_seed.to_le_bytes())
        .chain_update(server_seed.to_le_bytes())
        .chain_update(session_key.as_le_bytes())
        .finalize()
        .into();

    Proof::from_le_bytes(server_proof)
}

#[cfg(test)]
mod test {
    use crate::key::{Proof, SessionKey};
    use crate::normalized_string::NormalizedString;
    use crate::vanilla_header::internal::calculate_world_server_proof;

    #[test]
    fn verify_world_server_proof() {
        let contents = include_str!("../../tests/srp6_internal/calculate_world_server_proof.txt");
        for line in contents.lines() {
            let mut line = line.split_whitespace();

            let username = NormalizedString::new(line.next().unwrap()).unwrap();

            let session_key = SessionKey::from_be_hex_str(line.next().unwrap());

            let server_seed: u32 = line.next().unwrap().parse().unwrap();
            let client_seed = line.next().unwrap().parse().unwrap();

            let expected = Proof::from_be_hex_str(line.next().unwrap());

            let result = calculate_world_server_proof(
                &NormalizedString::new(username.to_string()).unwrap(),
                &session_key,
                server_seed,
                client_seed,
            );

            assert_eq!(result, expected);
        }
    }
}

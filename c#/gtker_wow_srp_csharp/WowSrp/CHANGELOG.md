# 0.3.0 - 2024-06-01

## Changed

* Renamed interface parameter names to `stream`, `buffer`, and `span` instead of `r`/`w`.
* Made `HeaderData` not `readonly`.

# 0.2.0 - 2024-05-31

## Added

* `NullCrypter` and `NullCrypterWrath` for reading unencrypted headers
  for [SMSG_AUTH_CHALENGE](https://gtker.com/wow_messages/docs/smsg_auth_challenge.html)
  and [CMSG_AUTH_SESSION](https://gtker.com/wow_messages/docs/cmsg_auth_session.html).

# 0.1.3 - 2024-05-23

## Added

* Implicit implementations for all header classes.

# 0.1.2 - 2024-05-22

## Added

* Removed implicit implementations for all header classes due to stack overflows.

# 0.1.1 - 2024-05-22

## Added

* Implicit implementations for all header classes.

# 0.1.0 - 2024-05-22

## Added

* Initial release.

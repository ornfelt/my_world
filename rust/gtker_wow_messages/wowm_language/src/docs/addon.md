# Addon

## Client Version 1.12

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/login_logout/smsg_addon_info.wowm:40`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/login_logout/smsg_addon_info.wowm#L40).
```rust,ignore
struct Addon {
    AddonType addon_type;
    InfoBlock info_block;
    if (info_block == AVAILABLE) {
        KeyVersion key_version;
        if (key_version != ZERO) {
            u8[256] public_key;
        }
        u32 update_available_flag;
    }
    UrlInfo url_info;
    if (url_info == AVAILABLE) {
        CString url;
    }
}
```

Used in:
* [SMSG_ADDON_INFO](smsg_addon_info.md)

## Client Version 2.4.3, Client Version 3

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/login_logout/smsg_addon_info.wowm:64`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/login_logout/smsg_addon_info.wowm#L64).
```rust,ignore
struct Addon {
    u8 addon_type;
    u8 uses_crc;
    Bool uses_diffent_public_key;
    u32 unknown1;
    u8 unknown2;
}
```
### Body

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x00 | 1 / - | u8 | addon_type | Other emus hardcode this to 2. More research is required |
| 0x01 | 1 / - | u8 | uses_crc | Other emus hardcode this to 1. |
| 0x02 | 1 / - | Bool | uses_diffent_public_key |  |
| 0x03 | 4 / Little | u32 | unknown1 | Other emus hardcode this to 0 |
| 0x07 | 1 / - | u8 | unknown2 | Other emus hardcode this to 0 |


Used in:

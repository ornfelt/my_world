# CMSG_AUCTION_SELL_ITEM

## Client Version 1, Client Version 2

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/auction/cmsg/cmsg_auction_sell_item.wowm:1`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/auction/cmsg/cmsg_auction_sell_item.wowm#L1).
```rust,ignore
cmsg CMSG_AUCTION_SELL_ITEM = 0x0256 {
    Guid auctioneer;
    Guid item;
    u32 starting_bid;
    u32 buyout;
    u32 auction_duration_in_minutes;
}
```
### Header

CMSG have a header of 6 bytes.

#### CMSG Header

| Offset | Size / Endianness | Type   | Name   | Description |
| ------ | ----------------- | ------ | ------ | ----------- |
| 0x00   | 2 / Big           | uint16 | size   | Size of the rest of the message including the opcode field but not including the size field.|
| 0x02   | 4 / Little        | uint32 | opcode | Opcode that determines which fields the message contains.|

### Body

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x06 | 8 / Little | [Guid](../types/packed-guid.md) | auctioneer |  |
| 0x0E | 8 / Little | [Guid](../types/packed-guid.md) | item |  |
| 0x16 | 4 / Little | u32 | starting_bid |  |
| 0x1A | 4 / Little | u32 | buyout |  |
| 0x1E | 4 / Little | u32 | auction_duration_in_minutes |  |

## Client Version 3.3.5

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/auction/cmsg/cmsg_auction_sell_item.wowm:11`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/auction/cmsg/cmsg_auction_sell_item.wowm#L11).
```rust,ignore
cmsg CMSG_AUCTION_SELL_ITEM = 0x0256 {
    Guid auctioneer;
    u32 unknown1;
    Guid item;
    u32 unknown2;
    u32 starting_bid;
    u32 buyout;
    u32 auction_duration_in_minutes;
}
```
### Header

CMSG have a header of 6 bytes.

#### CMSG Header

| Offset | Size / Endianness | Type   | Name   | Description |
| ------ | ----------------- | ------ | ------ | ----------- |
| 0x00   | 2 / Big           | uint16 | size   | Size of the rest of the message including the opcode field but not including the size field.|
| 0x02   | 4 / Little        | uint32 | opcode | Opcode that determines which fields the message contains.|

### Body

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x06 | 8 / Little | [Guid](../types/packed-guid.md) | auctioneer |  |
| 0x0E | 4 / Little | u32 | unknown1 |  |
| 0x12 | 8 / Little | [Guid](../types/packed-guid.md) | item |  |
| 0x1A | 4 / Little | u32 | unknown2 |  |
| 0x1E | 4 / Little | u32 | starting_bid |  |
| 0x22 | 4 / Little | u32 | buyout |  |
| 0x26 | 4 / Little | u32 | auction_duration_in_minutes |  |

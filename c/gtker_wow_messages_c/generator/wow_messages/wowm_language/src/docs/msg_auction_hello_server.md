# MSG_AUCTION_HELLO_Server

## Client Version 1.1, Client Version 1.2, Client Version 1.3, Client Version 1.4, Client Version 1.5, Client Version 1.6, Client Version 1.7, Client Version 1.8, Client Version 1.9, Client Version 1.10, Client Version 1.11

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/auction/msg/msg_auction_hello_server.wowm:1`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/auction/msg/msg_auction_hello_server.wowm#L1).
```rust,ignore
smsg MSG_AUCTION_HELLO_Server = 0x0255 {
    Guid auctioneer;
    u32 auction_house_id;
}
```
### Header

SMSG have a header of 4 bytes.

#### SMSG Header

| Offset | Size / Endianness | Type   | Name   | Description |
| ------ | ----------------- | ------ | ------ | ----------- |
| 0x00   | 2 / Big           | uint16 | size   | Size of the rest of the message including the opcode field but not including the size field.|
| 0x02   | 2 / Little        | uint16 | opcode | Opcode that determines which fields the message contains.|

### Body

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x04 | 8 / Little | [Guid](../types/packed-guid.md) | auctioneer |  |
| 0x0C | 4 / Little | u32 | auction_house_id |  |

## Client Version 1.12, Client Version 2, Client Version 3.0, Client Version 3.1, Client Version 3.2, Client Version 3.3.0, Client Version 3.3.1, Client Version 3.3.2

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/auction/msg/msg_auction_hello_server.wowm:8`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/auction/msg/msg_auction_hello_server.wowm#L8).
```rust,ignore
smsg MSG_AUCTION_HELLO_Server = 0x0255 {
    Guid auctioneer;
    AuctionHouse auction_house;
}
```
### Header

SMSG have a header of 4 bytes.

#### SMSG Header

| Offset | Size / Endianness | Type   | Name   | Description |
| ------ | ----------------- | ------ | ------ | ----------- |
| 0x00   | 2 / Big           | uint16 | size   | Size of the rest of the message including the opcode field but not including the size field.|
| 0x02   | 2 / Little        | uint16 | opcode | Opcode that determines which fields the message contains.|

### Body

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x04 | 8 / Little | [Guid](../types/packed-guid.md) | auctioneer |  |
| 0x0C | 4 / - | [AuctionHouse](auctionhouse.md) | auction_house |  |

## Client Version 3.3.3, Client Version 3.3.4, Client Version 3.3.5

### Wowm Representation

Autogenerated from `wowm` file at [`wow_message_parser/wowm/world/auction/msg/msg_auction_hello_server.wowm:15`](https://github.com/gtker/wow_messages/tree/main/wow_message_parser/wowm/world/auction/msg/msg_auction_hello_server.wowm#L15).
```rust,ignore
smsg MSG_AUCTION_HELLO_Server = 0x0255 {
    Guid auctioneer;
    AuctionHouse auction_house;
    Bool auction_house_enabled;
}
```
### Header

SMSG have a header of 4 bytes.

#### SMSG Header

| Offset | Size / Endianness | Type   | Name   | Description |
| ------ | ----------------- | ------ | ------ | ----------- |
| 0x00   | 2 / Big           | uint16 | size   | Size of the rest of the message including the opcode field but not including the size field.|
| 0x02   | 2 / Little        | uint16 | opcode | Opcode that determines which fields the message contains.|

### Body

| Offset | Size / Endianness | Type | Name | Comment |
| ------ | ----------------- | ---- | ---- | ------- |
| 0x04 | 8 / Little | [Guid](../types/packed-guid.md) | auctioneer |  |
| 0x0C | 4 / - | [AuctionHouse](auctionhouse.md) | auction_house |  |
| 0x10 | 1 / - | Bool | auction_house_enabled |  |

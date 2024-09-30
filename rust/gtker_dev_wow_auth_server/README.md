# `dev_wow_auth_server`

Very simple authentication server for World of Warcraft 1.0 through 3.3.5.

The server will accept **ALL** accounts as long as the username is the same as the password.
So a username of `a` and a password of `a` would be accepted, but a username of `a` and a password of `b` would not be.

Set your realmlist to `SET localhost` and connect through the client as normal.

To get the session key for an account, connect to `localhost` on port `8086`.
The server listens on this port and will reply to requests for a session key.

You should send, in pseudo code:
```
msg request_session_key {
    u8 name_length;
    String[name_length] name;
}
```
Or one byte with the total length of the name, and then the bytes of the string in UTF-8 (clients will already send their names this way).

The server will reply with, in pseudo code:
```
msg session_key_answer {
    u8 name_length;
    String[name_length] name;
    bool session_key_found;
    if session_key_found {
        u8[40] session_key;
    }
}
```

The first part of the message is the same as what you just sent. This is so it's possible to send several requests at the same time and still know which is which.

Next is a single byte, if it is 0 the message ends. If it is 1, it is followed by the session key in **little endian**.

The realm list contains a single realm called "Tester" that points to `0.0.0.0:8085` (localhost, but also accessible from outside).

Your world server should listen on port 8085 and handle world messages like [on the wiki](https://wowdev.wiki/Login).

## Options

If the default addresses do not fit you, you can provide the following arguments to the binary:
```bash
Usage: dev_wow_auth_server [OPTIONS]
Options:
  -a, --auth <AUTH>    Port of authentication/login server. Default is '127.0.0.1:3724'
  -w, --world <WORLD>  Port of world server (where the realm list points to). Default is '127.0.0.1:8085'
  -r, --reply <REPLY>  Port of reply server (where you query session keys). Default is '127.0.0.1:8086'
-h, --help           Print help information
-V, --version        Print version information
```

## Why

Unmodified WoW clients can not connect directly to a world server, they need to go through an authentication server first which then points them to the world server.
Writing an authentication server is not a very complex or time intensive task unless your programming language of choice does not have a WoW compatible library for SRP6, in that case you have to [implement the correct SRP6 algorithm yourself](https://gtker.com/implementation-guide-for-the-world-of-warcraft-flavor-of-srp6/) which can be rather tedious and error prone.
Depending on developer skill and previous cryptographic experience this can take from a few days to being a literally impossible task.
All this is before even seeing the realm screen.

This application allows you to skip writing the authentication server and go directly to writing the world server.
Other authentication servers exist, but they are usually very tightly integrated with their world server and require a substantial amount of setup in order to work.
Because this server does not use a database and is written in a compiled language it can be distributed as a single executable which drastically lowers the barrier of entry for writing a WoW world server.

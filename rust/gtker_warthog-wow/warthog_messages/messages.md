# MVP

## Account

* Is account name authed?
    * Yes/No

```
msg request_session_key = 0x00 {
    u8 name_length;
    String[name_length] name;
}

msg session_key_answer = 0x01 {
    u8 name_length;
    String[name_length] name;
    bool session_key_found;
    if session_key_found {
        u8[40] session_key;
    }
}
```

## Realm

* Register realm
    * OK/Fail

```
msg register_realm = 0x04 {
    u8 name_length;
    String[name_length] name;
    u8 address_length;
    String[name_length] address;
    float population;
    bool locked;
    u32 flag;
    u8 category;
    u8 realm_type;
    u8 version_major;
    u8 version_minor;
    u8 version_patch;
    u16 version_build;
}


msg register_realm_reply = 0x05 {
    bool success;
    if success {
        u8 realm_id;
    }
}
```

* How many characters on realm?
    * Reply

```
msg request_character_amount = 0x02 {
    u8 name_length;
    String[name_length] name;
}

msg character_amount_answer = 0x03 {
    u8 name_length;
    String[name_length] name;
    u8 amount_of_characters;
}
```

## Account Management

* Add user
    * OK/Fail
* Remove user
    * OK/Fail
* Change user
    * OK/Fail

```
msg add_user = 0x06 {
  u8 name_length;
  String[name_length] name;
  u8 password_length;
  String[password_length] password;
}

msg add_user_response = 0x07 {
  u8 name_length;
  String[name_length] name;
  bool success;
}

msg remove_user = 0x08 {
  u8 name_length;
  String[name_length] name;
}

msg remove_user_response = 0x09 {
  u8 name_length;
  String[name_length] name;
  bool success;
}

msg modify_user = 0x0A {
  u8 name_length;
  String[name_length] name;
}

msg modify_user_reply = 0x0B {
  u8 name_length;
  String[name_length] name;
  bool success;
}
```

#pragma once

#include <string>

#include "v8.h"

#include "ScriptRunner.h"

using v8::String;
using v8::External;
using v8::Local;

class Player
{
public:
    Player(const char* name, ScriptRunner* sr) : _name(name), _sr(sr) {}

    string const& GetName() const { return _name; }
    Isolate* GetIsolate() const { return _sr->GetIsolate(); }

private:
    string _name;

    ScriptRunner* _sr;
};

class PlayerJS
{
public:
    static void GetName(const FunctionCallbackInfo<Value>& args)
    {
        Local<External> e = Local<External>::Cast(args.Holder()->GetInternalField(0));
        Player* player = static_cast<Player*>(e->Value());

        Isolate* isolate = player->GetIsolate();
        args.GetReturnValue().Set(String::NewFromUtf8(isolate, player->GetName().c_str()));
    }
};

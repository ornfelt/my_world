#pragma once

#include "v8.h"

using v8::FunctionCallbackInfo;
using v8::Value;
using v8::Isolate;

class ScriptManager;
class Player;

class Creature
{
public:
    Creature(int id, ScriptManager* sm);

    void CastSpell(Player* target, int spellId);
    int GetId() const { return _id; }
    int GetGuid() const { return _guid; }

    Isolate* GetIsolate() const;

    // Hooks.
    void EnterCombat(Player* player);
private:
    int _id;
    int _guid;

    ScriptManager* _sm;
};

class CreatureJS
{
public:
    static void CastSpell(const FunctionCallbackInfo<Value>& args);
    static void GetId(const FunctionCallbackInfo<Value>& args);
    static void GetGuid(const FunctionCallbackInfo<Value>& args);
};

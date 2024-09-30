#pragma once

#include "v8.h"

using v8::Global;
using v8::Function;
using v8::Object;

class Creature;
class Player;
class ScriptRunner;

class CreatureAI
{
public:
    CreatureAI(Global<Function>* constructor, Creature* creature, ScriptRunner* sr);

    void EnterCombat(Player* player);

private:
    Global<Object> _scriptObj;

    ScriptRunner* _sr;
};

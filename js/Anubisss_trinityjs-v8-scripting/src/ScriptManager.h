#pragma once

#include <map>

#include "v8.h"

using std::map;

using v8::Global;
using v8::Function;
using v8::Isolate;

class Creature;
class CreatureAI;
class Player;
class ScriptRunner;

class ScriptManager
{
public:
    ScriptManager(map<int, Global<Function>* >& scripts, ScriptRunner* sr) : _scripts(scripts), _sr(sr) {}

    void RegisterAI(Creature* creature);
    Isolate* GetIsolate() const;

    void OnEnterCombat(Creature* creature, Player* player);

private:
    map<int /* creature ID */, Global<Function>* >& _scripts;
    map<int /* creature GUID */, CreatureAI*> _ais;

    ScriptRunner* _sr;
};

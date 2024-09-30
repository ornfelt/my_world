#include "Creature.h"

#include <atomic>

#include "ScriptManager.h"
#include "Player.h"
#include "Logger.h"

using std::atomic;

using v8::Local;
using v8::External;
using v8::Object;
using v8::Integer;

atomic<int> guid(0);

Creature::Creature(int id, ScriptManager* sm) : _id(id), _sm(sm)
{
    _guid = ++guid;

    ostringstream ss;
    ss << "[Creature::Creature] constructor, ID: " << _id << ", GUID: " << _guid;
    Logger::Log(ss);

    _sm->RegisterAI(this);
}

void Creature::CastSpell(Player* target, int spellId)
{
    ostringstream ss;
    ss << "[Creature::CastSpell] ID: " << _id << ", GUID: " << _guid << ", spell ID: " << spellId << " - target player name: " << target->GetName();
    Logger::Log(ss);
}

Isolate* Creature::GetIsolate() const
{
    return _sm->GetIsolate();
}

void Creature::EnterCombat(Player* player)
{
    ostringstream ss;
    ss << "[Creature::EnterCombat] ID: " << _id << ", GUID: " << _guid << " - player name: " << player->GetName();
    Logger::Log(ss);

    _sm->OnEnterCombat(this, player);
}

void CreatureJS::CastSpell(const FunctionCallbackInfo<Value>& args)
{
    Local<External> creatureExternal = Local<External>::Cast(args.Holder()->GetInternalField(0));
    Creature* creature = static_cast<Creature*>(creatureExternal->Value());

    Local<Object> playerObj = Local<Object>::Cast(args[0]);
    Local<External> playerExternal = Local<External>::Cast(playerObj->GetInternalField(0));
    Player* player = static_cast<Player*>(playerExternal->Value());

    creature->CastSpell(player, args[1]->ToInt32()->Value());
}

void CreatureJS::GetId(const FunctionCallbackInfo<Value>& args)
{
    Local<External> e = Local<External>::Cast(args.Holder()->GetInternalField(0));
    Creature* creature = static_cast<Creature*>(e->Value());

    Isolate* isolate = creature->GetIsolate();
    args.GetReturnValue().Set(Integer::New(isolate, creature->GetId()));
}

void CreatureJS::GetGuid(const FunctionCallbackInfo<Value>& args)
{
    Local<External> e = Local<External>::Cast(args.Holder()->GetInternalField(0));
    Creature* creature = static_cast<Creature*>(e->Value());

    Isolate* isolate = creature->GetIsolate();
    args.GetReturnValue().Set(Integer::New(isolate, creature->GetGuid()));
}

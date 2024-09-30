#include "ScriptManager.h"

#include "Creature.h"
#include "CreatureAI.h"
#include "Player.h"
#include "ScriptRunner.h"
#include "Logger.h"

void ScriptManager::RegisterAI(Creature* creature)
{
    ostringstream ss;
    ss << "[ScriptManager::RegisterAI] creature ID: " << creature->GetId() << ", creature GUID: " << creature->GetGuid();
    Logger::Log(ss);


    _ais[creature->GetGuid()] = new CreatureAI(_scripts[creature->GetId()], creature, _sr);
}

void ScriptManager::OnEnterCombat(Creature* creature, Player* player)
{
    ostringstream ss;
    ss << "[ScriptManager::OnEnterCombat] creature ID: " << creature->GetId() << ", creature GUID: " << creature->GetGuid() << " - player name: " << player->GetName();
    Logger::Log(ss);

    _ais[creature->GetGuid()]->EnterCombat(player);
}

Isolate* ScriptManager::GetIsolate() const
{
    return _sr->GetIsolate();
}

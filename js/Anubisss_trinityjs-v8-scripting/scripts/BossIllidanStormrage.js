'use strict'

const BOSS_ILLIDAN_STORMRAGE = {
  spells: {
    frenzy: 40683,
  },
}

class BossIllidanStormrage {
  constructor(creature) {
    this.creature = creature
    Log(`BossIllidanStormrage constructor, ID: ${ this.creature.GetId() }, GUID: ${ this.creature.GetGuid() }`)
  }

  EnterCombat(player) {
    Log(`BossIllidanStormrage EnterCombat, ID: ${ this.creature.GetId() }, GUID: ${ this.creature.GetGuid() } - player name: ${ player.GetName() }`)

    this.creature.CastSpell(player, BOSS_ILLIDAN_STORMRAGE.spells.frenzy)
  }
}

RegisterCreateScript(22917, BossIllidanStormrage)

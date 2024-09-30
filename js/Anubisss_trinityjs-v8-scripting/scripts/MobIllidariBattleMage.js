'use strict'

const MOB_ILLIDARI_BATTLE_MAG = {
  spells: {
    fireball: 41383,
    blizzard: 41382,
  },
}


class MobIllidariBattleMage {
  constructor(creature) {
    this.creature = creature
    this.firstTarget = null
    Log(`MobIllidariBattleMage constructor, ID: ${ this.creature.GetId() }, GUID: ${ this.creature.GetGuid() }`)
  }

  EnterCombat(player) {
    Log(`MobIllidariBattleMage EnterCombat, ID: ${ this.creature.GetId() }, GUID: ${ this.creature.GetGuid() } - player name: ${ player.GetName() }`)

    this.creature.CastSpell(player, MOB_ILLIDARI_BATTLE_MAG.spells.fireball)
    if (!this.firstTarget) {
      this.firstTarget = player
    } else {
      this.creature.CastSpell(this.firstTarget, MOB_ILLIDARI_BATTLE_MAG.spells.blizzard)
    }
  }
}

RegisterCreateScript(23402, MobIllidariBattleMage)

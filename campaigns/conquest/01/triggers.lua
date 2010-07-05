delayedEnding = 999999999

DisallowAllUnits()

local function GetOwnUnitsAmount(type)
  return ThisPlayer.UnitTypesCount[UnitTypeByIdent(type).Slot]
end

AddTrigger(
  function()
    return Players[1].TotalNumUnits == 0
  end,
  function()
    AddMessage("The outpost is now under your control.")
    AddMessage("You must finish constructing the base by building a gun turret and a training camp.")
    AddObjective("Build a gun turret")
    AddObjective("Build a training")
    DefineAllow("unit-engineer", AllowAll)
    DefineAllow("unit-gturret", AllowAll)
    DefineAllow("unit-camp", AllowAll)
    return false
  end)

AddTrigger(
  function()
    return GetOwnUnitsAmount("unit-gturret") >= 1 and
           GetOwnUnitsAmount("unit-camp") >= 1
  end,
  function()
    AddMessage("Well done, the base is now operational.")
    delayedEnding = GameCycle + 400
    return false
  end)

AddTrigger(
  function()
    return GameCycle >= delayedEnding
  end,
  function()
    return StopGame(GameVictory)
  end)

if (GameCycle == 0) then
  AddMessage("Lead your forces to the outpost and take control of it")
end


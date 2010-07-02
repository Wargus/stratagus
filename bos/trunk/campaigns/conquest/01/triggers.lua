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
    AddMessage("You must improve the defenses by building a gun turret.")
    SetObjectives("Build a gun turret")
    DefineAllow("unit-engineer", AllowAll)
    DefineAllow("unit-gturret", AllowAll)
    return false
  end)

AddTrigger(
  function()
    return GetOwnUnitsAmount("unit-gturret") >= 1
  end,
  function()
    AddMessage("")
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


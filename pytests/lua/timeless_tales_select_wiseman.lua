local map = "maps/ftm/(4)beethoven-day.smp"

local selected = false

local function log(message)
  print(message)
  if io and io.stdout then
    io.stdout:flush()
  end
end

CustomStartup = function()
  SetTitleScreens({})
  local originalAddTrigger = AddTrigger
  local injected = false
  AddTrigger = function(condition, action)
    originalAddTrigger(condition, action)
    if injected then
      return
    end
    injected = true
    log("PYTEST_TIMELESS_INJECTED_WISEMAN_TRIGGER")
    originalAddTrigger(
      function()
        return (not selected) and GameCycle > 450
      end,
      function()
        local units = GetUnits("any")
        for i = 1, table.getn(units) do
          if GetUnitVariable(units[i], "Ident") == "unit-caanoo-wiseman" then
            log("PYTEST_TIMELESS_SELECTING_WISEMAN unit=" .. units[i] ..
                " name=" .. GetUnitVariable(units[i], "Name") ..
                " x=" .. GetUnitVariable(units[i], "PosX") ..
                " y=" .. GetUnitVariable(units[i], "PosY"))
            SelectSingleUnit(units[i])
            selected = true
            log("PYTEST_TIMELESS_SELECTED_WISEMAN")
            return
          end
        end
      end
    )
  end
  RunMap(map)
end

Load("scripts/stratagus.lua")

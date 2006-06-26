function HandleCommandKey(key, ctrl, alt, shift)
  -- FIXME: pause game if not multiplayer
  if ((key == "h" and (ctrl or alt)) or key == "f1") then
    RunHelpMenu()
  elseif (key == "f5") then
    RunGameOptionsMenu()
  elseif (key == "f7") then
    RunGameSoundOptionsMenu()
  elseif (key == "f8") then
    RunGameSpeedOptionsMenu()
  elseif (key == "f9") then
    RunPreferencesMenu()
  elseif ((key == "m" and alt) or key == "f10") then
    RunGameMenu()
  elseif ((key == "s" and alt) or key == "f11") then
    RunSaveMenu()
  elseif ((key == "l" and alt) or key == "f12") then
    RunLoadMenu()
  elseif (key == "q" and (ctrl or alt)) then
    RunQuitToMenuConfirmMenu()
  elseif (key == "r" and (ctrl or alt)) then
    RunRestartConfirmMenu()
  elseif (key == "x" and (ctrl or alt)) then
    RunExitConfirmMenu()
  else
    return false
  end
  return true
end


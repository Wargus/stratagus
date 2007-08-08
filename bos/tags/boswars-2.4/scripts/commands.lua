function HandleIngameCommandKey(key, ctrl, alt, shift)
  if ((key == "h" and (ctrl or alt)) or key == "f1") then
    if (not IsNetworkGame()) then SetGamePaused(true) end
    RunHelpMenu()
  elseif (key == "f5") then
    if (not IsNetworkGame()) then SetGamePaused(true) end
    RunGameOptionsMenu()
  elseif (key == "f7") then
    if (not IsNetworkGame()) then SetGamePaused(true) end
    RunGameSoundOptionsMenu()
  elseif (key == "f9") then
    if (not IsNetworkGame()) then SetGamePaused(true) end
    RunPreferencesMenu()
  elseif ((key == "m" and alt) or key == "f10") then
    if (not IsNetworkGame()) then SetGamePaused(true) end
    RunGameMenu()
  elseif ((key == "s" and alt) or key == "f11") then
    if (not IsReplayGame() and not IsNetworkGame()) then
      SetGamePaused(true)
      RunSaveMenu()
    end
  elseif (key == "q" and (ctrl or alt)) then
    if (not IsNetworkGame()) then SetGamePaused(true) end
    RunQuitToMenuConfirmMenu()
  elseif (key == "r" and (ctrl or alt)) then
    if (not IsNetworkGame()) then SetGamePaused(true) end
    RunRestartConfirmMenu()
  elseif (key == "x" and (ctrl or alt)) then
    if (not IsNetworkGame()) then SetGamePaused(true) end
    RunExitConfirmMenu()
  else
    return false
  end
  return true
end

HandleCommandKey = HandleIngameCommandKey

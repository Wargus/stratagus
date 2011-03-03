--      (c) Copyright 2011      by Stratagus Team

function stratagus.RunPreferencesOptionsMenu()
  local menu = StratagusMenu()
  local offx = (Video.Width - 640) / 2 + 320 - 128
  local offy = 0
  
  menu:addLabel("Preferences", offx + 128, offy + 11)
  menu:addLabel("Game Speed", offx + 16, offy + 40 + 36 * 2, Fonts["stratagus-game"], false)
  local gamespeed = menu:addSlider(15, 75, 198, 18, offx + 32, offy + 40 + 36 * 2.5,
    function(sl) SetGameSpeed(sl:getValue()) end)
  gamespeed:setValue(GetGameSpeed())

  menu:addLabel("slow", offx + 34, offy + 40 + (36 * 3) + 6, Fonts["stratagus-small"], false)
  menu:addLabel("fast", offx + 230, offy + 40 + (36 * 3) + 6, Fonts["stratagus-small"], true)

  menu:addFullButton("~!OK", "o", offx + 128 - (224 / 2), offy + 288 - 40,
    function()
      stratagus.preferences.GameSpeed = GetGameSpeed()
      SavePreferences()
      menu:stop()
    end)
  return menu:run()
end


function SoundOptionsMenu()
  local menu = StratagusMenu()
  local offx = (Video.Width - 352) / 2
  local offy = (Video.Height - 352) / 2

  local soundslider
  local effectscheckbox
  local musicslider
  local musiccheckbox

  local function Update()
   SetEffectsVolume(soundslider:getValue())
   SetEffectsEnabled(effectscheckbox:isMarked())
   soundslider:Enable(effectscheckbox:isMarked())
   SetMusicVolume(musicslider:getValue())
   SetMusicEnabled(musiccheckbox:isMarked())
   musicslider:Enable(musiccheckbox:isMarked())
  end
  
  menu:addLabel("Sound Options", 176, 11)
  menu:writeText("Effects Volume", offx + 16, offy + 36 * 1)

  -- FIXME: disable if effects turned off
  local slider = {min = 0, max = 255, width = 198, height = 18};
  soundslider = menu:addSlider(slider.min, slider.max, slider.width, slider.height, offx + 32, offy + 36 * 1.5, Update)
  menu:addLabel("min", offx + 44, offy + 36 * 2 + 6, Fonts["stratagus-game"], true)
  menu:addLabel("max", offx + 218, offy + 36 * 2 + 6, Fonts["stratagus-game"], true)

  effectscheckbox = menu:addCheckBox("Enabled", offx + 240, offy + 36 * 1.5, Update)
  effectscheckbox:adjustSize()
  
  menu:writeText("Music Volume", offx + 16, offy + 36 * 3)

  musicslider = menu:addSlider(slider.min, slider.max, slider.width, slider.height, offx + 32, offy + 36 * 3.5, Update)
  menu:addLabel("min", offx + 44, offy + 36 * 4 + 6, Fonts["stratagus-game"], true)
  menu:addLabel("max", offx + 218, offy + 36 * 4 + 6, Fonts["stratagus-game"], true)
  
  musiccheckbox = menu:addCheckBox("Enabled", offx + 240, offy + 36 * 3.5,
    function() Update(); MusicStopped() end)
  musiccheckbox:adjustSize();

  menu:addHalfButton("~!OK", "o", offx + 123, offy + 55 + 26*12 + 14,
    function()
      stratagus.preferences.EffectsVolume = GetEffectsVolume()
      stratagus.preferences.EffectsEnabled = IsEffectsEnabled()
      stratagus.preferences.MusicVolume = GetMusicVolume()
      stratagus.preferences.MusicEnabled = IsMusicEnabled()
      SavePreferences()
      menu:stop()
    end)

  soundslider:setValue(GetEffectsVolume())
  effectscheckbox:setMarked(IsEffectsEnabled())
  musicslider:setValue(GetMusicVolume())
  musiccheckbox:setMarked(IsMusicEnabled())
  Update()

  return menu:run()
end


function RunSoundOptionsMenu()
  local continue = 1
  while (continue == 1) do
    continue = SoundOptionsMenu()
  end
end

--[[ ------------------------------------------ --]]

local function SetVideoSize(width, height)
  if (Video:ResizeScreen(width, height) == false) then
    return
  end
--  bckground:Resize(Video.Width, Video.Height)
--  backgroundWidget = ImageWidget(bckground)
  stratagus.preferences.VideoWidth = Video.Width
  stratagus.preferences.VideoHeight = Video.Height
  SavePreferences()
end

local function VideoOptionsMenu()
  local menu = StratagusMenu()
  local offx = (Video.Width - 352) / 2
  local offy = (Video.Height - 352) / 2
  local b

  menu:addLabel("Video Options", offx + 176, offy + 1)
  menu:addLabel("Video Resolution", offx + 16, offy + 34, Fonts["stratagus-game"], false)
  group = "resolution"
  b = menu:addRadioButton("640 x 480", group, offx + 16, offy + 55 + 26*0, function() SetVideoSize(640, 480) menu:stop(1) end)
  if (Video.Width == 640) then b:setMarked(true) end
  b = menu:addRadioButton("800 x 480", group, offx + 16, offy + 55 + 26*1, function() SetVideoSize(800, 480) menu:stop(1) end)
  if (Video.Width == 800 and Video.Height == 480) then b:setMarked(true) end
  b = menu:addRadioButton("800 x 600", group, offx + 16, offy + 55 + 26*2, function() SetVideoSize(800, 600) menu:stop(1) end)
  if (Video.Width == 800 and Video.Height == 600) then b:setMarked(true) end
  b = menu:addRadioButton("1024 x 768", group, offx + 16, offy + 55 + 26*3, function() SetVideoSize(1024, 768) menu:stop(1) end)
  if (Video.Width == 1024) then b:setMarked(true) end
  b = menu:addRadioButton("1280 x 800", group, offx + 16, offy + 55 + 26*4, function() SetVideoSize(1280, 800) menu:stop(1) end)
  if (Video.Height == 800) then b:setMarked(true) end
  b = menu:addRadioButton("1280 x 960", group, offx + 16, offy + 55 + 26*5, function() SetVideoSize(1280, 960) menu:stop(1) end)
  if (Video.Height == 960) then b:setMarked(true) end
  b = menu:addRadioButton("1280 x 1024", group, offx + 16, offy + 55 + 26*6, function() SetVideoSize(1280, 1024) menu:stop(1) end)
  if (Video.Height == 1024) then b:setMarked(true) end
  b = menu:addRadioButton("1400 x 1050", group, offx + 16, offy + 55 + 26*7, function() SetVideoSize(1400, 1050) menu:stop(1) end)
  if (Video.Width == 1400) then b:setMarked(true) end
  b = menu:addRadioButton("1600 x 1200", group, offx + 16, offy + 55 + 26*8, function() SetVideoSize(1600, 1200) menu:stop(1) end)
  if (Video.Width == 1600) then b:setMarked(true) end
  b = menu:addRadioButton("1680 x 1050", group, offx + 16, offy + 55 + 26*9, function() SetVideoSize(1680, 1050) menu:stop(1) end)
  if (Video.Width == 1680) then b:setMarked(true) end

  b = menu:addCheckBox("Full Screen", offx + 17, offy + 55 + 26*10 + 14,
    function()
      ToggleFullScreen()
      stratagus.preferences.VideoFullScreen = Video.FullScreen
      SavePreferences()
      menu:stop(1)
    end)
  b:setMarked(Video.FullScreen)

  local checkTexture = menu:addCheckBox("Set Maximum OpenGL Texture to 256", offx + 127, offy + 55 + 26*10 + 14,
    function()
      if (checkTexture:isMarked()) then
        stratagus.preferences.MaxOpenGLTexture = 256
      else
        stratagus.preferences.MaxOpenGLTexture = 0
      end
      SetMaxOpenGLTexture(stratagus.preferences.MaxOpenGLTexture)
      SavePreferences()
    end)
  if (stratagus.preferences.MaxOpenGLTexture == 128) then checkTexture:setMarked(true) end

  checkOpenGL = menu:addCheckBox("Use OpenGL / OpenGL ES 1.1 (restart required)", offx + 17, offy + 55 + 26*11 + 14,
    function()
--TODO: Add function for immediately change state of OpenGL
      stratagus.preferences.UseOpenGL = checkOpenGL:isMarked()
      SavePreferences()
--      menu:stop(1) --TODO: Enable if we have an OpenGL function
    end)
  checkOpenGL:setMarked(stratagus.preferences.UseOpenGL)--TODO: Enable if we have an OpenGL function

  menu:addHalfButton("~!OK", "o", offx + 123, offy + 55 + 26*12 + 14, function() menu:stop() end)
  return menu:run()
end

function RunVideoOptionsMenu()
  local continue = 1
  while (continue == 1) do
    continue = VideoOptionsMenu()
  end
end

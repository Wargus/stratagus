# Building Complex Maps

The Stratagus engine is very flexible when it comes to custom maps, even though
most of the flexibility is not exposed in the map editor. This document aims to
provide a bit of guidance if you want to create custom missions with complex
setups, triggers, and/or objectives.

## 1. Build the map in the map editor

#### 1.1 Create the basics

The first step should be to build the basic map in the map editor. This includes
setting up how many factions you need and of which colors, their units, the
terrain etc. Basically, anything that can be done in the editor, should be done
in the editor

#### 1.2 Touch up appearances

The map editor comes with a feature to draw "decoration" tiles, that is, to draw
tiles without automatically changing and adjusting the tiles around it. This
feature is really for the last touch ups, since afterwards the tiles cannot be
reverted to automatic updates from the UI. So, if you are sure you are done with
the terrain, you can use this touch-up feature to draw single tiles of specific
variant to have the maximum control over the terrain.

## 2. Adapt the Scripts

A map is saved into two files a `.smp` and a `.sms` file. You should not touch
these files. Instead, you can create (if they do not already exist) two
additional files to write custom lua code that can enhance the map when it is
loaded. For example, if your map is called `my_map`, then you would have a file
`my_map.sms` and `my_map.smp`. You can add files `my_map.sms.preamble` and
`my_map.sms.postamble`. The first is loaded *before* the map is loaded (so you
can, for example, change how units are created by changing the definition of the
`CreateUnit` function). The second is loaded *after* the map, so you can add
additional custom events and objectives. Let's talk about this latter case
first.

#### 2.1 Custom events and objectives

In-game events and objectives are coded the same way. Open the `.sms.postamble`
file in an editor of your choice. All objectives and events are done using
"Triggers". Triggers are pairs of functions that are run at regular intervals by
the game. A complex victory condition trigger can look like this:

```
local victoryTimer = -1

AddTrigger(
  function()
    if GetNumUnitsAt(
         GetThisPlayer(), "any",
         {Map.Info.MapWidth / 2 - 5, Map.Info.MapHeight / 2 - 5},
         {Map.Info.MapWidth / 2 + 5, Map.Info.MapHeight / 2 + 5}) > 5 then
      return true
    else
      return false
    end
  end,
  function()
    AddMessage("5 Units in the center!")
    victoryTimer = 10
    return false
  end
)

AddTrigger(
  function()
    if victoryTimer > 0 then
      victoryTimer = victoryTimer - 1
      AddMessage("Time remaining until victory: " .. victoryTimer)
    end
    return victoryTimer == 0
  end,
  function()
    return ActionVictory()
  end
)
```

Let's unpack this. We are defining two triggers. The first function is the
"condition function" of the trigger. The second function is the actual
trigger. The first function is run at regular intervals during the game until it
returns `true`. Only then is the second function run. If the second function
returns `false`, then that trigger will be removed and will never run again.

The first trigger condition here checks if the number of units of the active
player in the map center (in a 10x10 grid around the map center) is larger
than 5. If this is true, the condition returns true and the second function
runs. The second function shows a message that 5 units are now at the center and
sets the variable `victoryTimer` to 10.

The second trigger just keeps checking the `victoryTimer` variable. As long as
that variable is less than 0, nothing happens. But when the first trigger has
fired and set the variable to 10, then the second trigger will start counting it
down and show that as an in-game message. Once the `victoryTimer` has counted
down to 0, the condition of the second trigger returns `true` and the action
function runs. The action function in this case just calls `ActionVictory`. What
that means is the game ends with a victory.

For conditions where the game should be lost, `ActionDefeat` is used.

Note how powerful this can be. Of course, now we are just using a trigger to
show some message and set a variable, but you could have triggers that spawn or
transform units, change diplomacy, scroll the map somewhere, pause the game and
show an "in-game dialogue", or even change tiles on the map to have something
like a "natural disaster event" that changes the face of the earth. For all the
things you can do, check the Lua functions that are available:
https://stratagus.com/lua_bindings.html

#### 2.2 Custom alliances

A common request for complex games is to be able to declare custom alliances,
like some AI players being in a team with the player or player co-op against
AI. This can be achieved using a custom startup function.

After the game is loaded and everything is ready to start running, Stratagus
calls one last Lua function to do any last minute setup. This Lua function is
`GameStarting`.

As an example, you can add this to your `.sms.preamble` file:

```
local OldGameStarting = GameStarting
function GameStarting()
        OldGameStarting()
        SetDiplomacy(0, "allied", 2)
        SetSharedVision(0, true, 2)
        SetDiplomacy(2, "allied", 0)
        SetSharedVision(2, true, 0)
        GameStarting = OldGameStarting
end
```

This will ensure that at the beginning of the game, players 0 and 2 are always
allied. Just as with triggers, all the Lua functions are available to you here,
so anything can be done at this point.

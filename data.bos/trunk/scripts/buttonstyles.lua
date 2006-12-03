--            ____            
--           / __ )____  _____
--          / __  / __ \/ ___/
--         / /_/ / /_/ (__  ) 
--        /_____/\____/____/  
--
--  Invasion - Battle of Survival                  
--   A GPL'd futuristic RTS game
--
--      buttonstyles.lua - Define the widgets
--
--      (c) Copyright 2004 by Jimmy Salmon
--
--      This program is free software; you can redistribute it and/or modify
--      it under the terms of the GNU General Public License as published by
--      the Free Software Foundation; either version 2 of the License, or
--      (at your option) any later version.
--  
--      This program is distributed in the hope that it will be useful,
--      but WITHOUT ANY WARRANTY; without even the implied warranty of
--      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--      GNU General Public License for more details.
--  
--      You should have received a copy of the GNU General Public License
--      along with this program; if not, write to the Free Software
--      Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
--
--      $Id$

DefineButtonStyle("main", {
  Size = {200, 20},
  Font = "game",
  TextNormalColor = "yellow",
  TextReverseColor = "white",
  TextAlign = "Center",
  TextPos = {100, 4},
  Default = {
    File = "general/ui_buttons.png", Size = {300, 144}, Frame = 4,
  },
  Hover = {
    TextNormalColor = "white",
  },
  Selected = {
    Border = {
      Color = {252, 252, 0}, Size = 1,
    },
  },
  Clicked = {
    File = "general/ui_buttons.png", Size = {300, 144}, Frame = 5,
    TextNormalColor = "white",
    TextPos = {102, 6},
  },
  Disabled = {
    File = "general/ui_buttons.png", Size = {300, 144}, Frame = 3,
    TextNormalColor = "grey",
    TextReverseColor = "grey",
  },
})

DefineButtonStyle("black", {
  Size = {190, 12},
  Font = "game",
  TextNormalColor = "yellow",
  TextReverseColor = "white",
  TextAlign = "Center",
  TextPos = {100, 0},
  Default = {
      Size = {300, 144},
  },
  Hover = {
    TextNormalColor = "white",
  },
  Selected = {
    Border = {
      Color = {252, 252, 0}, Size = 1,
    },
  },
  Clicked = {
    Size = {300, 144},
    TextNormalColor = "white",
    TextPos = {102, 2},
  },
  Disabled = {
    Size = {300, 144}, 
    TextNormalColor = "grey",
    TextReverseColor = "grey",
  },
})


DefineButtonStyle("network", {
  Size = {80, 12},
  Font = "game",
  TextNormalColor = "yellow",
  TextReverseColor = "white",
  TextAlign = "Center",
  TextPos = {40, 0},
  Default = {
    Size = {300, 144},
  },
  Hover = {
    TextNormalColor = "white",
  },
  Selected = {
    Border = {
      Color = {252, 252, 0}, Size = 1,
    },
  },
  Clicked = {
    Size = {300, 144},
    TextNormalColor = "white",
    TextPos = {42, 2},
  },
  Disabled = {
    Size = {300, 144},
    TextNormalColor = "grey",
    TextReverseColor = "grey",
  },
})

DefineButtonStyle("gm-half", {
  Size = {106, 28},
  Font = "large",
  TextNormalColor = "yellow",
  TextReverseColor = "white",
  TextAlign = "Center",
  TextPos = {53, 7},
  Default = {
    File = "general/ui_buttons.png", Size = {300, 144}, Frame = 10,
  },
  Hover = {
    TextNormalColor = "white",
  },
  Selected = {
    Border = {
      Color = {252, 252, 0}, Size = 1,
    },
  },
  Clicked = {
    File = "general/ui_buttons.png", Size = {300, 144}, Frame = 11,
    TextNormalColor = "white",
    TextPos = {55, 9},
  },
  Disabled = {
    File = "general/ui_buttons.png", Size = {300, 144}, Frame = 9,
    TextNormalColor = "grey",
    TextReverseColor = "grey",
  },
})

DefineButtonStyle("gm-full", {
  Size = {224, 28},
  Font = "large",
  TextNormalColor = "yellow",
  TextReverseColor = "white",
  TextAlign = "Center",
  TextPos = {112, 7},
  Default = {
    File = "general/ui_buttons.png", Size = {300, 144}, Frame = 16,
  },
  Hover = {
    TextNormalColor = "white",
  },
  Selected = {
    Border = {
      Color = {252, 252, 0}, Size = 1,
    },
  },
  Clicked = {
    File = "general/ui_buttons.png", Size = {300, 144}, Frame = 17,
    TextNormalColor = "white",
    TextPos = {114, 9},
  },
  Disabled = {
    File = "general/ui_buttons.png", Size = {300, 144}, Frame = 15,
    TextNormalColor = "grey",
    TextReverseColor = "grey",
  },
})

DefineButtonStyle("folder", {
  Size = {39, 22},
  Font = "large",
  TextNormalColor = "yellow",
  TextReverseColor = "white",
  TextAlign = "Left",
  TextPos = {44, 6},
  Default = {
    File = "general/ui_buttons.png", Size = {300, 144}, Frame = 51,
  },
  Hover = {
    TextNormalColor = "white",
  },
  Selected = {
    Border = {
      Color = {252, 252, 0}, Size = 1,
    },
  },
  Clicked = {
    File = "general/ui_buttons.png", Size = {300, 144}, Frame = 52,
    TextNormalColor = "white",
    TextPos = {2, 2},
  },
  Disabled = {
    File = "general/ui_buttons.png", Size = {300, 144}, Frame = 50,
  },
})

DefineButtonStyle("icon", {
  Size = {46, 38},
  Font = "game",
  TextNormalColor = "yellow",
  TextReverseColor = "white",
  TextAlign = "Right",
  TextPos = {46, 26},
  Default = {
    Border = {
      Color = {0, 0, 0}, Size = 1,
    },
  },
  Hover = {
    TextNormalColor = "white",
    Border = {
      Color = {128, 128, 128}, Size = 1,
    },
  },
  Selected = {
    Border = {
      Color = {0, 252, 0}, Size = 1,
    },
  },
  Clicked = {
    TextNormalColor = "white",
    Border = {
      Color = {128, 128, 128}, Size = 1,
    },
  },
  Disabled = {
  },
})

DefineCheckboxStyle("round", {
  Size = {19, 19},
  Font = "game",
  TextNormalColor = "yellow",
  TextReverseColor = "white",
  TextAlign = "Left",
  TextPos = {24, 4},
  Default = {
    File = "general/ui_buttons.png", Size = {300, 144}, Frame = 19,
  },
  Hover = {
    TextNormalColor = "white",
  },
  Selected = {
    Border = {
      Color = {252, 252, 0}, Size = 1,
    },
  },
  Disabled = {
    File = "general/ui_buttons.png", Size = {300, 144}, Frame = 18,
  },
  Clicked = {
    File = "general/ui_buttons.png", Size = {300, 144}, Frame = 20,
    TextNormalColor = "white",
  },
  Checked = {
    File = "general/ui_buttons.png", Size = {300, 144}, Frame = 21,
  },
  CheckedHover = {
    TextNormalColor = "white",
  },
  CheckedSelected = {
    Border = {
      Color = {252, 252, 0}, Size = 1,
    },
  },
  CheckedClicked = {
    File = "general/ui_buttons.png", Size = {300, 144}, Frame = 22,
    TextNormalColor = "white",
  },
  CheckedDisabled = {
    File = "general/ui_buttons.png", Size = {300, 144}, Frame = 18,
  },
})

DefineCheckboxStyle("square", {
  Size = {19, 19},
  Font = "game",
  TextNormalColor = "yellow",
  TextReverseColor = "white",
  TextAlign = "Left",
  TextPos = {24, 4},
  Default = {
    File = "general/ui_buttons.png", Size = {300, 144}, Frame = 24,
  },
  Hover = {
    TextNormalColor = "white",
  },
  Selected = {
    Border = {
      Color = {252, 252, 0}, Size = 1,
    },
  },
  Clicked = {
    File = "general/ui_buttons.png", Size = {300, 144}, Frame = 25,
    TextNormalColor = "white",
  },
  Disabled = {
    File = "general/ui_buttons.png", Size = {300, 144}, Frame = 23,
  },
  Checked = {
    File = "general/ui_buttons.png", Size = {300, 144}, Frame = 26,
  },
  CheckedHover = {
    TextNormalColor = "white",
  },
  CheckedSelected = {
    Border = {
      Color = {252, 252, 0}, Size = 1,
    },
  },
  CheckedClicked = {
    File = "general/ui_buttons.png", Size = {300, 144}, Frame = 27,
    TextNormalColor = "white",
  },
  CheckedDisabled = {
    File = "general/ui_buttons.png", Size = {300, 144}, Frame = 23,
  },
})


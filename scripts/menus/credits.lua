--      (c) Copyright 2010      by Pali Rohár

function RunShowCreditsMenu()
  local menu = StratagusMenu(nil --[[,"ui/title_screen.png"]])
  local offx = (Video.Width - 640) / 2
  local offy = (Video.Height - 480) / 2

  local credits = {
	"Programmers",
	"  Jimmy Salmon",
	"  Francois Beerten",
	"  Nehal Mistry",
	"  Joris Dauphin",
	"  Russell Smith",
	"  Pali Rohar",
	"Patches",
	"  Martin Renold",
	"  Carlos Perello Marin",
	"  Pludov",
	"Past Programmers",
	"  Andreas 'Ari' Arens",
	"  Lutz 'Johns' Sammer",
	"  Edgar 'Froese' Toernig",
	"  Crestez Leonard",
	"  Mark Pazolli",
	"  Valery Shchedrin",
	"  Iftikhar Rathore",
	"  Charles K Hardin",
	"  Fabrice Rossi",
	"  DigiCat",
	"  Josh Cogliati",
	"  Patrick Mullen",
	"  Vladi Belperchinov-Shabanski",
	"  Cris Daniluk",
	"  Patrice Fortier",
	"  FT Rathore",
	"  Trent Piepho",
	"  Jon Gabrielson",
	"  Lukas Hejtmanek",
	"  Steinar Hamre",
	"  Ian Farmer",
	"  Sebastian Drews",
	"  Jarek Sobieszek",
	"  Anthony Towns",
	"  Stefan Dirsch",
	"  Al Koskelin",
	"  George J. Carrette",
	"  Dirk 'Guardian' Richartz",
	"  Michael O'Reilly",
	"  Dan Hensley",
	"  Sean McMillian",
	"  Mike Earl",
	"  Ian Turner",
	"  David Slimp",
	"  Iuri Fiedoruk",
	"  Luke Mauldin",
	"  Nathan Adams",
	"  Stephan Rasenberger",
	"  Dave Reed",
	"  Josef Spillner",
	"  James Dessart",
	"  Jan Uerpmann",
	"  Aaron Berger",
	"  Latimerius",
	"  Antonis Chaniotis",
	"  Samuel Hays",
	"  David Martinez Moreno",
	"  Flavio Silvestrow",
	"  Daniel Burrows",
	"  Dave Turner",
	"  Ben Hines",
	"  Kachalov Anton",
	"Past Patch Contributors",
	"  Martin Hajduch",
	"  Jeff Binder",
	"  Ludovic",
	"  Juan Pablo",
	"  Phil Hannent",
	"  Alexander MacLean",
	"",
	"Code used:",
	"  SDL Copyright by Sam Lantinga",
	"  ZLIB Copyright by Jean-loup Gailly and Mark Adler",
	"  BZ2LIB Copyright by Julian Seward",
	"  PNG Copyright by Glenn Randers-Pehrson",
	"  OGG/Vorbis Copyright by Xiph.org Foundation",
	"  VP3 codec Copyright by On2 Technologies Inc.",
	"  Guichan Copyright by Per Larsson and Olof Naessen",
	"  tolua++ Copyright by Codenix",
	"",
	"",
	"The Stratagus Team thanks all the people who have contributed",
	"patches, bug reports, and ideas.",
	"",
--	war1gus.Name .. " Homepage: ", war1gus.Homepage,
	"Stratagus Homepage: ", GetStratagusHomepage(),
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
  }

  local sw = ScrollingWidget(320, 275)
  menu:add(sw, offx + 140, offy + 80)
  sw:setBackgroundColor(Color(0,0,0,0))
  sw:setActionCallback(function() sw:restart() end)
  for i,f in ipairs(credits) do
    sw:add(Label(f), 0, 24 * (i - 1) + 275)
  end

  menu:addHalfButton("~!Continue", "c", offx + 455, offy + 440, function() menu:stop() end)

  local speed = GetGameSpeed()
  SetGameSpeed(30)

  menu:run()

  SetGameSpeed(speed)
end


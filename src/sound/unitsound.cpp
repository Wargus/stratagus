//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name unitsound.c	-	The unit sounds. */
/*
**	(c) Copyright 1999,2000 by Fabrice Rossi
**
**	$Id$
*/

//@{

/*----------------------------------------------------------------------------
--	Include
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "clone.h"

#ifdef WITH_SOUND	// {

#include "video.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "sound_server.h"
#include "tileset.h"
#include "map.h"

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

#define MaxSimpleGroups 7
//FIXME: should I use separate types?

/// sound group definition
typedef struct _sound_group_ {
    char* Name;                     /// name of the group
    char* Sounds[MaxSimpleGroups];  /// list of sound files
} SoundGroup;

/// simple sound definition
typedef struct _simple_sound_ {
    char *Name; /// name of the sound
    char *File; /// corresponding sound file
} SimpleSound;
 
/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/// table of acknowledgment sounds
SoundGroup Acknowledgments[]={
{	"basic human voices acknowledge",
    {	"basic human voices acknowledgement 1.wav",
	"basic human voices acknowledgement 2.wav",
	"basic human voices acknowledgement 3.wav",
	"basic human voices acknowledgement 4.wav"	},
},

{	"basic orc voices acknowledge",
    {	"basic orc voices acknowledgement 1.wav",
	"basic orc voices acknowledgement 2.wav",
	"basic orc voices acknowledgement 3.wav",
	"basic orc voices acknowledgement 4.wav"	},
},

{	"peasant-acknowledge",
    {	"peasant acknowledgement 1.wav",
	"peasant acknowledgement 2.wav",
	"peasant acknowledgement 3.wav",
	"peasant acknowledgement 4.wav"		},
},

{	"knight-acknowledge",
    {	"knight acknowledgement 1.wav",
	"knight acknowledgement 2.wav",
	"knight acknowledgement 3.wav",
	"knight acknowledgement 4.wav"		},
},

{	"ogre-acknowledge",
    {	"ogre acknowledgement 1.wav",
	"ogre acknowledgement 2.wav",
	"ogre acknowledgement 3.wav"		},
},

{	"elven archer-ranger acknowledge",
    {	"elven archer-ranger acknowledgement 1.wav",
	"elven archer-ranger acknowledgement 2.wav",
	"elven archer-ranger acknowledgement 3.wav",
	"elven archer-ranger acknowledgement 4.wav"	},
},

{	"troll axethrower-berserker acknowledge",
    {	"troll axethrower-berserker acknowledgement 1.wav",
	"troll axethrower-berserker acknowledgement 2.wav",
	"troll axethrower-berserker acknowledgement 3.wav"	},
},

{	"mage-acknowledge",
    {	"mage acknowledgement 1.wav",
	"mage acknowledgement 2.wav",
	"mage acknowledgement 3.wav"		},
},

{	"death-knight-acknowledge",
    {	"death knight acknowledgement 1.wav",
	"death knight acknowledgement 2.wav",
	"death knight acknowledgement 2.wav"	},
},

{	"paladin-acknowledge",
    {	"paladin acknowledgement 1.wav",
	"paladin acknowledgement 2.wav",
	"paladin acknowledgement 3.wav",
	"paladin acknowledgement 4.wav"		},
},

{	"ogre-mage-acknowledge",
    {	"ogre-mage acknowledgement 1.wav",
	"ogre-mage acknowledgement 2.wav",
	"ogre-mage acknowledgement 3.wav"		},
},

{	"dwarves-acknowledge",
    {	"dwarven demolition squad acknowledgement 1.wav",
	"dwarven demolition squad acknowledgement 2.wav",
	"dwarven demolition squad acknowledgement 3.wav",
	"dwarven demolition squad acknowledgement 4.wav",
	"dwarven demolition squad acknowledgement 5.wav"	},
},

{	"goblin-sappers-acknowledge",
    {	"goblin sappers acknowledgement 1.wav",
	"goblin sappers acknowledgement 2.wav",
	"goblin sappers acknowledgement 3.wav",
	"goblin sappers acknowledgement 4.wav"	},
},


{	"alleria-acknowledge",
    {	"alleria acknowledgement 1.wav",
	"alleria acknowledgement 2.wav",
	"alleria acknowledgement 3.wav"		},
},

{	"teron-gorefiend-acknowledge",
    {	"teron gorefiend acknowledgement 1.wav",
	"teron gorefiend acknowledgement 2.wav",
	"teron gorefiend acknowledgement 3.wav"	},
},

{	"kurdran-and-sky'ree-acknowledge",
    {	"kurdran acknowledgement 1.wav",
	"kurdran acknowledgement 2.wav",
	"kurdran acknowledgement 3.wav"		},
},

{	"dentarg-acknowledge",
    {	"dentarg acknowledgement 1.wav",
	"dentarg acknowledgement 2.wav",
	"dentarg acknowledgement 3.wav"		},
},

{	"khadgar-acknowledge",
    {	"khadgar acknowledgement 1.wav",
	"khadgar acknowledgement 2.wav",
	"khadgar acknowledgement 3.wav"		},
},

{	"grom-hellscream-acknowledge",
    {	"grom hellscream acknowledgement 1.wav",
	"grom hellscream acknowledgement 2.wav",
	"grom hellscream acknowledgement 3.wav"	},
},

{	"tanker acknowledge",
    {	"tanker acknowledgement.wav"	},
},

{	"ships human acknowledge",
    {	"ships human acknowledgement 1.wav",
	"ships human acknowledgement 2.wav",
	"ships human acknowledgement 3.wav"		},
},

{	"ships orc acknowledge",
    {	"ships orc acknowledgement 1.wav",
	"ships orc acknowledgement 2.wav",
	"ships orc acknowledgement 3.wav"		},
},

{	"deathwing-acknowledge",
    {	"deathwing acknowledgement 1.wav",
	"deathwing acknowledgement 2.wav",
	"deathwing acknowledgement 3.wav"		},
},

{	"gnomish-flying-machine-acknowledge",
    {	"gnomish flying machine acknowledgement 1.wav"	},
},

{	"goblin-zeppelin-acknowledge",
    {	"goblin zeppelin acknowledgement 1.wav"	},
},

{	"gryphon-rider-acknowledge",
    {	"gryphon rider griffon1.wav",
	"gryphon rider griffon2.wav"	},
},

{	"dragon-acknowledge",
    {	"dragon acknowledgement 1.wav",
	"dragon acknowledgement 2.wav"		},
},

{	"turalyon-acknowledge",
    {	"turalyon acknowledgement 1.wav",
	"turalyon acknowledgement 2.wav",
	"turalyon acknowledgement 3.wav"		},
},

{	"danath-acknowledge",
    {	"danath acknowledgement 1.wav",
	"danath acknowledgement 2.wav",
	"danath acknowledgement 3.wav"		},
},

{	"korgath-bladefist-acknowledge",
    {	"korgath bladefist acknowledgement 1.wav",
	"korgath bladefist acknowledgement 2.wav",
	"korgath bladefist acknowledgement 3.wav"	},
}
};

/// table of selection sounds
SoundGroup Selections[]={
{     "basic human voices selected",
    { 	"basic human voices selected 1.wav",
	"basic human voices selected 2.wav",
	"basic human voices selected 3.wav",
	"basic human voices selected 4.wav",
	"basic human voices selected 5.wav",
	"basic human voices selected 6.wav" },
},
{     "basic orc voices selected",
    { 	"basic orc voices selected 1.wav",
	"basic orc voices selected 2.wav",
	"basic orc voices selected 3.wav",
	"basic orc voices selected 4.wav",
	"basic orc voices selected 5.wav",
	"basic orc voices selected 6.wav" },
},
{     "death knight selected",
    {   "death knight selected 1.wav",
	"death knight selected 2.wav"},
},
{
      "dwarven demolition squad selected",
    {   "dwarven demolition squad selected 1.wav",
	"dwarven demolition squad selected 2.wav" },
},
{
      "elven archer-ranger selected",
    {   "elven archer-ranger selected 1.wav",
	"elven archer-ranger selected 2.wav",
	"elven archer-ranger selected 3.wav",
        "elven archer-ranger selected 4.wav" },
},
{
      "goblin sappers selected",
    {   "goblin sappers selected 1.wav",
	"goblin sappers selected 2.wav",
        "goblin sappers selected 3.wav",
        "goblin sappers selected 4.wav"  },
},
{
      "knight selected",
    {
	"knight selected 1.wav",
	"knight selected 2.wav",
        "knight selected 3.wav",
        "knight selected 4.wav"},
},
{
      "paladin selected",
    {   "paladin selected 1.wav",
        "paladin selected 2.wav",
        "paladin selected 3.wav",
        "paladin selected 4.wav" },
},
{
      "ogre selected",
    {   "ogre selected 1.wav",
        "ogre selected 2.wav",
        "ogre selected 3.wav",
	"ogre selected 4.wav" },
},
{     "ogre-mage selected",
    {   "ogre-mage selected 1.wav",
        "ogre-mage selected 2.wav",
        "ogre-mage selected 3.wav",
        "ogre-mage selected 4.wav"},
},
{
      "ships human selected",
    {   "ships human selected 1.wav",
	"ships human selected 2.wav",
        "ships human selected 3.wav"},
},
{
      "ships orc selected",
    {   "ships orc selected 1.wav",
	"ships orc selected 2.wav",
        "ships orc selected 3.wav"},
},
{
      "troll axethrower-berserker selected",
    {   "troll axethrower-berserker selected 1.wav",
        "troll axethrower-berserker selected 2.wav",
	"troll axethrower-berserker selected 3.wav"  },
},
{
      "mage selected",
    {   "mage selected 1.wav",
        "mage selected 2.wav",
        "mage selected 3.wav" },
},
{
      "peasant selected",
    {   "peasant selected 1.wav",
        "peasant selected 2.wav",
        "peasant selected 3.wav",
	"peasant selected 4.wav" },
},
{
      "alleria selected",
    {   "alleria selected 1.wav",
        "alleria selected 2.wav",
        "alleria selected 3.wav" },
},
{
      "danath selected",
    {   "danath selected 1.wav",
        "danath selected 2.wav",
        "danath selected 3.wav" },
},
{
      "khadgar selected",
    {   "khadgar selected 1.wav",
        "khadgar selected 2.wav",
        "khadgar selected 3.wav" },
}, 
{
      "kurdran selected",
    {   "kurdran selected 1.wav",
        "kurdran selected 2.wav",
        "kurdran selected 3.wav" },
},
{
      "turalyon selected",
    {   "turalyon selected 1.wav",
        "turalyon selected 2.wav",
        "turalyon selected 3.wav" },
},
{
      "deathwing selected",
    {   "deathwing selected 1.wav",
        "deathwing selected 2.wav",
        "deathwing selected 3.wav" },
},
{
      "dentarg selected",
    {   "dentarg selected 1.wav",
        "dentarg selected 2.wav",
        "dentarg selected 3.wav" },
},
{
      "grom hellscream selected",
    {   "grom hellscream selected 1.wav",
        "grom hellscream selected 2.wav",
        "grom hellscream selected 3.wav" },
},
{
      "korgath bladefist selected",
    {   "korgath bladefist selected 1.wav",
        "korgath bladefist selected 2.wav",
        "korgath bladefist selected 3.wav" },
},
{
      "teron gorefiend selected",
    {   "teron gorefiend selected 1.wav",
        "teron gorefiend selected 2.wav",
        "teron gorefiend selected 3.wav" },
}
};

/// table of annoyed sounds
SoundGroup Annoyed[]={
    {
	"basic human voices annoyed",
	{
	    "basic human voices annoyed 1.wav",
	    "basic human voices annoyed 2.wav", 
	    "basic human voices annoyed 3.wav", 
	    "basic human voices annoyed 4.wav", 
	    "basic human voices annoyed 5.wav", 
	    "basic human voices annoyed 6.wav", 
	    "basic human voices annoyed 7.wav"
	}
    },
    {
	"basic orc voices annoyed",
	{
	    "basic orc voices annoyed 1.wav",
	    "basic orc voices annoyed 2.wav",
	    "basic orc voices annoyed 3.wav",
	    "basic orc voices annoyed 4.wav",
	    "basic orc voices annoyed 5.wav",
	    "basic orc voices annoyed 6.wav",
	    "basic orc voices annoyed 7.wav"
	}
    },
    {
	"death knight annoyed",
	{
	    "death knight annoyed 1.wav",
	    "death knight annoyed 2.wav",
	    "death knight annoyed 3.wav",
	}
    },
    {
	"dwarven demolition squad annoyed",
	{
	    "dwarven demolition squad annoyed 1.wav",
	    "dwarven demolition squad annoyed 2.wav",
	    "dwarven demolition squad annoyed 3.wav",
	}
    },
    {
	"elven archer-ranger annoyed",
	{
	    "elven archer-ranger annoyed 1.wav",
	    "elven archer-ranger annoyed 2.wav",
	    "elven archer-ranger annoyed 3.wav",
	}
    },
    {
	"gnomish flying machine annoyed",
	{
	    "gnomish flying machine annoyed 1.wav",
	    "gnomish flying machine annoyed 2.wav",
	    "gnomish flying machine annoyed 3.wav",
	    "gnomish flying machine annoyed 4.wav",
	    "gnomish flying machine annoyed 5.wav",
	}
    },
    {
	"goblin sappers annoyed",
	{
	    "goblin sappers annoyed 1.wav",
	    "goblin sappers annoyed 2.wav",
	    "goblin sappers annoyed 3.wav",
	}
    },
    {
	"goblin zeppelin annoyed",
	{
	    "goblin zeppelin annoyed 1.wav",
	    "goblin zeppelin annoyed 2.wav",
	}
    },
    {
	"knight annoyed",
	{
	    "knight annoyed 1.wav",
	    "knight annoyed 2.wav",
	    "knight annoyed 3.wav",
	}
    },
    {
	"paladin annoyed",
	{
	    "paladin annoyed 1.wav",
	    "paladin annoyed 2.wav",
	    "paladin annoyed 3.wav",
	}
    },
    {
	"ogre annoyed",
	{
	    "ogre annoyed 1.wav",
	    "ogre annoyed 2.wav",
	    "ogre annoyed 3.wav",
	    "ogre annoyed 4.wav",
	    "ogre annoyed 5.wav",
	}
    },
    {
	"ogre-mage annoyed",
	{
	    "ogre-mage annoyed 1.wav",
	    "ogre-mage annoyed 2.wav",
	    "ogre-mage annoyed 3.wav",
	}
    },
    {
	"ships human annoyed",
	{
	    "ships human annoyed 1.wav",
	    "ships human annoyed 2.wav",
	    "ships human annoyed 3.wav",
	}
    },
    {
	"ships orc annoyed",
	{
	    "ships orc annoyed 1.wav",
	    "ships orc annoyed 2.wav",
	    "ships orc annoyed 3.wav",
	}
    },
    {
	"ships submarine annoyed",
	{
	    "ships submarine annoyed 1.wav",
	    "ships submarine annoyed 2.wav",
	    "ships submarine annoyed 3.wav",
	    "ships submarine annoyed 4.wav",
	}
    },
    {
	"troll axethrower-berserker annoyed",
	{
	    "troll axethrower-berserker annoyed 1.wav",
	    "troll axethrower-berserker annoyed 2.wav",
	    "troll axethrower-berserker annoyed 3.wav",
	}
    },
    {
	"mage annoyed",
	{
	    "mage annoyed 1.wav",
	    "mage annoyed 2.wav",
	    "mage annoyed 3.wav",
	}
    },
    {
	"peasant annoyed",
	{
	    "peasant annoyed 1.wav",
	    "peasant annoyed 2.wav",
	    "peasant annoyed 3.wav",
	    "peasant annoyed 4.wav",
	    "peasant annoyed 5.wav",
	    "peasant annoyed 6.wav",
	    "peasant annoyed 7.wav"
	}
    },
    {
	"alleria annoyed",
	{
	    "alleria annoyed 1.wav",
	    "alleria annoyed 2.wav",
	    "alleria annoyed 3.wav",
	}
    },
    {
	"danath annoyed",
	{
	    "danath annoyed 1.wav",
	    "danath annoyed 2.wav",
	    "danath annoyed 3.wav",
	}
    },
    {
	"khadgar annoyed",
	{
	    "khadgar annoyed 1.wav",
	    "khadgar annoyed 2.wav",
	    "khadgar annoyed 3.wav",
	}
    },
    {
	"kurdran annoyed",
	{
	    "kurdran annoyed 1.wav",
	    "kurdran annoyed 2.wav",
	    "kurdran annoyed 3.wav",
	}
    },
    {
	"turalyon annoyed",
	{
	    "turalyon annoyed 1.wav",
	    "turalyon annoyed 2.wav",
	    "turalyon annoyed 3.wav",
	}
    },
    {
	"deathwing annoyed",
	{
	    "deathwing annoyed 1.wav",
	    "deathwing annoyed 2.wav",
	    "deathwing annoyed 3.wav",
	}
    },
    {
	"dentarg annoyed",
	{
	    "dentarg annoyed 1.wav",
	    "dentarg annoyed 2.wav",
	    "dentarg annoyed 3.wav",
	}
    },
    {
	"grom hellscream annoyed",
	{
	    "grom hellscream annoyed 1.wav",
	    "grom hellscream annoyed 2.wav",
	    "grom hellscream annoyed 3.wav",
	}
    },
    {
	"korgath bladefist annoyed",
	{
	    "korgath bladefist annoyed 1.wav",
	    "korgath bladefist annoyed 2.wav",
	    "korgath bladefist annoyed 3.wav",
	}
    },
    {
	"teron gorefiend annoyed",
	{
	    "teron gorefiend annoyed 1.wav",
	    "teron gorefiend annoyed 2.wav",
	    "teron gorefiend annoyed 3.wav",
	}
    }
};

/// selection structure
typedef struct _selection_group_ {
    char* Name;   /// name of the selection sound
    char* First;  /// name of the sound
    char* Second; /// name of the annoyed sound
} SelectionGroup;


/// table of selection groups
SelectionGroup SelectionGroups[]={
    {
	"footman-selected",
	"basic human voices selected",
	"basic human voices annoyed"
    },
    {
	"grunt-selected",
	"basic orc voices selected",
	"basic orc voices annoyed"
    },
    {
	"peasant-selected",
	"peasant selected",
	"peasant annoyed"
    },
    {
	"knight-selected",
	"knight selected",
	"knight annoyed"
    },
    {
	"ogre-selected",
	"ogre selected",
	"ogre annoyed"
    },
    {
	"archer-selected",
	"elven archer-ranger selected",
	"elven archer-ranger annoyed"
    },
    {
	"axethrower-selected",
	"troll axethrower-berserker selected",
	"troll axethrower-berserker annoyed"
    },
    {
	"mage-selected",
	"mage selected",
	"mage annoyed"
    },
    {
	"death-knight-selected",
	"death knight selected",
	"death knight annoyed"
    },
    {
	"paladin-selected",
	"paladin selected",
	"paladin annoyed"
    },
    {
	"ogre-mage-selected",
	"ogre-mage selected",
	"ogre-mage annoyed"
    },
    {
	"dwarves-selected",
	"dwarven demolition squad selected",
	"dwarven demolition squad annoyed"
    },
    {
	"goblin-sappers-selected",
	"goblin sappers selected",
	"goblin sappers annoyed"
    },
    {
	"alleria-selected",
	"alleria selected",
	"alleria annoyed"
    },
    {
	"teron-gorefiend-selected",
	"teron gorefiend selected",
	"teron gorefiend annoyed"
    },
    {
	"kurdan-and-sky'ree-selected",
	"kurdran selected",
	"kurdran annoyed"
    },
    {
	"dentarg-selected",
	"dentarg selected",
	"dentarg annoyed"
    },
    {
	"khadgar-selected",
	"khadgar selected",
	"khadgar annoyed"
    },
    {
	"grom-hellscream-selected",
	"grom hellscream selected",
	"grom hellscream annoyed"
    },
    {
	"human-oil-tanker-selected",
	"ships human selected",
	"ships human annoyed"
    },
    {
	"orc-oil-tanker-selected",
	"ships orc selected",
	"ships orc annoyed"
    },
    {
	"deathwing-selected",
	"deathwing selected",
	"deathwing annoyed"
    },
    {
	"gnomish-submarine-selected",
	"ships human selected",
	"ships submarine annoyed"
    },
    {
	"gnomish-flying-machine-selected",
	"click",
	"gnomish flying machine annoyed"
    },
    {
	"goblin-zeppelin-selected",
	"click",
	"goblin zeppelin annoyed"
    },
    {
	"turalyon-selected",
	"turalyon selected",
	"turalyon annoyed"
    },
    {
	"danath-selected",
	"danath selected",
	"danath annoyed"
    },
    {
	"korgath-bladefist-selected",
	"korgath bladefist selected",
	"korgath bladefist annoyed"
    }
    //    {
    //	"-selected",
    //	" selected",
    //	" annoyed"
    //    },
};

/// table of unclassified sound groups
SoundGroup OtherGroups[]={
    {	
	"building destroyed",
	{	
	    "explosion 1.wav",
	    "explosion 2.wav",
	    "explosion 3.wav"	
	},
    },
    {
	"sword attack",
	{
	    "sword attack 1.wav",
	    "sword attack 2.wav", 
	    "sword attack 3.wav"
	}
    },
    {
	"tree chopping",
	{
	    "tree chopping 1.wav",
	    "tree chopping 2.wav",
	    "tree chopping 3.wav",
	    "tree chopping 4.wav"
	}
    }
};

/// table of simple sounds
SimpleSound SimpleSounds[]={
    { "placement error",
	"placement error.wav" },
    { "placement sucess",
	"placement sucess.wav" },
    { "building construction",
	"building construction.wav" },
    { "explosion",
	"explosion.wav" },
    { "basic human voices work complete",
	"basic human voices work complete.wav" },
    { "basic orc voices work complete",
	"basic orc voices work complete.wav" },
    { "peasant work complete",
	"peasant work complete.wav" },
    { "basic human voices ready",
	"basic human voices ready.wav" },
    { "basic orc voices ready",
	"basic orc voices ready.wav" },
    { "basic human voices help 1",
	"basic human voices help 1.wav" },
    { "basic orc voices help 1",
	"basic orc voices help 1.wav" },
    { "basic human voices help 2",
	"basic human voices help 2.wav" },
    { "basic orc voices help 2",
	"basic orc voices help 2.wav" },
    { "basic human voices dead",
	"basic human voices dead.wav" },
    { "basic orc voices dead",
	"basic orc voices dead.wav" },
    { "ship sinking",
	"ship sinking.wav" },
    { "catapult-ballista movement",
	"catapult-ballista movement.wav" },
    // building selection sounds
    { "blacksmith",
	"blacksmith.wav" },
    { "church-selected",
	"church.wav" },
    { "altar-of-storms-selected",
	"altar of storms.wav" },
    { "stables-selected",
	"stables.wav" },
    { "ogre-mound-selected",
	"ogre mound.wav" },
    { "farm-selected",
	"farm.wav" },
    { "pig-farm-selected",
	"pig farm.wav" },
    { "gold-mine-selected",
	"gold mine.wav" },
    { "shipyard",
	"shipyard.wav" },
    { "oil platform",
	"oil platform.wav" },
    { "oil refinery",
	"oil refinery.wav" },
    { "lumbermill",
	"lumbermill.wav" },
    { "transport docking",
	"transport docking.wav" },
    { "burning",
	"burning.wav" },
    { "gryphon-aviary-selected",
	"gryphon aviary.wav" },
    { "dragon-roost-selected",
	"dragon roost.wav" },
    { "foundry",
	"foundry.wav" },
    { "gnomish-inventor-selected",
	"gnomish inventor.wav" },
    { "goblin-alchemist-selected",
	"goblin alchemist.wav" },
    { "mage-tower-selected",
	"mage tower.wav" },
    { "temple-of-the-damned-selected",
	"temple of the damned.wav" },
    { "capture (human)",
	"capture (human).wav" },
    { "capture (orc)",
	"capture (orc).wav" },
    { "rescue (human)",
	"rescue (human).wav" },
    { "rescue (orc)",
	"rescue (orc).wav" },
    { "bloodlust",
	"bloodlust.wav" },
    { "death and decay",
	"death and decay.wav" },
    { "death coil",
	"death coil.wav" },
    { "exorcism",
	"exorcism.wav" },
    { "flame shield",
	"flame shield.wav" },
    { "haste",
	"haste.wav" },
    { "healing",
	"healing.wav" },
    { "holy vision",
	"holy vision.wav" },
    { "blizzard",
	"blizzard.wav" },
    { "invisibility",
	"invisibility.wav" },
    { "eye of kilrogg",
	"eye of kilrogg.wav" },
    { "polymorph",
	"polymorph.wav" },
    { "slow",
	"slow.wav" },
    { "lightning",
	"lightning.wav" },
    { "touch of darkness",
	"touch of darkness.wav" },
    { "unholy armour",
	"unholy armour.wav" },
    { "whirlwind",
	"whirlwind.wav" },
    // ready sounds
    { "peon-ready",
	"peon ready.wav" },
    { "death-knight-ready",
	"death knight ready.wav" },
    { "dwarves-ready",
	"dwarven demolition squad ready.wav" },
    { "elven archer-ranger ready",
	"elven archer-ranger ready.wav" },
    { "gnomish-flying-machine-ready",
	"gnomish flying machine ready.wav" },
    { "goblin-sappers-ready",
	"goblin sappers ready.wav" },
    { "goblin-zeppelin-ready",
	"goblin zeppelin ready.wav" },
    { "knight-ready",
	"knight ready.wav" },
    { "paladin-ready",
	"paladin ready.wav" },
    { "ogre-ready",
	"ogre ready.wav" },
    { "ogre-mage-ready",
	"ogre-mage ready.wav" },
    { "ships human ready",
	"ships human ready.wav" },
    { "ships orc ready",
	"ships orc ready.wav" },
    { "troll axethrower-berserker ready",
	"troll axethrower-berserker ready.wav" },
    { "mage-ready",
	"mage ready.wav" },
    { "peasant-ready",
	"peasant ready.wav" },
    { "dragon-ready",
	"dragon ready 2.wav" },
    { "click",
	"click.wav" },
    // selection sounds
    { "dragon-selected",
	"dragon selected.wav" },
    { "gryphon-rider-selected",
	"gryphon rider selected.wav" },
    { "sheep selected",
	"sheep selected.wav" },
    { "seal selected",
	"seal selected.wav" },
    { "pig selected",
	"pig selected.wav" },
    { "warthog selected",
	"warthog selected.wav" },
    // annoyed sounds
    { "sheep annoyed",
	"sheep annoyed.wav"},
    { "seal annoyed",
	"seal annoyed.wav"},
    { "pig annoyed",
	"pig annoyed.wav"},
    { "warthog annoyed",
	"warthog annoyed.wav"},
    // attack sounds
    { "catapult-ballista attack",
	"catapult-ballista attack.wav" },
    { "punch",
	"punch.wav" },
    { "fireball throw",
	"fireball throw.wav" },
    { "bow throw",
	"bow throw.wav" },
    { "axe throw",
	"axe throw.wav" },
    { "fist",
	"fist.wav" },
    { "lightning",
	"lightning.wav" },
    { "touch of darkness",
	"touch of darkness.wav" }
};

/// structure for remaping a sound to a new name
typedef struct _remap_ {
    char* NewName;			/// Name in unit type definition
    char* BaseName;			/// Name used in sound definition
} Remap;

/// table of sound remapings
Remap Remaps[]={
    //acknowledge sounds
    { "footman-acknowledge",
      "basic human voices acknowledge"},
    { "grunt-acknowledge",
      "basic orc voices acknowledge"},
    { "peon-acknowledge",
      "basic orc voices acknowledge"},
    { "ballista-acknowledge",
      "catapult-ballista movement"},
    { "catapult-acknowledge",
      "catapult-ballista movement"},
    { "archer-acknowledge",
      "elven archer-ranger acknowledge"},
    { "axethrower-acknowledge",
      "troll axethrower-berserker acknowledge"},
    { "ranger-acknowledge",
      "elven archer-ranger acknowledge"},
    { "berserker-acknowledge",
      "troll axethrower-berserker acknowledge"},
    { "human-oil-tanker-acknowledge",
      "tanker acknowledge"},
    { "orc-oil-tanker-acknowledge",
      "tanker acknowledge"},
    { "human-transport-acknowledge",
      "ships human acknowledge"},
    { "orc-transport-acknowledge",
      "ships orc acknowledge"},
    { "elven-destroyer-acknowledge",
      "ships human acknowledge"},
    { "troll-destroyer-acknowledge",
      "ships orc acknowledge"},
    { "battleship-acknowledge",
      "ships human acknowledge"},
    { "ogre-juggernaught-acknowledge",
      "ships orc acknowledge"},
    { "gnomish-submarine-acknowledge",
      "ships human acknowledge"},
    { "giant-turtle-acknowledge",
      "ships orc acknowledge"},
    { "cho'gall-acknowledge",
      "ogre-mage-acknowledge"},
    { "lothar-acknowledge",
      "knight-acknowledge"},
    { "gul'dan-acknowledge",
      "death-knight-acknowledge"},
    { "uther-lightbringer-acknowledge",
      "paladin-acknowledge"},
    { "zuljin-acknowledge",
      "troll axethrower-berserker acknowledge"},
    { "peon-with-gold-acknowledge",
      "basic orc voices acknowledge"},
    { "peasant-with-gold-acknowledge",
      "peasant-acknowledge"},
    { "peon-with-wood-acknowledge",
      "basic orc voices acknowledge"},
    { "peasant-with-wood-acknowledge",
      "peasant-acknowledge"},
    { "human-oil-tanker-full-acknowledge",
      "tanker acknowledge"},
    { "orc-oil-tanker-full-acknowledge",
      "tanker acknowledge"},
    // ready sounds
    { "footman-ready",
      "basic human voices ready"},
    { "grunt-ready",
      "basic orc voices ready"},
    { "ballista-ready",
      "basic human voices ready"},
    { "catapult-ready",
      "basic orc voices ready"},
    { "archer-ready",
      "elven archer-ranger ready"},
    { "axethrower-ready",
      "troll axethrower-berserker ready"},
    { "ranger-ready",
      "elven archer-ranger ready"},
    { "berserker-ready",
      "troll axethrower-berserker ready"},
    { "human-oil-tanker-ready",
      "ships human ready"},
    { "orc-oil-tanker-ready",
      "ships orc ready"},
    { "human-transport-ready",
      "ships human ready"},
    { "orc-transport-ready",
      "ships orc ready"},
    { "elven-destroyer-ready",
      "ships human ready"},
    { "troll-destroyer-ready",
      "ships orc ready"},
    { "battleship-ready",
      "ships human ready"},
    { "ogre-juggernaught-ready",
      "ships orc ready"},
    { "gnomish-submarine-ready",
      "ships human ready"},
    { "giant-turtle-ready",
      "ships orc ready"},
    // selection sounds
    { "peon-selected",
      "grunt-selected"},
    { "ballista-selected",
      "click"},
    { "catapult-selected",
      "click"},
    { "ranger-selected",
      "elven archer-ranger selected"},
    { "berserker-selected",
      "troll axethrower-berserker selected"},
    { "human-transport-selected",
      "human-oil-tanker-selected"},
    { "orc-transport-selected",
      "orc-oil-tanker-selected"},
    { "elven-destroyer-selected",
      "human-oil-tanker-selected"},
    { "troll-destroyer-selected",
      "orc-oil-tanker-selected"},
    { "battleship-selected",
      "human-oil-tanker-selected"},
    { "ogre-juggernaught-selected",
      "orc-oil-tanker-selected"},
    { "giant-turtle-selected",
      "orc-oil-tanker-selected"},
    { "eye-of-kilrogg-selected",
      "click"},
    { "cho'gall-selected",
      "mage-selected"},
    { "lothar-selected",
      "knight-selected"},
    { "gul'dan",
      "death-knight-selected"},
    { "uther-lightbringer-selected",
      "paladin-selected"},
    { "zuljin-selected",
      "troll axethrower-berserker-selected"},
    { "skeleton-selected",
      "click"},
    { "daemon-selected",
      "click"},
    { "human-barracks-selected",
      "click"},
    { "orc-barracks-selected",
      "click"},
    { "human-watch-tower-selected",
      "click"},
    { "orc-watch-tower-selected",
      "click"},
    { "human-shipyard-selected",
      "shipyard"},
    { "orc-shipyard-selected",
      "shipyard"},
    { "town-hall-selected",
      "click"},
    { "great-hall-selected",
      "click"},
    { "elven-lumber-mill-selected",
      "lumbermill"},
    { "troll-lumber-mill-selected",
      "lumbermill"},
    { "human-foundry-selected",
      "foundry"},
    { "orc-foundry-selected",
      "foundry"},
    { "human-blacksmith-selected",
      "blacksmith"},
    { "orc-blacksmith-selected",
      "blacksmith"},
    { "human-refinery-selected",
      "oil refinery"},
    { "orc-refinery-selected",
      "oil refinery"},
    { "human-oil-platform-selected",
      "oil platform"},
    { "orc-oil-platform-selected",
      "oil platform"},
    { "keep-selected",
      "click"},
    { "stronghold-selected",
      "click"},
    { "castle-selected",
      "click"},
    { "fortress-selected",
      "click"},
    { "oil-patch-selected",
      "click"},
    { "human-guard-tower-selected",
      "click"},
    { "orc-guard-tower-selected",
      "click"},
    { "human-cannon-tower-selected",
      "click"},
    { "orc-cannon-tower-selected",
      "click"},
    { "peon-with-gold-selected",
      "peon-selected"},
    { "peasant-with-gold-selected",
      "peasant-selected"},
    { "peon-with-wood-selected",
      "peon-selected"},
    { "peasant-with-wood-selected",
      "peasant-selected"},
    { "human-oil-tanker-full-selected",
      "human-oil-tanker-selected"},
    { "orc-oil-tanker-full-selected",
      "orc-oil-tanker-selected"},
    //attack sounds
    { "footman-attack",
      "sword attack"},
    { "grunt-attack",
      "sword attack"},
    { "peasant-attack",
      "fist"},
    { "peon-attack",
      "fist"},
    { "ballista-attack",
      "catapult-ballista attack"},
    { "catapult-attack",
      "catapult-ballista attack"},
    { "knight-attack",
      "sword attack"},
    { "ogre-attack",
      "punch"},
    { "archer-attack",
      "bow throw"},
    { "axethrower-attack",
      "axe throw"},
    { "mage-attack",
      "lightning" },
    { "death-knight-attack",
      "touch of darkness"},
    { "paladin-attack",
      "sword attack"},
    { "ogre-mage-attack",
      "punch"},
    { "dwarves-attack",
      "sword attack"},
    { "goblin-sappers-attack",
      "sword attack"},
    { "ranger-attack",
      "bow throw"},
    { "berserker-attack",
      "axe throw"},
    { "alleria-attack",
      "bow throw"},
    { "teron-gorefiend-attack",
      "touch of darkness"},
    { "kurdan-and-sky'ree-attack",
      "lightning"},
    { "dentarg-attack",
      "punch"},
    { "khadgar-attack",
      "lightning"},
    { "grom-hellscream-attack",
      "sword attack"},
    //FIXME: what sound for ships?
    { "elven-destroyer-attack",
      "catapult-ballista attack"},
    { "troll-destroyer-attack", 
      "catapult-ballista attack"},
    { "battleship-attack", 
      "catapult-ballista attack"},
    { "ogre-juggernaught-attack", 
      "catapult-ballista attack"},
    { "gnomish-submarine-attack",
      "catapult-ballista attack"},
    { "giant-turtle-attack",
      "catapult-ballista attack"},
    { "deathwing-attack",
      "fireball throw"},
    { "gryphon-rider-attack",
      "lightning"},
    { "dragon-attack",
      "fireball throw"},
    //FIXME: what sound for those heroes?
    //	{ "turalyon-attack",
	//},
	//{ "danath-attack",
	//},
	//{ "korgath-bladefist-attack",
	//},
	//{ "cho'gall-attack",
	//},
	//{ "lothar-attack",
	//},
	//{ "gul'dan-attack",
	//},
	//{ "uther-lightbringer-attack",
	//},
	//{ "zuljin-attack",
	//},
    { "skeleton-attack",
      "fist"},
    //	{ "daemon-attack",
    //	},
    //FIXME: what attack sounds for towers?
    //	{ "human-guard-tower-attack",
	//},
	//{ "orc-guard-tower-attack",
	//},
	//{ "human-cannon-tower-attack",
	//},
	//{ "orc-cannon-tower-attack",
	//},
    { "peon-with-gold-attack",
      "fist"},
    { "peasant-with-gold-attack",
      "fist"},
    { "peon-with-wood-attack",
      "fist"},
    { "peasant-with-wood-attack",
      "fist"}
    //    { "",
    //      ""},
};

/** Computes the number of sounds in a sound group
 * @param group list of file names
 * @return number of sounds
 */
local int NbSoundsInGroup(char* group[]) {
    int i;

    for(i=0;i<MaxSimpleGroups;i++) {
	if (group[i]==NULL) {
	    return i;
	}
    }
    return MaxSimpleGroups;
}

/** Loads all simple sounds (listed in the SimpleSounds array).
 */
local void LoadSimpleSounds(void) {
    int i;
    int nb;

    nb=sizeof(SimpleSounds)/sizeof(*SimpleSounds);
    DebugLevel3("Loading Simple Sounds (%d sounds)\n",nb);
    for(i=0;i<nb;i++) {
	DebugLevel3("Loading %s with name %s\n",SimpleSounds[i].File,
		    SimpleSounds[i].Name);
	MakeSound(SimpleSounds[i].Name,&(SimpleSounds[i].File),1);
    }
}

/** Loads all sound groups.
 ** Special groups are created.
 */
local void LoadSoundGroups(void) {
    int i;
    int nb;
    int nb_sounds;

    nb=sizeof(Acknowledgments)/sizeof(*Acknowledgments);
    DebugLevel3("Loading Acknowledgment Sound Groups (%d groups)\n",nb);
    for(i=0;i<nb;i++) {
	nb_sounds=NbSoundsInGroup(Acknowledgments[i].Sounds);
	DebugLevel3("Load group %s (%d sounds)\n",Acknowledgments[i].Name,
		    nb_sounds);
	MakeSound(Acknowledgments[i].Name,Acknowledgments[i].Sounds,
		  nb_sounds);
    }
    nb=sizeof(Selections)/sizeof(*Selections);
    DebugLevel3("Loading Selection Sound Groups (%d groups)\n",nb);
    for(i=0;i<nb;i++) {
	nb_sounds=NbSoundsInGroup(Selections[i].Sounds);
	DebugLevel3("Load group %s (%d sounds)\n",Selections[i].Name,
		    nb_sounds);
	MakeSound(Selections[i].Name,Selections[i].Sounds,nb_sounds);
    }
    nb=sizeof(Annoyed)/sizeof(*Annoyed);
    DebugLevel3("Loading Annoyed Sound Groups (%d groups)\n",nb);
    for(i=0;i<nb;i++) {
	nb_sounds=NbSoundsInGroup(Annoyed[i].Sounds);
	DebugLevel3("Load group %s (%d sounds)\n",Annoyed[i].Name,
		    nb_sounds);
	MakeSound(Annoyed[i].Name,Annoyed[i].Sounds,nb_sounds);
    }
    nb=sizeof(OtherGroups)/sizeof(*OtherGroups);
    DebugLevel3("Loading Other Sound Groups (%d groups)\n",nb);
    for(i=0;i<nb;i++) {
	nb_sounds=NbSoundsInGroup(OtherGroups[i].Sounds);
	DebugLevel3("Load group %s (%d sounds)\n",OtherGroups[i].Name,
		    nb_sounds);
	MakeSound(OtherGroups[i].Name,OtherGroups[i].Sounds,
		  nb_sounds);
    }
    nb=sizeof(SelectionGroups)/sizeof(*SelectionGroups);
    DebugLevel3("Making Special Sound Groups (%d groups)\n",nb);
    for(i=0;i<nb;i++) {
	//FIXME: might be more efficient
	DebugLevel3("Group %s (%s,%s)\n",SelectionGroups[i].Name,
		    SelectionGroups[i].First,SelectionGroups[i].Second);
	MakeSoundGroup(SelectionGroups[i].Name,
		       SoundIdForName(SelectionGroups[i].First),
		       SoundIdForName(SelectionGroups[i].Second));
    }
}

/** Performs remaping listed in the Remaps array. Maps also critter sounds to
 ** their correct values.
 */
local void RemapSounds(void) {
    int i;
    int nb;

    nb=sizeof(Remaps)/sizeof(*Remaps);
    for(i=0;i<nb;i++) {
	//FIXME: should be more efficient
	MapSound(Remaps[i].NewName,SoundIdForName(Remaps[i].BaseName));
    }
    // critter mapping
    switch( TheMap.Terrain ) {
    case TilesetSummer:
	MakeSoundGroup("critter-selected",
		       SoundIdForName("sheep selected"),
		       SoundIdForName("sheep annoyed"));
	break;
    case TilesetWinter:
	MakeSoundGroup("critter-selected",
		       SoundIdForName("seal selected"),
		       SoundIdForName("seal annoyed"));
	break;
    case TilesetWasteland:
	MakeSoundGroup("critter-selected",
		       SoundIdForName("pig selected"),
		       SoundIdForName("pig annoyed"));
	break;
    case TilesetSwamp:
	MakeSoundGroup("critter-selected",
		       SoundIdForName("warthog selected"),
		       SoundIdForName("warthog annoyed"));
	break;
    default:
	DebugLevel2("Unknown Terrain %d\n",TheMap.Terrain);
    }
    
}

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

global void LoadUnitSounds(void) {
    if( SoundFildes!=-1 ) {
	LoadSimpleSounds();
	LoadSoundGroups();
	RemapSounds();
    }
}

global void MapUnitSounds(void) {
    int i;
    int nb;
    SoundId HumanHelp,HumanCityHelp,OrcHelp,OrcCityHelp;
    SoundId HumanDead,OrcDead,BuildingDestroyed,ShipSinking;

    if( SoundFildes!=-1 ) {
	SetSoundRange(SoundIdForName("tree chopping"),32);
	nb=sizeof(UnitTypes)/sizeof(*UnitTypes);
	// help sounds
	HumanHelp=SoundIdForName("basic human voices help 1");
	HumanCityHelp=SoundIdForName("basic human voices help 2");
	OrcHelp=SoundIdForName("basic orc voices help 1");
	OrcCityHelp=SoundIdForName("basic orc voices help 2");
	// Ranges for help sounds
	SetSoundRange(HumanHelp,INFINITE_SOUND_RANGE);
	SetSoundRange(HumanCityHelp,INFINITE_SOUND_RANGE);
	SetSoundRange(OrcHelp,INFINITE_SOUND_RANGE);
	SetSoundRange(OrcCityHelp,INFINITE_SOUND_RANGE);
	// death sounds
	HumanDead=SoundIdForName("basic human voices dead");
	OrcDead=SoundIdForName("basic orc voices dead");
	BuildingDestroyed=SoundIdForName("building destroyed");
	ShipSinking=SoundIdForName("ship sinking");
	for(i=0;i<nb;i++) {
	    UnitTypes[i].Sound.Selected.Sound=
		SoundIdForName(UnitTypes[i].Sound.Selected.Name);
	    UnitTypes[i].Sound.Acknowledgement.Sound=
		SoundIdForName(UnitTypes[i].Sound.Acknowledgement.Name);
	    // Acknowledge sounds have infinite range
	    SetSoundRange(UnitTypes[i].Sound.Acknowledgement.Sound,
			  INFINITE_SOUND_RANGE);
	    UnitTypes[i].Sound.Ready.Sound=
		SoundIdForName(UnitTypes[i].Sound.Ready.Name);
	    //FIXME: will be modified
	    UnitTypes[i].Weapon.Attack.Sound=
		SoundIdForName(UnitTypes[i].Weapon.Attack.Name);
	    //FIXME: very dirty trick based on the fact that unit types
	    // alternate between human and orc, starting with human.
	    // should be at least defined as remaps
	    //FIXME: (Fabrice) I don't think it's 100% correct for death
	    //sounds.
	    if (i%2) {
		//orc
		if (UnitTypes[i].Building) {
		    UnitTypes[i].Sound.Help.Sound=OrcCityHelp;
		    UnitTypes[i].Sound.Dead.Sound=BuildingDestroyed;
		} else {
		    UnitTypes[i].Sound.Help.Sound=OrcHelp;
		    if (UnitTypes[i].SeaUnit) {
			UnitTypes[i].Sound.Dead.Sound=ShipSinking;
		    } else {
			UnitTypes[i].Sound.Dead.Sound=OrcDead;
		    }
		}
	    } else {
		//human
		if (UnitTypes[i].Building) {
		    UnitTypes[i].Sound.Help.Sound=HumanCityHelp;
		    UnitTypes[i].Sound.Dead.Sound=BuildingDestroyed;
		} else {
		    UnitTypes[i].Sound.Help.Sound=HumanHelp;
		    if (UnitTypes[i].SeaUnit) {
			UnitTypes[i].Sound.Dead.Sound=ShipSinking;
		    } else {
			UnitTypes[i].Sound.Dead.Sound=HumanDead;
		    }
		}
	    }
	}
    }
}

#endif	// } WITH_SOUND

//@}

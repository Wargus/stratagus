#include <stdio.h>

#define local

local int PudReadLong(FILE* input)
{
    unsigned long temp_long;

    if( fread(&temp_long,4,1,input)!=1 ) {
	perror("fread()");
	exit(-1);
    }

    return temp_long;
}

local int PudReadWord(FILE* input)
{
    unsigned short temp_short;

    if( fread(&temp_short,2,1,input)!=1 ) {
	perror("fread()");
	exit(-1);
    }

    return temp_short;
}

local int PudReadByte(FILE* input)
{
    unsigned char temp_char;

    if( fread(&temp_char,1,1,input)!=1 ) {
	perror("fread()");
	exit(-1);
    }

    return temp_char;
}

typedef struct _upgrade_  {
    char*	Name;			// upgrade name

    unsigned	UpgradeTime;
    unsigned	GoldCost;
    unsigned	WoodCost;
    unsigned	OilCost;
    unsigned	Icon;
    unsigned	Group;
    unsigned	Flags;
} Upgrade;

Upgrade Upgrades[52] = {
    { "sword 1" },
    { "sword 2" },
    { "axe 1" },
    { "axe 2" },
    { "arrow 1" },
    { "arrow 2" },
    { "spear 1" },
    { "spear 2" },
    { "human shield 1" },
    { "human shield 2" },
    { "orc shield 1" },
    { "orc shield 2" },
    { "human ship cannon 1" },
    { "human ship cannon 2" },
    { "orc ship cannon 1" },
    { "orc ship cannon 2" },
    { "human ship armor 1" },
    { "human ship armor 2" },
    { "orc ship armor 1" },
    { "orc ship armor 2" },
    { "catapult 1" },
    { "catapult 2" },
    { "ballista 1" },
    { "ballista 2" },
    { "train rangers" },
    { "longbow" },
    { "ranger scouting" },
    { "ranger marksmanship" },
    { "train berserkers" },
    { "lighter axes" },
    { "berserker scouting" },
    { "berserker regeneration" },
    { "train ogre-mages" },
    { "train paladins" },
    { "holy vision" },
    { "healing" },
    { "exorcism" },
    { "flame shield" },
    { "fireball" },
    { "slow" },
    { "invisibility" },
    { "polymorph" },
    { "blizzard" },
    { "eye of kilrogg" },
    { "bloodlust" },
    { "raise dead" },
    { "death coil" },
    { "whirlwind" },
    { "haste" },
    { "unholy armor" },
    { "runes" },
    { "death and decay" },
};

int main(int argc,char** argv)
{
    int i;
    int v;

    for( i=0; i<52; ++i ) {		// upgrade time
	v=PudReadByte(stdin);
	Upgrades[i].UpgradeTime=v;
    }
    for( i=0; i<52; ++i ) {		// gold
	v=PudReadWord(stdin);
	Upgrades[i].GoldCost=v;
    }
    for( i=0; i<52; ++i ) {		// wood
	v=PudReadWord(stdin);
	Upgrades[i].WoodCost=v;
    }
    for( i=0; i<52; ++i ) {		// oil
	v=PudReadWord(stdin);
	Upgrades[i].OilCost=v;
    }
    for( i=0; i<52; ++i ) {		// icon
	v=PudReadWord(stdin);
	Upgrades[i].Icon=v;
    }
    for( i=0; i<52; ++i ) {		// group
	v=PudReadWord(stdin);
	Upgrades[i].Group=v;
    }
    for( i=0; i<52; ++i ) {		// flags
	v=PudReadLong(stdin);
	Upgrades[i].Flags=v;
    }

    printf(" = {\n");
    for( i=0; i<sizeof(Upgrades)/sizeof(*Upgrades); ++i ) {
	printf(" { \"%s\"\n",Upgrades[i].Name);
	printf("    ,%4d, %4d,%4d,%4d, %3d, %2d, 0x%08X"
	    ,Upgrades[i].UpgradeTime
	    ,Upgrades[i].GoldCost
	    ,Upgrades[i].WoodCost
	    ,Upgrades[i].OilCost
	    ,Upgrades[i].Icon
	    ,Upgrades[i].Group
	    ,Upgrades[i].Flags);

	printf(" },\n");
    }
    printf("};\n");
}

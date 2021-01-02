/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

/*************************************************************************** 
*       ROT 1.4 is copyright 1996-1997 by Russ Walsh                       * 
*       By using this code, you have agreed to follow the terms of the     * 
*       ROT license, in the file doc/rot.license                           * 
***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "interp.h"


char *	const	dir_name	[]		=
{
    "north", "east", "south", "west", "up", "down",
    "north", "east", "south", "west", "up", "down"
};

const	sh_int	rev_dir		[]		=
{
    2, 3, 0, 1, 5, 4, 2, 3, 0, 1, 5, 4
};

const	sh_int	movement_loss	[SECT_MAX]	=
{
    1, 2, 2, 3, 4, 6, 4, 1, 6, 10, 6
};

/* item type list */
const struct item_type		item_table	[]	=
{
    {	ITEM_LIGHT,	"light"		},
    {	ITEM_SCROLL,	"scroll"	},
    {	ITEM_WAND,	"wand"		},
    {   ITEM_STAFF,	"staff"		},
    {   ITEM_WEAPON,	"weapon"	},
    {   ITEM_TREASURE,	"treasure"	},
    {   ITEM_ARMOR,	"armor"		},
    {	ITEM_POTION,	"potion"	},
    {	ITEM_CLOTHING,	"clothing"	},
    {   ITEM_FURNITURE,	"furniture"	},
    {	ITEM_TRASH,	"trash"		},
    {	ITEM_CONTAINER,	"container"	},
    {	ITEM_DRINK_CON, "drink"		},
    {	ITEM_KEY,	"key"		},
    {	ITEM_FOOD,	"food"		},
    {	ITEM_MONEY,	"money"		},
    {	ITEM_BOAT,	"boat"		},
    {	ITEM_CORPSE_NPC,"npc_corpse"	},
    {	ITEM_CORPSE_PC,	"pc_corpse"	},
    {   ITEM_FOUNTAIN,	"fountain"	},
    {	ITEM_PILL,	"pill"		},
    {	ITEM_PROTECT,	"protect"	},
    {	ITEM_MAP,	"map"		},
    {	ITEM_PORTAL,	"portal"	},
    {	ITEM_WARP_STONE,"warp_stone"	},
    {	ITEM_ROOM_KEY,	"room_key"	},
    {	ITEM_GEM,	"gem"		},
    {	ITEM_JEWELRY,	"jewelry"	},
    {   ITEM_JUKEBOX,	"jukebox"	},
    {	ITEM_DEMON_STONE,"demon_stone"	},
    {	ITEM_EXIT,	"exit"		},
    {	ITEM_PIT,	"pit"		},
    {	ITEM_COMPONENT,	"component"	},
    {   0,		NULL		}
};


/* weapon selection table */
const	struct	weapon_type	weapon_table	[]	=
{
   { "sword",	OBJ_VNUM_SCHOOL_SWORD,	WEAPON_SWORD,	&gsn_sword	},
   { "mace",	OBJ_VNUM_SCHOOL_MACE,	WEAPON_MACE,	&gsn_mace 	},
   { "dagger",	OBJ_VNUM_SCHOOL_DAGGER,	WEAPON_DAGGER,	&gsn_dagger	},
   { "axe",	OBJ_VNUM_SCHOOL_AXE,	WEAPON_AXE,	&gsn_axe	},
   { "staff",	OBJ_VNUM_SCHOOL_STAFF,	WEAPON_SPEAR,	&gsn_spear	},
   { "flail",	OBJ_VNUM_SCHOOL_FLAIL,	WEAPON_FLAIL,	&gsn_flail	},
   { "whip",	OBJ_VNUM_SCHOOL_WHIP,	WEAPON_WHIP,	&gsn_whip	},
   { "polearm",	OBJ_VNUM_SCHOOL_POLEARM,WEAPON_POLEARM,	&gsn_polearm	},
   { NULL,	0,				0,	NULL		}
};


 
/* wiznet table and prototype for future flag setting */
const   struct wiznet_type      wiznet_table    []              =
{
   {    "on",           WIZ_ON,         0  },
   {    "prefix",	WIZ_PREFIX,	0  },
   {    "ticks",        WIZ_TICKS,      IM },
   {    "logins",       WIZ_LOGINS,     IM },
   {    "sites",        WIZ_SITES,      IM },
   {    "links",        WIZ_LINKS,      L7 },
   {	"newbies",	WIZ_NEWBIE,	0  },
   {	"spam",		WIZ_SPAM,	L5 },
   {    "deaths",       WIZ_DEATHS,     IM },
   {    "resets",       WIZ_RESETS,     L4 },
   {    "mobdeaths",    WIZ_MOBDEATHS,  L4 },
   {    "flags",	WIZ_FLAGS,	L5 },
   {	"penalties",	WIZ_PENALTIES,	L5 },
   {	"saccing",	WIZ_SACCING,	L5 },
   {	"levels",	WIZ_LEVELS,	0  },
   {	"load",		WIZ_LOAD,	L2 },
   {	"restore",	WIZ_RESTORE,	L2 },
   {	"snoops",	WIZ_SNOOPS,	L2 },
   {	"switches",	WIZ_SWITCHES,	L2 },
   {	"secure",	WIZ_SECURE,	L1 },
   {	NULL,		0,		0  }
};

/* attack table  -- not very organized :(
 * Be sure to keep MAX_DAMAGE_MESSAGE up
 * to date in merc.h
 */
const 	struct attack_type	attack_table	[]		=
{
    { 	"none",		"hit",		-1		},  /*  0 */
    {	"slice",	"slice", 	DAM_SLASH	},	
    {   "stab",		"stab",		DAM_PIERCE	},
    {	"slash",	"slash",	DAM_SLASH	},
    {	"whip",		"whip",		DAM_SLASH	},
    {   "claw",		"claw",		DAM_SLASH	},  /*  5 */
    {	"blast",	"blast",	DAM_BASH	},
    {   "pound",	"pound",	DAM_BASH	},
    {	"crush",	"crush",	DAM_BASH	},
    {   "grep",		"grep",		DAM_SLASH	},
    {	"bite",		"bite",		DAM_PIERCE	},  /* 10 */
    {   "pierce",	"pierce",	DAM_PIERCE	},
    {   "suction",	"suction",	DAM_ACID	},
    {	"beating",	"beating",	DAM_BASH	},
    {   "digestion",	"digestion",	DAM_ACID	},
    {	"charge",	"charge",	DAM_PIERCE	},  /* 15 */
    { 	"slap",		"slap",		DAM_BASH	},
    {	"punch",	"punch",	DAM_BASH	},
    {	"wrath",	"wrath",	DAM_ENERGY	},
    {	"magic",	"magic",	DAM_ENERGY	},
    {   "divine",	"divine power",	DAM_HOLY	},  /* 20 */
    {	"cleave",	"cleave",	DAM_SLASH	},
    {	"scratch",	"scratch",	DAM_PIERCE	},
    {   "peck",		"peck",		DAM_PIERCE	},
    {   "peckb",	"peck",		DAM_BASH	},
    {   "chop",		"chop",		DAM_SLASH	},  /* 25 */
    {   "sting",	"sting",	DAM_PIERCE	},
    {   "smash",	 "smash",	DAM_BASH	},
    {   "shbite",	"shocking bite",DAM_LIGHTNING	},
    {	"flbite",	"flaming bite", DAM_FIRE	},
    {	"frbite",	"freezing bite", DAM_COLD	},  /* 30 */
    {	"acbite",	"acidic bite", 	DAM_ACID	},
    {	"chomp",	"chomp",	DAM_PIERCE	},
    {  	"drain",	"life drain",	DAM_NEGATIVE	},
    {   "thrust",	"thrust",	DAM_PIERCE	},
    {   "slime",	"slime",	DAM_ACID	},
    {	"shock",	"shock",	DAM_LIGHTNING	},
    {   "thwack",	"thwack",	DAM_BASH	},
    {   "flame",	"flame",	DAM_FIRE	},
    {   "chill",	"chill",	DAM_COLD	},
    {   "typo",		"typo",		DAM_SLASH	}, /* 40 */
    {   "ejab",         "elbow jab",    DAM_MONK        },
    {   "skick",        "shin kick",    DAM_MONK        },
    {   "bfist",        "backfist",    DAM_MONK        },
    {   "pstrike",      "palm strike",  DAM_MONK        },
    {   "tfist",        "tiger fist",   DAM_MONK        }, /* 45*/
    {   "dkick",        "dragon kick",  DAM_MONK        },
    {   "dfist",        "dragon fist",  DAM_MONK        },
    {   "ucut",         "uppercut",     DAM_MONK        },
    {   "kslam",        "knee slam",    DAM_MONK        },
    {   "dkick",        "drop kick",    DAM_MONK        }, /* 50*/
    {   "spkick",       "spin kick",    DAM_MONK        },
    {   "tkick",        "tornado kick", DAM_MONK        },
    {   "estrike",      "elemental strike", DAM_MONK    },
    {   "cpalm",        "chaos palm",   DAM_MONK        },
    {   "cfist",        "chaos fist",   DAM_MONK        }, /* 55*/
    {   "mstrike",      "mental strike", DAM_MONK       },
    {   "rfist",        "rock fist",    DAM_MONK       },
    {   "lpalm",        "lightning palm", DAM_MONK      },
    {   "kfury",        "kick of fury", DAM_MONK        },
    {   "iblow",        "inferno blow", DAM_MONK        }, /*  60*/
    {   "drowning",	"drowning",	DAM_DROWNING    },
    {   "ray",		"ray",		DAM_LIGHT	},
    {   "mblast",	"mindblast",    DAM_MENTAL      },
    {   "rend",		"rend",		DAM_POISON	},
    {   "iron",		"iron",		DAM_IRON        }, /*  65*/
    {   "screech",      "screech",	DAM_SOUND       },
    {   "sliver",	"sliver",	DAM_WOOD	},
    {   "silver",       "silver",       DAM_SILVER      },
    {	"shard",	"shard",	DAM_EARTH	},
    {   NULL,		NULL,		0		}
};

/* race table */
const 	struct	race_type	race_table	[]		=
{
/*
    {
	name,		pc_race?,
	act bits,
	aff_by bits,
	off bits,
	imm,
	res,
	vuln,
	shd bits,
	form,		parts 
    },
*/
    { "unique",		FALSE, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    
    {
       "cylith",		TRUE,
        0,
        AFF_BERSERK,
        0,
        0,
        RES_BASH,
        VULN_ACID,
        0,
        A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },
       
    {
      "efrit",			TRUE,
	0,
        AFF_FARSIGHT|AFF_HASTE,
        0,
        0, 
        RES_COLD|RES_LIGHTNING,
        VULN_SLASH|VULN_PIERCE,
	SHD_ICE,
        A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },
    
    {
       "drezil",		TRUE,
        0,
        AFF_FARSIGHT|AFF_DETECT_HIDDEN|AFF_DETECT_INVIS,   
        0,
        0,
        RES_EARTH|RES_WOOD,
        VULN_POISON|VULN_DISEASE|VULN_FIRE,
        SHD_EARTH,
        A|H|M|V,		A|B|C|D|E|F|G|H|I|J|K
    },
    
    {
       "rahmat",		TRUE,
        0, 
        0,
        0,
        0,
        RES_LIGHTNING,
        VULN_BASH,
        SHD_SHOCK,
        A|H|M|V,			A|B|C|D|E|F|G|H|I|J|K
    },
     
    {
        "sivah",		TRUE,
        0,
        AFF_PASS_DOOR,
        0,
        0,
        0,
        0,
        0,
        A|H|M|V,			A|B|D|D|E|F|G|H|I|J|K
    },
    
    {
        "revada",		TRUE,    
        0,
        AFF_REGENERATION,
        0,
        0,
        RES_NEGATIVE,
        VULN_HOLY,
        0,
        A|H|M|V,			A|B|C|D|E|F|G|H|I|J|K
    },
    {
       "vixta",			TRUE,
       0,
       AFF_INFRARED,
       0,
       0,
       RES_CHARM|RES_NEGATIVE,
       VULN_HOLY|VULN_LIGHT,
       0,
       A|H|M|V,				A|B|C|D|E|F|G|H|I|J|K
    },
    {
       "phoenix",		TRUE,
       0,
       0,
       0,
       0,
       RES_FIRE|RES_MENTAL,
       VULN_COLD|VULN_DROWNING,
       SHD_FIRE,
       A|H|M|V,				A|B|C|D|E|F|G|H|I|J|K
    },
    {
       "shador",		TRUE,
	0,
        0,
        0,
        0,
        RES_CHARM,
        VULN_LIGHT,
        SHD_SHADOW,
        A|H|M|V,			A|B|C|D|E|F|G|H|I|J|K
    },
    {
       "sareef",		TRUE,
        0,  
        0,
        0,
        0,
        RES_SOUND,
        VULN_WOOD,
        SHD_PRISMATIC,
        A|H|M|V,			A|B|C|D|E|F|G|H|I|J|K
    },
    {
	"bakali",		TRUE,
	0,
        0,
        0,
        0,
        RES_FIRE,
        VULN_COLD,
        SHD_FIRE,
        A|H|M|V,			A|B|C|D|E|F|G|H|I|J|K
    },
    {
	"felinian",		TRUE,
	0,
	0,
	0,
        0,
        RES_CHARM|RES_COLD,
        VULN_SILVER|VULN_FIRE,
        0,
        A|H|M|V,			A|B|C|D|E|F|G|H|I|J|K
    },
    {
	"maho",			TRUE,
	0,
	AFF_FARSIGHT,
	0,
	0,
        RES_IRON|RES_SILVER,
        VULN_LIGHT,
        0,
        A|H|M|V,			A|B|C|D|E|F|G|H|I|J|K	
    },
    { 
	"human",		FALSE,
	0,
	0, 
	0,
	0, 
	0,
	0,
	0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"elf",			FALSE,
	0,
	AFF_INFRARED,
	0,
	0,
	RES_CHARM,
	VULN_IRON,
	0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"dwarf",		FALSE,
	0,
	AFF_INFRARED,
	0,
	0,
	RES_POISON|RES_DISEASE,
	VULN_DROWNING,
	0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"giant",		FALSE,
	0,
	0,
	0,
	0,
	RES_FIRE|RES_COLD,
	VULN_MENTAL|VULN_LIGHTNING,
	0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    { 
	"pixie",		FALSE,
	0,
	AFF_FLYING|AFF_DETECT_MAGIC|AFF_INFRARED,
	OFF_DODGE,
	0,
	RES_CHARM|RES_MENTAL,
	VULN_IRON,
	0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K|P
    },
    {
     	"sprite",		FALSE,
        0,
        AFF_FLYING|AFF_DETECT_HIDDEN|AFF_DETECT_INVIS,
        OFF_DODGE|OFF_FAST,
        0,
        RES_MENTAL,
        VULN_LIGHTNING,
        0, 
        A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K|P
    },
    { 
	"halfling",		FALSE,
	0,
	AFF_PASS_DOOR,
	0,
	0,
	RES_POISON|RES_DISEASE,
	VULN_LIGHT,
	0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    { 
	"halforc",		FALSE,
	0,
	AFF_BERSERK,
	0,
	0,
	RES_MAGIC|RES_WEAPON,
	VULN_MENTAL,
	0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    { 
	"goblin",		FALSE,
	0,
	AFF_INFRARED,
	0,
	0,
	RES_MENTAL,
	VULN_SILVER|VULN_LIGHT|VULN_WOOD|VULN_HOLY,
	0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    { 
	"halfelf",		FALSE,
	0,
	AFF_FARSIGHT,
	0,
	0,
	0,
	0,
	0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"avian",		FALSE,
	0,
    AFF_FLYING|AFF_DETECT_INVIS|AFF_DETECT_HIDDEN|AFF_FARSIGHT|AFF_INFRARED,
	0,
	0,
	RES_LIGHTNING,
	VULN_DISEASE|VULN_POISON,
	0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K|P
    },

    {
	"gnome",		FALSE,
	0,
	AFF_INFRARED,
	0,
	0,
	RES_MENTAL,
	VULN_DROWNING,
	0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"draconian",		FALSE,
	0,
	AFF_FLYING,
	0,
	IMM_POISON|IMM_DISEASE,
	RES_FIRE|RES_COLD,
	VULN_SLASH|VULN_PIERCE|VULN_LIGHTNING,
	0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K|P|Q
    },

    {
        "centaur",              FALSE,
        0,
	0,
	0,
        0,
	0,
	0,
        0,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },

    {
        "gnoll",                FALSE,
        0,
	AFF_DETECT_HIDDEN|AFF_DARK_VISION,
	0,
        0,
	0,
	VULN_SILVER,
        0,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K|U|V
    },

    {
        "heucuva",              FALSE,
        0,
	AFF_DARK_VISION,
	0,
        IMM_NEGATIVE|IMM_POISON|IMM_DISEASE,
	RES_WEAPON,
	VULN_HOLY|VULN_LIGHT|VULN_SILVER,
        0,
        A|H|M|V,        A|B|C|G|H|I|J|K
    },

    {
        "kenku",                FALSE,
        0,
	AFF_FARSIGHT|AFF_DETECT_HIDDEN|AFF_INFRARED|AFF_FLYING,
        0,
        0,
	RES_COLD,
	0,
        0,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K|P
    },

    {
        "minotaur",             FALSE,
        0,
	AFF_FARSIGHT,
	0,
        IMM_POISON,
	0,
	VULN_BASH,
        0,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K|W
    },

    {
        "satyr",                FALSE,
        0,
	AFF_DETECT_HIDDEN|AFF_DETECT_EVIL|AFF_DETECT_GOOD,
        0,
	IMM_FIRE,
	RES_NEGATIVE,
	VULN_HOLY|VULN_LIGHT,
        0,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },

    {
        "titan",                FALSE,
        0,
	AFF_DETECT_INVIS|AFF_BERSERK,
	0,
        IMM_CHARM,
	RES_WEAPON,
	0,
        0,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"bat",			FALSE,
	0,
	AFF_FLYING|AFF_DARK_VISION,
	OFF_DODGE|OFF_FAST,
	0,
	0,
	VULN_LIGHT,
	0,
	A|G|V,		A|C|D|E|F|H|J|K|P
    },

    {
	"bear",			FALSE,
	0,
	0,
	OFF_CRUSH|OFF_DISARM|OFF_BERSERK,
	0,
	RES_BASH|RES_COLD,
	0,
	0,
	A|G|V,		A|B|C|D|E|F|H|J|K|U|V
    },

    {
	"cat",			FALSE,
	0,
	AFF_DARK_VISION,
	OFF_FAST|OFF_DODGE,
	0,
	0,
	0,
	0,
	A|G|V,		A|C|D|E|F|H|J|K|Q|U|V
    },

    {
	"centipede",		FALSE,
	0,
	AFF_DARK_VISION,
	0,
	0,
	RES_PIERCE|RES_COLD,
	VULN_BASH,
	0,
 	A|B|G|O,		A|C|K	
    },

    {
	"dog",			FALSE,
	0,
	0,
	OFF_FAST,
	0,
	0,
	0,
	0,
	A|G|V,		A|C|D|E|F|H|J|K|U|V
    },

    {
	"doll",			FALSE,
	0,
	0,
	0,
 IMM_COLD|IMM_POISON|IMM_HOLY|IMM_NEGATIVE|IMM_MENTAL|IMM_DISEASE|IMM_DROWNING,
	RES_BASH|RES_LIGHT,
	VULN_SLASH|VULN_FIRE|VULN_ACID|VULN_LIGHTNING|VULN_ENERGY,
	0,
	E|J|M|cc,	A|B|C|G|H|K
    },

    { 	"dragon", 		FALSE, 
	0,
	AFF_INFRARED|AFF_FLYING,
	0,
	0,
	RES_FIRE|RES_BASH|RES_CHARM, 
	VULN_PIERCE|VULN_COLD,
	0,
	A|H|Z,		A|C|D|E|F|G|H|I|J|K|P|Q|U|V|X
    },

    {
	"fido",			FALSE,
	0,
	0,
	OFF_DODGE|ASSIST_RACE,
	0,
	0,
	VULN_MAGIC,
	0,
	A|B|G|V,	A|C|D|E|F|H|J|K|Q|V
    },		
   
    {
	"fox",			FALSE,
	0,
	AFF_DARK_VISION,
	OFF_FAST|OFF_DODGE,
	0,
	0,
	0,
	0,
	A|G|V,		A|C|D|E|F|H|J|K|Q|V
    },

    {
	"hobgoblin",		FALSE,
	0,
	AFF_INFRARED,
	0,
	0,
	RES_DISEASE|RES_POISON,
	0,
	0,
	A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K|Y
    },

    {
	"kobold",		FALSE,
	0,
	AFF_INFRARED,
	0,
	0,
	RES_POISON,
	VULN_MAGIC,
	0,
	A|B|H|M|V,	A|B|C|D|E|F|G|H|I|J|K|Q
    },

    {
	"lizard",		FALSE,
	0,
	0,
	0,
	0,
	RES_POISON,
	VULN_COLD,
	0,
	A|G|X|cc,	A|C|D|E|F|H|K|Q|V
    },

    {
	"modron",		FALSE,
	0,
	AFF_INFRARED,
	ASSIST_RACE|ASSIST_ALIGN,
	IMM_CHARM|IMM_DISEASE|IMM_MENTAL|IMM_HOLY|IMM_NEGATIVE,
	RES_FIRE|RES_COLD|RES_ACID,
	0,
	0,
	H,		A|B|C|G|H|J|K
    },

    {
	"orc",			FALSE,
	0,
	AFF_INFRARED,
	0,
	0,
	RES_DISEASE,
	VULN_LIGHT,
	0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"pig",			FALSE,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	A|G|V,	 	A|C|D|E|F|H|J|K
    },	

    {
	"rabbit",		FALSE,
	0,
	0,
	OFF_DODGE|OFF_FAST,
	0,
	0,
	0,
	0,
	A|G|V,		A|C|D|E|F|H|J|K
    },
    
    {
	"school monster",	FALSE,
	ACT_NOALIGN,
	0,
	0,
	IMM_CHARM|IMM_SUMMON,
	0,
	VULN_MAGIC,
	0,
	A|M|V,		A|B|C|D|E|F|H|J|K|Q|U
    },	

    {
	"snake",		FALSE,
	0,
	0,
	0,
	0,
	RES_POISON,
	VULN_COLD,
	0,
	A|G|X|Y|cc,	A|D|E|F|K|L|Q|V|X
    },
 
    {
	"song bird",		FALSE,
	0,
	AFF_FLYING,
	OFF_FAST|OFF_DODGE,
	0,
	0,
	0,
	0,
	A|G|W,		A|C|D|E|F|H|K|P
    },

    {
	"troll",		FALSE,
	0,
	AFF_REGENERATION|AFF_INFRARED|AFF_DETECT_HIDDEN,
	OFF_BERSERK,
 	0,
	RES_CHARM|RES_BASH,
	VULN_FIRE|VULN_ACID,
	0,
	A|B|H|M|V,		A|B|C|D|E|F|G|H|I|J|K|U|V
    },

    {
	"water fowl",		FALSE,
	0,
	AFF_SWIM|AFF_FLYING,
	0,
	0,
	RES_DROWNING,
	0,
	0,
	A|G|W,		A|C|D|E|F|H|K|P
    },		
  
    {
	"wolf",			FALSE,
	0,
	AFF_DARK_VISION,
	OFF_FAST|OFF_DODGE,
	0,
	0,
	0,	
	0,
	A|G|V,		A|C|D|E|F|J|K|Q|V
    },

    {
	"wyvern",		FALSE,
	0,
	AFF_FLYING|AFF_DETECT_INVIS|AFF_DETECT_HIDDEN,
	OFF_BASH|OFF_FAST|OFF_DODGE,
	IMM_POISON,
	0,
	VULN_LIGHT,
	0,
	A|B|G|Z,		A|C|D|E|F|H|J|K|Q|V|X
    },

    { "unique",		FALSE, 0, 0, 0, 0, 0, 0, 0, 0, 0 },

    {
	NULL, 0, 0, 0, 0, 0, 0
    }
};

const	struct	pc_race_type	pc_race_table	[]	=
{
    {
	"null race",	"",
	0,
	{ 100, 100, 100, 100, 100, 100, 100,
	  100, 100, 100, 100, 100, 100, 100 },
	{ "" },
	{ 13, 13, 13, 13, 13 },
	{ 18, 18, 18, 18, 18 },
	0
    },
 
/*
    {
	"race name", 	short name,
	points,
	{ class multipliers },
	{ bonus skills },
	{ base stats },
	{ max stats },
	size 
    },
*/
    {
        "cylith",	"Cylit",
        10,
        { 150, 150, 150, 100, 150, 150, 150, 150, 150,
          300, 300, 300, 150, 300, 300, 300, 300, 300 },
        { "" },
        { 22, 11, 12, 14, 21 },
        { 26, 15, 16, 18, 25 },
        SIZE_GIANT
    },

    {
 	"efrit",	"Efrit",
        6,
        { 100, 150, 150, 150, 150, 150, 150, 150, 150,
          150, 300, 300, 300, 300, 300, 300, 300, 300 },
        { "" },
        { 10, 22, 20, 16, 12 },
        { 14, 26, 24, 20, 16 },
        SIZE_TINY
    },

    {
        "drezil",	"Drezi",
        6,
        { 150, 150, 150, 150, 150, 100, 150, 150, 150,
          300, 300, 300, 300, 300, 150, 300, 300, 300 },
        { "" },
        { 12, 19, 21, 15, 13 },
        { 16, 23, 25, 19, 17 },
        SIZE_MEDIUM
    },
    {
        "rahmat",	"Rahma",
        9,
        { 150, 100, 150, 150, 150, 150, 150, 150, 150,
          300, 150, 300, 300, 300, 300, 300, 300, 300 },
        { "" },
        { 13, 19, 20, 14, 14 },
        { 17, 23, 24, 18, 18 },
        SIZE_SMALL
    },
    {
        "sivah",	"Sivah",
        2,
        { 100, 100, 100, 100, 100, 100, 100, 100, 100,
          150, 150, 150, 150, 150, 150, 150, 150, 150 },
        { "" },
        { 16, 16, 16, 16, 16 },
        { 20, 20, 20, 20, 20 },
        SIZE_LARGE
    },
    {
        "revada",	"Revad",
        5,
        { 150, 150, 150, 150, 150, 150, 100, 150, 150,
          300, 300, 300, 300, 300, 300, 150, 300, 300 },
        { "" },
        { 14, 17, 16, 19, 14 },
        { 18, 21, 20, 23, 18 },
        SIZE_MEDIUM
    },
    {
	"vixta",	"Vixta",
	5,
	{ 150, 150, 100, 150, 150, 150, 150, 150, 150,
	  300, 300, 150, 300, 300, 300, 300, 300, 300 },
	{ "" },
	{ 16, 13, 13, 21, 17 },
	{ 20, 17, 17, 25, 21 },
	SIZE_SMALL
    },
    {
	"phoenix",	"Phnix",
	7,
	{ 150, 150, 150, 150, 100, 150, 150, 150, 150,
	  300, 300, 300, 300, 150, 300, 300, 300, 300 },
	{ "" },
	{ 17, 15, 15, 15, 18 },
	{ 21, 19, 19, 19, 22 },
	SIZE_HUGE
    },
    {
       "shador",	"Shado",
        6,
        { 150, 150, 150, 150, 150, 150, 150, 150, 100,
          300, 300, 300, 300, 300, 300, 300, 300, 150 },
        { "" },
        { 12, 18, 16, 20, 14 },
        { 16, 22, 20, 24, 18 },
        SIZE_LARGE
    },
    {
       "sareef",	"Saree",
       10,
       { 150, 150, 150, 150, 150, 150, 150, 100, 150,
         300, 300, 300, 300, 300, 300, 300, 150, 300 },
       { "" },
       { 19, 12, 11, 18, 20 },
       { 23, 16, 15, 22, 24 },
       SIZE_GIANT
    },
    {
       "bakali",	"Bakal",
       10,
       { 150, 150, 150, 100, 150, 150, 150, 150, 150,
         300, 300, 300, 150, 300, 300, 300, 300, 300 },
       { "" },
       { 22, 11, 11, 14, 22 },
       { 26, 15, 15, 18, 26 },
       SIZE_GIANT
    },
    {
       "felinian",	"Felin",
       6,
       { 150, 150, 100, 150, 150, 150, 150, 150, 150,
         300, 300, 150, 300, 300, 300, 300, 300, 300 },
       { "" },
       { 16, 13, 13, 22, 16 },
       { 20, 17, 17, 26, 20 },
       SIZE_MEDIUM
    },
    {
       "maho",	"Maho",
       7,
       { 150, 150, 150, 150, 150, 150, 150, 150, 100,
         300, 300, 300, 300, 300, 300, 300, 300, 150 },
       { "" },
       { 13, 18, 18, 18, 13 },
       { 17, 22, 22, 22, 17 },
       SIZE_MEDIUM
    }
};

	
      	

/*
 * Class table.
 */
const	struct	class_type	class_table	[MAX_CLASS]	=
{

/*
    {
	name, who, attr_prime, weapon,
	{guild1, guild2, guild3}, thac0_00, thac0_32, hp_min, hp_max, mana?,
	base_group, default_group
    }
*/

    {
	"wizard", "Wiz",  STAT_INT,  OBJ_VNUM_SCHOOL_DAGGER,
	{ 20105, 20106, 20107 },  75,  20, 6,  19,  29, TRUE,
	"wizard basics", "wizard default"
    },

    {
	"priest", "Pri",  STAT_WIS,  OBJ_VNUM_SCHOOL_MACE,
	{ 20105, 20106, 20107 },  75,  20, 2,  20, 30, TRUE,
	"priest basics", "priest default"
    },

    {
	"mercenary", "Mer",  STAT_DEX,  OBJ_VNUM_SCHOOL_DAGGER,
	{ 20105, 20106, 20107 },  75,  20,  -4,  24, 34, FALSE,
	"mercenary basics", "mercenary default"
    },

    {
	"gladiator", "Gla",  STAT_STR,  OBJ_VNUM_SCHOOL_SWORD,
	{ 20105, 20106, 20107 },  75,  20,  -10,  36, 46, FALSE,
	"gladiator basics", "gladiator default"
    },

    {
	"strider", "Str",  STAT_STR,  OBJ_VNUM_SCHOOL_SPEAR,
	{ 20105, 20106, 20107 },  75,  20,  -4,  22,  32, TRUE,
	"strider basics", "strider default"
    },

    {
	"shaman", "Shm",  STAT_WIS,  OBJ_VNUM_SCHOOL_POLEARM,
	{ 20105, 20106, 20107 },  75,  20,  0,  20,  30,  TRUE,
	"shaman basics", "shaman default"
    },

    {
	"lich", "Lic",  STAT_CON,  OBJ_VNUM_SCHOOL_DAGGER,
	{ 20105, 20106, 20107 },  75,  20, -30,  22,  32, TRUE,
	"lich basics", "lich default"
    },

    {   "shaolin", "Sha", STAT_CON, OBJ_VNUM_SCHOOL_DAGGER,
        { 20105, 20106, 20107 }, 75, 20, -10, 34, 44, FALSE,
        "shaolin basics", "shaolin default"
    },

    {   "assassin", "Asn", STAT_DEX, OBJ_VNUM_SCHOOL_DAGGER,
        { 20105, 20106, 20107 }, 75, 20, -4, 24, 34, TRUE,
        "assassin basics", "assassin default"
    },

    {
	"sorcerer", "Sor",  STAT_INT,  OBJ_VNUM_SCHOOL_DAGGER,
	{ 20105, 20106, 20107 },  75,  10, -4,  24,  34, TRUE,
	"sorcerer basics", "sorcerer default"
    },

    {
	"bishop", "Bsh",  STAT_WIS,  OBJ_VNUM_SCHOOL_MACE,
	{ 20105, 20106, 20107 },  75,  10, 2,  25, 35, TRUE,
	"bishop basics", "bishop default"
    },

    {
	"blade", "Bld",  STAT_DEX,  OBJ_VNUM_SCHOOL_DAGGER,
	{ 20105, 20106, 20107 },  75,  10,  -14,  29, 39, FALSE,
	"blade basics", "blade default"
    },

    {
	"knight", "Kni",  STAT_STR,  OBJ_VNUM_SCHOOL_SWORD,
	{ 20105, 20106, 20107 },  75,  10,  -20,  41, 51, FALSE,
	"knight basics", "knight default"
    },

    {
	"guardian", "Grd",  STAT_INT,  OBJ_VNUM_SCHOOL_DAGGER,
	{ 20105, 20106, 20107 },  75,  10,  -14,  27,  37, TRUE,
	"guardian basics", "guardian default"
    },

    {
	"elder", "Eld",  STAT_WIS,  OBJ_VNUM_SCHOOL_POLEARM,
	{ 20105, 20106, 20107 },  75,  10,  -10,  25,  35,  TRUE,
	"elder basics", "elder default"
    },

    {
	"necromancer", "Ncr",  STAT_CON,  OBJ_VNUM_SCHOOL_DAGGER,
	{ 20105, 20105, 20106 },  75,  10, -40,  27,  37, TRUE,
	"necromancer basics", "necromancer default"
    },

    {   "sensai", "Sen", STAT_CON, OBJ_VNUM_SCHOOL_DAGGER,
        { 20105, 20106, 20107 }, 75, 10, -20, 39, 49, FALSE,
        "sensai basics", "sensai default"
    },
    
    {   "deathmaster", "Dth", STAT_DEX, OBJ_VNUM_SCHOOL_DAGGER,
        { 20105, 20106, 20107 }, 75, 10, -14, 29, 39, TRUE,
        "deathmaster basics", "deathmaster default"
    }
};


/*
 * Attribute bonus tables.
 */
const	struct	str_app_type	str_app		[31]		=
{
    { -5, -4,   0,  0 },  /* 0  */
    { -5, -4,   3,  1 },  /* 1  */
    { -3, -2,   3,  2 },
    { -3, -1,  10,  3 },  /* 3  */
    { -2, -1,  25,  4 },
    { -2, -1,  55,  5 },  /* 5  */
    { -1,  0,  80,  6 },
    { -1,  0,  90,  7 },
    {  0,  0, 100,  8 },
    {  0,  0, 100,  9 },
    {  0,  0, 115, 10 }, /* 10  */
    {  0,  0, 115, 11 },
    {  0,  0, 130, 12 },
    {  0,  0, 130, 13 }, /* 13  */
    {  0,  1, 140, 14 },
    {  1,  1, 150, 15 }, /* 15  */
    {  1,  2, 165, 16 },
    {  2,  3, 180, 22 },
    {  2,  3, 200, 25 }, /* 18  */
    {  3,  4, 225, 30 },
    {  3,  5, 250, 35 }, /* 20  */
    {  4,  6, 300, 40 },
    {  4,  6, 350, 45 },
    {  5,  7, 400, 50 },
    {  5,  8, 450, 55 },
    {  6,  9, 500, 60 }, /* 25   */
    {  6,  9, 550, 65 },
    {  7, 10, 600, 70 },
    {  7, 11, 650, 75 },
    {  8, 12, 700, 80 },
    {  8, 12, 750, 85 } /*  30   */
};



const	struct	int_app_type	int_app		[31]		=
{
    {  3 },	/*  0 */
    {  5 },	/*  1 */
    {  7 },
    {  8 },	/*  3 */
    {  9 },
    { 10 },	/*  5 */
    { 11 },
    { 12 },
    { 13 },
    { 15 },
    { 17 },	/* 10 */
    { 19 },
    { 22 },
    { 25 },
    { 28 },
    { 31 },	/* 15 */
    { 34 },
    { 37 },
    { 40 },	/* 18 */
    { 44 },
    { 49 },	/* 20 */
    { 55 },
    { 60 },
    { 65 },
    { 70 },
    { 75 },	/* 25 */
    { 80 },
    { 85 },
    { 90 },
    { 95 },
    {100 }     /* 30 */ 
};



const	struct	wis_app_type	wis_app		[31]		=
{
    { 0 },	/*  0 */
    { 0 },	/*  1 */
    { 0 },
    { 0 },	/*  3 */
    { 0 },
    { 1 },	/*  5 */
    { 1 },
    { 1 },
    { 1 },
    { 1 },
    { 1 },	/* 10 */
    { 1 },
    { 1 },
    { 1 },
    { 1 },
    { 2 },	/* 15 */
    { 2 },
    { 2 },
    { 3 },	/* 18 */
    { 3 },
    { 3 },	/* 20 */
    { 3 },
    { 4 },
    { 4 },
    { 4 },
    { 5 },	/* 25 */
    { 5 }, 
    { 5 },
    { 6 },
    { 6 },
    { 6 }      /* 30 */
};



const	struct	dex_app_type	dex_app		[31]		=
{
    {   60 },   /* 0 */
    {   50 },   /* 1 */
    {   50 },
    {   40 },
    {   30 },
    {   20 },   /* 5 */
    {   10 },
    {    0 },
    {    0 },
    {    0 },
    {    0 },   /* 10 */
    {    0 },
    {    0 },
    {    0 },
    {    0 },
    { - 10 },   /* 15 */
    { - 15 },
    { - 20 },
    { - 30 },
    { - 40 },
    { - 50 },   /* 20 */
    { - 60 },
    { - 75 },
    { - 90 },
    { -105 },
    { -120 },    /* 25 */
    { -130 },
    { -140 },
    { -150 },
    { -160 },
    { -170 }    /* 30 */   
};


const	struct	con_app_type	con_app		[31]		=
{
    { -4, 20 },   /*  0 */
    { -3, 25 },   /*  1 */
    { -2, 30 },
    { -2, 35 },	  /*  3 */
    { -1, 40 },
    { -1, 45 },   /*  5 */
    { -1, 50 },
    {  0, 55 },
    {  0, 60 },
    {  0, 65 },
    {  0, 70 },   /* 10 */
    {  0, 75 },
    {  0, 80 },
    {  0, 85 },
    {  0, 88 },
    {  1, 90 },   /* 15 */
    {  2, 95 },
    {  2, 97 },
    {  3, 99 },   /* 18 */
    {  3, 99 },
    {  4, 99 },   /* 20 */
    {  4, 99 },
    {  5, 99 },
    {  6, 99 },
    {  7, 99 },
    {  8, 99 },    /* 25 */
    {  8, 99 },
    {  9, 99 },
    {  9, 99 },
    { 10, 99 },
    { 10, 99 }     /* 30 */
};



/*
 * Liquid properties.
 * Used in world.obj.
 * Be sure MAX_LIQUID in merc.h matches
 * the number of entries below.
 */
const	struct	liq_type	liq_table	[]	=
{
/*    name			color	proof, full, thirst, food, ssize */
    { "water",			"clear",	{   0, 1, 10, 0, 16 }	},
    { "beer",			"amber",	{  12, 1,  8, 1, 12 }	},
    { "red wine",		"burgundy",	{  30, 1,  8, 1,  5 }	},
    { "ale",			"brown",	{  15, 1,  8, 1, 12 }	},
    { "dark ale",		"dark",		{  16, 1,  8, 1, 12 }	},

    { "whisky",			"golden",	{ 120, 1,  5, 0,  2 }	},
    { "lemonade",		"pink",		{   0, 1,  9, 2, 12 }	},
    { "firebreather",		"boiling",	{ 190, 0,  4, 0,  2 }	},
    { "local specialty",	"clear",	{ 151, 1,  3, 0,  2 }	},
    { "slime mold juice",	"green",	{   0, 2, -8, 1,  2 }	},

    { "milk",			"white",	{   0, 2,  9, 3, 12 }	},
    { "tea",			"tan",		{   0, 1,  8, 0,  6 }	},
    { "coffee",			"black",	{   0, 1,  8, 0,  6 }	},
    { "blood",			"red",		{   0, 2, -1, 2,  6 }	},
    { "salt water",		"clear",	{   0, 1, -2, 0,  1 }	},

    { "coke",			"brown",	{   0, 2,  9, 2, 12 }	}, 
    { "root beer",		"brown",	{   0, 2,  9, 2, 12 }   },
    { "elvish wine",		"green",	{  35, 2,  8, 1,  5 }   },
    { "white wine",		"golden",	{  28, 1,  8, 1,  5 }   },
    { "champagne",		"golden",	{  32, 1,  8, 1,  5 }   },

    { "mead",			"honey-colored",{  34, 2,  8, 2, 12 }   },
    { "rose wine",		"pink",		{  26, 1,  8, 1,  5 }	},
    { "benedictine wine",	"burgundy",	{  40, 1,  8, 1,  5 }   },
    { "vodka",			"clear",	{ 130, 1,  5, 0,  2 }   },
    { "cranberry juice",	"red",		{   0, 1,  9, 2, 12 }	},

    { "orange juice",		"orange",	{   0, 2,  9, 3, 12 }   }, 
    { "absinthe",		"green",	{ 200, 1,  4, 0,  2 }	},
    { "brandy",			"golden",	{  80, 1,  5, 0,  4 }	},
    { "aquavit",		"clear",	{ 140, 1,  5, 0,  2 }	},
    { "schnapps",		"clear",	{  90, 1,  5, 0,  2 }   },

    { "icewine",		"purple",	{  50, 2,  6, 1,  5 }	},
    { "amontillado",		"burgundy",	{  35, 2,  8, 1,  5 }	},
    { "sherry",			"red",		{  38, 2,  7, 1,  5 }   },	
    { "framboise",		"red",		{  50, 1,  7, 1,  5 }   },
    { "rum",			"amber",	{ 151, 1,  4, 0,  2 }	},

    { "cordial",		"clear",	{ 100, 1,  5, 0,  2 }   },
    { "dr pepper",		"brown",	{   0, 2,  9, 2, 12 }	},
    { NULL,			NULL,		{   0, 0,  0, 0,  0 }	}
};



/*
 * The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections.
 */
#define SLOT(n)	n

const	struct	skill_type	skill_table	[MAX_SKILL]	=
{

/*
 * Magic spells.
 */

/*
    {
	"NAME",
	{ SKILL_LEVELS 1st TIER,
	  SKILL_LEVELS 2nd TIER },
	{ DIFFICULTY 1st TIER,
	  DIFFICULTY 2nd TIER },
	SPELL_FUN,		TARGET,		MIN_POSITION,
	GSN,		SOCKET,	SLOT(#),	MIN_MANA,	BEATS,
	"DAMAGE NOUN",	"OFF MESSAGE",	"OFF MESSAGE (OBJ)"
    }

	NAME		Name of Spell/Skill
	SKILL_LEVELS	Levels to obtain spell/skill per class
	DIFFICULTY	Spells: multiplier for base mana
			Skills: charge for gaining skill
	SPELL_FUN	Routine to call for spells
	TARGET		Legal targets
	MIN_POSTITION	Position for caster/user
	GSN		gsn for skills and some spells
	SOCKET		Can spell be cast on person from same socket
			(to limit multiplaying)
	SLOT(#)		A unique slot number for spells
	MIN_MANA	Base mana for spells (multiplied by DIFFICULTY)
	BEATS		Waiting time after use
	DAMAGE NOUN	Damage Message
	OFF MESSAGE	Wear off message
	OFF MESSAGE (OBJ)	Wear off message for objects
*/

    {
	"reserved",
	{ 199, 199, 199, 199, 199, 199, 199, 199, 199,
	  199, 199, 199, 199, 199, 199, 199, 199, 199 },
	{ 99, 99, 99, 99, 99, 99, 99, 99, 99,
	  99, 99, 99, 99, 99, 99, 99, 99, 99 },
	0,			TAR_IGNORE,	POS_STANDING,
	NULL,		TRUE,	SLOT( 0),	0,	0,
	"",		"",		""
    },
    {
	"chill touch",
        { 199, 199, 199, 199, 199, 199, 199, 199, 199,
          199, 199, 199, 199, 199, 199, 199, 199, 199 },
        { 99, 99, 99, 99, 99, 99, 99, 99, 99,
          99, 99, 99, 99, 99, 99, 99, 99, 99 },
        0,			TAR_IGNORE,	POS_STANDING,
        NULL,		TRUE,	SLOT( 0),	0,	0,
        "",		"You feel less cold.",	""
    }, 
    {
        "ward",
        { 75, 90, 102, 102, 102, 102, 102, 103, 103,
          65, 80, 102, 102, 102, 102, 102, 103, 103 },
        { 1, 1, 2, 2, 2, 1, 1, 1, 1,
          1, 1, 2, 2, 2, 1, 1, 1, 1  },
        spell_ward, TAR_IGNORE, POS_STANDING,
        NULL, FALSE, SLOT(600),  100, 12,
        "",    "You feel less warded.", "" },
         
    {
	"regeneration",
        { 103, 91, 103, 103, 103, 103, 103, 103, 103,
          103, 81, 103, 103, 103, 103, 103, 103, 103 },
        { 2, 2, 2, 2, 2, 2, 2, 2, 2, 
          2, 2, 2, 2, 2, 2, 2, 2, 2 },
        spell_regeneration, TAR_CHAR_DEFENSIVE, POS_STANDING,
        NULL, FALSE, SLOT(650), 75, 12,
        "", "!Regeneration!", "" 
    },
    {
        "renewal",
        { 103, 103, 103, 103, 103, 103, 103, 103, 103,
          45, 45, 103, 103, 103, 63, 103, 103, 103 },
        { 2, 2, 2, 2, 2, 2, 2, 2, 2,
          2, 2, 2, 2, 2, 2, 2, 2, 2 },
        spell_renewal,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL, FALSE, SLOT(651), 350, 12,
        "", "!Renewal!", "" 
    },
    {
	"acid blast",
	{ 55, 93, 103, 103, 103, 45, 65, 103, 90,
	  45, 83, 103, 103, 103, 35, 55, 103, 80 },
	{ 1, 1, 2, 2, 2, 1, 1, 1, 2,
	  1, 1, 2, 2, 2, 1, 1, 1, 2 },
	spell_acid_blast,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		TRUE,	SLOT(70),	50,	6,
	"acid blast",	"!Acid Blast!",		""
    },

    {
	"animate",
	{ 103, 103, 103, 103, 103, 103, 45, 103, 103,
	  103, 103, 103, 103, 103, 103, 35, 103, 103 },
	{ 1, 1, 2, 2, 2, 1, 1, 1, 1,
	  1, 1, 2, 2, 2, 1, 1, 1, 1 },
	spell_animate,		TAR_OBJ_INV,		POS_STANDING,
	NULL,		TRUE,	SLOT(239),	 100,	12,
	"",			"!Animate!",	""
    },

    {
	"immaterial armor",
	{ 13, 2, 103, 103, 40, 23, 70, 75, 45,
	  3, 1, 103, 103, 30, 13, 60, 65, 35 },
	{ 1, 1, 2, 2, 2, 1, 2, 2, 2,
	  1, 1, 2, 2, 2, 1, 2, 2, 2 },
	spell_armor,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		FALSE,	SLOT( 1),	 50,	12,
	"",			"The suit of immaterial armor surrounding you vanishes with a bright flash.",	""
    },
    {
	"essence of mist",
	{ 103, 103, 103, 103, 103, 103, 65, 103, 103,
          103, 103, 103, 103, 103, 103, 55, 103, 103 },
        { 3, 3, 3, 3, 3, 3, 3, 3, 3,
          3, 3, 3, 3, 3, 3, 3, 3, 3 },
        spell_essence_of_mist,	TAR_CHAR_SELF,		POS_STANDING,
	&gsn_essence_of_mist,		FALSE,	SLOT(656),	150,	24,
	"",			"A sharp pain runs down your spine as your body returns to its true form.",	""
    },
    {
	"essence of bat",
	{ 103, 103, 103, 103, 103, 103, 85, 103, 103,
          103, 103, 103, 103, 103, 103, 75, 103, 103 },
        { 3, 3, 3, 3, 3, 3, 3, 3, 3,
          3, 3, 3, 3, 3, 3, 3, 3, 3 },
        spell_essence_of_bat,	TAR_CHAR_SELF,		POS_STANDING,
	&gsn_essence_of_bat,		FALSE,	SLOT(657),	250,	24,
	"",			"A sharp pain runs down your spine as your body returns to its true form.",	""
    },
    {
	"essence of wolf",
	{ 103, 103, 103, 103, 103, 103, 103, 103, 103,
          103, 103, 103, 103, 103, 103, 91, 103, 103 },
        { 3, 3, 3, 3, 3, 3, 3, 3, 3,
          3, 3, 3, 3, 3, 3, 3, 3, 3 },
        spell_essence_of_wolf,	TAR_CHAR_SELF,		POS_STANDING,
	&gsn_essence_of_wolf,		FALSE,	SLOT(658),	350,	24,
	"",			"A sharp pain runs down your spine as your body returns to its true form.",	""
    },
    {
	"nourish",
        { 103, 103, 103, 103, 15, 13, 103, 103, 103,
          103, 103, 103, 103, 5, 3, 103, 103, 103 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1, 1, 1, 1 },
        spell_nourish,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		FALSE,	SLOT(655),	20,	12,
        "",			"!Nourish!",	""
    },
    {
	"bless",
	{ 103, 10, 102, 102, 102, 16, 102, 103, 103,
	  103, 1, 102, 102, 102, 6, 102, 103, 103 },
	{ 1, 1, 2, 2, 2, 1, 2, 1, 1,
	  1, 1, 2, 2, 2, 1, 2, 1, 1 },
	spell_bless,		TAR_OBJ_CHAR_DEF,	POS_STANDING,
	NULL,		FALSE,	SLOT( 3),	 30,	12,
	"",			"You feel less righteous.", 
	"$p's holy aura fades."
    },
    {
	"divine blessing",
	{ 103, 60, 103, 103, 103, 103, 103, 103, 103,
	  103, 50, 103, 103, 103, 91, 103, 103, 103 },
        { 2, 2, 2, 2, 2, 2, 2, 2, 2,
          2, 2, 2, 2, 2, 2, 2, 2, 2 },
        spell_divine_blessing,	TAR_CHAR_SELF,		POS_STANDING,
	&gsn_divine_blessing,		FALSE,	SLOT(652),	100, 	12,
	"",			"The shimmering aura surrounding you vanishes.",			""
    },
    {
	"infernal offering",
	{ 103, 60, 103, 103, 103, 103, 103, 103, 103,
	  103, 50, 103, 103, 103, 91, 103, 103, 103 },
	{ 2, 2, 2, 2, 2, 2, 2, 2, 2,
          2, 2, 2, 2, 2, 2, 2, 2, 2 },
	spell_infernal_offering, TAR_CHAR_SELF,		POS_STANDING,
	&gsn_infernal_offering,		FALSE, SLOT(653),	300,	12,
	"",			"All of a sudden the infernal power leaves you.",		""
    },
    {
	"blindness",
	{ 24, 15, 103, 103, 103, 14, 34, 103, 21,
	  14, 5, 103, 103, 103, 4, 24, 103, 11 },
	{ 1, 1, 2, 2, 2, 1, 1, 1, 2,
	  1, 1, 2, 2, 2, 1, 1, 1, 2 },
	spell_blindness,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_blindness,	TRUE,	SLOT( 4),	 20,	12,
	"",			"You can see again.",	""
    },

    {
	"cancellation",
	{ 51, 51, 102, 102, 102, 46, 102, 101, 103,
	  41, 41, 101, 101, 101, 36, 101, 90, 101 },
	{ 1, 1, 2, 2, 2, 1, 2, 2, 2,
	  1, 1, 2, 2, 2, 1, 2, 2, 2 },
	spell_cancellation,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,		FALSE,	SLOT(507),	30,	12,
	""			"!cancellation!",	""
    },

    {
	"cause damage",
	{ 5, 2, 103, 103, 103, 5, 5, 103, 103,
	  1, 1, 103, 103, 103, 1, 4, 103, 103 },
	{ 1, 1, 2, 2, 2, 1, 1, 1, 1,
	  1, 1, 2, 2, 2, 1, 1, 1, 1 },
	spell_cause_damage,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		TRUE,	SLOT(62),	20,	6,
	"harmful force",		"!Cause Damage!",	""
    },

    {
	"chain lightning",
	{ 66, 101, 103, 103, 90, 65, 103, 103, 103,
	  55, 90, 103, 103, 80, 55, 103, 103, 103 },
	{ 1, 2, 2, 2, 2, 1, 1, 1, 1,
	  1, 2, 2, 2, 2, 1, 1, 1, 1 },
	spell_chain_lightning,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		TRUE,	SLOT(500),	35,	12,
	"lightning",		"!Chain Lightning!",	""
    }, 

    {
	"charm person",
	{ 40, 30, 50, 103, 50, 40, 43, 103, 5,
	  30, 20, 40, 103, 40, 30, 33, 103, 1 },
	{ 1, 1, 2, 2, 2, 1, 1, 1, 2,
	  1, 1, 2, 2, 2, 1, 1, 1, 2 },
	spell_charm_person,	TAR_CHAR_OFFENSIVE,	POS_STANDING,
	&gsn_charm_person,	TRUE,	SLOT( 7),	 35,	12,
	"",			"You feel more self-confident.",	""
    },

    {
        "conjure",
        { 85, 102, 102, 102, 102, 95, 102, 103, 103,
          59, 102, 102, 102, 102, 71, 102, 103, 103 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1, 1, 1, 1 },
        spell_conjure,        TAR_IGNORE,             POS_STANDING,
        NULL,          TRUE,         SLOT(280),       100,    12,
        "",                     "!Conjure!",  ""
    },

    {
	"continual light",
	{ 11, 8, 102, 102, 7, 5, 102, 103, 103,
	  1, 7, 102, 102, 1, 4, 102, 103, 103 },
	{ 1, 1, 2, 2, 1, 1, 2, 1, 1,
	  1, 1, 2, 2, 1, 1, 2, 1, 1 },
	spell_continual_light,	TAR_IGNORE,		POS_STANDING,
	NULL,		TRUE,	SLOT(57),	 10,	12,
	"",			"!Continual Light!",	""
    },

    {
	"cure blindness",
	{ 103, 11, 102, 102, 103, 16, 102, 103, 103,
	  103, 1, 102, 102, 103, 6, 102, 103, 103 },
	{ 1, 1, 2, 2, 2, 1, 2, 2, 1,
	  1, 1, 2, 2, 2, 1, 2, 2, 1 },
	spell_cure_blindness,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,		FALSE,	SLOT(14),	 20,	12,
	"",			"!Cure Blindness!",	""
    },

    {
	"cure disease",
	{ 102, 26, 102, 102, 103, 32, 102, 103, 103,
	  102, 16, 102, 102, 103, 22, 102, 103, 103 },
	{ 1, 1, 2, 2, 2, 1, 2, 1, 1,
	  1, 1, 2, 2, 2, 1, 2, 1, 1 },
	spell_cure_disease,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		FALSE,	SLOT(501),	20,	12,
	"",			"!Cure Disease!",	""
    },

    {
	"cure wounds",
	{ 102, 4, 102, 102, 3, 8, 102, 103, 103,
	  102, 3, 102, 102, 1, 1, 102, 103, 103 },
	{ 1, 1, 2, 2, 2, 1, 2, 1, 1,
	  1, 1, 2, 2, 2, 1, 2, 1, 1 },
	spell_cure_wounds,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,		FALSE,	SLOT(16),	10,	6,
	"",			"!Cure Wounds!",		""
    },

    {
	"cure poison",
	{ 102, 28, 102, 102, 103, 33, 102, 103, 103,
	  102, 18, 102, 102, 103, 23, 102, 103, 103 },
	{ 1, 1, 2, 2, 2, 1, 2, 1, 1,
	  1, 1, 2, 2, 2, 1, 2, 1, 1 },
	spell_cure_poison,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		FALSE,	SLOT(43),	 20,	12,
	"",			"!Cure Poison!",	""
    },

    {
	"curse",
	{ 35, 35, 103, 103, 103, 38, 45, 103, 91,
	  25, 25, 103, 103, 103, 28, 35, 103, 80 },
	{ 1, 1, 2, 2, 2, 1, 1, 1, 2,
	  1, 1, 2, 2, 2, 1, 1, 1, 2 },
	spell_curse,		TAR_OBJ_CHAR_OFF,	POS_FIGHTING,
	&gsn_curse,	TRUE,	SLOT(17),	30,	6,
	"curse",		"The curse wears off.", 
	"$p is no longer impure."
    },

    {
	"demonfire",
	{ 76, 68, 103, 103, 103, 75, 77, 103, 103,
	  66, 58, 103, 103, 103, 65, 67, 103, 103 },
	{ 1, 1, 2, 2, 2, 1, 1, 1, 1,
	  1, 1, 2, 2, 2, 1, 1, 1, 1 },
	spell_demonfire,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		TRUE,	SLOT(505),	50,	6,
	"torments",		"!Demonfire!",		""
    },
    {
	"blaze",
	{ 47, 103, 103, 103, 77, 47, 90, 103, 103,
          37, 103, 103, 103, 67, 37, 80, 103, 103 },
        { 2, 2, 2, 2, 2, 2, 2, 2, 2,
          2, 2, 2, 2, 2, 2, 2, 2, 2 },
        spell_blaze,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		TRUE,	SLOT(657),	100,	12,
        "bright flame",		"!Blaze!",		""
    },
    {
	"cascade",
	{ 103, 103, 103, 103, 103, 55, 103, 103, 103,
	  103, 103, 103, 103, 101, 45, 103, 103, 103 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1, 1, 1, 1 },
        spell_cascade,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_cascade,	TRUE,	SLOT(654),	50,	6,
	"cascade",		"The waves slowly wash away.", ""
    },
    {
	"outrage",
	{ 103, 103, 10, 103, 103, 103, 103, 103, 103,
          103, 103,  1, 103, 103, 103, 103, 103, 103 },
        { 2, 2, 2, 2, 2, 2, 2, 2, 2,
          2, 2, 2, 2, 2, 2, 2, 2, 2 },
        spell_outrage,		TAR_CHAR_SELF,		POS_STANDING,
	NULL,		FALSE,	SLOT(655),	100,	12,
	"",			"The rage leaves your body.", ""
    }, 
    {
        "sandstorm",
        { 101, 78, 103, 103, 103, 70, 103, 103, 103,
          90, 68, 103, 103, 103, 60, 103, 103, 103 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1, 1, 1, 1 },
        spell_sandstorm, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, TRUE, SLOT(601), 50, 6,
        "sandstorm", "!Sandstorm!", ""
    },
    {
       "swarm",
       { 103, 103, 103, 103, 103, 103, 70, 103, 103,
         103, 103, 103, 103, 103, 103, 60, 103, 103 },
       { 1, 1, 1, 1, 1, 1, 1, 1, 1, 
         1, 1, 1, 1, 1, 1, 1, 1, 1 },
        spell_swarm, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, TRUE, SLOT(610), 50, 6,
        "swarm", "!Swarm!",""
    },
    { 
        "death touch",
        { 103, 103, 103, 103, 103, 103, 103, 103, 103,
          103, 103, 103, 103, 103, 103, 101, 103, 103 },
        { 1, 1, 1, 1, 1, 1, 2, 1, 1, 
          1, 1, 1, 1, 1, 1, 2, 1, 1 },
        spell_death_touch, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, TRUE, SLOT(611), 50, 12,
        "death touch", "!Death Touch!",""
    },
    {
       "bloodbath",
       { 103, 103, 103, 103, 103, 103, 91, 103, 103,
         103, 103, 103, 103, 103, 103, 85, 103, 103 },
       { 1, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 1, 1, 1, 1, 1 },
       spell_bloodbath, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
       &gsn_bloodbath, TRUE, SLOT(612), 50, 12,
       "bloodbath", "The wound right below your heart disappears.",""
    },

    {
       "elemental fury",
       { 103, 103, 103, 103, 103, 103, 103, 103, 103,
         103, 90, 103, 103, 103, 90, 103, 103, 103 },
       { 1, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 1, 1, 1, 1, 1 },
       spell_elemental_fury, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
       NULL, TRUE, SLOT(608), 50, 6,
        "elemental fury", "!Elemental fury!", ""
    },
    {
	"hailstorm",
	{ 103, 103, 103, 103, 95, 103, 103, 103, 103,
          103, 103, 103, 103, 85, 101, 103, 103, 103 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1, 1, 1, 1 },
        spell_hailstorm, TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,	TRUE,	SLOT(641),	100,	12,
	"hailstorm",	"!Hailstorm!",	""
    },
    {
	"tornado",
	{ 103, 103, 103, 103, 103, 91, 103, 103, 103,
          103, 103, 103, 103, 103, 90, 103, 103, 103 },
        { 1, 1, 1, 1, 1, 2, 1, 1, 1,
          1, 1, 1, 1, 1, 2, 1, 1, 1 },
        spell_tornado,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,	TRUE,	SLOT(642),	50,	6,
	"tornado",	"!Tornado!",	""
    }, 
    {
	"detect evil",
	{ 22, 22, 102, 102, 10, 10, 17, 20, 103,
	  12, 12, 102, 102, 1, 1, 7, 11, 103 },
	{ 1, 1, 2, 2, 1, 1, 1, 1, 1,
	  1, 1, 2, 2, 1, 1, 1, 1, 1 },
	spell_detect_evil,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,		FALSE,	SLOT(18),	 25,	12,
	"",			"The red in your vision disappears.",	""
    },
    {
        "ultravision",
        { 103, 103, 90, 103, 103, 103, 103, 103, 101,
          103, 103, 80, 103, 103, 103, 103, 103, 90 },
        { 1, 1, 2, 1, 1, 1, 1, 1, 2, 
          1, 1, 2, 1, 1, 1, 1, 1, 2 },
        spell_ultravision, 	TAR_CHAR_SELF,		POS_STANDING,
        &gsn_ultravision,	FALSE, SLOT(630),	50, 12,
       "",			"Your vision is no longer perfect.",""
    },
    {
        "detect good",
	{ 23, 22, 102, 102, 10, 10, 17, 20, 103,
	  13, 12, 102, 102, 1, 1, 7, 11, 103 },
	{ 1, 1, 2, 2, 1, 1, 1, 1, 1,
	  1, 1, 2, 2, 1, 1, 1, 1, 1 },
        spell_detect_good,      TAR_CHAR_SELF,          POS_STANDING,
        NULL,		FALSE,	SLOT(513),        25,     12,
        "",                     "The gold in your vision disappears.",	""
    },

    {
	"detect hidden",
	{ 24, 22, 24, 102, 29, 26, 18, 15, 5,
	  14, 12, 14, 102, 19, 16, 8, 10, 1 },
	{ 1, 1, 2, 2, 1, 1, 1, 1, 1,
	  1, 1, 2, 2, 1, 1, 1, 1, 1 },
	spell_detect_hidden,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,		FALSE,	SLOT(44),	 25,	12,
	"",			"You feel less aware of your surroundings.",	
	""
    },

    {
	"detect invis",
	{ 25, 16, 11, 102, 25, 25, 4, 15, 5,
	  15, 6, 1, 102, 15, 15, 3, 10, 1 },
	{ 1, 1, 2, 2, 1, 1, 1, 1, 1,
	  1, 1, 2, 2, 1, 1, 1, 1, 1 },
	spell_detect_invis,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,		FALSE,	SLOT(19),	 25,	12,
	"",			"You no longer see invisible objects.",
	""
    },

    {
	"detect magic",
	{ 26, 11, 8, 102, 24, 21, 3, 103, 103,
	  16, 1, 1, 102, 14, 11, 2, 103, 103 },
	{ 1, 1, 2, 2, 1, 1, 1, 1, 1,
	  1, 1, 2, 2, 1, 1, 1, 1, 1 },
	spell_detect_magic,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,		FALSE,	SLOT(20),	 25,	12,
	"",			"The detect magic wears off.",	""
    },

    {
       	"true sight",
        { 103, 103, 103, 103, 103, 103, 103, 103, 103,
          30, 30, 103, 103, 30, 30, 103, 103, 103 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1, 1, 1, 1 },
        spell_true_sight,	TAR_CHAR_SELF,		POS_STANDING,
        NULL,		FALSE,  SLOT(610),	50,	12,
        "",			"You can no longer see so well.", ""
    },

    {
	"detect poison",
	{ 27, 14, 18, 102, 8, 11, 11, 103, 103,
	  17, 4, 8, 102, 1, 1, 1, 103, 103 },
	{ 1, 1, 2, 2, 1, 1, 1, 1, 1,
	  1, 1, 2, 2, 1, 1, 1, 1, 1 },
	spell_detect_poison,	TAR_OBJ_INV,		POS_STANDING,
	NULL,		TRUE,	SLOT(21),	 25,	12,
	"",			"!Detect Poison!",	""
    },

    {
	"dispel magic",
	{ 48, 48, 103, 103, 103, 36, 60, 103, 70,
	  38, 38, 103, 103, 103, 35, 50, 103, 60 },
	{ 1, 1, 2, 2, 2, 1, 2, 1, 2, 
	  1, 1, 2, 2, 2, 1, 2, 1, 2 },
	spell_dispel_magic,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		TRUE,	SLOT(59),	25,	6,
	"",			"!Dispel Magic!",	""
    },
    {
       "cleansing",
       { 103, 103, 103, 103, 101, 103, 103, 103, 103,
         103, 103, 103, 103, 90, 103, 103, 103, 103 },
       { 1, 1, 1, 1, 2, 1, 1, 1, 1,
         1, 1, 1, 1, 2, 1, 1, 1, 1 },
       spell_cleansing,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
       NULL,		TRUE,	SLOT(633),	25,	6,
       "",			"!Cleansing!",		""
    },
    {
	"earthquake",
	{ 83, 20, 103, 103, 36, 11, 103, 103, 103,
	  73, 10, 103, 103, 26, 1, 103, 103, 103 },
	{ 1, 1, 2, 2, 2, 1, 1, 1, 1,
	  1, 1, 2, 2, 2, 1, 1, 1, 1 },
	spell_earthquake,	TAR_IGNORE,		POS_FIGHTING,
	NULL,		TRUE,	SLOT(23),	40,	12,
	"earthquake",		"!Earthquake!",		""
    },

    {
	"empower",
	{ 36, 40, 103, 103, 103, 42, 103, 103, 103,
	  26, 30, 103, 103, 103, 32, 103, 103, 103 },
	{ 1, 1, 2, 2, 2, 1, 1, 1, 1,
	  1, 1, 2, 2, 2, 1, 1, 1, 1 },
	spell_empower,		TAR_IGNORE,	POS_RESTING,
	NULL,		TRUE,	SLOT(234),	 50,	12,
	"",			"!Empower!",	""
    },

    {
	"energy drain",
	{ 38, 44, 103, 103, 103, 47, 10, 103, 75,
	  28, 34, 103, 103, 103, 37, 1, 103, 65 },
	{ 1, 1, 2, 2, 2, 1, 1, 1, 2,
	  1, 1, 2, 2, 2, 1, 1, 1, 2 },
	spell_energy_drain,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		TRUE,	SLOT(25),	50,	12,
	"energy drain",		"!Energy Drain!",	""
    },

    {
	"faerie fire",
	{ 12, 103, 103, 103, 17, 2, 103, 103, 103,
	  2, 103, 103, 103, 7, 1, 103, 103, 103 },
	{ 1, 1, 2, 2, 1, 1, 2, 1, 1,
	  1, 1, 2, 2, 1, 1, 2, 1, 1 },
	spell_faerie_fire,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		TRUE,	SLOT(72),	 30,	6,
	"faerie fire",		"The pink aura around you fades away.",
	""
    },

    {
	"faerie fog",
	{ 28, 41, 103, 103, 35, 18, 103, 103, 103,
	  18, 31, 103, 103, 25, 8, 103, 103, 103 },
	{ 1, 1, 2, 2, 1, 1, 2, 2, 1,
	  1, 1, 2, 2, 1, 1, 2, 2, 1  },
	spell_faerie_fog,	TAR_IGNORE,		POS_STANDING,
	NULL,		TRUE,	SLOT(73),	50,	12,
	"faerie fog",		"!Faerie Fog!",		""
    },

    {
	"farsight",
	{ 4, 8, 103, 103, 15, 9, 6, 30, 20,
	  3, 7, 103, 103, 5, 8, 5, 28, 15 },
	{ 1, 1, 2, 2, 1, 1, 1, 1, 2,
	  1, 1, 2, 2, 1, 1, 1, 1, 2 },
	spell_farsight,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		FALSE,	SLOT(521),	30,	12,
	"",			"The green in your vision disappears.",
	""
    },	

    {
	"fireproof",
	{ 25, 24, 102, 102, 102, 21, 102, 103, 103,
	  15, 14, 102, 102, 102, 11, 102, 103, 103 },
	{ 1, 1, 2, 2, 2, 1, 2, 1, 1,
	  1, 1, 2, 2, 2, 1, 2, 1, 1 },
	spell_fireproof,	TAR_OBJ_INV,		POS_STANDING,
	NULL,		TRUE,	SLOT(523),	20,	12,
	"",			"",	"$p's protective aura fades."
    },
    {
	"corrupt potion",
	{ 103, 103, 103, 103, 103, 103, 103, 103, 70,
	  103, 103, 103, 103, 103, 103, 103, 103, 60 },
	{ 1, 1, 2, 2, 2, 1, 2, 1, 2,
	  1, 1, 2, 2, 2, 1, 2, 1, 2 },
	spell_corrupt_potion,	TAR_OBJ_INV,		POS_STANDING,
	NULL,		TRUE,	SLOT(523),	100,	24,
	"",			"",	""
    },
    {
	"fireshield",
	{ 50, 60, 102, 102, 60, 55, 102, 103, 103,
	  40, 50, 102, 102, 50, 45, 102, 103, 103 },
	{ 3, 3, 5, 5, 5, 3, 5, 1, 1,
	  3, 3, 5, 5, 5, 3, 5, 1, 1 },
	spell_fireshield,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		FALSE,	SLOT(411),	50,	12,
	"fireball",		"Your fiery shield gutters out.",
	""
    },

    {
	"leech",
        { 103, 103, 103, 103, 103, 103, 103, 103, 103,
          103, 103, 103, 103, 103, 103, 91, 103, 103 },
        { 1, 1, 1, 1, 1, 1, 2, 1, 1,
          1, 1, 1, 1, 1, 1, 2, 1, 1 },
        spell_leech,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_leech,	TRUE,	SLOT(644),	100,	12,
	"leech",		"The leech stops moving.",	""
    },
    {
	"fly",
	{ 19, 36, 102, 102, 31, 66, 33, 7, 103,
	  1, 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 1, 1, 2, 2, 2, 1, 1, 1, 1,
	  1, 1, 1, 1, 1, 1, 1, 1, 1 },
	spell_fly,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	&gsn_fly,		FALSE,	SLOT(56),	20,	12,
	"",			"You slowly float to the ground.",	""
    },

    {
        "frenzy",
	{ 66, 48, 103, 103, 103, 50, 55, 103, 85,
	  56, 38, 103, 103, 103, 40, 45, 75, 75 },
	{ 1, 1, 2, 2, 2, 1, 2, 2, 2,
	  1, 1, 2, 2, 2, 1, 2, 2, 2 },
        spell_frenzy,           TAR_CHAR_DEFENSIVE,     POS_STANDING,
        NULL,		FALSE,	SLOT(504),      50,     12,
        "",                     "Your rage ebbs.",	""
    },

    {
	"gate",
	{ 54, 34, 102, 102, 42, 55, 56, 103, 103,
	  44, 24, 101, 101, 32, 45, 46, 101, 101 },
	{ 1, 1, 2, 2, 2, 1, 1, 2, 2,
	  1, 1, 2, 2, 2, 1, 1, 2, 2 },
	spell_gate,		TAR_IGNORE,		POS_FIGHTING,
	NULL,		FALSE,	SLOT(83),	80,	12,
	"",			"!Gate!",		""
    },

    {
	"vigor",
	{ 103, 40, 103, 50, 103, 30, 103, 50, 103,
	  103, 30, 103, 40, 103, 20, 103, 40, 103 },
	{ 1, 1, 2, 2, 2, 1, 2, 2, 2,
	  1, 1, 2, 2, 2, 1, 2, 2, 2 },
	spell_vigor,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		FALSE,	SLOT(39),	30,	12,
	"",			"You no longer feel that vigorous.",	""
    },
    {
        "mastermind",
	{ 30, 60, 103, 103, 103, 80, 103, 103, 103,
	  20, 50, 103, 103, 103, 70, 103, 103, 103 },
	{ 1, 1, 2, 2, 2, 1, 2, 2, 2,
	  1, 1, 2, 2, 2, 1, 2, 2, 2 },
	spell_mastermind,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		FALSE,	SLOT(39),	30,	12,
	"",			"You no longer feel that astute.",	""
    },

    {
        "protective force",
        {103, 103, 103, 103, 60, 103, 103, 103, 103,
         103, 103, 103, 103, 50, 103, 103, 103, 103 },
        {1, 1, 1, 1, 2, 1, 1, 1, 1, 
         1, 1, 1, 1, 2, 1, 1, 1, 1 },
        spell_protective_force,		TAR_CHAR_SELF,	POS_STANDING,
        &gsn_protective_force,	FALSE,	SLOT(631),	50,	12,
        "",			"Nature is no longer protecting you.",	""
    },
    {
	"resistance",
        {103, 103, 103, 103, 90, 103, 103, 103, 103,
         103, 103, 103, 103, 80, 103, 103, 103, 103 },
        {1, 1, 1, 1, 2, 1, 1, 1, 1,
         1, 1, 1, 1, 2, 1, 1, 1, 1 },
        spell_resistance,		TAR_CHAR_SELF,	POS_STANDING,
        &gsn_resistance,	FALSE,	SLOT(632),	50,	12,
        "",			"You are no longer that resistant.", ""
    },

    {
	"haste",
	{ 42, 54, 103, 103, 60, 26, 52, 49, 49,
	  32, 44, 103, 103, 50, 16, 42, 44, 44 },
	{ 1, 1, 2, 2, 2, 1, 1, 2, 2, 
	  1, 1, 2, 2, 2, 1, 1, 2, 2 },
	spell_haste,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		FALSE,	SLOT(502),	30,	12,
	"",			"You feel yourself slow down.",	""
    },

    {
	"heal",
	{ 102, 32, 102, 102, 65, 55, 102, 103, 103,
	  102, 22, 102, 102, 55, 45, 102, 103, 103 },
	{ 1, 1, 2, 2, 2, 1, 2, 1, 1,
	  1, 1, 2, 2, 2, 1, 2, 1, 1 },
	spell_heal,		TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,		FALSE,	SLOT(28),	50,	12,
	"",			"!Heal!",		""
    },
  
    {
	"heat metal",
	{ 73, 42, 103, 103, 103, 22, 103, 103, 103,
	  63, 32, 103, 103, 103, 12, 103, 103, 103 },
	{ 1, 1, 2, 2, 2, 1, 1, 1, 1,
	  1, 1, 2, 2, 2, 1, 1, 1, 1 },
	spell_heat_metal,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		TRUE,	SLOT(516), 	25,	12,
	"spell",		"!Heat Metal!",		""
    },

    {
	"iceshield",
	{ 30, 30, 102, 102, 56, 35, 102, 103, 103,
	  20, 20, 102, 102, 46, 25, 102, 103, 103 },
	{ 3, 3, 5, 5, 5, 3, 5, 1, 1,
	  3, 3, 5, 5, 5, 3, 5, 1, 1 },
	spell_iceshield,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		FALSE,	SLOT(410),	50,	12,
	"chilling touch",	"Your icy shield slowly melts away.",
	""
    },

    {
	"identify",
	{ 30, 32, 35, 102, 46, 42, 28, 20, 20,
	  20, 22, 25, 102, 36, 32, 18, 10, 10 },
	{ 1, 1, 2, 2, 1, 1, 1, 1, 1,
	  1, 1, 2, 2, 1, 1, 1, 1, 1 },
	spell_identify,		TAR_OBJ_INV,		POS_STANDING,
	NULL,		TRUE,	SLOT(53),	20,	12,
	"",			"!Identify!",		""
    },

    {
	"infravision",
	{ 18, 26, 10, 102, 102, 18, 12, 5, 5,
	  8, 16, 5, 102, 102, 8, 2, 1, 1 },
	{ 1, 1, 2, 2, 2, 1, 2, 2, 2,
	  1, 1, 2, 2, 2, 1, 2, 2, 2 },
	spell_infravision,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		FALSE,	SLOT(77),	 25,	12,
	"",			"You no longer see in the dark.",	""
    },

    {
	"invisibility",
	{ 9, 37, 17, 102, 12, 39, 13, 103, 10,
	  8, 27, 7, 102, 2, 29, 3, 103, 5 },
	{ 1, 1, 2, 2, 2, 1, 1, 1, 2,
	  1, 1, 2, 2, 2, 1, 1, 1, 2 },
	spell_invis,		TAR_OBJ_CHAR_DEF,	POS_STANDING,
	&gsn_invis,	FALSE,	SLOT(29),	 25,	12,
	"",			"You are no longer invisible.",		
	"$p fades into view."
    },

    {
	"know alignment",
	{ 24, 17, 102, 102, 36, 27, 33, 103, 103,
	  14, 7, 102, 102, 26, 17, 23, 103, 103 },
	{ 1, 1, 2, 2, 1, 1, 1, 1, 1,
	  1, 1, 2, 2, 1, 1, 1, 1, 1 },
	spell_know_alignment,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,		FALSE,	SLOT(58),	 10,	12,
	"",			"!Know Alignment!",	""
    },

    {
	"locate object",
	{ 17, 30, 102, 102, 38, 35, 24, 25, 103,
	  7, 20, 102, 102, 28, 25, 14, 20, 103 },
	{ 1, 1, 2, 2, 1, 1, 1, 1, 1,
	  1, 1, 2, 2, 1, 1, 1, 1, 1 },
	spell_locate_object,	TAR_IGNORE,		POS_STANDING,
	NULL,		TRUE,	SLOT(31),	20,	12,
	"",			"!Locate Object!",	""
    },

    {
	"magic missile",
	{ 1, 103, 103, 103, 103, 103, 103, 103, 103,
	  1, 103, 103, 103, 103, 103, 103, 103, 103 },
	{ 1, 1, 2, 2, 2, 1, 1, 1, 1,
	  1, 1, 2, 2, 2, 1, 1, 1, 1 },
	spell_magic_missile,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		TRUE,	SLOT(32),	25,	12,
	"magic missile",	"!Magic Missile!",	""
    },

    {
	"mass healing",
	{ 102, 75, 102, 102, 103, 85, 102, 103, 103,
	  102, 65, 102, 102, 103, 75, 102, 103, 103 },
	{ 2, 2, 4, 4, 2, 2, 2, 1, 1,
	  2, 2, 4, 4, 2, 2, 2, 1, 1 },
	spell_mass_healing,	TAR_IGNORE,		POS_STANDING,
	NULL,		FALSE,	SLOT(508),	100,	24,
	"",			"!Mass Healing!",	""
    },

    {
	"mass invis",
	{ 43, 49, 103, 103, 103, 63, 54, 103, 103,
	  33, 39, 103, 103, 103, 53, 44, 103, 103 },
	{ 1, 1, 2, 2, 2, 1, 2, 1, 1,
	  1, 1, 2, 2, 2, 1, 2, 1, 1 },
	spell_mass_invis,	TAR_IGNORE,		POS_STANDING,
	&gsn_mass_invis,	FALSE,	SLOT(69),	50,	12,
	"",			"You are no longer invisible.",		""
    },

    {
	"pass door",
	{ 48, 63, 49, 102, 60, 62, 42, 63, 51,
	  38, 53, 39, 102, 50, 52, 32, 53, 41 },
	{ 1, 1, 2, 2, 2, 1, 1, 1, 2,
	  1, 1, 2, 2, 2, 1, 1, 1, 2 },
	spell_pass_door,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,		FALSE,	SLOT(74),	20,	12,
	"",			"You feel solid again.",	""
    },

    {
	"plague",
	{ 46, 34, 103, 103, 103, 39, 65, 103, 76,
	  36, 24, 103, 103, 103, 29, 55, 103, 71 },
	{ 1, 1, 2, 2, 2, 1, 1, 1, 2,
	  1, 1, 2, 2, 2, 1, 1, 1, 2 },
	spell_plague,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_plague,	TRUE,	SLOT(503),	25,	6,
	"sickness",		"Your sores vanish.",	""
    },

    {
	"poison",
	{ 33, 24, 103, 103, 49, 27, 22, 103, 34,
	  23, 14, 103, 103, 39, 17, 12, 103, 24 },
	{ 1, 1, 2, 2, 2, 1, 1, 1, 2,
	  1, 1, 2, 2, 2, 1, 1, 1, 2 },
	spell_poison,		TAR_OBJ_CHAR_OFF,	POS_FIGHTING,
	&gsn_poison,	TRUE,	SLOT(33),	20,	6,
	"poison",		"You feel less sick.",	
	"The poison on $p dries up."
    },
    {
	"blade poison",
        { 103, 103, 103, 103, 103, 103, 103, 103, 91,
          103, 103, 103, 103, 103, 103, 103, 103, 81 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 2,
          1, 1, 1, 1, 1, 1, 1, 1, 2 },
        spell_blade_poison,	TAR_OBJ_INV,		POS_STANDING,
        &gsn_blade_poison, TRUE, SLOT(651),	75,	12,
        "blade poison",		"You feel the effects of the blade poison subside.",
        "The blade poison on $p vanishes."
    },

    {
	"protection evil",
	{ 103, 18, 102, 102, 103, 19, 66, 103, 103,
	  103, 8, 102, 102, 103, 9, 56, 101, 103 },
	{ 1, 1, 2, 2, 2, 1, 2, 2, 1,
	  1, 1, 2, 2, 2, 1, 2, 2, 1 },
	spell_protection_evil,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,		FALSE,	SLOT(34), 	35,	12,
	"",			"You feel less protected.",	""
    },

    {
        "protection good",
	{ 103, 18, 102, 102, 103, 19, 66, 103, 103,
	  103, 8, 102, 102, 103, 9, 56, 103, 103 },
	{ 1, 1, 2, 2, 2, 1, 2, 2, 1,
	  1, 1, 2, 2, 2, 1, 2, 2, 1 },
        spell_protection_good,  TAR_CHAR_SELF,          POS_STANDING,
        NULL,		FALSE,	SLOT(514),	35,	12,
        "",                     "You feel less protected.",	""
    },
    
    {
       "protection neutral",
       { 103,18,103,103,103,19,66,103,103, 
         103,8,103,103,103,9,56,103,103 },
       { 1, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 1, 1, 1, 1, 1 },
       spell_protection_neutral, TAR_CHAR_SELF, POS_STANDING,
       NULL, FALSE, SLOT(607), 35, 12,
       "", "You are no longer protected from neutrality.", ""
    }, 

    {
	"protection voodoo",
	{ 102, 102, 102, 102, 102, 102, 102, 103, 103,
	  102, 102, 102, 102, 102, 102, 102, 103, 103 },
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1,
	  1, 1, 1, 1, 1, 1, 1, 1, 1 },
	spell_protection_voodoo, TAR_CHAR_SELF,		POS_STANDING,
	NULL,		FALSE,	SLOT(345),	35,	12,
	"",			"",	""
    },

    {
	"quest pill",
	{ 110, 110, 110, 110, 110, 110, 110, 110, 110,
	  110, 110, 110, 110, 110, 110, 110, 110, 110 },
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1,
	  1, 1, 1, 1, 1, 1, 1, 1, 1 },
	spell_quest_pill,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,		TRUE,	SLOT(530),	 35,	18,
	"",			"!Quest Pill!",	""
    },

    {
	"refresh",
	{ 16, 9, 102, 102, 18, 10, 16, 10, 103,
	  6, 4, 102, 102, 8, 1, 6, 7, 103 },
	{ 1, 1, 2, 2, 2, 1, 2, 1, 1,
	  1, 1, 2, 2, 2, 1, 2, 1, 1 },
	spell_refresh,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		FALSE,	SLOT(81),	30,	12,
	"refresh",		"!Refresh!",		""
    },

    {
	"remove curse",
	{ 103, 35, 102, 102, 103, 49, 102, 103, 103,
	  103, 25, 102, 102, 103, 39, 102, 103, 103 },
	{ 1, 1, 2, 2, 2, 1, 2, 1, 1,
	  1, 1, 2, 2, 2, 1, 2, 1, 1 },
	spell_remove_curse,	TAR_OBJ_CHAR_DEF,	POS_STANDING,
	NULL,		FALSE,	SLOT(35),	 25,	12,
	"",			"!Remove Curse!",	""
    },

    {
	"restore mana",
	{ 102, 102, 102, 102, 102, 102, 102, 103, 103,
	  102, 102, 102, 102, 102, 102, 102, 103, 103 },
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1,
	  1, 1, 1, 1, 1, 1, 1, 1, 1 },
	spell_restore_mana,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		FALSE,	SLOT(251),	1,	12,
	"restore mana",		"!Restore Mana!",	""
    },

    {
	"resurrect",
	{ 103, 103, 103, 103, 103, 103, 28, 103, 103,
	  103, 103, 103, 103, 103, 103, 18, 103, 103 },
	{ 1, 1, 2, 2, 2, 1, 1, 1, 1,
	  1, 1, 2, 2, 2, 1, 1, 1, 1 },
	spell_resurrect,	TAR_IGNORE,		POS_STANDING,
	NULL,		TRUE,	SLOT(235),	 35,	12,
	"",			"!Resurrect!",	""
    },

    {
	"sanctuary",
	{ 52, 39, 103, 103, 103, 40, 103, 103, 103,
	  42, 29, 103, 103, 103, 30, 103, 103, 103 },
	{ 1, 1, 2, 2, 2, 1, 2, 1, 1,
	  1, 1, 2, 2, 2, 1, 2, 1, 1 },
	spell_sanctuary,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	&gsn_sanctuary,	FALSE,	SLOT(36),	75,	12,
	"",			"The white aura around your body fades.",
	""
    },

    {
	"force shield",
	{ 40, 60, 102, 102, 88, 70, 102, 103, 103,
	  30, 50, 102, 102, 78, 60, 102, 103, 103 },
	{ 1, 1, 2, 2, 2, 1, 2, 2, 1,
	  1, 1, 2, 2, 2, 1, 2, 2, 1 },
	spell_shield,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	&gsn_shield,		FALSE,	SLOT(67),	30,	12,
	"force projection",			"Your {yg{Yl{Wowi{Yn{yg{x force shield shimmers then fades away.",
	""
    },

    {
	"shockshield", 
	{ 60, 40, 102, 102, 80, 90, 60, 103, 103,
	  50, 30, 102, 102, 70, 80, 50, 103, 103 },
	{ 3, 3, 5, 5, 5, 3, 5, 5, 1,
	  3, 3, 5, 5, 5, 3, 5, 5, 1 },
	spell_shockshield,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		FALSE,	SLOT(412),	50,	12,
	"lightning bolt",	"Your crackling shield sizzles and fades.",
	""
    },
    {
	"shadowshield",
	{ 65, 65, 102, 102, 102, 103, 65, 103, 65,
	  55, 55, 102, 102, 102, 103, 55, 103, 55 },
	{ 3, 3, 5, 5, 5, 3, 5, 5, 4,
	  3, 3, 5, 5, 5, 3, 5, 5, 4 },
	spell_shadowshield,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		FALSE,	SLOT(602),	50,	12,
	"shadowy force",	"The shadows surrounding you fade away.",
	""
    },
    {
	"bladebarrier",
	{ 70, 103, 102, 102, 102, 103, 102, 103, 103,
	  60, 103, 102, 102, 102, 103, 102, 103, 103 },
	{ 3, 3, 5, 5, 5, 3, 5, 5, 1,
	  3, 3, 5, 5, 5, 3, 5, 5, 1 },
	spell_bladebarrier,	TAR_CHAR_SELF,	POS_STANDING,
	NULL,		FALSE,	SLOT(603),	50,	12,
	"blade",	"Your bladebarrier vanishes.",
	""
    },
    {
	"prismaticshield",
	{ 72, 65, 102, 102, 102, 103, 102, 78, 103,
	  62, 55, 102, 102, 102, 103, 102, 68, 103 },
	{ 3, 3, 5, 5, 5, 3, 5, 5, 1,
	  3, 3, 5, 5, 5, 3, 5, 5, 1 },
	spell_prismaticshield,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		FALSE,	SLOT(604),	50,	12,
	"sliver of light",	"Your are no longer surrounded by rays of light.",
	""
    },
    {
	"acidshield",
	{ 75, 103, 102, 102, 102, 103, 102, 103, 103,
	  65, 103, 102, 102, 102, 103, 102, 103, 103 },
	{ 3, 3, 5, 5, 5, 3, 5, 5, 1,
	  3, 3, 5, 5, 5, 3, 5, 5, 1 },
	spell_acidshield,	TAR_CHAR_SELF,	POS_STANDING,
	NULL,		FALSE,	SLOT(605),	50,	12,
	"acidic shield",	"Your {yacidic{x shield vanishes.",
	""
    },
    {
	"earthshield",
	{ 78, 103, 102, 102, 85, 85, 102, 103, 103,
	  68, 103, 102, 102, 75, 75, 102, 103, 103 },
	{ 3, 3, 5, 5, 5, 3, 5, 5, 1,
	  3, 3, 5, 5, 5, 3, 5, 5, 1 },
	spell_earthshield,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,		FALSE,	SLOT(606),	50,	12,
	"shard",	"You are no longer protected by an earthshield.",
	""
    },
    {
        "mana shield",
        {103, 103, 103, 103, 103, 103, 103, 103, 103,
         101, 103, 103, 103, 103, 103, 103, 103, 103 },
        { 3, 3, 3, 3, 3, 3, 3, 3, 3,
          3, 3, 3, 3, 3, 3, 3, 3, 3 },
        spell_mana_shield,	TAR_CHAR_SELF,	POS_STANDING,
        NULL,		FALSE,	SLOT(634),	150,	12,
        "",		"Your mana shield vanishes into thin air.", ""
    },
    {
	"sleep",
	{ 20, 103, 103, 103, 103, 46, 15, 103, 30,
	  10, 95, 103, 103, 103, 36, 5, 103, 20 },
	{ 1, 1, 2, 2, 2, 1, 1, 1, 2,
	  1, 1, 2, 2, 2, 1, 1, 1, 2 },
	spell_sleep,		TAR_CHAR_OFFENSIVE,	POS_STANDING,
	&gsn_sleep,	TRUE,	SLOT(38),	50,	24,
	"",			"You feel less tired.",	""
    },
    {
       "sever",
       { 103, 75, 103, 103, 103, 103, 103, 103, 103,
         103, 50, 103, 103, 103, 103, 103, 103, 103 },
        { 1, 3, 1, 1, 1, 1, 1, 1, 1,
          1, 3, 1, 1, 1, 1, 1, 1, 1 },
       spell_sever,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
       &gsn_sever,	TRUE,	SLOT(608),	75,	12,
       "",	"{GYou are no longer severed.{x", ""
    },
    {
        "slow",
	{ 45, 40, 103, 103, 103, 49, 51, 65, 75,
	  35, 30, 103, 103, 103, 39, 41, 55, 65 },
	{ 1, 1, 2, 2, 2, 1, 1, 1, 2,
	  1, 1, 2, 2, 2, 1, 1, 1, 2 },
        spell_slow,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        NULL,		TRUE,	SLOT(515),      40,     12,
        "",                     "You feel yourself speed up.",	""
    },
    {
        "grow",
        { 103, 103, 103, 103, 70, 103, 103, 103, 103,
          103, 103, 103, 103, 60, 103, 103, 103, 103 },
        { 1, 1, 1, 1, 2, 1, 1, 1, 1,
          1, 1, 1, 1, 2, 1, 1, 1, 1 },
        spell_grow,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
        NULL,		TRUE,	SLOT(633),	40,	6,
        "",			"The vines disappear.",		""
    },
    {
	"stone skin",
	{ 49, 70, 102, 102, 103, 90, 102, 103, 103,
	  39, 60, 102, 102, 90, 80, 102, 103, 103 },
	{ 1, 1, 2, 2, 2, 1, 2, 2, 1,
	  1, 1, 2, 2, 2, 1, 2, 2, 1 },
	spell_stone_skin,	TAR_CHAR_DEFENSIVE,		POS_STANDING,
	NULL,		FALSE,	SLOT(66),	75,	12,
	"",			"Your skin feels soft again.",	""
    },

    {
	"summon",
	{ 48, 23, 102, 102, 33, 56, 51, 103, 103,
	  38, 13, 102, 102, 23, 46, 41, 103, 101 },
	{ 1, 1, 2, 2, 2, 1, 1, 1, 2,
	  1, 1, 2, 2, 2, 1, 1, 1, 2 },
	spell_summon,		TAR_IGNORE,		POS_STANDING,
	NULL,		FALSE,	SLOT(40),	50,	12,
	"",			"!Summon!",		""
    },

    {
	"teleport",
	{ 24, 44, 102, 102, 59, 67, 59, 103, 103,
	  14, 34, 102, 102, 49, 57, 49, 103, 103 },
	{ 1, 1, 2, 2, 2, 1, 1, 1, 1,
	  1, 1, 2, 2, 2, 1, 1, 1, 1 },
	spell_teleport,		TAR_CHAR_SELF,		POS_FIGHTING,
	NULL,	 	TRUE,	SLOT( 2),	35,	12,
	"",			"!Teleport!",		""
    },

    {
	"transport",
	{ 25, 46, 102, 102, 45, 53, 51, 103, 103,
	  15, 36, 102, 102, 35, 43, 41, 103, 103 },
	{ 1, 1, 2, 2, 2, 1, 1, 1, 1,
	  1, 1, 2, 2, 2, 1, 1, 1, 1 },
	spell_transport,	TAR_OBJ_TRAN,		POS_STANDING,
	NULL,		FALSE,	SLOT(524),	30,	12,
	"",			"!Transport!",		""
    },

    {
	"voodoo",
	{ 102, 80, 102, 102, 102, 70, 65, 103, 95,
	  102, 70, 102, 102, 102, 60, 30, 103, 85 },
	{ 1, 1, 1, 1, 1, 1, 1, 1, 2,
	  1, 1, 1, 1, 1, 1, 1, 1, 2 },
	spell_voodoo,		TAR_IGNORE,		POS_STANDING,
	NULL,		TRUE,	SLOT(286),	80,	12,
	"",			"!Voodoo!",		""
    },

    {
	"weaken",
	{ 21, 28, 103, 103, 103, 31, 25, 34, 59,
	  11, 18, 103, 103, 103, 21, 15, 30, 49 },
	{ 1, 1, 2, 2, 2, 1, 1, 1, 2,
	  1, 1, 2, 2, 2, 1, 1, 1, 2 },
	spell_weaken,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		TRUE,	SLOT(68),	30,	12,
	"spell",		"You feel stronger.",	""
    },
    {
	"shrink",
	{ 60, 95, 103, 103, 103, 77, 103, 103, 103,
          50, 85, 103, 103, 103, 67, 103, 103, 103 },
        { 2, 2, 2, 2, 2, 2, 2, 2, 2,
          2, 2, 2, 2, 2, 2, 2, 2, 2 },
        spell_shrink,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
        NULL,		TRUE,	SLOT(658),	50,	6,
        "",			"Your head jerks in pain as your body returns to its true size.", ""
    },

/*
 * Dragon breath
 */
    {
	"acid breath",
	{ 62, 103, 103, 103, 103, 103, 103, 103, 103,
	  52, 103, 103, 103, 103, 103, 103, 103, 103 },
	{ 1, 1, 2, 2, 2, 1, 1, 1, 1,
	  1, 1, 2, 2, 2, 1, 1, 1, 1 },
	spell_acid_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		TRUE,	SLOT(200),	100,	12,
	"blast of acid",	"!Acid Breath!",	""
    },

    {
	"fire breath",
	{ 80, 103, 103, 103, 103, 103, 103, 103, 103,
	  70, 103, 103, 103, 103, 103, 103, 103, 103 },
	{ 1, 1, 2, 2, 2, 1, 1, 1, 1,
	  1, 1, 2, 2, 2, 1, 1, 1, 1 },
	spell_fire_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		TRUE,	SLOT(201),	100,	12,
	"blast of flame",	"The smoke leaves your eyes.",	""
    },

    {
	"frost breath",
	{ 67, 103, 103, 103, 103, 103, 103, 103, 103,
	  57, 103, 103, 103, 103, 103, 103, 103, 103 },
	{ 1, 1, 2, 2, 2, 1, 1, 1, 1,
	  1, 1, 2, 2, 2, 1, 1, 1, 1 },
	spell_frost_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		TRUE,	SLOT(202),	100,	12,
	"blast of frost",	"!Frost Breath!",	""
    },

    {
	"gas breath",
	{ 77, 103, 103, 103, 103, 103, 103, 103, 103,
	  67, 103, 103, 103, 103, 103, 103, 103, 103 },
	{ 1, 1, 2, 2, 2, 1, 1, 1, 1,
	  1, 1, 2, 2, 2, 1, 1, 1, 1 },
	spell_gas_breath,	TAR_IGNORE,		POS_FIGHTING,
	NULL,		TRUE,	SLOT(203),	100,	12,
	"blast of gas",		"!Gas Breath!",		""
    },

    {
	"lightning breath",
	{ 74, 103, 103, 103, 103, 103, 103, 103, 103,
	  64, 103, 103, 103, 103, 103, 103, 103, 103 },
	{ 1, 1, 2, 2, 2, 1, 1, 1, 1,
	  1, 1, 2, 2, 2, 1, 1, 1, 1 },
	spell_lightning_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,		TRUE,	SLOT(204),	100,	12,
	"blast of lightning",	"!Lightning Breath!",	""
    },

/*
 * Spells for mega1.are from Glop/Erkenbrand.
 */
    {
        "general purpose",
	{ 104, 104, 104, 104, 104, 104, 104, 104, 104,
	  104, 104, 104, 104, 104, 104, 104, 104, 104 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0 },
        spell_general_purpose,  TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        NULL,		TRUE,	SLOT(401),      0,      12,
        "general purpose ammo", "!General Purpose Ammo!",	""
    },
 
    {
        "high explosive",
	{ 104, 104, 104, 104, 104, 104, 104, 104, 104,
	  104, 104, 104, 104, 104, 104, 104, 104, 104 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0 },
        spell_high_explosive,   TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        NULL,		TRUE,	SLOT(402),      0,      12,
        "high explosive ammo",  "!High Explosive Ammo!",	""
    },


/* combat and weapons skills */


    {
	"axe",
	{ 103, 103, 103, 1, 103, 103, 103, 103, 103,
	  103, 103, 103, 1, 103, 103, 103, 103, 103, },
	{ 0, 0, 0, 3, 0, 0, 0, 0, 0,
	  0, 0, 0, 2, 0, 0, 0, 0, 0 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_axe,	TRUE,	SLOT( 0),       0,      0,
        "",                     "!Axe!",		""
    },

    {
        "dagger",
	{ 1, 103, 1, 1, 1, 103, 1, 103, 1,
	  1, 103, 1, 1, 1, 103, 1, 103, 1 },
	{ 2, 0, 2, 1, 2, 0, 1, 0, 2,
	  2, 0, 1, 1, 1, 0, 1, 0, 1 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_dagger,	TRUE,	SLOT( 0),       0,      0,
        "",                     "!Dagger!",		""
    },
 
    {
	"flail",
	{ 103, 1, 1, 1, 1, 1, 1, 103, 103,
	  103, 1, 1, 1, 1, 1, 1, 103, 103 },
	{ 0, 3, 6, 3, 4, 3, 5, 0, 0,
	  0, 2, 5, 2, 3, 2, 4, 0, 0 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_flail,	TRUE,	SLOT( 0),       0,      0,
        "",                     "!Flail!",		""
    },

    {
	"mace",
	{ 103, 1, 1, 1, 1, 1, 1, 103, 103,
	  103, 1, 1, 1, 1, 1, 1, 103, 103 },
	{ 0, 1, 3, 2, 3, 2, 3, 0, 0,
	  0, 1, 3, 1, 2, 1, 2, 0, 0 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_mace,	TRUE,	SLOT( 0),       0,      0,
        "",                     "!Mace!",		""
    },

    {
	"polearm",
	{ 103, 103, 1, 1, 103, 1, 1, 103, 103,
	  103, 103, 1, 1, 103, 1, 1, 103, 103 },
	{ 0, 0, 6, 3, 0, 2, 5, 0, 0,
	  0, 0, 5, 2, 0, 1, 4, 0, 0 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_polearm,	TRUE,	SLOT( 0),       0,      0,
        "",                     "!Polearm!",		""
    },
    
    {
	"shield block",
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1,
	  1, 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 6, 4, 6, 1, 2, 4, 3, 3, 6,
	  5, 3, 5, 1, 1, 3, 2, 2, 5 },
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_shield_block,	TRUE,	SLOT(0),	0,	0,
	"",			"!Shield!",		""
    },
    
    {
        "shield levitation",
        { 103, 101, 103, 103, 103, 90, 101, 103, 103,
          103, 90, 103, 103, 101, 80, 90, 103, 101 },
        { 0, 4, 0, 0, 0, 4, 4, 0, 0, 
          0, 3, 0, 0, 5, 3, 3, 0, 5 },
        spell_null,		TAR_IGNORE,		POS_FIGHTING,
 	&gsn_shield_levitation,	TRUE,	SLOT(0),	0,	0,
        "",		"!Shield Levitation!",		""
    },
 
    {
	"spear",
	{ 1, 1, 1, 1, 1, 1, 1, 103, 1,
	  1, 1, 1, 1, 1, 1, 1, 103, 1 },
	{ 4, 4, 4, 2, 1, 3, 5, 0, 4,
	  3, 3, 3, 1, 1, 2, 4, 0, 3 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_spear,	TRUE,	SLOT( 0),       0,      0,
        "",                     "!Spear!",		""
    },

    {
	"sword",
	{ 103, 103, 1, 1, 103, 103, 1, 103, 1,
	  103, 103, 1, 1, 103, 103, 1, 103, 1 },
	{ 0, 0, 3, 1, 0, 0, 3, 0, 3,
	  0, 0, 2, 1, 0, 0, 2, 0, 2 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_sword,	TRUE,	SLOT( 0),       0,      0,
        "",                     "!sword!",		""
    },

    {
	"whip",
	{ 1, 1, 1, 1, 1, 1, 1, 103, 1,
	  1, 1, 1, 1, 1, 1, 1, 103, 1 },
	{ 6, 5, 5, 3, 3, 4, 5, 0, 5,
	  5, 4, 4, 3, 2, 3, 4, 0, 4 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_whip,	TRUE,	SLOT( 0),       0,      0,
        "",                     "!Whip!",	""
    },
    {
	"scribe",
	{ 103, 103, 103, 103, 103, 60, 103, 103, 103,
      	  103, 103, 103, 103, 103, 50, 103, 103, 103 },
        { 0, 0, 0, 0, 0, 4, 0, 0, 0,
	  0, 0, 0, 0, 0, 3, 0, 0, 0 },
        spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_scribe,	TRUE,	SLOT(0),	0,	12,
	"",			"!Scribe!",	""
    },
    {
	"brew",
	{ 103, 103, 103, 103, 103, 60, 103, 103, 103,
      	  103, 103, 103, 103, 103, 50, 103, 103, 103 },
        { 0, 0, 0, 0, 0, 4, 0, 0, 0,
	  0, 0, 0, 0, 0, 3, 0, 0, 0 },
        spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_brew,	TRUE,	SLOT(0),	0,	12,
	"",			"!Brew!",	""
    },
    {
        "backstab",
	{ 103, 103, 1, 103, 103, 103, 7, 103, 103,
	  103, 103, 1, 103, 103, 103, 6, 103, 103 },
	{ 0, 0, 2, 0, 0, 0, 2, 0, 0,
	  0, 0, 2, 0, 0, 0, 1, 0, 0 },
        spell_null,             TAR_IGNORE,             POS_STANDING,
        &gsn_backstab,	TRUE,	SLOT( 0),        0,     24,
        "backstab",             "!Backstab!",		""
    },
    {
	"ambush",
	{ 103, 103, 103, 103, 91, 103, 103, 103, 103,
          103, 103, 103, 103, 81, 103, 103, 103, 103 },
        { 0, 0, 0, 0, 4, 0, 0, 0, 0,
          0, 0, 0, 0, 3, 0, 0, 0, 0 },
        spell_null,		TAR_IGNORE,		POS_STANDING,
        &gsn_ambush,	TRUE,	SLOT( 0),	0,	12,
        "unexpected attack",	"You leave your hiding place.",		""
    },
    {
	"charge",
        { 103, 103, 103, 70, 103, 103, 103, 103, 103,
          103, 103, 103, 60, 103, 103, 103, 103, 103 },
        { 0, 0, 0, 4, 0, 0, 0, 0, 0,
          0, 0, 0, 3, 0, 0, 0, 0, 0 },
        spell_null,	TAR_IGNORE,	POS_STANDING,
        &gsn_charge,	TRUE,	SLOT(0),	0,	24,
        "charge",	"!Charge!",	""
    },
    {
  	"blackjack",
        {103, 103, 85, 103, 103, 103, 103, 103, 103,
         103, 103, 75, 103, 103, 103, 103, 103, 103 },
        { 0, 0, 3, 0, 0, 0, 0, 0, 0,
          0, 0, 2, 0, 0, 0, 0, 0, 0 },
        spell_null,		TAR_IGNORE,		POS_STANDING,
        &gsn_blackjack,	TRUE,	SLOT(0),	0,	24,
        "blackjack",		"You feel the pain in your skull subside.",
        ""
    },
    {
       "assassinate",
       { 103, 103, 103, 103, 103, 103, 103, 103, 101,
         103, 103, 103, 103, 103, 103, 103, 103, 90 },
       { 0, 0, 0, 0, 0, 0, 0, 0, 5,
         0, 0, 0, 0, 0, 0, 0, 0, 4 },
        spell_null,     TAR_IGNORE, POS_STANDING,
        &gsn_assassinate, TRUE, SLOT( 0), 0, 24,
        "assassination",         "Your weaknesses are no longer exposed.", ""
    },
    {
	"bash",
	{ 103, 103, 103, 1, 103, 103, 103, 50, 103,
	  103, 103, 103, 1, 103, 103, 103, 40, 103 },
	{ 0, 0, 0, 2, 0, 0, 0, 3, 0,
	  0, 0, 0, 1, 0, 0, 0, 2, 0 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_bash,	TRUE,	SLOT( 0),       0,      12,
        "bash",                 "!Bash!",		""
    },
    
    {
	"chop",
        { 103, 103, 103, 103, 103, 103, 103, 45, 103,
          103, 103, 103, 103, 103, 103, 103, 35, 103 },
        { 0, 0, 0, 0, 0, 0, 0, 4, 0,
          0, 0, 0, 0, 0, 0, 0, 3, 0 },
        spell_null,		TAR_IGNORE,		POS_FIGHTING,
        &gsn_chop,	TRUE,	SLOT( 0),	0,	12,
        "chop",			"Your hands are no longer that limp.",""
    },
    {
       "shield smash",
       { 103, 103, 103, 50, 103, 103, 103, 75, 103,
         103, 103, 103, 40, 103, 103, 103, 65, 103 },
       { 0, 0, 0, 3, 0, 0, 0, 4, 0,
         0, 0, 0, 2, 0, 0, 0, 3, 0 },
       spell_null,	TAR_IGNORE,	POS_FIGHTING,
       &gsn_shield_smash,	TRUE,	SLOT( 0),	0, 12,
       "shield smash",		"!Shield Smash!",	""
    },
    {
	"berserk",
	{ 103, 103, 103, 27, 103, 103, 103, 103, 103,
	  103, 103, 103, 17, 103, 103, 103, 103, 103 },
	{ 0, 0, 0, 2, 0, 0, 0, 0, 0,
	  0, 0, 0, 1, 0, 0, 0, 0, 0 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_berserk,	TRUE,	SLOT( 0),       0,      12,
        "",                     "You feel your pulse slow down.",	""
    },
     
    {
       "rub",
       { 30, 30, 30, 30, 30, 30, 30, 30, 30,
         20, 20, 20, 20, 20, 20, 20, 20, 20 },
       { 1, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 1, 1, 1, 1, 1 },
       spell_null,	TAR_IGNORE,	POS_STANDING,
       &gsn_rub, TRUE, SLOT(0), 0, 12,
       "", "!Rub!", ""
    },

    {
        "circle",
	{ 103, 103, 38, 103, 103, 103, 103, 103, 103,
	  103, 103, 26, 103, 103, 103, 103, 103, 103 },
	{ 0, 0, 4, 0, 0, 0, 0, 0, 0,
	  0, 0, 3, 0, 0, 0, 0, 0, 0 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_circle,	TRUE,	SLOT( 0),        0,     24,
        "circle",             "!Circle!",		""
    },
    {
	"whirlwind",
	{ 103, 103, 103, 75, 103, 103, 103, 103, 103,
          103, 103, 103, 65, 103, 103, 103, 103, 103 },
        { 0, 0, 0, 4, 0, 0, 0, 0, 0,
          0, 0, 0, 3, 0, 0, 0, 0, 0 },
        spell_null,		TAR_IGNORE,		POS_FIGHTING,
 	&gsn_whirlwind,	TRUE,	SLOT( 0),	0,	12,
	"whirlwind",		"!Whirlwind!",		""
    }, 
    {
        "strike",
	{ 103, 103, 103, 103, 103, 103, 103, 103, 50,
	  103, 103, 103, 103, 103, 103, 103, 103, 45 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 5,
	  0, 0, 0, 0, 0, 0, 0, 0, 4 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_strike,	TRUE,	SLOT( 0),        0,     12,
        "strike",             "!Strike!",		""
    },
    {
	"dirt kicking",
	{ 15, 15, 4, 4, 1, 9, 3, 5, 7,
	  5, 5, 1, 1, 1, 8, 2, 3, 4 },
	{ 4, 4, 4, 4, 4, 6, 2, 4, 4,
	  3, 3, 3, 3, 3, 5, 1, 3, 3 }, 
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_dirt,	TRUE,	SLOT( 0),	0,	12,
	"kicked dirt",		"You rub the dirt out of your eyes.",	""
    },

    {
        "disarm",
	{ 103, 103, 18, 18, 32, 103, 20, 103, 18,
	  103, 103, 8, 8, 22, 103, 10, 103, 8 },
	{ 0, 0, 5, 3, 5, 0, 3, 0, 6,
	  0, 0, 4, 2, 4, 0, 2, 0, 5 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_disarm,	TRUE,	SLOT( 0),        0,     12,
        "",                     "!Disarm!",		""
    },
 
    {
        "dodge",
	{ 103, 20, 1, 19, 10, 30, 10, 1, 1,
	  103, 10, 1, 9, 1, 20, 1, 1, 1 },
	{ 0, 5, 4, 6, 5, 5, 2, 4, 4,
	  0, 4, 3, 5, 4, 4, 1, 3, 3 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_dodge,	TRUE,	SLOT( 0),        0,     0,
        "",                     "!Dodge!",		""
    },
    {
 	"phase",
	{103, 103, 103, 103, 103, 103, 85, 103, 103,
         103, 103, 103, 103, 103, 103, 75, 103, 103}, 
        {0, 0, 0, 0, 0, 0, 5, 0, 0,
         0, 0, 0, 0, 0, 0, 4, 0, 0},
        spell_null,		TAR_IGNORE,		POS_FIGHTING,
        &gsn_phase,	TRUE,	SLOT( 0),	0,	0,
        "",			"!Phase!",		""
    },
    {
        "dual wield",
	{ 103, 70, 35, 3, 29, 70, 40, 103, 50,
	  103, 60, 8, 1, 15, 60, 30, 103, 45 },
	{ 0, 3, 3, 2, 3, 7, 3, 0, 3,
	  0, 2, 2, 1, 3, 6, 2, 0, 2 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_dual_wield,	TRUE,	SLOT( 0),        0,     0,
        "",			"!Dual Wield!",		""
    },

     
    {
        "enhanced damage",
	{ 103, 103, 103, 1, 30, 103, 103, 103, 103,
	  103, 103, 103, 1, 20, 103, 103, 103, 103 },
	{ 0, 0, 0, 3, 6, 0, 0, 0, 0,
	  0, 0, 0, 2, 5, 0, 0, 0, 0 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_enhanced_damage,   TRUE,	SLOT( 0),        0,     0,
        "",                     "!Enhanced Damage!",	""
    },
    {
	"advanced disarm",
        { 103, 103, 40, 103, 103, 103, 103, 103, 103,
          103, 103, 30, 103, 103, 103, 103, 103, 103 },
        { 0, 0, 4, 0, 0, 0, 0, 0, 0,
          0, 0, 3, 0, 0, 0, 0, 0, 0 },
        spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_advanced_disarm,	TRUE,	SLOT( 0),	0,	0,
	"",			"!Advanced disarm!",	""
    },

    {
        "feed",
	{ 103, 103, 103, 103, 103, 103, 29, 103, 103,
	  103, 103, 103, 103, 103, 103, 11, 103, 103 },
	{ 0, 0, 0, 0, 0, 0, 2, 0, 0,
	  0, 0, 0, 0, 0, 0, 1, 0, 0 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_feed,	TRUE,	SLOT( 0),	0,	12,
        "feed",             "!Feed!",		""
    },

    {
	"gouge",
	{ 103, 103, 52, 103, 103, 103, 103, 103, 103,
	  103, 103, 24, 103, 103, 103, 103, 103, 103 },
	{ 0, 0, 2, 0, 0, 0, 0, 0, 0,
	  0, 0, 1, 0, 0, 0, 0, 0, 0 }, 
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_gouge,	TRUE,	SLOT( 0),	0,	12,
	"gouge",		"Your vision clears.",	""
    },

    {
        "grip",
	{ 103, 103, 103, 25, 103, 103, 103, 103, 103,
	  103, 103, 103, 10, 103, 103, 103, 103, 103 },
	{ 0, 0, 0, 2, 0, 0, 0, 0, 0,
	  0, 0, 0, 1, 0, 0, 0, 0, 0 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_grip,	TRUE,	SLOT( 0),        0,     0,
        "",                     "!Grip!",		""
    },

    {
	"hand to hand",
	{ 103, 103, 22, 9, 22, 103, 12, 1, 103,
	  103, 103, 12, 8, 12, 103, 2, 1, 103 },
	{ 0, 0, 4, 2, 4, 0, 2, 1, 0,
	  0, 0, 3, 1, 3, 0, 1, 1, 0 },
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_hand_to_hand,	TRUE,	SLOT( 0),	0,	0,
	"",			"!Hand to Hand!",	""
    },

    {
        "kick",
	{ 103, 18, 21, 12, 20, 17, 15, 10, 21,
	  103, 8, 11, 2, 10, 7, 5, 5, 11 },
	{ 0, 4, 6, 3, 4, 6, 2, 3, 6,
	  0, 3, 5, 2, 3, 5, 1, 2, 5 },
        spell_null,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        &gsn_kick,	TRUE,	SLOT( 0),        0,     12,
        "kick",                 "!Kick!",		""
    },

    {
        "parry",
	{ 103, 30, 19, 1, 15, 27, 17, 103, 19,
	  103, 20, 9, 1, 5, 17, 7, 103, 9 },
	{ 0, 5, 4, 4, 5, 5, 2, 0, 4,
	  0, 4, 3, 3, 4, 4, 1, 0, 3 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_parry,	TRUE,	SLOT( 0),        0,     0,
        "",                     "!Parry!",		""
    },

    {
        "rescue",
	{ 103, 103, 103, 1, 13, 103, 103, 25, 103,
	  103, 103, 103, 1, 3, 103, 103, 20, 103 },
	{ 0, 0, 0, 4, 3, 0, 0, 4, 0,
	  0, 0, 0, 3, 2, 0, 0, 3, 0 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_rescue,	TRUE,	SLOT( 0),        0,     12,
        "",                     "!Rescue!",		""
    },

    {
	"trip",
	{ 25, 25, 25, 25, 25, 25, 25, 25, 25,
	  20, 20, 20, 20, 20, 20, 20, 20, 20 },
	{ 3, 3, 3, 3, 3, 3, 3, 3, 3,
	  2, 2, 2, 2, 2, 2, 2, 2, 2 },
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_trip,	TRUE,	SLOT( 0),	0,	12,
	"trip",			"!Trip!",		""
    },
    
    {
       "earthbind",
       { 80, 80, 80, 80, 80, 80, 80, 80, 80,
         70, 70, 70, 70, 70, 70, 70, 70, 70 },
       { 2, 2, 2, 2, 2, 2, 2, 2, 2,
         2, 2, 2, 2, 2, 2, 2, 2, 2 },
       spell_null, TAR_IGNORE, POS_FIGHTING,
       &gsn_earthbind, TRUE, SLOT(0), 0, 12,
       "earthbind", "!Earthbind!", ""
    },

    {
        "stun",
	{ 103, 103, 103, 45, 103, 103, 103, 103, 103,
	  103, 103, 103, 23, 103, 103, 103, 103, 103 },
	{ 0, 0, 0, 3, 0, 0, 0, 0, 0,
	  0, 0, 0, 2, 0, 0, 0, 0, 0 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_stun,	TRUE,	SLOT( 0),        0,     0,
        "",			"!Stun!",		""
    },
    {    
       "critical hit",
       { 102,102,50,102,102,102,102, 103, 60,
         102,102,45,102,102,102,102, 103, 50 },
       { 0,0,3,0,0,0,0,0,4, 
         0,0,2,0,0,0,0,0,3 },
       spell_null, TAR_IGNORE, POS_FIGHTING,
       &gsn_critical_hit, TRUE, SLOT(0), 0, 0,
       "", "!Critical Hit!", ""
    },
  
    {
        "second attack",
	{ 45, 36, 18, 1, 7, 25, 20, 5, 30,
	  35, 26, 8, 1, 6, 15, 10, 1, 20 },
	{ 4, 4, 5, 3, 4, 5, 2, 4, 5,
	  3, 3, 4, 2, 3, 4, 1, 3, 4 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_second_attack,	TRUE,	SLOT( 0),        0,     0,
        "",                     "!Second Attack!",	""
    },

    {
        "third attack",
	{ 65, 46, 36, 18, 29, 45, 30, 25, 40,
	  55, 36, 26, 8, 19, 35, 20, 20, 30 },
	{ 7, 5, 6, 4, 4, 5, 3, 3, 7,
	  6, 4, 5, 3, 3, 4, 2, 2, 6 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_third_attack,	TRUE,	SLOT( 0),        0,     0,
        "",                     "!Third Attack!",	""
    },

    {
	"fourth attack",
	{ 103, 56, 90, 65, 80, 60, 50, 47, 103,
	  103, 46, 80, 55, 70, 50, 40, 40, 103 },
	{ 0, 7, 7, 4, 5, 6, 4, 4, 0,
	  0, 6, 6, 3, 4, 5, 3, 3, 0 },
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_fourth_attack,	TRUE,	SLOT(0),	0,	0,
	"",			"!Fourth Attack!",	""
    },

    {
	"fifth attack",
	{ 103, 103, 101, 80, 103, 103, 103, 65, 103,
	  103, 103, 90, 70, 103, 103, 103, 60, 103 },
	{ 0, 0, 7, 4, 0, 0, 0, 5, 0,
	  0, 0, 6, 3, 0, 0, 0, 4, 0 },
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_fifth_attack,	TRUE,	SLOT(0),	0,	0,
	"",			"!Fifth Attack!",	""
    },
    
    {
       "sixth attack",
       { 103, 103, 103, 90, 103, 103, 103, 91, 103,
         103, 103, 103, 80, 103, 103, 103, 85, 103 },
       { 0, 0, 0, 5, 0, 0, 0, 6, 0,
         0, 0, 0, 4, 0, 0, 0, 5, 0 },
       spell_null, TAR_IGNORE, POS_FIGHTING,
       &gsn_sixth_attack,  TRUE, SLOT(0), 0,0,
       "", "!Sixth Attack!", ""
    },
    
    { "seventh attack",
      { 103, 103, 103, 101, 103, 103, 103, 103, 103,
        103, 103, 103, 91, 103, 103, 103, 103, 103 },
      { 0, 0, 0, 6, 0, 0, 0, 0, 0,
        0, 0, 0, 5, 0, 0, 0, 0, 0 },
      spell_null, TAR_IGNORE, POS_FIGHTING,
      &gsn_seventh_attack, TRUE, SLOT(0), 0,0,
      "", "!Seventh Attack!", ""
    },
    
    { "2nd dual",
      { 103, 90, 103, 70, 103, 103, 103, 103, 60,
        103, 80, 103, 60, 103, 103, 101, 103, 50 },
      { 0, 5, 0, 3, 0, 0, 0, 0, 5,
        0, 4, 0, 2, 0, 0, 4, 0, 4 },
      spell_null, TAR_IGNORE, POS_FIGHTING,
      &gsn_2nd_dual, TRUE, SLOT(0), 0,0,
      "", "!2nd dual!", ""
    },
    
    { "3rd dual",
      { 103, 103, 103, 85, 103, 103, 103, 103, 103,
        103, 103, 103, 75, 103, 103, 103, 103, 101 },
      { 0, 0, 0, 4, 0, 0, 0, 0, 0,
        0, 0, 0, 3, 0, 0, 0, 0, 6 },
      spell_null, TAR_IGNORE, POS_FIGHTING,
      &gsn_3rd_dual, TRUE, SLOT(0), 0,0,
      "", "!3rd dual!", ""
    },
    
    { "4th dual",
      { 103, 103, 103, 95, 103, 103, 103, 103, 103,
        103, 103, 103, 85, 103, 103, 103, 103, 103 },
      { 0, 0, 0, 5, 0, 0, 0, 0, 0,
        0, 0, 0, 4, 0, 0, 0, 0, 0},
       spell_null, TAR_IGNORE, POS_FIGHTING,
       &gsn_4th_dual, TRUE, SLOT(0), 0,0,
       "", "!4th dual!", ""
    },
    {
      "counterattack",
      {103, 103, 103, 103, 75, 103, 103, 103, 103,
       103, 103, 103, 103, 65, 103, 103, 103, 103 },
      { 0, 0, 0, 0, 5, 0, 0, 0, 0,
        0, 0, 0, 0, 4, 0, 0, 0, 0},
      spell_null,	TAR_IGNORE,	POS_FIGHTING,
      &gsn_counter,	TRUE,	SLOT(0),	0,	0,
      "counterattack",	"!Counterattack!",""
    },

/* non-combat skills */

    { 
	"fast healing",
	{ 102, 13, 102, 9, 55, 34, 2, 1, 103,
	  102, 12, 102, 1, 45, 24, 1, 1, 103 },
	{ 0, 4, 0, 4, 4, 5, 1, 1, 0,
	  0, 3, 0, 3, 3, 4, 1, 1, 0 },
	spell_null,		TAR_IGNORE,		POS_SLEEPING,
	&gsn_fast_healing,	TRUE,	SLOT( 0),	0,	0,
	"",			"!Fast Healing!",	""
    },

    {
	"haggle",
	{ 102, 102, 1, 102, 42, 102, 102, 103, 103,
	  102, 102, 1, 102, 32, 102, 102, 103, 103 },
	{ 0, 0, 3, 0, 5, 0, 0, 0, 0,
	  0, 0, 2, 0, 4, 0, 0, 0, 0 },
	spell_null,		TAR_IGNORE,		POS_RESTING,
	&gsn_haggle,	TRUE,	SLOT( 0),	0,	0,
	"",			"!Haggle!",		""
    },

    {
	"hide",
	{ 50, 50, 50, 50, 50, 50, 50, 50, 50,
	  40, 40, 40, 40, 40, 40, 40, 40, 40 },
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1,
	  1, 1, 1, 1, 1, 1, 1, 1, 1 },
	spell_null,		TAR_IGNORE,		POS_RESTING,
	&gsn_hide,	TRUE,	SLOT( 0),	 0,	12,
	"",			"!Hide!",		""
    },
    {
        "search",
        { 50, 50, 50, 50, 50, 50, 50, 50, 50,
          40, 40, 40, 40, 40, 40, 40, 40, 40 },
        { 1, 1, 1, 1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1, 1, 1, 1 },
         spell_null,		TAR_IGNORE,	POS_RESTING,
         &gsn_search,	TRUE, 	SLOT( 0),	0,	12,
         "",		"!Search!",	""
     },
    {
	"forage",
	{ 103, 103, 103, 103, 103, 50, 103, 103, 103,
          103, 103, 103, 103, 103, 40, 103, 103, 103 },
        { 0, 0, 0, 0, 0, 4, 0, 0, 0,
          0, 0, 0, 0, 0, 3, 0, 0, 0 },
        spell_null,		TAR_IGNORE,	POS_RESTING,
	&gsn_forage,	TRUE,	SLOT( 0),	0,	12,
        "",		"!Forage!",	""
    },
    {
	"lore",
	{ 20, 20, 20, 20, 20, 20, 20, 20, 20,
	  10, 10, 10, 10, 10, 10, 10, 10, 10 },
	{ 2, 2, 2, 2, 2, 2, 2, 2, 2,
	  1, 1, 1, 1, 1, 1, 1, 1, 1 },
	spell_null,		TAR_IGNORE,		POS_RESTING,
	&gsn_lore,	TRUE,	SLOT( 0),	0,	12,
	"",			"!Lore!",		""
    },

    {
	"meditation",
	{ 9, 9, 102, 102, 15, 13, 102, 9, 103,
	  8, 1, 102, 102, 5, 3, 70, 4, 103 },
	{ 3, 3, 0, 0, 5, 3, 0, 5, 0,
	  2, 2, 0, 0, 4, 2, 4, 4, 0 },
	spell_null,		TAR_IGNORE,		POS_SLEEPING,
	&gsn_meditation,	TRUE,	SLOT( 0),	0,	0,
	"",			"Meditation",		""
    },

    {
	"peek",
	{ 103, 102, 1, 102, 102, 102, 4, 103, 1,
	  103, 102, 1, 102, 102, 102, 1, 103, 1 },
	{ 0, 0, 3, 0, 0, 0, 4, 0, 3,
	  0, 0, 2, 0, 0, 0, 3, 0, 2 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_peek,	TRUE,	SLOT( 0),	 0,	 0,
	"",			"!Peek!",		""
    },

    {
	"pick lock",
	{ 102, 102, 10, 102, 102, 102, 37, 103, 46,
	  102, 102, 6, 102, 102, 102, 27, 103, 36 },
	{ 0, 0, 4, 0, 0, 0, 8, 0, 4,
	  0, 0, 3, 0, 0, 0, 7, 0, 3 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_pick_lock,	TRUE,	SLOT( 0),	 0,	12,
	"",			"!Pick!",		""
    },

    {
	"sneak",
	{ 90, 90, 6, 15, 2, 90, 2, 1, 1,
	  80, 80, 5, 5, 1, 80, 1, 1, 1 },
	{ 3, 3, 4, 5, 3, 3, 1, 1, 2,
	  2, 2, 3, 4, 2, 2, 1, 1, 1 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_sneak,	TRUE,	SLOT( 0),	 0,	12,
	"",			"You no longer feel stealthy.",	""
    },
    {
        "shadow walk",
        { 103, 103, 80, 103, 103, 103, 103, 103, 90,
          103, 103, 70, 103, 103, 103, 103, 103, 80 },
        { 0, 0, 5, 0, 0, 0, 0, 0, 6,
          0, 0, 4, 0, 0, 0, 0, 0, 5 },
        spell_null,		TAR_IGNORE,		POS_STANDING,
        &gsn_shadow_walk, TRUE,	SLOT( 0),	0,	12,
        "",			"You step out of the shadows.",""
    },
    {
	"steal",
	{ 102, 102, 7, 102, 102, 102, 102, 103, 103,
	  102, 102, 5, 102, 102, 102, 102, 103, 103 },
	{ 0, 0, 4, 0, 0, 0, 0, 0, 0,
	  0, 0, 3, 0, 0, 0, 0, 0, 0 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_steal,	TRUE,	SLOT( 0),	 0,	24,
	"",			"!Steal!",		""
    },
    {
	"slip",
	{ 102, 102, 103, 102, 102, 102, 102, 103, 28,
	  102, 102, 103, 102, 102, 102, 102, 103, 18 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 4,
	  0, 0, 0, 0, 0, 0, 0, 0, 3 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_slip,	TRUE,	SLOT( 0),	 0,	12,
	"",			"!Slip!",		""
    },
    {
	"thaumaturgy",
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1, 1, 1, 1 },
        { 3, 4, 5, 6, 4, 3, 4, 5, 4,
          2, 3, 4, 5, 3, 2, 3, 3, 3 },
        spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_thaumaturgy,	TRUE, SLOT(0),	0,	12,
        "",			"!Thaumaturgy!",	""
    },
    {
	"track",
	{ 102, 102, 102, 102, 20, 65, 102, 103, 103,
	  102, 102, 102, 102, 10, 55, 102, 103, 103 },
	{ 0, 0, 0, 0, 2, 4, 0, 0, 0,
	  0, 0, 0, 0, 1, 3, 0, 0, 0 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_track,	TRUE,	SLOT( 0),	 0,	12,
	"",			"!Track!",		""
    },
    {
	"recall",
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1,
	  1, 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 2, 2, 2, 2, 2, 2, 2, 2, 2,
	  1, 1, 2, 1, 1, 1, 1, 1, 1 },
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_recall,	TRUE,	SLOT( 0),	0,	12,
	"",			"!Recall!",		""
    },
   
    { "tiger style",
      {103, 103, 103, 103, 103, 103, 103, 1, 103,
       103, 103, 103, 103, 103, 103, 103, 1, 103 },
      {0, 0, 0, 0, 0, 0, 0, 3, 0,
       0, 0, 0, 0, 0, 0, 0, 2, 0 },
      spell_null, TAR_IGNORE, POS_FIGHTING,
      &gsn_tiger_style, TRUE, SLOT( 0), 0, 0,
      "",     "!Tiger style!", ""
    },
    { "dragon style",
      {103, 103, 103, 103, 103, 103, 103, 65, 103,
       103, 103, 103, 103, 103, 103, 103, 55, 103 },
      {0, 0, 0, 0, 0, 0, 0, 4, 0,
       0, 0, 0, 0, 0, 0, 0, 3, 0 },
      spell_null, TAR_IGNORE, POS_FIGHTING,
      &gsn_dragon_style, TRUE, SLOT( 0), 0, 0,
      "",     "!Dragon style!", ""
    },
    { "chaos style",
      {103, 103, 103, 103, 103, 103, 103, 90, 103,
       103, 103, 103, 103, 103, 103, 103, 80, 103 },
      {0, 0, 0, 0, 0, 0, 0, 5, 0,
       0, 0, 0, 0, 0, 0, 0, 4, 0 },
      spell_null, TAR_IGNORE, POS_FIGHTING,
      &gsn_chaos_style, TRUE, SLOT( 0), 0, 0,
      "",     "!Chaos style!", ""
    },
    { "elemental style",
      {103, 103, 103, 103, 103, 103, 103, 103, 103,
       103, 103, 103, 103, 103, 103, 103, 101, 103 },
      {0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 5, 0 },
      spell_null, TAR_IGNORE, POS_FIGHTING,
      &gsn_elemental_style, TRUE, SLOT( 0), 0, 0,
      "",     "!Elemental style!", ""
    },
    { "offhand",
      {103,103,45,103,103,103,103,103, 103,
       103,103,35,103,103,103,103,103, 103 },
      {0,0,6,0,0,0,0,0,0,
       0,0,5,0,0,0,0,0,0},
      spell_null, TAR_IGNORE, POS_FIGHTING,
      &gsn_offhand, TRUE, SLOT( 0), 0,0,
      "", "!Offhand!", ""
    }
};

const   struct  group_type      group_table     [MAX_GROUP]     =
{

    {
	"rom basics",
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ "thaumaturgy", "recall","hide","search" }
    },

    {
	"wizard basics",
	{ 0, -1, -1, -1, -1, -1, -1, -1, -1
	  -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "dagger" }
    },

    {
	"priest basics",
	{ -1, 0, -1, -1, -1, -1, -1, -1, -1,
	  -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "mace" }
    },
   
    {
	"mercenary basics",
	{ -1, -1, 0, -1, -1, -1, -1, -1, -1,
	  -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "dagger", "steal", "pass door" }
    },

    {
	"gladiator basics",
	{ -1, -1, -1, 0, -1, -1, -1, -1, -1,
	  -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "sword", "second attack", "dual wield" }
    },

    {
	"strider basics",
	{ -1, -1, -1, -1, 0, -1, -1, -1, -1,
	  -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "dagger", "second attack", "track" }
    },

    {
	"shaman basics",
	{ -1, -1, -1, -1, -1, 0, -1, -1, -1,
	  -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "mace" }
    },

    {
	"lich basics",
	{ -1, -1, -1, -1, -1, -1, 0, -1, -1,
	  -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "dagger",  "sneak" }
    },

    {   "shaolin basics",
        { -1, -1, -1, -1, -1, -1, -1, 0, -1,
          -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { "hand to hand", "sneak", "second attack", "dodge" }
    },

    {   "assassin basics",
        { -1, -1, -1, -1, -1, -1, -1, -1, 0,
          -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        { "dagger", "sneak", "second attack" }
    },

    {
	"sorcerer basics",
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1,
	  0, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "dagger" }
    },

    {
	"bishop basics",
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1,
	  -1, 0, -1, -1, -1, -1, -1, -1, -1 },
	{ "mace" }
    },
   
    {
	"blade basics",
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1,
	  -1, -1, 0, -1, -1, -1, -1, -1, -1 },
	{ "dagger", "steal" }
    },

    {
	"knight basics",
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1,
	  -1, -1, -1, 0, -1, -1, -1, -1, -1 },
	{ "sword", "second attack", "dual wield" }
    },

    {
	"guardian basics",
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1,
	  -1, -1, -1, -1, 0, -1, -1, -1, -1 },
	{ "dagger", "second attack", "track" }
    },

    {
	"elder basics",
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1,
	  -1, -1, -1, -1, -1, 0, -1, -1, -1 },
	{ "mace" }
    },

    {
	"necromancer basics",
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1,
	  -1, -1, -1, -1, -1, -1, 0, -1, -1 },
	{ "dagger", "sneak" }
    },
    
    { "sensai basics",
      { -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, 0, -1 },
       { "hand to hand", "sneak", "second attack", "dodge" }
    },

    { "deathmaster basics",
      { -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, 0 },
       { "dagger", "sneak", "second attack" }
    },

    {
	"wizard default",
	{ 40, -1, -1, -1, -1, -1, -1, -1, -1,
	  -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{
	 "lore",
	 "beguiling",
	 "combat",
	 "detection",
	 "enhancement",
	 "illusion",
	 "maladictions",
	 "protective",
	 "shielding",
         "rub"
	}
    },

    {
	"priest default",
	{ -1, 40, -1, -1, -1, -1, -1, -1, -1,
	  -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{
	 "flail",
	 "attack",
	 "benedictions", 
	 "curative",
	 "detection",
	 "healing",
	 "maladictions",
	 "protective",
	 "shield block", 
	 "transportation",
         "second attack"
	}
    },
 
    {
	"mercenary default",
	{ -1, -1, 40, -1, -1, -1, -1, -1, -1,
	  -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{
	 "backstab",
	 "circle",
	 "disarm",
	 "dodge",
	 "mace",
	 "lore",
	 "pick lock",
	 "sneak",
	 "sword",
	 "trip",
	 "second attack",
	}
    },

    {
	"gladiator default",
	{ -1, -1, -1, 40, -1, -1, -1, -1, -1,
	  -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{
	 "weaponsmaster",
	 "bash",
	 "enhanced damage", 
	 "grip",
	 "parry",
	 "rescue",
	 "shield block",
	 "third attack",
	}
    },

    {
	"strider default",
	{ -1, -1, -1, -1, 40, -1, -1, -1, -1,
	  -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{
	 "weaponsmaster",
	 "dirt kick",
	 "enhanced damage",
	 "hand to hand",
	 "kick",
	 "parry",
	 "shield block",
         "attack"
	}
    },

    {
	"shaman default",
	{ -1, -1, -1, -1, -1, 40, -1, -1, -1,
	  -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{
	 "lore",
	 "shield block",
	 "second attack",
	 "attack",
	 "benedictions",
	 "combat",
	 "creation",
	 "curative",
	 "healing",
	}
    },

    {
	"lich default",
	{ -1, -1, -1, -1, -1, -1, 40, -1, -1,
	  -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{
	 "backstab",
	 "disarm",
	 "dodge",
	 "feed",
	 "hand to hand",
	 "parry",
	 "beguiling",
	 "detection",
	 "enhancement",
	 "illusion",
	 "maladictions",
	}
    },
    
    { "shaolin default",
      { -1, -1, -1, -1, -1, -1, -1, 40, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1 },
      {
        "fast healing",
        "meditation",
        "detection",
        "shield block",
        "shielding",
        "lore",
        "dirt kicking",
        "third attack",
        "fourth attack",
        "maladictions",
        "protective",
        "tiger style"
      }
    },
    {
	"assassin default",
	{ -1, -1, -1, -1, -1, -1, -1, -1, 40,
	  -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{
	 "strike",
	 "disarm",
	 "sword",
	 "parry",
	 "pick lock",
	 "maladictions",
	 "trip",
	 "third attack",
	 "rub"
	}
    },
    {
	"sorcerer default",
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1,
	  40, -1, -1, -1, -1, -1, -1, -1, -1 },
	{
	 "lore",
	 "beguiling",
	 "combat",
	 "detection",
	 "enhancement",
	 "illusion",
	 "maladictions",
	 "protective",
	 "shielding",
	 "transportation",
	 "weather",
         "elemental",
         "harmful"
	}
    },

    {
	"bishop default",
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1,
	  -1, 40, -1, -1, -1, -1, -1, -1, -1 },
	{
	 "flail",
	 "shield block", 
	 "attack",
	 "benedictions", 
	 "creation",
	 "curative",
	 "detection",
	 "healing",
	 "maladictions",
	 "protective",
	 "transportation",
	 "weather",
         "second attack",
         "combat"
	}
    },
 
    {
	"blade default",
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1,
	  -1, -1, 40, -1, -1, -1, -1, -1, -1 },
	{
	 "backstab",
	 "circle",
	 "disarm",
	 "dodge",
	 "dual wield",
	 "hide",
	 "mace",
	 "peek",
	 "pick lock",
	 "sneak",
	 "sword",
	 "trip",
	 "second attack",
	 "beguiling",
         "transportation",
         "protective"
	}
    },

    {
	"knight default",
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1,
	  -1, -1, -1, 40, -1, -1, -1, -1, -1 },
	{
	 "weaponsmaster",
	 "bash",
	 "disarm",
	 "enhanced damage", 
	 "parry",
	 "rescue",
	 "shield block",
	 "third attack",
	 "fourth attack",
         "berserk",
         "grip"
	}
    },

    {
	"guardian default",
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1,
	  -1, -1, -1, -1, 40, -1, -1, -1, -1 },
	{
	 "weaponsmaster",
	 "dual wield",
	 "enhanced damage",
	 "hand to hand",
	 "kick",
	 "parry",
	 "third attack",
	 "beguiling",
	 "healing",
	}
    },

    {
	"elder default",
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1,
	  -1, -1, -1, -1, -1, 40, -1, -1, -1 },
	{
	 "lore",
	 "shield block",
	 "second attack",
	 "attack",
	 "benedictions",
	 "combat",
	 "creation",
	 "curative",
	 "healing",
	 "protective",
	 "weather",
         "elemental",
         "parry"
	}
    },

    {
	"necromancer default",
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1,
	  -1, -1, -1, -1, -1, -1, 40, -1, -1 },
	{
	 "backstab",
	 "detection",
	 "disarm",
	 "dodge",
	 "fast healing",
	 "hand to hand",
	 "shield block",
	 "beguiling",
	 "enhancement",
	 "illusion",
	 "maladictions",
	 "transportation",
         "feed",
         "second attack",
         "attack",
         "third attack"
	}
    },

    { "sensai default",
      { -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, 40, -1 },
     {    
        "fast healing",
        "meditation",
        "detection",
        "shield block",
        "shielding",
        "lore",
        "dirt kicking",
        "third attack",
        "fourth attack",
        "fifth attack",
        "protective",
        "tiger style",
        "rescue",
        "shield smash",
        "bash"
     }
    },
    {
	"deathmaster default",
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1,
	  -1, -1, -1, -1, -1, -1, -1, -1, 40 },
	{
	 "strike",
	 "disarm",
	 "dodge",
	 "dual wield",
	 "hide",
	 "sword",
	 "pick lock",
	 "sword",
	 "trip",
	 "third attack",
	 "maladictions",
         "illusion",
         "transportation"
	}
    },
    {
	"weaponsmaster",
	{ 10, 12, 27, 20, 12, 12, 26, -1, 16,
	  10, 12, 27, 20, 12, 12, 26, -1, 16 },
	{ "axe", "dagger", "flail", "mace", "polearm", "spear", "sword",
	  "whip" }
    },

    {
	"attack",
	{ 6, 5, -1, -1, 2, 6, 4, -1, -1,
	  5, 4, -1, -1, 1, 5, 3, -1, -1 },
	{ "demonfire", "earthquake", "heat metal",
          "sever", "shrink" }
    },

    {
	"beguiling",
	{ 5, 4, 2, -1, 4, 4, 6, -1, 6,
	  4, 3, 1, -1, 3, 3, 5, -1, 5 },
	{ "animate", "charm person", "resurrect", "sleep" }
    },

    {
	"benedictions",
	{ 2, 5, -1, -1, -1, 5, 3, -1, 2,
	  1, 4, -1, -1, -1, 4, 2, 2, 1 },
	{ "bless", "frenzy", "remove curse",
          "divine blessing", "infernal offering" }
    },

    {
	"combat",
	{ 5, 5, -1, -1, 4, 6, 5, -1, 2,
	  4, 4, -1, -1, 3, 5, 4, -1, 1 },
	{ "acid blast", "chain lightning", "magic missile",
	  "cleansing", "blaze" }
    },

    {
	"creation",
	{ 4, 4, -1, -1, 4, 4, -1, -1, -1,
	  3, 3, -1, -1, 3, 3, -1, -1, -1 },
	{ "continual light", "conjure", "empower", "grow", "nourish" }
    },

    {
	"curative",
	{ -1, 4, -1, -1, -1, 5, -1, -1, -1,
	  -1, 3, -1, -1, -1, 4, -1, -1, -1 },
	{ "cure blindness", "cure disease", "cure poison" }
    }, 

    {
	"detection",
	{ 4, 3, 6, -1, 6, 6, 4, 6, 6,
	  3, 2, 5, -1, 5, 5, 3, 5, 5 },
 	{ "detect evil", "detect good", "detect hidden", "detect invis", 
	  "detect magic", "detect poison", "farsight", "identify", 
	  "know alignment", "locate object", "true sight", "ultravision" } 
    },

    {
	"draconian",
	{ 8, -1, -1, -1, -1, -1, -1, -1, -1,
	  7, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ "acid breath", "fire breath", "frost breath", "gas breath",
	  "lightning breath"  }
    },

    {
	"enchantment",
	{ 4, 5, -1, -1, -1, 3, -1, -1, 3,
	  3, 4, -1, -1, -1, 3, -1, -1, 2 },
	{ "fireproof", "corrupt potion" }
    },

    { 
	"enhancement",
	{ 5, 6, 3, 3, 4, 4, 6, 4, 4,
	  4, 5, 2, 2, 3, 3, 5, 3, 3 },
	{ "vigor", "haste", "infravision", "refresh", "renewal", "outrage",
          "mastermind" }
    },

    {
	"harmful",
	{ 2, 2, -1, -1, -1, 2, 2, -1, -1,
	  1, 1, -1, -1, -1, 1, 1, -1, -1 },
	{ "cause damage" }
    },

    {   
	"healing",
	{ -1, 3, -1, -1, 5, 4, -1, -1, -1,
	  -1, 2, -1, -1, 4, 3, -1, -1, -1 },
 	{ "cure wounds", "heal", "mass healing", "regeneration" }
    },

    {
	"illusion",
	{ 4, 6, 7, -1, 2, 3, 3, -1, 2,
	  3, 5, 6, -1, 1, 2, 2, -1, 1 },
	{ "invis", "mass invis" }
    },
  
    {
	"maladictions",
	{ 5, 5, -1, -1, 3, 8, 8, 5, 6,
	  4, 4, -1, -1, 2, 7, 7, 4, 5 },
	{ "blindness",  "curse", "energy drain", "plague",
	  "poison", "slow", "voodoo", "weaken","swarm", "death touch",
          "bloodbath","leech","blade poison" }
    },

    { 
	"protective",
	{ 4, 4, -1, -1, 8, 5, 7, 6, 6,
	  3, 3, 2, 2, 7, 4, 6, 5, 5 },
	{ "immaterial armor", "cancellation", "dispel magic", "fireproof",
	  "protection evil", "protection good", "sanctuary", "force shield", 
	  "stone skin", "ward", "protection neutral",
          "protective force","resistance" }
    },

    {
	"shielding",
	{ 8, 8, -1, -1, 8, 8, 4, 2, 3,
	  7, 7, -1, -1, 7, 7, 3, 2, 2 },
	{ "iceshield", "fireshield", "shockshield","shadowshield",
          "bladebarrier","prismaticshield","acidshield", "earthshield",
          "mana shield" }
    },

    {
	"transportation",
	{ 4, 4, 2, -1, 5, 4, 4, 3, 4,
	  3, 3, 3, 3, 4, 3, 3, 2, 3 },
	{ "fly", "gate", "pass door", "summon", "teleport",
	  "transport" }
    },
   
    {
	"weather",
	{ 4, 4, -1, -1, 4, 4, -1, -1, -1,
	  3, 3, -1, -1, 3, 3, -1, -1, -1 },
	{ "faerie fire", "faerie fog" }
    },
    {
        "elemental",
        { 3, 4, -1, -1, 4, 6, -1, -1, -1,
          2, 3, -1, -1, 3, 5, -1, -1, -1 },
        { "sandstorm", "elemental fury", "hailstorm", "tornado", "cascade" }
    },
    {
	"transformation",
	{ -1, -1, -1, -1, -1, -1, 6, -1, -1,
          -1, -1, -1, -1, -1, -1, 5, -1, -1 },
        { "essence of mist", "essence of bat", "essence of wolf" }
    }
};

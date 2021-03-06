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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"

/* command procedures needed */
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_wear		);

/*
 * Local functions.
 */
void	say_spell	args( ( CHAR_DATA *ch, int sn ) );

/* imported functions */
bool    remove_obj      args( ( CHAR_DATA *ch, int iWear, bool fReplace ) );
void 	wear_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace ) );



/*
 * Lookup a skill by name.
 */
int skill_lookup( const char *name )
{
    int sn;

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
	if ( skill_table[sn].name == NULL )
	    break;
	if ( LOWER(name[0]) == LOWER(skill_table[sn].name[0])
	&&   !str_prefix( name, skill_table[sn].name ) )
	    return sn;
    }

    return -1;
}

int skill_lookup_exact( const char *name )
{
    int sn;

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
	if ( skill_table[sn].name == NULL )
	    break;
	if ( LOWER(name[0]) == LOWER(skill_table[sn].name[0])
	&&   !strcmp( name, skill_table[sn].name ) )
	    return sn;
    }

    return -1;
}


int find_spell( CHAR_DATA *ch, const char *name )
{
    /* finds a spell the character can cast if possible */
    int sn, found = -1;

    if (IS_NPC(ch))
	return skill_lookup(name);

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
	if (skill_table[sn].name == NULL)
	    break;
	if (LOWER(name[0]) == LOWER(skill_table[sn].name[0])
	&&  !str_prefix(name,skill_table[sn].name))
	{
	    if ( found == -1)
		found = sn;
	    if (ch->level >= skill_table[sn].skill_level[ch->class]
	    &&  ch->pcdata->learned[sn] > 0)
		    return sn;
	}
    }
    return found;
}



/*
 * Lookup a skill by slot number.
 * Used for object loading.
 */
int slot_lookup( int slot )
{
    extern bool fBootDb;
    int sn;

    if ( slot <= 0 )
	return -1;

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
	if ( slot == skill_table[sn].slot )
	    return sn;
    }

    if ( fBootDb )
    {
	bug( "Slot_lookup: bad slot %d.", slot );
	abort( );
    }

    return -1;
}



/*
 * Utter mystical words for an sn.
 */
void say_spell( CHAR_DATA *ch, int sn )
{
    char buf  [MAX_STRING_LENGTH];
    char buf2 [MAX_STRING_LENGTH];
    CHAR_DATA *rch;
    char *pName;
    int iSyl;
    int length;

    struct syl_type
    {
	char *	old;
	char *	new;
    };

    static const struct syl_type syl_table[] =
    {
	{ " ",		" "		},
	{ "ar",		"alea"		},
	{ "au",		"voda"		},
	{ "bless",	"fremo"		},
	{ "blind",	"isme"		},
	{ "bur",	"dela"		},
	{ "cu",		"ji"		},
	{ "de",		"oro"		},
	{ "en",		"uvo"		},
	{ "light",	"tars"		},
	{ "lo",		"vi"		},
	{ "mor",	"tor"		},
	{ "move",	"ledo"		},
	{ "ness",	"sola"		},
	{ "ning",	"orna"		},
	{ "per",	"teta"		},
	{ "ra",		"fra"		},
	{ "fresh",	"eva"		},
	{ "re",		"sor"	},
	{ "son",	"penta"		},
	{ "tect",	"eia"		},
	{ "tri",	"cara"		},
	{ "ven",	"novo"		},
	{ "a", "a" }, { "b", "b" }, { "c", "q" }, { "d", "e" },
	{ "e", "z" }, { "f", "y" }, { "g", "o" }, { "h", "p" },
	{ "i", "u" }, { "j", "y" }, { "k", "t" }, { "l", "r" },
	{ "m", "w" }, { "n", "i" }, { "o", "a" }, { "p", "s" },
	{ "q", "d" }, { "r", "f" }, { "s", "g" }, { "t", "h" },
	{ "u", "j" }, { "v", "z" }, { "w", "x" }, { "x", "n" },
	{ "y", "l" }, { "z", "k" },
	{ "", "" }
    };

    buf[0]	= '\0';
    for ( pName = skill_table[sn].name; *pName != '\0'; pName += length )
    {
	for ( iSyl = 0; (length = strlen(syl_table[iSyl].old)) != 0; iSyl++ )
	{
	    if ( !str_prefix( syl_table[iSyl].old, pName ) )
	    {
		strcat( buf, syl_table[iSyl].new );
		break;
	    }
	}

	if ( length == 0 )
	    length = 1;
    }

    sprintf( buf2, "$n utters the words, '%s'.", buf );
    sprintf( buf,  "$n utters the words, '%s'.", skill_table[sn].name );

    for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
    {
	if ( rch != ch )
	    act( ch->class==rch->class ? buf : buf2, ch, NULL, rch, TO_VICT );
    }

    return;
}



/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool saves_spell( int level, CHAR_DATA *victim, int dam_type )
{
    int save;

   /* save = 50 + ( victim->level - level) * 5 - victim->saving_throw * 2;
    if (IS_AFFECTED(victim,AFF_BERSERK))
	save += victim->level/2;

    switch(check_immune(victim,dam_type,gsn_resistance))
    {
	case IS_IMMUNE:		return TRUE;
	case IS_RESISTANT:	save += 5;	break;
	case IS_VULNERABLE:	save -= 5;	break;
    }*/
    if(!IS_NPC(victim))
     save = (victim->level-level)*5 - victim->saving_throw;
    else
     save = (victim->level-level)/2 +  ((get_curr_stat(victim,STAT_INT)*2)/3+
                                           (get_curr_stat(victim,STAT_WIS)*2)/3);
    if(IS_AFFECTED(victim,AFF_BERSERK))
       save+=(victim->level/8+1);
    switch(check_immune(victim,dam_type,gsn_resistance))
    {
 	case IS_IMMUNE:		return TRUE;
        case IS_RESISTANT:	save += (victim->level/10+1); break;
        case IS_VULNERABLE:	save -= (level/10+1);
    }

    /*if (!IS_NPC(victim) && class_table[victim->class].fMana)
	save = 9 * save / 10;*/
    save = URANGE( 5, save, 95 );
    return number_range(0,110) < save;
}

/* RT save for dispels */

bool saves_dispel( int dis_level, int spell_level, int duration)
{
    int save;
    int sp_lev;

    if(spell_level>103)
     sp_lev=103+(spell_level-103)/7;
    else
     sp_lev=spell_level;
    if (duration == -1)
      sp_lev += 3;  
      /* very hard to dispel permanent effects */

    save = 50 + (sp_lev - dis_level) * 5;
    save = URANGE( 5, save, 95 );
    return number_percent( ) < save;
}

/* co-routine for dispel magic and cancellation */

bool check_dispel( int dis_level, CHAR_DATA *victim, int sn)
{
    AFFECT_DATA *af;

    if (is_affected(victim, sn))
    {
        for ( af = victim->affected; af != NULL; af = af->next )
        {
            if ( af->type == sn )
            {
                if (!saves_dispel(dis_level,af->level,af->duration))
                {
                    affect_strip(victim,sn);
        	    if ( skill_table[sn].msg_off )
        	    {
            		send_to_char( skill_table[sn].msg_off, victim );
            		send_to_char( "\n\r", victim );
        	    }
		    return TRUE;
		}
		else
		    af->level--;
            }
        }
    }
    return FALSE;
}

/* for finding mana costs -- temporary version */
int mana_cost (CHAR_DATA *ch, int min_mana, int level)
{
    if (ch->level + 2 == level)
	return 1000;
    return UMAX(min_mana,(100/(2 + ch->level - level)));
}



/*
 * The kludgy global is for spells who want more stuff from command line.
 */
char *target_name;
char *third_name;

void do_cast( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    void *vo;
    int mana;
    int sn;
    int target;

    /*
     * Switched NPC's can cast spells, but others can't.
     */
    if ( IS_NPC(ch) && ch->desc == NULL)
	return;

    target_name = one_argument( argument, arg1 );
    third_name = one_argument( target_name, arg2 );
    strcpy( target_name, arg2 );
    one_argument( third_name, arg3 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Cast which what where?\n\r", ch );
	return;
    }

    if (ch->stunned)
    {
        send_to_char("You're still a little woozy.\n\r",ch);
        return;
    }

    if ( ( sn = find_spell( ch,arg1 ) ) < 0
    || ( !IS_NPC(ch) && (ch->level < skill_table[sn].skill_level[ch->class]
    ||   		 ch->pcdata->learned[sn] == 0)))
    {
	send_to_char( "You don't know any spells of that name.\n\r", ch );
	return;
    }
  
    if ( ch->position < skill_table[sn].minimum_position )
    {
	send_to_char( "You can't concentrate enough.\n\r", ch );
	return;
    }

    if (ch->level + 2 == skill_table[sn].skill_level[ch->class])
	mana = 50;
    else
    	mana = UMAX(
	    skill_table[sn].min_mana,
	    100 / ( 2 + ch->level - skill_table[sn].skill_level[ch->class] ) );

    if (!strcmp(skill_table[sn].name, "restore mana") )
	mana = 1;

    /*
     * Locate targets.
     */
    victim	= NULL;
    obj		= NULL;
    vo		= NULL;
    target	= TARGET_NONE;
      
    switch ( skill_table[sn].target )
    {
    default:
	bug( "Do_cast: bad target for sn %d.", sn );
	return;

    case TAR_IGNORE:
	break;

    case TAR_CHAR_OFFENSIVE:
	if ( arg2[0] == '\0' )
	{
	    if ( ( victim = ch->fighting ) == NULL )
	    {
		send_to_char( "Cast the spell on whom?\n\r", ch );
		return;
	    }
	}
	else
	{
	    if ( ( victim = get_char_room( ch, target_name ) ) == NULL )
	    {
		send_to_char( "They aren't here.\n\r", ch );
		return;
	    }
	}
/*
        if ( ch == victim )
        {
            send_to_char( "You can't do that to yourself.\n\r", ch );
            return;
        }
*/


	if ( !IS_NPC(ch) )
	{

            if (is_safe(ch,victim) && victim != ch)
	    {
		send_to_char("Not on that target.\n\r",ch);
		return; 
	    }
	}

        if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
	{
	    send_to_char( "You can't do that on your own follower.\n\r",
		ch );
	    return;
	}

	if ( ( !IS_NPC( ch ) && !IS_NPC( victim ) )
	&& ( !IS_IMMORTAL( ch ) )
	&& ( !IS_IMMORTAL( victim ) )
	&& ( ch != victim )
	&& ( !skill_table[sn].socket )
	&& ( !strcmp(ch->pcdata->socket, victim->pcdata->socket ) ) )
	{
	    send_to_char("Spell failed.\n\r",ch);
	    return;
	}

	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_CHAR_DEFENSIVE:
	if ( arg2[0] == '\0' )
	{
	    victim = ch;
	}
	else
	{
	    if ( ( victim = get_char_room( ch, target_name ) ) == NULL )
	    {
		send_to_char( "They aren't here.\n\r", ch );
		return;
	    }
	}

	if ( ( !IS_NPC( ch ) && !IS_NPC( victim ) )
	&& ( !IS_IMMORTAL( ch ) )
	&& ( !IS_IMMORTAL( victim ) )
	&& ( ch != victim )
	&& ( !skill_table[sn].socket )
	&& ( !strcmp(ch->pcdata->socket, victim->pcdata->socket ) ) )
	{
	    send_to_char("Spell failed.\n\r",ch);
	    return;
	}

	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_CHAR_SELF:
	if ( arg2[0] != '\0' && !is_name( target_name, ch->name ) )
	{
	    send_to_char( "You cannot cast this spell on another.\n\r", ch );
	    return;
	}

	vo = (void *) ch;
	target = TARGET_CHAR;
	break;

    case TAR_OBJ_INV:
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "What should the spell be cast upon?\n\r", ch );
	    return;
	}

	if ( ( obj = get_obj_carry( ch, target_name ) ) == NULL )
	{
	    send_to_char( "You are not carrying that.\n\r", ch );
	    return;
	}

	vo = (void *) obj;
	target = TARGET_OBJ;
	break;

    case TAR_OBJ_CHAR_OFF:
	if (arg2[0] == '\0')
	{
	    if ((victim = ch->fighting) == NULL)
	    {
		send_to_char("Cast the spell on whom or what?\n\r",ch);
		return;
	    }
	
	    target = TARGET_CHAR;
	}
	else if ((victim = get_char_room(ch,target_name)) != NULL)
	{
	    target = TARGET_CHAR;
	}

	if (target == TARGET_CHAR) /* check the sanity of the attack */
	{
	    if(is_safe_spell(ch,victim,FALSE) && victim != ch)
	    {
		send_to_char("Not on that target.\n\r",ch);
		return;
	    }

            if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
            {
                send_to_char( "You can't do that on your own follower.\n\r",
                    ch );
                return;
            }

	    if ( ( !IS_NPC( ch ) && !IS_NPC( victim ) )
	    && ( !IS_IMMORTAL( ch ) )
	    && ( !IS_IMMORTAL( victim ) )
	    && ( ch != victim )
	    && ( !skill_table[sn].socket )
	    && ( !strcmp(ch->pcdata->socket, victim->pcdata->socket ) ) )
	    {
		send_to_char("Spell failed.\n\r",ch);
		return;
	    }

	    vo = (void *) victim;
 	}
	else if ((obj = get_obj_here(ch,target_name)) != NULL)
	{
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	}
	else
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
	break; 

    case TAR_OBJ_CHAR_DEF:
        if (arg2[0] == '\0')
        {
            vo = (void *) ch;
            target = TARGET_CHAR;                                                 
        }
        else if ((victim = get_char_room(ch,target_name)) != NULL)
        {

	    if ( ( !IS_NPC( ch ) && !IS_NPC( victim ) )
	    && ( !IS_IMMORTAL( ch ) )
	    && ( !IS_IMMORTAL( victim ) )
	    && ( ch != victim )
	    && ( !skill_table[sn].socket )
	    && ( !strcmp(ch->pcdata->socket, victim->pcdata->socket ) ) )
	    {
		send_to_char("Spell failed.\n\r",ch);
		return;
	    }

            vo = (void *) victim;
            target = TARGET_CHAR;
	}
	else if ((obj = get_obj_carry(ch,target_name)) != NULL)
	{
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	}
	else
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
	break;

    case TAR_OBJ_TRAN:
	if (arg2[0] == '\0')
	{
	    send_to_char("Transport what to whom?\n\r",ch);
	    return;
	}
	if (arg3[0] == '\0')
	{
	    send_to_char("Transport it to whom?\n\r",ch);
	    return;
	}
        if ( ( obj = get_obj_carry( ch, target_name ) ) == NULL )
        {
            send_to_char( "You are not carrying that.\n\r", ch );
            return;
        }
	if ( ( victim = get_char_world( ch, third_name ) ) == NULL
	|| IS_NPC(victim) )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}
	if ( !IS_NPC(ch) && ch->mana < mana )
	{
	    send_to_char( "You don't have enough mana.\n\r", ch );
	    return;
	}
	if ( obj->wear_loc != WEAR_NONE )
	{
	    send_to_char( "You must remove it first.\n\r", ch );
	    return;
	}
	if (IS_SET(victim->act,PLR_NOTRAN)
	&& ch->level < SQUIRE )
	{
	    send_to_char( "They don't want it.\n\r", ch);
	    return;
	}
	if ( victim->carry_number + get_obj_number( obj ) > can_carry_n( victim ) )
	{
	    act( "$N has $S hands full.", ch, NULL, victim, TO_CHAR );
	    return;
	}
	if (get_carry_weight(victim) + get_obj_weight(obj) > can_carry_w( victim ) )
	{
	    act( "$N can't carry that much weight.", ch, NULL, victim, TO_CHAR );
	    return;
	}
	if ( !can_see_obj( victim, obj ) )
	{
	    act( "$N can't see it.", ch, NULL, victim, TO_CHAR );
	    return;
	}
	if ( ( !IS_NPC( ch ) && !IS_NPC( victim ) )
	&& ( !IS_IMMORTAL( ch ) )
	&& ( !IS_IMMORTAL( victim ) )
	&& ( ch != victim )
	&& ( !skill_table[sn].socket )
	&& ( !strcmp(ch->pcdata->socket, victim->pcdata->socket ) ) )
	{
	    send_to_char("Spell failed.\n\r",ch);
	    return;
	}
	WAIT_STATE( ch, skill_table[sn].beats );
	if ( !can_drop_obj( ch, obj ) || IS_OBJ_STAT(obj,ITEM_QUEST) )
	{
	    send_to_char( "It seems happy where it is.\n\r", ch);
	    ch->mana -= mana / 3;
	    return;
	}
	if ((obj->pIndexData->vnum == OBJ_VNUM_VOODOO)
	&& (ch->level <= HERO))
	{
	    send_to_char( "You can't transport voodoo dolls.\n\r",ch);
	    ch->mana -= mana / 3;
	    return;
	}

	if ( number_percent( ) > get_skill(ch,sn) )
	{
	    send_to_char( "You lost your concentration.\n\r", ch );
	    ch->mana -= mana / 2;
            if(!IS_SET(ch->in_room->room_flags,ROOM_SAFE))
       	     check_improve(ch,sn,TRUE,1);
	}
	else
	{
	    ch->mana -= mana;
	    obj_from_char( obj );
	    obj_to_char( obj, victim );
	    act( "$p glows {Ggreen{x, then disappears.", ch, obj, victim, TO_CHAR);
	    act( "$p suddenly appears in your inventory.", ch, obj, victim, TO_VICT);
	    act( "$p glows {Ggreen{x, then disappears from $n's inventory.", ch, obj, victim, TO_NOTVICT);
	    if(!IS_SET(ch->in_room->room_flags,ROOM_SAFE))
             check_improve(ch,sn,TRUE,1);
	    if (IS_OBJ_STAT(obj,ITEM_FORCED)
	    && (victim->level <= HERO) ) {
		do_wear(victim, obj->name);
	    }
	}
	return;
	break;
    }
	    
    if ( !IS_NPC(ch) && ch->mana < mana )
    {
	send_to_char( "You don't have enough mana.\n\r", ch );
	return;
    }
      
	say_spell( ch, sn );
      
    WAIT_STATE( ch, skill_table[sn].beats );
      
    if ( number_percent( ) > get_skill(ch,sn) )
    {
	send_to_char( "You lost your concentration.\n\r", ch );
        if(!IS_SET(ch->in_room->room_flags,ROOM_SAFE)
           || (IS_SET(ch->in_room->room_flags,ROOM_SAFE)&&((skill_table[sn].target!=TAR_OBJ_CHAR_OFF
               && skill_table[sn].target!=TAR_CHAR_OFFENSIVE))
               || (ch->fighting!=NULL)))
 	 check_improve(ch,sn,FALSE,1);
	ch->mana -= mana / 2;
    }
    else
    {
        ch->mana -= mana;
        if((skill_table[sn].target == TAR_CHAR_OFFENSIVE)
           && is_affected(victim,gsn_resistance)
           && (number_percent()<(30-(ch->level/5)+(victim->level/20))))
        {
         act("You resist $N's spell!",victim,NULL,ch,TO_CHAR);
         act("$n resists $N's spell!",victim,NULL,ch,TO_NOTVICT);
         act("$n resists your spell!",victim,NULL,ch,TO_VICT);
         return;
        }
        if((get_curr_stat(ch,STAT_INT)<=20) ||
           (get_curr_stat(ch,STAT_WIS)<=20))
              (*skill_table[sn].spell_fun) ( sn, ch->level-1, ch, vo,target);
        else if((get_curr_stat(ch,STAT_INT)>=26) &&
           (get_curr_stat(ch,STAT_WIS)>=26))
              (*skill_table[sn].spell_fun) ( sn, ch->level+1, ch, vo,target);
        else
            (*skill_table[sn].spell_fun) ( sn, ch->level, ch, vo,target);
        if(!IS_SET(ch->in_room->room_flags,ROOM_SAFE)
           || (IS_SET(ch->in_room->room_flags,ROOM_SAFE)&&((skill_table[sn].target!=TAR_OBJ_CHAR_OFF
               && skill_table[sn].target!=TAR_CHAR_OFFENSIVE))
               || (ch->fighting!=NULL)))
  	    check_improve(ch,sn,TRUE,1);
    }

    if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE
    ||   (skill_table[sn].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR))
    &&   victim != ch
    &&   victim->master != ch)
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

        if(has_killed_flag(ch))
         remove_killed_flag(ch);

	for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next = vch->next_in_room;
	    if ( victim == vch && victim->fighting == NULL )
	    {
		multi_hit( victim, ch, TYPE_UNDEFINED );
		break;
	    }
	}
    }
    return;
}



/*
 * Cast spells at targets using a magical object.
 */
void obj_cast_spell( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj )
{
    void *vo;
    int target = TARGET_NONE;

    if ( sn <= 0 )
	return;

    if ( sn >= MAX_SKILL || skill_table[sn].spell_fun == 0 )
    {
	bug( "Obj_cast_spell: bad sn %d.", sn );
	return;
    }

    if (ch->fighting != NULL)
    {
	WAIT_STATE( ch, skill_table[sn].beats );
    }

    switch ( skill_table[sn].target )
    {
    default:
	bug( "Obj_cast_spell: bad target for sn %d.", sn );
	return;

    case TAR_IGNORE:
	vo = NULL;
	break;

    case TAR_CHAR_OFFENSIVE:
	if ( victim == NULL )
	    victim = ch->fighting;
	if ( victim == NULL )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}
	if (is_safe(ch,victim) && ch != victim)
	{
	    send_to_char("Something isn't right...\n\r",ch);
	    return;
	}
	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_CHAR_DEFENSIVE:
    case TAR_CHAR_SELF:
	if ( victim == NULL )
	    victim = ch;
	vo = (void *) victim;
	target = TARGET_CHAR;
	break;

    case TAR_OBJ_INV:
	if ( obj == NULL )
	{
	    send_to_char( "You can't do that.\n\r", ch );
	    return;
	}
	vo = (void *) obj;
	target = TARGET_OBJ;
	break;

    case TAR_OBJ_CHAR_OFF:
        if ( (victim == NULL) && (obj == NULL))
	    if (ch->fighting != NULL)
		victim = ch->fighting;
	    else
	    {
		send_to_char("You can't do that.\n\r",ch);
		return;
	    }

	    if (victim != NULL)
	    {
		if (is_safe_spell(ch,victim,FALSE) && ch != victim)
		{
		    send_to_char("Something isn't right...\n\r",ch);
		    return;
		}

		vo = (void *) victim;
		target = TARGET_CHAR;
	    }
	    else
	    {
	    	vo = (void *) obj;
	    	target = TARGET_OBJ;
	    }
        break;


    case TAR_OBJ_CHAR_DEF:
	if (victim == NULL && obj == NULL)
	{
	    vo = (void *) ch;
	    target = TARGET_CHAR;
	}
	else if (victim != NULL)
	{
	    vo = (void *) victim;
	    target = TARGET_CHAR;
	}
	else
	{
	    vo = (void *) obj;
	    target = TARGET_OBJ;
	}
	
	break;
    }

    target_name = "";
    (*skill_table[sn].spell_fun) ( sn, level, ch, vo,target);

    

    if ( (skill_table[sn].target == TAR_CHAR_OFFENSIVE
    ||   (skill_table[sn].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR))
    &&   victim != ch
    &&   victim->master != ch )
    {
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

        if(has_killed_flag(ch))
         remove_killed_flag(ch);
	for ( vch = ch->in_room->people; vch; vch = vch_next )
	{
	    vch_next = vch->next_in_room;
	    if ( victim == vch && victim->fighting == NULL )
	    {
		multi_hit( victim, ch, TYPE_UNDEFINED );
		break;
	    }
	}
    }

    return;
}



/*
 * Spell functions.
 */
void spell_acid_blast( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }

    dam = dice(level/2+45,6); 
    if ( saves_spell( level, victim, DAM_ACID ) )
	dam /= 2;
    damage_old( ch, victim, dam, sn,DAM_ACID,TRUE);
    return;
}

void spell_armor( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
     
    if ( victim->fighting )
    {
      if(victim == ch)
       send_to_char("A faint glow surrounds you for a brief moment, then vanishes.\n\r",ch);
      else
       act("A faint glow surrounds $N for a brief moment, then vanishes.",
        ch,NULL,victim,TO_CHAR);
      return;
    }

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already clad in a suit of immaterial armor.\n\r",ch);
	else
	  act("$N is already clad in a suit of immaterial armor.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.where	 = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = 24;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = 0;
    af.subtype	 = level*number_range(1,3);
    affect_to_char( victim, &af );
    send_to_char( "You are clad in a suit of immaterial armor.\n\r", victim );
    if ( ch != victim )
	act("$N is clad in a suit of immaterial armor.",ch,NULL,victim,TO_CHAR);
    return;
}



void spell_bless( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    /* deal with the object case first */
    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;
	if (IS_OBJ_STAT(obj,ITEM_BLESS))
	{
	    act("$p is already blessed.",ch,obj,NULL,TO_CHAR);
	    return;
	}

	if (IS_OBJ_STAT(obj,ITEM_EVIL))
	{
	    AFFECT_DATA *paf;

	    paf = affect_find(obj->affected,gsn_curse);
	    if (!saves_dispel(level,paf != NULL ? paf->level : obj->level,0))
	    {
		if (paf != NULL)
		    affect_remove_obj(obj,paf);
		act("$p glows a pale blue.",ch,obj,NULL,TO_ALL);
		REMOVE_BIT(obj->extra_flags,ITEM_EVIL);
		return;
	    }
	    else
	    {
		act("The evil of $p is too powerful for you to overcome.",
		    ch,obj,NULL,TO_CHAR);
		return;
	    }
	}
	
	af.where	= TO_OBJECT;
	af.type		= sn;
	af.level	= level;
	af.duration	= 6 + level;
	af.location	= APPLY_SAVES;
	af.modifier	= -1;
	af.bitvector	= ITEM_BLESS;
	affect_to_obj(obj,&af);

	act("$p glows with a holy aura.",ch,obj,NULL,TO_ALL);
	return;
    }

    /* character target */
    victim = (CHAR_DATA *) vo;


    if ( victim->position == POS_FIGHTING || is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already blessed.\n\r",ch);
	else
	  act("$N already has divine favor.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = 6+level;
    af.location  = APPLY_HITROLL;
    af.modifier  = level / 8;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = 0 - level / 8;
    affect_to_char( victim, &af );
    send_to_char( "You feel righteous.\n\r", victim );
    if ( ch != victim )
	act("You grant $N the favor of your god.",ch,NULL,victim,TO_CHAR);
    return;
}



void spell_blindness( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    
    if( IS_AFFECTED(victim, AFF_BLIND))
    {
     act("{B$N has already been blinded.{x",ch,NULL,victim,TO_CHAR);
     return;
    }
    if( saves_spell(level,victim,DAM_OTHER))
    {
     send_to_char("{xYour attempt to blind them failed.\n\r{x",ch);
     return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.location  = APPLY_HITROLL;
    af.modifier  = -4;
    af.duration  = 1005+(level/5);
    af.bitvector = AFF_BLIND;
    affect_to_char( victim, &af );
    send_to_char( "You are blinded!\n\r", victim );
    act("$n appears to be blinded.",victim,NULL,NULL,TO_ROOM);
    return;
}

void spell_cancellation( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    bool found = FALSE;
 
    level += 2;

/*    if ((!IS_NPC(ch) && IS_NPC(victim) && 
	 !(IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) ) ||
        (IS_NPC(ch) && !IS_NPC(victim)) )
    {
	send_to_char("You failed, try dispel magic.\n\r",ch);
	return;
    }*/
    if(ch!=victim)
    {
      send_to_char("You failed, try dispel magic.\n\r",ch);
      return;
    }

    /* unlike dispel magic, the victim gets NO save */
 
    /* begin running through the spells */
 
    if (check_dispel(level,victim,skill_lookup("immaterial armor")))
    {
        act("$n's suit of immaterial armor vanishes.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("bless")))
        found = TRUE;
 
    /*if (check_dispel(level,victim,skill_lookup("blindness")))
    {
        found = TRUE;
        act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
    }*/

    if (check_dispel(level,victim,skill_lookup("charm person")))
    {
        found = TRUE;
        act("$n regains $s free will.",victim,NULL,NULL,TO_ROOM);
    }

    /*if (check_dispel(level,victim,skill_lookup("curse")))
        found = TRUE;*/
 
    if (check_dispel(level,victim,skill_lookup("detect evil")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("detect good")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect hidden")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect invis")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect hidden")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect magic")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("faerie fire")))
    {
        act("$n's outline fades.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("fly")))
    {
        act("$n falls to the ground!",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("frenzy")))
    {
	act("$n no longer looks so wild.",victim,NULL,NULL,TO_ROOM);;
	found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("vigor")))
    {
        act("$n no longer looks so vigorous.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("mastermind")))
    {
    	act("$n no longer looks so astute.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("haste")))
    {
	act("$n is no longer moving so quickly.",victim,NULL,NULL,TO_ROOM);
	found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("infravision")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("invis")))
    {
        act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("mass invis")))
    {
        act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("pass door")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("protection evil")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("protection good")))
        found = TRUE; 
   
    if (check_dispel(level,victim,skill_lookup("protection neutral")))
        found = TRUE;

    if (check_dispel(level, victim,skill_lookup("true sight")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("ward")))
        found = TRUE;
    if (check_dispel(level,victim,skill_lookup("fireshield")))
        found = TRUE;
    if (check_dispel(level,victim,skill_lookup("iceshield")))
        found = TRUE;
    if (check_dispel(level,victim,skill_lookup("earthshield")))
        found = TRUE;
    if (check_dispel(level,victim,skill_lookup("shockshield")))
        found = TRUE;
    if (check_dispel(level,victim,skill_lookup("prismaticshield")))
        found = TRUE;
    if (check_dispel(level,victim,skill_lookup("acidshield")))
        found = TRUE;
    if (check_dispel(level,victim,skill_lookup("bladebarrier")))
        found = TRUE;
    if (check_dispel(level,victim,skill_lookup("shadowshield")))
        found = TRUE;
    if (check_dispel(level,victim,skill_lookup("mana shield")))
    {
        act("$n's mana shield vanishes into thin air.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
    if (check_dispel(level,victim,skill_lookup("ultravision")))
     found = TRUE;
    if (check_dispel(level,victim,skill_lookup("sanctuary")))
    {
        act("The white aura around $n's body vanishes.",
            victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("force shield")))
    {
        act("The force shield protecting $n vanishes.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("sleep")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("slow")))
    {
        act("$n is no longer moving so slowly.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("stone skin")))
    {
        act("$n's skin regains its normal texture.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("weaken")))
    {
        act("$n looks stronger.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("divine blessing")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("infernal offering")))
        found = TRUE;
    if (check_dispel(level,victim,skill_lookup("outrage")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("essence of mist")))
    {
        act("$n's body returns to its true form.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
    if (check_dispel(level,victim,skill_lookup("essence of bat")))
    {
        act("$n's body returns to its true form.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
    if (check_dispel(level,victim,skill_lookup("essence of wolf")))
    {
        act("$n's body returns to its true form.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
    if (check_dispel(level,victim,skill_lookup("resistance")))
        found = TRUE;
    if (check_dispel(level,victim,skill_lookup("protective force")))
        found = TRUE;
    if (check_dispel(level,victim,skill_lookup("shrink")))
    {
        act("$n's body returns to its true size.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
    if (found)
        send_to_char("Ok.\n\r",ch);
    else
        send_to_char("Spell failed.\n\r",ch);
}

void spell_cause_damage( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    int dam;
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }
    dam = 10+level/3+dice(4,level);
    if(saves_spell(level,victim,DAM_HARM))
       dam/=2;
    damage_old( ch, victim, 10+level/3+dice(3,level), sn,DAM_HARM,TRUE);
    return;
}

void spell_chain_lightning(int sn,int level,CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *tmp_vict,*last_vict,*next_vict;
    bool found;
    int dam;

    /* first strike */

    act("A lightning bolt leaps from $n's hand and arcs to $N.",
        ch,NULL,victim,TO_ROOM);
    act("A lightning bolt leaps from your hand and arcs to $N.",
	ch,NULL,victim,TO_CHAR);
    act("A lightning bolt leaps from $n's hand and hits you!",
	ch,NULL,victim,TO_VICT);  

    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }
    dam = dice(level,5);
    if (saves_spell(level,victim,DAM_LIGHTNING))
 	dam /= 3;
    damage_old(ch,victim,dam,sn,DAM_LIGHTNING,TRUE);
    last_vict = victim;
    level -= 4;   /* decrement damage */

    /* new targets */
    while (level > 33)
    {
	found = FALSE;
	for (tmp_vict = ch->in_room->people; 
	     tmp_vict != NULL; 
	     tmp_vict = next_vict) 
	{
	  next_vict = tmp_vict->next_in_room;
	  if (!is_safe_spell(ch,tmp_vict,TRUE) && tmp_vict != last_vict)
	  {
	    found = TRUE;
	    last_vict = tmp_vict;
	    act("The bolt arcs to $n!",tmp_vict,NULL,NULL,TO_ROOM);
	    act("The bolt hits you!",tmp_vict,NULL,NULL,TO_CHAR);
	    dam = dice(level,5);
	    if (saves_spell(level,tmp_vict,DAM_LIGHTNING))
		dam /= 3;
	    damage_old(ch,tmp_vict,dam,sn,DAM_LIGHTNING,TRUE);
	    level -= 4;  /* decrement damage */
	  }
	}   /* end target searching loop */
	
	if (!found) /* no target found, hit the caster */
	{
	  if (ch == NULL)
     	    return;

	  if (last_vict == ch) /* no double hits */
	  {
	    act("The bolt seems to have fizzled out.",ch,NULL,NULL,TO_ROOM);
	    act("The bolt grounds out through your body.",
		ch,NULL,NULL,TO_CHAR);
	    return;
	  }
	
	  last_vict = ch;
	  act("The bolt arcs to $n...whoops!",ch,NULL,NULL,TO_ROOM);
	  send_to_char("You are struck by your own lightning!\n\r",ch);
	  dam = dice(level,5);
	  if (saves_spell(level,ch,DAM_LIGHTNING))
	    dam /= 3;
	  damage_old(ch,ch,dam,sn,DAM_LIGHTNING,TRUE);
	  level -= 4;  /* decrement damage */
	  if (ch == NULL) 
	    return;
	}
    /* now go back and find more targets */
    }
}
	  
void spell_charm_person( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    CHAR_DATA *fch,*fch_next;
    int count=1,mc=UMIN(MAX_CHARMIES,1+ch->level/25);

    if (is_safe(ch,victim)) return;

    if ( victim == ch )
    {
	send_to_char( "You like yourself even better!\n\r", ch );
	return;
    }
    for(fch = ch->in_room->people;fch!=NULL;fch=fch_next)
    {
     fch_next = fch->next_in_room;
     if(fch->master == ch && IS_AFFECTED(fch,AFF_CHARM))
       count++;
    }
    if(count>mc)
    {
     printf_to_char(ch,"You can't charm more than %d mobiles at your level.\n\r",
       mc);
     return;
    }
    if ( IS_AFFECTED(victim, AFF_CHARM)
    ||   IS_AFFECTED(ch, AFF_CHARM)
    ||   level < victim->level
    ||   IS_SET(victim->imm_flags,IMM_CHARM)
    ||   saves_spell( level, victim,DAM_CHARM) )
    {
        send_to_char("You failed.\n\r",ch);
        if(has_killed_flag(ch))
         remove_killed_flag(ch);
        multi_hit(victim,ch,TYPE_UNDEFINED);
	return;
    }

    if (IS_SET(victim->in_room->room_flags,ROOM_LAW))
    {
	send_to_char(
	    "The mayor does not allow charming in the city limits.\n\r",ch);
	return;
    }
  
    if ( victim->master )
	stop_follower( victim );
    add_follower( victim, ch );
    victim->leader = ch;
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = number_fuzzy( level / 4 );
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( victim, &af );
    act( "Isn't $n just so nice?", ch, NULL, victim, TO_VICT );
    if ( ch != victim )
	act("$N looks at you with adoring eyes.",ch,NULL,victim,TO_CHAR);
    return;
}

void spell_continual_light(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    OBJ_DATA *light;

    if (target_name[0] != '\0')  /* do a glow on some object */
    {
	light = get_obj_carry(ch,target_name);
	
	if (light == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}

	if (IS_OBJ_STAT(light,ITEM_GLOW))
	{
	    act("$p is already glowing.",ch,light,NULL,TO_CHAR);
	    return;
	}

	SET_BIT(light->extra_flags,ITEM_GLOW);
	act("$p glows with a white light.",ch,light,NULL,TO_ALL);
	return;
    }

    light = create_object( get_obj_index( OBJ_VNUM_LIGHT_BALL ), 0 );
    obj_to_room( light, ch->in_room );
    act( "$n twiddles $s thumbs and $p appears.",   ch, light, NULL, TO_ROOM );
    act( "You twiddle your thumbs and $p appears.", ch, light, NULL, TO_CHAR );
    return;
}

void spell_cure_blindness(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( !is_affected( victim, gsn_blindness ) )
    {
        if (victim == ch)
          send_to_char("You aren't blind.\n\r",ch);
        else
          act("$N doesn't appear to be blinded.",ch,NULL,victim,TO_CHAR);
        return;
    }
 
    if (check_dispel(level+4,victim,gsn_blindness))
    {
        send_to_char( "Your vision returns!\n\r", victim );
        act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
    }
    else
        send_to_char("Spell failed.\n\r",ch);
}



void spell_cure_wounds( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int heal;

    heal = dice(2, level) + level/3 + 10;
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );
    send_to_char( "Your wounds start to heal and you feel better.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}

/* RT added to cure plague */
void spell_cure_disease( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( !is_affected( victim, gsn_plague ) )
    {
        if (victim == ch)
          send_to_char("You aren't ill.\n\r",ch);
        else
          act("$N doesn't appear to be diseased.",ch,NULL,victim,TO_CHAR);
        return;
    }
    
    if (check_dispel(level+2,victim,gsn_plague))
    {
	send_to_char("Your sores vanish.\n\r",victim);
	act("$n looks relieved as $s sores vanish.",victim,NULL,NULL,TO_ROOM);
    }
    else
	send_to_char("Spell failed.\n\r",ch);
}

void spell_cure_poison( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
 
    if ( !is_affected( victim, gsn_poison ) )
    {
        if (victim == ch)
          send_to_char("You aren't poisoned.\n\r",ch);
        else
          act("$N doesn't appear to be poisoned.",ch,NULL,victim,TO_CHAR);
        return;
    }
 
    if (check_dispel(level+2,victim,gsn_poison))
    {
        send_to_char("A warm feeling runs through your body.\n\r",victim);
        act("$n looks much better.",victim,NULL,NULL,TO_ROOM);
    }
    else
        send_to_char("Spell failed.\n\r",ch);
}

void spell_curse( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    /* deal with the object case first */
    if (target == TARGET_OBJ)
    {
        obj = (OBJ_DATA *) vo;
        if (IS_OBJ_STAT(obj,ITEM_EVIL))
        {
            act("$p is already filled with evil.",ch,obj,NULL,TO_CHAR);
            return;
        }

        if (IS_OBJ_STAT(obj,ITEM_BLESS))
        {
            AFFECT_DATA *paf;

            paf = affect_find(obj->affected,skill_lookup("bless"));
            if (!saves_dispel(level,paf != NULL ? paf->level : obj->level,0))
            {
                if (paf != NULL)
                    affect_remove_obj(obj,paf);
                act("$p glows with a red aura.",ch,obj,NULL,TO_ALL);
                REMOVE_BIT(obj->extra_flags,ITEM_BLESS);
                return;
            }
            else
            {
                act("The holy aura of $p is too powerful for you to overcome.",
                    ch,obj,NULL,TO_CHAR);
                return;
            }
        }

        af.where        = TO_OBJECT;
        af.type         = sn;
        af.level        = level;
        af.duration     = 2 * level;
        af.location     = APPLY_SAVES;
        af.modifier     = +1;
        af.bitvector    = ITEM_EVIL;
        affect_to_obj(obj,&af);

        act("$p glows with a malevolent aura.",ch,obj,NULL,TO_ALL);
        return;
    }

    /* character curses */
    victim = (CHAR_DATA *) vo;
    if (IS_AFFECTED(victim,AFF_CURSE))
    {
     act("{B$N already looks very uncomfortable.{x",ch,NULL,victim,TO_CHAR);
     return;
    }
    if (saves_spell(level,victim,DAM_NEGATIVE))
    {
     send_to_char("{xYour attempt to curse them failed.{x\n\r",ch);
     return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 1005+(level/4);
    af.location  = APPLY_HITROLL;
    af.modifier  = -1 * (level / 8);
    af.bitvector = AFF_CURSE;
    affect_to_char( victim, &af );

    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = level / 8;
    affect_to_char( victim, &af );

    send_to_char( "You feel unclean.\n\r", victim );
    if ( ch != victim )
	act("$N looks very uncomfortable.",ch,NULL,victim,TO_CHAR);
    return;
}

/* RT replacement demonfire spell */

void spell_demonfire(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if ( !IS_NPC(ch) && !IS_EVIL(ch) )
    {
        victim = ch;
	send_to_char("The demons turn upon you!\n\r",ch);
    }

    ch->alignment = UMAX(-1000, ch->alignment - 50);
    if ( ch->pet != NULL )
	ch->pet->alignment = ch->alignment;

    if (victim != ch)
    {
	act("$n calls forth the demons of Hell upon $N!",
	    ch,NULL,victim,TO_NOTVICT);
        act("$n has assailed you with the demons of Hell!",
	    ch,NULL,victim,TO_VICT);
	send_to_char("You conjure forth the demons of hell!\n\r",ch);
    }
    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }
    dam = dice( level, 5 );
    if ( saves_spell( level, victim,DAM_NEGATIVE) )
        dam /= 2;
    damage_old( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE);
    spell_curse(gsn_curse, level, ch, (void *) victim,TARGET_CHAR);
}

void spell_sandstorm(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if (victim != ch)
    {
	act("$n calls forth a blinding sandstorm upon $N!",
	    ch,NULL,victim,TO_NOTVICT);
        act("$n has assailed you with a blinding sandstorm!",
	    ch,NULL,victim,TO_VICT);
	send_to_char("You conjure forth a blinding sandstorm!\n\r",ch);
    }
    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }
    dam = dice( level, 4 );
    if ( saves_spell( level, victim,DAM_EARTH) )
        dam /= 2;
    damage_old( ch, victim, dam, sn, DAM_EARTH ,TRUE);
    spell_blindness(gsn_blindness, level+1, ch, (void *) victim,TARGET_CHAR);
}

void spell_elemental_fury(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if (victim != ch)
    {
	act("$n summons the elements to strike $N with a furious attack!",
	    ch,NULL,victim,TO_NOTVICT);
        act("$n summons the elements to strike you with a furious attack!",
	    ch,NULL,victim,TO_VICT);
	send_to_char("You summon the elements to strike your opponent!\n\r",ch);
    }
    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }
    dam = dice(level,6);
    if ( saves_spell( level, victim,DAM_HOLY) )
        dam /= 2;
    damage_old( ch, victim, dam, sn, DAM_HOLY ,TRUE);
}

void spell_blaze(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    act("An intense outburst of blazing flames scorches $n!",
        victim,NULL,NULL,TO_ROOM);
    send_to_char("An intense outburst of blazing flames scorches you!\n\r",victim);

    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }

    dam = dice(level,8);
    if ( saves_spell( level, victim,DAM_FIRE) )
        dam /= 2;
    damage_old( ch, victim, dam, sn, DAM_FIRE ,TRUE);
}

void spell_hailstorm(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if (victim != ch)
    {
	act("$n summons a devastating hailstorm!",
	    ch,NULL,NULL,TO_ROOM);
	send_to_char("You summon a devastating hailstorm!\n\r",ch);
    }
    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }
    dam = dice(level,7);
    if ( saves_spell( level, victim,DAM_COLD) )
        dam /= 2;
    damage_old( ch, victim, dam, sn, DAM_COLD,TRUE);
}

void spell_death_touch(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
   CHAR_DATA *victim = (CHAR_DATA *) vo;
   int dam;

   if (victim != ch)
    {
	act("$n raises $s hand and touches $N.",
	    ch,NULL,victim,TO_NOTVICT);
        act("$n raises $s hand and touches you.",
	    ch,NULL,victim,TO_VICT);
	send_to_char("You raise your hand and touch your opponent.\n\r",ch);
    }
   if((ch->fighting==NULL) && (!IS_NPC(ch)) && (!IS_NPC(victim)))
   {
    ch->attacker=TRUE;
    victim->attacker=FALSE;
   }
   dam = dice(level,5);
   if( saves_spell(level,victim,DAM_NEGATIVE))
    dam /= 2;
   if( (number_percent()<(40+level-victim->level)) &&(ch!=victim) )
   {
    victim->stunned=1;
    DAZE_STATE(victim,2*PULSE_VIOLENCE);
    FLEE_DAZE(victim,80);
   }
   damage(ch,victim,dam,sn,DAM_NEGATIVE,TRUE);
}

void spell_bloodbath(int sn,int level,CHAR_DATA *ch,void *vo, int target)
{
   CHAR_DATA *victim = (CHAR_DATA *) vo;
   AFFECT_DATA af;

   if(victim==ch)
   {
    send_to_char("You dare not cast this spell on yourself.\n\r",ch);
    return;
   }

   if(is_affected(victim,gsn_bloodbath))
   {
    send_to_char("Blood is alreay gushing forth from your opponent's wounds.\n\r",ch);
    return;
   }
   if(saves_spell(level,victim,DAM_HARM))
   {
    send_to_char("Spell failed.\n\r",ch);
    return;
   }
   if (victim != ch)
   {
	act("{x$N screams in terrible agony as {Rb{rl{Ro{ro{Rd{x gushes forth from $s many wounds!",
	    ch,NULL,victim,TO_NOTVICT);
        send_to_char("{xYou scream in terrible agony as {Rb{rl{Ro{ro{Rd{x gushes forth from your many wounds!\n\r",victim);
	act("{x$N screams in terrible agony as {Rb{rl{Ro{ro{Rd{x gushes forth from $s many wounds!",
	    ch,NULL,victim,TO_CHAR);

   }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 1005+(level/4);
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    damage(ch,victim,1,sn,DAM_HARM,FALSE);
}

void spell_swarm(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if (victim != ch)
    {
	act("$n calls forth a swarm of diseases upon $N!",
	    ch,NULL,victim,TO_NOTVICT);
        act("$n has assailed you with a swarm of diseases!",
	    ch,NULL,victim,TO_VICT);
	send_to_char("You conjure forth a swarm of diseases!\n\r",ch);
    }
    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }
    dam = dice( level, 4 );
    if ( saves_spell( level, victim,DAM_DISEASE) )
        dam /= 2;
    damage_old( ch, victim, dam, sn, DAM_DISEASE ,TRUE);
    spell_blindness(gsn_blindness, level, ch, (void *) victim,TARGET_CHAR);
    spell_slow(skill_lookup("slow"),level,ch,(void*)victim,TARGET_CHAR);
}

void spell_detect_evil( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_EVIL) )
    {
	if (victim == ch)
	  send_to_char("You can already sense evil.\n\r",ch);
	else
	  act("$N can already detect evil.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_EVIL;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_ultravision( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected(victim,gsn_ultravision) )
    {
	if (victim == ch)
	  send_to_char("You already have ultravision.\n\r",ch);
	else
	  act("$N already has ultravision.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.where     = TO_AFFECTS;
    af.type      = gsn_ultravision;
    af.level	 = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "Your vision becomes perfect.\n\r", victim );
    act("$n's eyes {Wg{Yl{yo{Ww{x with a ghostly light.",victim,NULL,NULL,TO_ROOM);
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_detect_good( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( IS_AFFECTED(victim, AFF_DETECT_GOOD) )
    {
        if (victim == ch)
          send_to_char("You can already sense good.\n\r",ch);
        else
          act("$N can already detect good.",ch,NULL,victim,TO_CHAR);
        return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_GOOD;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_detect_hidden(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_HIDDEN) )
    {
        if (victim == ch)
          send_to_char("You are already as alert as you can be. \n\r",ch);
        else
          act("$N can already sense hidden lifeforms.",ch,NULL,victim,TO_CHAR);
        return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_DETECT_HIDDEN;
    affect_to_char( victim, &af );
    send_to_char( "Your awareness improves.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_true_sight(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
 CHAR_DATA *victim = (CHAR_DATA*) vo;
 AFFECT_DATA af;

 if(is_affected(victim,sn))
 {
  if(victim==ch)
   send_to_char("You already have true sight.\n\r",victim);
  else
   act("$N already has true sight.",ch,NULL,victim,TO_CHAR);
  return;
 }

 af.where = TO_AFFECTS;
 af.type = sn;
 af.level = level;
 af.duration = 50 + level/2;
 af.modifier = 0;
 af.location = APPLY_NONE;
 af.bitvector = AFF_DETECT_HIDDEN;
 affect_to_char(victim,&af);

 af.bitvector = AFF_DETECT_INVIS;
 affect_to_char(victim,&af);
 
 af.bitvector = AFF_DETECT_MAGIC;
 affect_to_char(victim,&af);

 af.bitvector = AFF_DETECT_GOOD;
 affect_to_char(victim,&af);

 af.bitvector = AFF_DETECT_EVIL;
 affect_to_char(victim,&af);
 
 send_to_char("Your vision sharpens.\n\r",victim);

 if(ch!=victim)
  send_to_char("Ok.\n\r",ch);
 return;
}

void spell_detect_invis( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_INVIS) )
    {
        if (victim == ch)
          send_to_char("You can already see invisible.\n\r",ch);
        else
          act("$N can already see invisible things.",ch,NULL,victim,TO_CHAR);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_INVIS;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_detect_magic( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_DETECT_MAGIC) )
    {
        if (victim == ch)
          send_to_char("You can already sense magical auras.\n\r",ch);
        else
          act("$N can already detect magic.",ch,NULL,victim,TO_CHAR);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_MAGIC;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_detect_poison( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;

    if ( obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD )
    {
	if ( obj->value[3] != 0 )
	    send_to_char( "You smell poisonous fumes.\n\r", ch );
	else
	    send_to_char( "It looks delicious.\n\r", ch );
    }
    else
    {
	send_to_char( "It doesn't look poisoned.\n\r", ch );
    }

    return;
}

/* modified for enhanced use */

void spell_dispel_magic( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    bool found = FALSE;

    if(!IS_NPC(ch))
    {
     if((ch->class==class_lookup("wizard"))
        || (ch->class==class_lookup("sorcerer")))
           level+=2;
     else if((ch->class==class_lookup("shaman"))
        || (ch->class==class_lookup("elder"))
        || (ch->class==class_lookup("priest"))
        || (ch->class==class_lookup("bishop")))
           level+=1;
    }

    if (saves_spell(level, victim,DAM_OTHER))
    {
	send_to_char( "You feel a brief tingling sensation.\n\r",victim);
	send_to_char( "You failed.\n\r", ch);
	return;
    }

    /* begin running through the spells */ 

    if (check_dispel(level,victim,skill_lookup("immaterial armor")))
    {
        found = TRUE;
        act("$n's suit of immaterial armor vanishes.",victim,NULL,NULL,TO_ROOM);    }
 
    if (check_dispel(level,victim,skill_lookup("bless")))
        found = TRUE;
 
    /*if (check_dispel(level,victim,skill_lookup("blindness")))
    {
        found = TRUE;
        act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
    }*/
 
    if (check_dispel(level,victim,skill_lookup("charm person")))
    {
        found = TRUE;
        act("$n regains $s free will.",victim,NULL,NULL,TO_ROOM);
    }
 
    /*if (check_dispel(level,victim,skill_lookup("curse")))
        found = TRUE;*/
 
    if (check_dispel(level,victim,skill_lookup("detect evil")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("detect good")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect hidden")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect invis")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect hidden")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("detect magic")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("faerie fire")))
    {
        act("$n's outline fades.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("fly")))
    {
        act("$n falls to the ground!",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("frenzy")))
    {
        act("$n no longer looks so wild.",victim,NULL,NULL,TO_ROOM);;
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("vigor")))
    {
        act("$n no longer looks so vigorous.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("mastermind")))
    {
    	act("$n no longer looks so astute.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("haste")))
    {
        act("$n is no longer moving so quickly.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("infravision")))
        found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("invis")))
    {
        act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("mass invis")))
    {
        act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("pass door")))
        found = TRUE;
 

    if (check_dispel(level,victim,skill_lookup("protection evil")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("protection good")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("protection neutral")))
       found = TRUE;
 
    if (check_dispel(level,victim,skill_lookup("sanctuary")))
    {
        act("The white aura around $n's body vanishes.",
            victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (IS_SHIELDED(victim,SHD_SANCTUARY) 
	&& !saves_dispel(level, victim->level,-1)
	&& !is_affected(victim,skill_lookup("sanctuary")))
    {
	REMOVE_BIT(victim->shielded_by,SHD_SANCTUARY);
        act("The white aura around $n's body vanishes.",
            victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (IS_SHIELDED(victim,SHD_SANCTUARY) 
	&& !saves_dispel(level, victim->level,-1)
	&& !is_affected(victim,skill_lookup("sanctuary")))
    {
	REMOVE_BIT(victim->shielded_by,SHD_SANCTUARY);
        act("The white aura around $n's body vanishes.",
            victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("force shield")))
    {
        act("The force shield protecting $n vanishes.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("sleep")))
        found = TRUE;

    /*if (check_dispel(level,victim,skill_lookup("slow")))
    {
        act("$n is no longer moving so slowly.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }*/
 
    if (check_dispel(level,victim,skill_lookup("stone skin")))
    {
        act("$n's skin regains its normal texture.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
 
    if (check_dispel(level,victim,skill_lookup("mana shield")))
    {
        act("$n's mana shield vanishes into thin air.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
    if (check_dispel(level,victim,skill_lookup("essence of mist")))
    {
        act("$n's body returns to its true form.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
    if (check_dispel(level,victim,skill_lookup("essence of bat")))
    {
        act("$n's body returns to its true form.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
    if (check_dispel(level,victim,skill_lookup("essence of wolf")))
    {
        act("$n's body returns to its true form.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (found)
        send_to_char("Ok.\n\r",ch);
    else
        send_to_char("Spell failed.\n\r",ch);
	return;
}

void spell_earthquake( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    send_to_char( "The earth trembles beneath your feet!\n\r", ch );
    act( "$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
	vch_next	= vch->next;
	if ( vch->in_room == NULL )
	    continue;
	if ( vch->in_room == ch->in_room )
	{
	    if ( vch != ch && !is_safe_spell(ch,vch,TRUE))
	    {
		if ( ( ch->fighting == NULL )
		&& ( !IS_NPC( ch ) )
		&& ( !IS_NPC( vch ) ) )
		{
		    ch->attacker = TRUE;
		    vch->attacker = FALSE;
		}
		if (IS_AFFECTED(vch,AFF_FLYING))
		    damage_old(ch,vch,0,sn,DAM_BASH,TRUE);
		else
		    damage_old( ch,vch,level + dice(2, 8), sn, DAM_BASH,TRUE);
	    }
	    continue;
	}

	if ( vch->in_room->area == ch->in_room->area )
	    send_to_char( "The earth trembles and shivers.\n\r", vch );
    }

    return;
}

/*
 * Drain XP, MANA, HP.
 * Caster gains HP.
 */
void spell_energy_drain( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if (victim != ch)
    {
        ch->alignment = UMAX(-1000, ch->alignment - 50);
	if ( ch->pet != NULL )
	    ch->pet->alignment = ch->alignment;
    }

    if ( saves_spell( level, victim,DAM_NEGATIVE) )
    {
	send_to_char("You feel a momentary chill.\n\r",victim);
        if(ch!=victim)
          send_to_char("You failed.\n\r",ch);  	
	return;
    }


    if ( victim->level <= 2 )
    {
	dam		 = ch->hit + 1;
    }
    else
    {
	gain_exp( victim, 0 - number_range( level/2, 3 * level / 2 ) );
	victim->mana	/= 2;
	victim->move	/= 2;
	dam		 = number_range(level, 2*level);
	ch->hit		+= dam;
    }
    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }

    send_to_char("You feel your life slipping away!\n\r",victim);
    send_to_char("Wow....what a rush!\n\r",ch);
    damage_old( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE);

    return;
}

void spell_corrupt_potion(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
 OBJ_DATA *obj = (OBJ_DATA *) vo;
 AFFECT_DATA af,*paf=NULL;

 if(obj->pIndexData->item_type!=ITEM_POTION)
 {
  send_to_char("You can only corrupt potions.\n\r",ch);
  return;
 }
 if(affect_find(obj->affected,sn))
 {
  act("$p has already been corrupted.",ch,obj,NULL,TO_CHAR);
  return;
 }

 af.where = TO_OBJECT;
 af.type  = sn;
 af.level = level;
 af.duration = -1;
 af.location = APPLY_NONE;
 af.modifier = 0;
 af.bitvector = 0;

 affect_to_obj(obj,&af);
 obj->value[0] = level;
 obj->value[1] = skill_lookup("weaken");
 obj->value[2] = skill_lookup("poison");
 obj->value[3] = skill_lookup("curse");
 obj->value[4] = skill_lookup("plague");

 act("You corrupt $p.",ch,obj,NULL,TO_CHAR);
 act("$n corrupts $p.",ch,obj,NULL,TO_ROOM);
}

void spell_fireproof(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;
 
    if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF) || IS_OBJ_STAT(obj,ITEM_QUEST))
    {
        act("$p is already protected from burning.",ch,obj,NULL,TO_CHAR);
        return;
    }
 
    af.where     = TO_OBJECT;
    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy(level / 2);
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = ITEM_BURN_PROOF;
 
    affect_to_obj(obj,&af);
 
    act("You protect $p from fire.",ch,obj,NULL,TO_CHAR);
    act("$p is surrounded by a protective aura.",ch,obj,NULL,TO_ROOM);
}

void spell_faerie_fire( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) )
    {
        act("$N is already surrounded by a pink outline.\n\r",ch,NULL,
            victim,TO_CHAR);
	return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = 1005+(level/5);
    af.location  = APPLY_AC;
    af.modifier  = 2 * level;
    af.bitvector = AFF_FAERIE_FIRE;
    affect_to_char( victim, &af );
    send_to_char( "You are surrounded by a pink outline.\n\r", victim );
    act( "$n is surrounded by a pink outline.", victim, NULL, NULL, TO_ROOM );
    return;
}

void spell_faerie_fog( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *ich;

    act( "$n conjures a cloud of purple smoke.", ch, NULL, NULL, TO_ROOM );
    send_to_char( "You conjure a cloud of purple smoke.\n\r", ch );

    for ( ich = ch->in_room->people; ich != NULL; ich = ich->next_in_room )
    {
	if (ich->invis_level > 0)
	    continue;

	if ( ich == ch || saves_spell( level, ich,DAM_OTHER) )
	    continue;

	affect_strip ( ich, gsn_invis			);
	affect_strip ( ich, gsn_mass_invis		);
	affect_strip ( ich, gsn_sneak			);
        affect_strip ( ich, gsn_shadow_walk		);
        affect_strip ( ich, gsn_ambush			);
	REMOVE_BIT   ( ich->affected_by, AFF_HIDE	);
	REMOVE_BIT   ( ich->shielded_by, SHD_INVISIBLE	);
	REMOVE_BIT   ( ich->affected_by, AFF_SNEAK	);
        REMOVE_BIT   ( ich->shielded_by, SHD_INVISIBLE	);
        REMOVE_BIT   ( ich->shielded_by, SHD_SWALK	);
	act( "$n is revealed!", ich, NULL, NULL, TO_ROOM );
	send_to_char( "You are revealed!\n\r", ich );
    }

    return;
}

void spell_fly( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_FLYING) )
    {
	if (victim == ch)
	  send_to_char("You are already airborne.\n\r",ch);
	else
	  act("$N doesn't need your help to fly.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level + 3;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_FLYING;
    affect_to_char( victim, &af );
    send_to_char( "Your feet rise off the ground.\n\r", victim );
    act( "$n's feet rise off the ground.", victim, NULL, NULL, TO_ROOM );
    return;
}

/* RT clerical berserking spell */

void spell_frenzy(int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim,sn) || IS_AFFECTED(victim,AFF_BERSERK))
    {
	if (victim == ch)
	  send_to_char("You are already in a frenzy.\n\r",ch);
	else
	  act("$N is already in a frenzy.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if ((IS_GOOD(ch) && !IS_GOOD(victim)) ||
	(IS_NEUTRAL(ch) && !IS_NEUTRAL(victim)) ||
	(IS_EVIL(ch) && !IS_EVIL(victim))
       )
    {
	act("Your god doesn't seem to like $N",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type 	 = sn;
    af.level	 = level;
    af.duration	 = level / 3;
    af.modifier  = level / 6;
    af.bitvector = 0;

    af.location  = APPLY_HITROLL;
    affect_to_char(victim,&af);

    af.location  = APPLY_DAMROLL;
    affect_to_char(victim,&af);

    af.modifier  = 10 * (level / 12);
    af.location  = APPLY_AC;
    affect_to_char(victim,&af);

    send_to_char("You are filled with holy wrath!\n\r",victim);
    act("$n gets a wild look in $s eyes!",victim,NULL,NULL,TO_ROOM);
}

/* RT ROM-style gate */
    
void spell_gate( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    bool gate_pet;
    

    if(global_quest==QUEST_CLOSED && ch->on_quest &&
      IS_SET(quest_flags,QUEST_NOGATE))
    {
	send_to_char("You can't use the gate spell for the duration of the quest.\n\r",ch);
        return;
    }
    if(has_pkflag(ch))
    {
     send_to_char("Your heart is pounding too fast.\n\r",ch);
     return;
    }
    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   victim->in_room == NULL
    ||   !can_see_room(ch,victim->in_room) 
    ||   (IS_SET(victim->in_room->room_flags, ROOM_SAFE)
          && IS_NPC(victim))
    ||   (!IS_NPC(victim) && IS_SET(victim->in_room->room_flags,ROOM_SAFE)
          && (game_state!=GS_PARADOX))
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
    || 	 (victim->in_room->clan>0)
    ||	 (IS_NPC(victim) 
          && ((victim->level>=level+3)
              || IS_SET(victim->imm_flags,IMM_SUMMON)
              || saves_spell(level,victim,DAM_OTHER)))
    ||	 (!IS_NPC(ch) 
          && !IS_NPC(victim) 
          && ((!is_clan(ch) 
              && IS_SET(victim->act,PLR_NOSUMMON))
              || (is_clan(ch) && !in_range(ch,victim) && !is_same_clan(ch,victim)))))
    {
     send_to_char("You failed.\n\r",ch);
     return;
    }

    if(strstr(victim->in_room->area->builders,"Unlinked"))
    {
      send_to_char("You can't gate to areas that aren't linked yet!\n\r",ch);
      return;
    }
    
    if (ch->pet != NULL && ch->in_room == ch->pet->in_room)
	gate_pet = TRUE;
    else
	gate_pet = FALSE;
    
    act("$n steps through a gate and vanishes.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You step through a gate and vanish.\n\r",ch);
    char_from_room(ch);
    char_to_room(ch,victim->in_room);

    act("$n has arrived through a gate.",ch,NULL,NULL,TO_ROOM);
    do_look(ch,"auto");

    if (gate_pet)
    {
	act("$n steps through a gate and vanishes.",ch->pet,NULL,NULL,TO_ROOM);
	send_to_char("You step through a gate and vanish.\n\r",ch->pet);
	char_from_room(ch->pet);
	char_to_room(ch->pet,victim->in_room);
	act("$n has arrived through a gate.",ch->pet,NULL,NULL,TO_ROOM);
	do_look(ch->pet,"auto");
    }
}



void spell_vigor(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You already feel unusually vigorous.\n\r",ch);
	else
	  act("$N looks already unusually vigorous.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level/2;
    af.location  = APPLY_STR_P;
    af.modifier  = 2;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    af.location  = APPLY_STR;
    affect_to_char( victim, &af );
    af.location  = APPLY_CON_P;
    affect_to_char( victim, &af );
    af.location  = APPLY_CON;
    affect_to_char( victim, &af );
    send_to_char( "Your body is filled with vigor.\n\r", victim );
    act("$n's body is filled with vigor.",victim,NULL,NULL,TO_ROOM);
    return;
}

void spell_mastermind(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You already feel unusually astute.\n\r",ch);
	else
	  act("$N looks already unusually astute.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level/2;
    af.location  = APPLY_WIS_P;
    af.modifier  = 2;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    af.location  = APPLY_WIS;
    affect_to_char( victim, &af );
    af.location  = APPLY_INT_P;
    affect_to_char( victim, &af );
    af.location  = APPLY_INT;
    affect_to_char( victim, &af );
    send_to_char( "Your perception of reality is altered and you feel incredibly ingenious.\n\r", victim );
    act("$n watches $s surroundings with keen perception.",victim,NULL,NULL,TO_ROOM);
    return;
}

/* RT haste spell */

void spell_haste( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_HASTE)
    ||   IS_SET(victim->off_flags,OFF_FAST))
    {
	if (victim == ch)
	  send_to_char("You can't move any faster!\n\r",ch);
 	else
	  act("$N is already moving as fast as $E can.",
	      ch,NULL,victim,TO_CHAR);
        return;
    }

    if (is_affected(victim,gsn_cascade))
    {
      if (victim == ch)
       send_to_char("You struggle against the waves but to no avail.\n\r",ch);
      else
       act("$N is surrounded by waves keeping you from increasing $s speed.",
             ch,NULL,victim,TO_CHAR);
      return;
    }

    if (IS_AFFECTED(victim,AFF_SLOW))
    {
	if (!check_dispel(level,victim,skill_lookup("slow")))
	{
	    if (victim != ch)
	        send_to_char("Spell failed.\n\r",ch);
	    send_to_char("You feel momentarily faster.\n\r",victim);
	    return;
	}
        act("$n is moving less slowly.",victim,NULL,NULL,TO_ROOM);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    if (victim == ch)
      af.duration  = level/2;
    else
      af.duration  = level/4;
    af.location  = APPLY_DEX;
    af.modifier  = 1 + (level >= 18) + (level >= 25) + (level >= 32);
    af.bitvector = AFF_HASTE;
    affect_to_char( victim, &af );
    send_to_char( "You feel yourself moving more quickly.\n\r", victim );
    act("$n is moving more quickly.",victim,NULL,NULL,TO_ROOM);
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}



void spell_heal( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    victim->hit = UMIN( victim->hit + 100 + level*2, victim->max_hit );
    update_pos( victim );
    send_to_char( "A warm feeling fills your body.\n\r", victim );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_heat_metal( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj_lose, *obj_next;
    int dam = 0;
    bool fail = TRUE;
 
   if (!saves_spell(level + 2,victim,DAM_FIRE) 
   &&  !IS_SET(victim->imm_flags,IMM_FIRE))
   {
        for ( obj_lose = victim->carrying;
	      obj_lose != NULL; 
	      obj_lose = obj_next)
        {
	    obj_next = obj_lose->next_content;
            if ( number_range(1,2 * level) > obj_lose->level 
	    &&   !saves_spell(level,victim,DAM_FIRE)
	    &&   !IS_OBJ_STAT(obj_lose,ITEM_NONMETAL)
	    &&   !IS_OBJ_STAT(obj_lose,ITEM_BURN_PROOF)
            &&   !IS_OBJ_STAT(obj_lose,ITEM_QUEST))
            {
                switch ( obj_lose->item_type )
                {
               	case ITEM_ARMOR:
		if (obj_lose->wear_loc != -1) /* remove the item */
		{
		    if (can_drop_obj(victim,obj_lose)
		    &&  (obj_lose->weight / 10) < 
			number_range(1,2 * get_curr_stat(victim,STAT_DEX))
		    &&  remove_obj( victim, obj_lose->wear_loc, TRUE ))
		    {
		        act("$n yelps and throws $p to the ground!",
			    victim,obj_lose,NULL,TO_ROOM);
		        act("You remove and drop $p before it burns you.",
			    victim,obj_lose,NULL,TO_CHAR);
			dam += (number_range(1,obj_lose->level) / 3);
                        obj_from_char(obj_lose);
                        obj_to_room(obj_lose, victim->in_room);
                        fail = FALSE;
                    }
		    else /* stuck on the body! ouch! */
		    {
			act("Your skin is seared by $p!",
			    victim,obj_lose,NULL,TO_CHAR);
			dam += (number_range(1,obj_lose->level));
			fail = FALSE;
		    }

		}
		else /* drop it if we can */
		{
		    if (can_drop_obj(victim,obj_lose))
		    {
                        act("$n yelps and throws $p to the ground!",
                            victim,obj_lose,NULL,TO_ROOM);
                        act("You and drop $p before it burns you.",
                            victim,obj_lose,NULL,TO_CHAR);
                        dam += (number_range(1,obj_lose->level) / 6);
                        obj_from_char(obj_lose);
                        obj_to_room(obj_lose, victim->in_room);
			fail = FALSE;
                    }
		    else /* cannot drop */
		    {
                        act("Your skin is seared by $p!",
                            victim,obj_lose,NULL,TO_CHAR);
                        dam += (number_range(1,obj_lose->level) / 2);
			fail = FALSE;
                    }
		}
                break;
                case ITEM_WEAPON:
		if (obj_lose->wear_loc != -1) /* try to drop it */
		{
		    if (IS_WEAPON_STAT(obj_lose,WEAPON_FLAMING))
			continue;

		    if (can_drop_obj(victim,obj_lose) 
		    &&  remove_obj(victim,obj_lose->wear_loc,TRUE))
		    {
			act("$n is burned by $p, and throws it to the ground.",
			    victim,obj_lose,NULL,TO_ROOM);
			send_to_char(
			    "You throw your red-hot weapon to the ground!\n\r",
			    victim);
			dam += 1;
			obj_from_char(obj_lose);
			obj_to_room(obj_lose,victim->in_room);
			fail = FALSE;
		    }
		    else /* YOWCH! */
		    {
			send_to_char("Your weapon sears your flesh!\n\r",
			    victim);
			dam += number_range(1,obj_lose->level);
			fail = FALSE;
		    }
		}
                else /* drop it if we can */
                {
                    if (can_drop_obj(victim,obj_lose))
                    {
                        act("$n throws a burning hot $p to the ground!",
                            victim,obj_lose,NULL,TO_ROOM);
                        act("You and drop $p before it burns you.",
                            victim,obj_lose,NULL,TO_CHAR);
                        dam += (number_range(1,obj_lose->level) / 6);
                        obj_from_char(obj_lose);
                        obj_to_room(obj_lose, victim->in_room);
                        fail = FALSE;
                    }
                    else /* cannot drop */
                    {
                        act("Your skin is seared by $p!",
                            victim,obj_lose,NULL,TO_CHAR);
                        dam += (number_range(1,obj_lose->level) / 2);
                        fail = FALSE;
                    }
                }
                break;
		}
	    }
	}
    } 
    if (fail)
    {
        send_to_char("Your spell had no effect.\n\r", ch);
	send_to_char("You feel momentarily warmer.\n\r",victim);
    }
    else /* damage! */
    {
	if ( ( ch->fighting == NULL )
	&& ( !IS_NPC( ch ) )
	&& ( !IS_NPC( victim ) ) )
	{
	    ch->attacker = TRUE;
	    victim->attacker = FALSE;
	}
	if (saves_spell(level,victim,DAM_FIRE))
	    dam = 2 * dam / 3;
	damage_old(ch,victim,dam,sn,DAM_FIRE,TRUE);
    }
}

void spell_identify( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    char buf[MAX_STRING_LENGTH];
    AFFECT_DATA *paf;

    sprintf( buf,
	"Object '%s' is type %s, extra flags %s.\n\rWeight is %d, value is %d, level is %d.\n\r",

	obj->name,
	item_type_name( obj ),
	extra_bit_name( obj->extra_flags ),
	obj->weight / 10,
	obj->cost,
	obj->level
	);
    send_to_char( buf, ch );
    printf_to_char(ch,"Wear bits: %s\n\r",wear_bit_name(obj->wear_flags));
    switch ( obj->item_type )
    {
    case ITEM_SCROLL: 
    case ITEM_POTION:
    case ITEM_PILL:
	sprintf( buf, "Level %d spells of:", obj->value[0] );
	send_to_char( buf, ch );

	if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
	{
	    send_to_char( " '", ch );
	    send_to_char( skill_table[obj->value[1]].name, ch );
	    send_to_char( "'", ch );
	}

	if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )
	{
	    send_to_char( " '", ch );
	    send_to_char( skill_table[obj->value[2]].name, ch );
	    send_to_char( "'", ch );
	}

	if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
	{
	    send_to_char( " '", ch );
	    send_to_char( skill_table[obj->value[3]].name, ch );
	    send_to_char( "'", ch );
	}

	if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL)
	{
	    send_to_char(" '",ch);
	    send_to_char(skill_table[obj->value[4]].name,ch);
	    send_to_char("'",ch);
	}

	send_to_char( ".\n\r", ch );
	break;

    case ITEM_WAND: 
    case ITEM_STAFF: 
	sprintf( buf, "Has %d charges of level %d",
	    obj->value[2], obj->value[0] );
	send_to_char( buf, ch );
      
	if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
	{
	    send_to_char( " '", ch );
	    send_to_char( skill_table[obj->value[3]].name, ch );
	    send_to_char( "'", ch );
	}

	send_to_char( ".\n\r", ch );
	break;

    case ITEM_DRINK_CON:
        sprintf(buf,"It holds %s-colored %s.\n\r",
            liq_table[obj->value[2]].liq_color,
            liq_table[obj->value[2]].liq_name);
        send_to_char(buf,ch);
        break;

    case ITEM_CONTAINER:
    case ITEM_PIT:
	sprintf(buf,"Capacity: %d#  Maximum weight: %d#  flags: %s\n\r",
	    obj->value[0], obj->value[3], cont_bit_name(obj->value[1]));
	send_to_char(buf,ch);
	if (obj->value[4] != 100)
	{
	    sprintf(buf,"Weight multiplier: %d%%\n\r",
		obj->value[4]);
	    send_to_char(buf,ch);
	}
	break;
		
    case ITEM_WEAPON:
 	send_to_char("Weapon type is ",ch);
	switch (obj->value[0])
	{
	    case(WEAPON_EXOTIC) : send_to_char("exotic.\n\r",ch);	break;
	    case(WEAPON_SWORD)  : send_to_char("sword.\n\r",ch);	break;	
	    case(WEAPON_DAGGER) : send_to_char("dagger.\n\r",ch);	break;
	    case(WEAPON_SPEAR)	: send_to_char("spear/staff.\n\r",ch);	break;
	    case(WEAPON_MACE) 	: send_to_char("mace/club.\n\r",ch);	break;
	    case(WEAPON_AXE)	: send_to_char("axe.\n\r",ch);		break;
	    case(WEAPON_FLAIL)	: send_to_char("flail.\n\r",ch);	break;
	    case(WEAPON_WHIP)	: send_to_char("whip.\n\r",ch);		break;
	    case(WEAPON_POLEARM): send_to_char("polearm.\n\r",ch);	break;
	    default		: send_to_char("unknown.\n\r",ch);	break;
 	}
	if (obj->clan)
	{
	    sprintf( buf, "Damage is variable.\n\r");
	} else
	{
	    if (obj->pIndexData->new_format)
		sprintf(buf,"Damage is %dd%d (average %d).\n\r",
		obj->value[1],obj->value[2],
		(1 + obj->value[2]) * obj->value[1] / 2);
	    else
		sprintf( buf, "Damage is %d to %d (average %d).\n\r",
	    	obj->value[1], obj->value[2],
	    	( obj->value[1] + obj->value[2] ) / 2 );
	}
	send_to_char( buf, ch );
        printf_to_char(ch,"Damage noun is %s.\n\r",attack_table[obj->value[3]].noun);
        if (obj->value[4])  /* weapon flags */
        {
            sprintf(buf,"Weapons flags: %s\n\r",weapon_bit_name(obj->value[4]));
            send_to_char(buf,ch);
        }
	break;

    case ITEM_ARMOR:
	if (obj->clan)
	{
	    sprintf( buf, "Armor class is variable.\n\r");
	} else
	{
	    sprintf( buf, 
	    "Armor class is %d pierce, %d bash, %d slash, and %d vs. magic.\n\r", 
	    obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
	}
	send_to_char( buf, ch );
	break;
    }
    if (is_clan_obj(obj))
    {
	sprintf( buf, "This object is owned by the [{%s%s{x] clan.\n\r",
	    clan_table[obj->clan].pkill ? "B" : "M",
	    clan_table[obj->clan].who_name
	    );
	send_to_char( buf, ch );
    }
    if (is_class_obj(obj))
    {
	/*sprintf( buf, "This object may only be used by a %s.\n\r",
	    class_table[obj->class].name
	    );*/
        sprintf( buf, "This object may only be used by a %s/%s.\n\r",
            ((obj->class>(MAX_CLASS/2))?
                (class_table[obj->class-(MAX_CLASS/2)].name) :
                (class_table[obj->class].name)),
            ((obj->class>(MAX_CLASS/2))?
                (class_table[obj->class].name) :
                (class_table[obj->class+(MAX_CLASS/2)].name)));  
	send_to_char( buf, ch );
    }
    if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    {
       if(paf->location!=APPLY_SPELL_AFFECT)
	{
	    sprintf( buf, "Affects %s by %d.\n\r",
		affect_loc_name( paf->location ), paf->modifier );
	    send_to_char(buf,ch);
            if (paf->bitvector)
            {
                switch(paf->where)
                {
                    case TO_AFFECTS:
                        sprintf(buf,"Adds %s affect.\n",
                            affect_bit_name(paf->bitvector));
                        break;
                    case TO_OBJECT:
                        sprintf(buf,"Adds %s object flag.\n",
                            extra_bit_name(paf->bitvector));
                        break;
                    case TO_IMMUNE:
                        sprintf(buf,"Adds immunity to %s.\n",
                            imm_bit_name(paf->bitvector));
                        break;
                    case TO_RESIST:
                        sprintf(buf,"Adds resistance to %s.\n\r",
                            imm_bit_name(paf->bitvector));
                        break;
                    case TO_VULN:
                        sprintf(buf,"Adds vulnerability to %s.\n\r",
                            imm_bit_name(paf->bitvector));
                        break;
                    case TO_SHIELDS:
                        sprintf(buf,"Adds %s shield.\n",
                            shield_bit_name(paf->bitvector));
                        break;
                    default:
                        sprintf(buf,"Unknown bit %d: %d\n\r",
                            paf->where,paf->bitvector);
                        break;
                }
	        send_to_char( buf, ch );
	    }
	}
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	if ( paf->location != APPLY_SPELL_AFFECT )
	{
	    sprintf( buf, "Affects %s by %d",
	    	affect_loc_name( paf->location ), paf->modifier );
	    send_to_char( buf, ch );
            if ( paf->duration > -1)
                sprintf(buf,", %d hours.\n\r",paf->duration);
            else
                sprintf(buf,".\n\r");
	    send_to_char(buf,ch);
            if (paf->bitvector)
            {
                switch(paf->where)
                {
                    case TO_AFFECTS:
                        sprintf(buf,"Adds %s affect.\n",
                            affect_bit_name(paf->bitvector));
                        break;
                    case TO_OBJECT:
                        sprintf(buf,"Adds %s object flag.\n",
                            extra_bit_name(paf->bitvector));
                        break;
		    case TO_WEAPON:
			sprintf(buf,"Adds %s weapon flags.\n",
			    weapon_bit_name(paf->bitvector));
			break;
                    case TO_IMMUNE:
                        sprintf(buf,"Adds immunity to %s.\n",
                            imm_bit_name(paf->bitvector));
                        break;
                    case TO_RESIST:
                        sprintf(buf,"Adds resistance to %s.\n\r",
                            imm_bit_name(paf->bitvector));
                        break;
                    case TO_VULN:
                        sprintf(buf,"Adds vulnerability to %s.\n\r",
                            imm_bit_name(paf->bitvector));
                        break;
                    case TO_SHIELDS:
                        sprintf(buf,"Adds %s shield.\n",
                            shield_bit_name(paf->bitvector));
                        break;
                    default:
                        sprintf(buf,"Unknown bit %d: %d\n\r",
                            paf->where,paf->bitvector);
                        break;
                }
                send_to_char(buf,ch);
            }
	}
    }
    return;
}



void spell_infravision( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_INFRARED) )
    {
	if (victim == ch)
	  send_to_char("You can already see in the dark.\n\r",ch);
	else
	  act("$N already has infravision.\n\r",ch,NULL,victim,TO_CHAR);
	return;
    }
    act( "$n's eyes glow red.\n\r", ch, NULL, NULL, TO_ROOM );

    af.where	 = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = 2 * level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_INFRARED;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes glow red.\n\r", victim );
    return;
}



void spell_invis( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    /* object invisibility */
    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;	

	if (IS_OBJ_STAT(obj,ITEM_INVIS))
	{
	    act("$p is already invisible.",ch,obj,NULL,TO_CHAR);
	    return;
	}
	
	af.where	= TO_OBJECT;
	af.type		= sn;
	af.level	= level;
	af.duration	= level + 12;
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= ITEM_INVIS;
	affect_to_obj(obj,&af);

	act("$p fades out of sight.",ch,obj,NULL,TO_ALL);
	return;
    }

    /* character invisibility */
    victim = (CHAR_DATA *) vo;

    if ( IS_SHIELDED(victim, SHD_INVISIBLE) )
	return;

    act( "$n fades out of existence.", victim, NULL, NULL, TO_ROOM );

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level + 12;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = SHD_INVISIBLE;
    affect_to_char( victim, &af );
    send_to_char( "You fade out of existence.\n\r", victim );
    return;
}



void spell_know_alignment(int sn,int level,CHAR_DATA *ch,void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    char *msg;
    int ap;

    ap = victim->alignment;

         if ( ap >  700 ) msg = "$N has a pure and good aura.";
    else if ( ap >  350 ) msg = "$N is of excellent moral character.";
    else if ( ap >  100 ) msg = "$N is often kind and thoughtful.";
    else if ( ap > -100 ) msg = "$N doesn't have a firm moral commitment.";
    else if ( ap > -350 ) msg = "$N lies to $S friends.";
    else if ( ap > -700 ) msg = "$N is a black-hearted murderer.";
    else msg = "$N is the embodiment of pure evil!.";

    act( msg, ch, NULL, victim, TO_CHAR );
    return;
}

void spell_locate_object( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    char buf[MAX_INPUT_LENGTH];
    BUFFER *buffer;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int number = 0, max_found;

    found = FALSE;
    number = 0;
    max_found = IS_IMMORTAL(ch) ? 100 : level;

    buffer = new_buf();
 
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( !can_see_obj( ch, obj ) || !is_name( target_name, obj->name ) 
    	||   IS_OBJ_STAT(obj,ITEM_NOLOCATE) || number_percent() > 2 * level
	||   ch->level < obj->level || IS_OBJ_STAT(obj,ITEM_QUEST))
	    continue;

	found = TRUE;
        number++;

	for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
	    ;

	if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by))
	{
	    sprintf( buf, "%s is carried by %s\n\r",
                obj->short_descr,
		PERS(in_obj->carried_by, ch) );
	}
	else
	{
	    if (IS_IMMORTAL(ch) && in_obj->in_room != NULL)
		sprintf( buf, "%s is in %s [Room %d]\n\r",
                    obj->short_descr,
		    in_obj->in_room->name, in_obj->in_room->vnum);
	    else 
	    	sprintf( buf, "%s is in %s\n\r",
                     obj->short_descr,
		    in_obj->in_room == NULL
		    	? "somewhere" : in_obj->in_room->name );
	}

	buf[0] = UPPER(buf[0]);
	add_buf(buffer,buf);

    	if (number >= max_found)
	    break;
    }

    if ( !found )
	send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
    else
	page_to_char(buf_string(buffer),ch);

    free_buf(buffer);

    return;
}



void spell_magic_missile( int sn, int level, CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] = 
    {
	 0,
	 3,  3,  3,  3,  4,	 4,  4,  4,  5,  5,
	 6,  6,  6,  6,  6,	 6,  6,  6,  6,  6,
	 7,  7,  7,  7,  7,	 7,  7,  7,  7,  7,
	 8,  8,  8,  8,  8,	 8,  8,  8,  8,  8,
	 9,  9,  9,  9,  9,	 9,  9,  9,  9,  9,
	10, 10, 10, 10, 10,	10, 10, 10, 10, 10,
	11, 11, 11, 11, 11,	11, 11, 11, 11, 11,
	12, 12, 12, 12, 12,	12, 12, 12, 12, 12,
	13, 13, 13, 13, 13,	13, 13, 13, 13, 13,
	14, 14, 14, 14, 14,	14, 14, 14, 14, 14
    };
    int dam;
    int number,i;

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell( level, victim,DAM_ENERGY) )
	dam /= 2;
    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }
    number = 1 + (level>5) + (level>10) + (level>15) + (level>20);
    for(i=0;i<number;i++)
     if(victim)
      damage_old( ch, victim, dam*(number-i), sn, DAM_ENERGY ,TRUE);
     else
      break;
    return;
}

void spell_mass_healing(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *gch;
    int heal_num, refresh_num;
    
    heal_num = skill_lookup("heal");
    refresh_num = skill_lookup("refresh"); 

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ((IS_NPC(ch) && IS_NPC(gch)) ||
	    (!IS_NPC(ch) && !IS_NPC(gch)))
	{
	    spell_heal(heal_num,level,ch,(void *) gch,TARGET_CHAR);
	    spell_refresh(refresh_num,level,ch,(void *) gch,TARGET_CHAR);  
	}
    }
}
	    

void spell_mass_invis( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    AFFECT_DATA af;
    CHAR_DATA *gch;

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( !is_same_group( gch, ch ) || IS_SHIELDED(gch, SHD_INVISIBLE) )
	    continue;
	act( "$n slowly fades out of existence.", gch, NULL, NULL, TO_ROOM );
	send_to_char( "You slowly fade out of existence.\n\r", gch );

	af.where     = TO_SHIELDS;
	af.type      = sn;
    	af.level     = level/2;
	af.duration  = 24;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = SHD_INVISIBLE;
	affect_to_char( gch, &af );
    }
    send_to_char( "Ok.\n\r", ch );

    return;
}



void spell_null( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    send_to_char( "That's not a spell!\n\r", ch );
    return;
}



void spell_pass_door( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_PASS_DOOR) )
    {
	if (victim == ch)
	  send_to_char("You are already out of phase.\n\r",ch);
	else
	  act("$N is already shifted out of phase.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy( level / 4 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_PASS_DOOR;
    affect_to_char( victim, &af );
    act( "$n turns translucent.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You turn translucent.\n\r", victim );
    return;
}

/* RT plague spell, very nasty */

void spell_plague( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (saves_spell(level,victim,DAM_DISEASE) || 
        (IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD)))
    {
	if (ch == victim)
	  send_to_char("You feel momentarily ill, but it passes.\n\r",ch);
	else
	  act("$N seems to be unaffected.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type 	  = sn;
    af.level	  = level * 3/4;
    af.duration  = 1005+(level/5);
    af.location  = APPLY_STR;
    af.modifier  = -5; 
    af.bitvector = AFF_PLAGUE;
    affect_join(victim,&af);
   
    send_to_char
      ("You scream in agony as plague sores erupt from your skin.\n\r",victim);
    act("$n screams in agony as plague sores erupt from $s skin.",
	victim,NULL,NULL,TO_ROOM);
}

void spell_poison( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;


    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;

	if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON)
	{
	    if (IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF)
                || IS_OBJ_STAT(obj,ITEM_QUEST))
	    {
		act("Your spell fails to corrupt $p.",ch,obj,NULL,TO_CHAR);
		return;
	    }
	    obj->value[3] = 1;
	    act("$p is infused with poisonous vapors.",ch,obj,NULL,TO_ALL);
	    return;
	}

	if (obj->item_type == ITEM_WEAPON)
	{
	    if (IS_WEAPON_STAT(obj,WEAPON_FLAMING)
	    ||  IS_WEAPON_STAT(obj,WEAPON_FROST)
	    ||  IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC)
	    ||  IS_WEAPON_STAT(obj,WEAPON_SHARP)
	    ||  IS_WEAPON_STAT(obj,WEAPON_VORPAL)
	    ||  IS_WEAPON_STAT(obj,WEAPON_SHOCKING)
            || IS_OBJ_STAT(obj,ITEM_QUEST)
	    ||  IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
	    {
		act("You can't seem to envenom $p.",ch,obj,NULL,TO_CHAR);
		return;
	    }

	    if (IS_WEAPON_STAT(obj,WEAPON_POISON))
	    {
		act("$p is already envenomed.",ch,obj,NULL,TO_CHAR);
		return;
	    }

	    af.where	 = TO_WEAPON;
	    af.type	 = sn;
	    af.level	 = level / 2;
	    af.duration	 = level/8;
 	    af.location	 = 0;
	    af.modifier	 = 0;
	    af.bitvector = WEAPON_POISON;
	    affect_to_obj(obj,&af);

	    act("$p is coated with deadly venom.",ch,obj,NULL,TO_ALL);
	    return;
	}

	act("You can't poison $p.",ch,obj,NULL,TO_CHAR);
	return;
    }

    victim = (CHAR_DATA *) vo;

    if ( saves_spell( level, victim,DAM_POISON) )
    {
	act("$n turns slightly green, but it passes.",victim,NULL,NULL,TO_ROOM);
	send_to_char("You feel momentarily ill, but it passes.\n\r",victim);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 1005+(level/5);
    af.location  = APPLY_STR;
    af.modifier  = -2;
    af.bitvector = AFF_POISON;
    affect_join( victim, &af );
    send_to_char( "You feel very sick.\n\r", victim );
    act("$n looks very ill.",victim,NULL,NULL,TO_ROOM);
    return;
}



void spell_protection_evil(int sn,int level,CHAR_DATA *ch,void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( IS_SHIELDED(victim, SHD_PROTECT_EVIL) 
    ||   IS_SHIELDED(victim, SHD_PROTECT_GOOD))
    {
        if (victim == ch)
          send_to_char("You are already protected.\n\r",ch);
        else
          act("$N is already protected.",ch,NULL,victim,TO_CHAR);
        return;
    }

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 24;
    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = -1;
    af.bitvector = SHD_PROTECT_EVIL;
    affect_to_char( victim, &af );
    send_to_char( "You feel holy and pure.\n\r", victim );
    if ( ch != victim )
        act("$N is protected from evil.",ch,NULL,victim,TO_CHAR);
    return;
}
 
void spell_protection_good(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( IS_SHIELDED(victim, SHD_PROTECT_GOOD) 
    ||   IS_SHIELDED(victim, SHD_PROTECT_EVIL))
    {
        if (victim == ch)
          send_to_char("You are already protected.\n\r",ch);
        else
          act("$N is already protected.",ch,NULL,victim,TO_CHAR);
        return;
    }

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 24;
    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = -1;
    af.bitvector = SHD_PROTECT_GOOD;
    affect_to_char( victim, &af );
    send_to_char( "You feel aligned with darkness.\n\r", victim );
    if ( ch != victim )
        act("$N is protected from good.",ch,NULL,victim,TO_CHAR);
    return;
}

void spell_protection_neutral(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected(victim, sn))
    {
        if (victim == ch)
          send_to_char("You are already protected.\n\r",ch);
        else
          act("$N is already protected.",ch,NULL,victim,TO_CHAR);
        return;
    }

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 24;
    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = -10;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "You are protected from neutrality.\n\r", victim );
    if ( ch != victim )
        act("$N is protected from neutrality.",ch,NULL,victim,TO_CHAR);
    return;
}

void spell_refresh( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    victim->move = UMIN( victim->move + level, victim->max_move );
    if (victim->max_move == victim->move)
        send_to_char("You feel fully refreshed!\n\r",victim);
    else
        send_to_char( "You feel less tired.\n\r", victim );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_remove_curse( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    bool found = FALSE;

    /* do object cases first */
    if (target == TARGET_OBJ)
    {
	obj = (OBJ_DATA *) vo;

	if (IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE))
	{
	    if (!IS_OBJ_STAT(obj,ITEM_NOUNCURSE) && !IS_OBJ_STAT(obj,ITEM_QUEST)
	    &&  !saves_dispel(level + 2,obj->level,0))
	    {
		REMOVE_BIT(obj->extra_flags,ITEM_NODROP);
		REMOVE_BIT(obj->extra_flags,ITEM_NOREMOVE);
		act("$p glows blue.",ch,obj,NULL,TO_ALL);
		return;
	    }

	    act("The curse on $p is beyond your power.",ch,obj,NULL,TO_CHAR);
	    return;
	}
	else
	{
	    act("There is no curse on $p.",ch,obj,NULL,TO_CHAR);
	    return;
	}
    }

    /* characters */
    victim = (CHAR_DATA *) vo;
    if(!is_affected(victim,gsn_curse))
    {
     if(victim!=ch)
      act("$N isn't even cursed.",ch,NULL,victim,TO_CHAR);
     else
      send_to_char("You aren't even cursed.\n\r",ch);	
    /* return;*/
    }
    if (check_dispel(level+1,victim,gsn_curse))
    {
	send_to_char("You feel better.\n\r",victim);
	act("$n looks more relaxed.",victim,NULL,NULL,TO_ROOM);
    }
    else
    {
      send_to_char("You failed.\n\r",ch);
    }

   for (obj = victim->carrying; (obj != NULL && !found); obj = obj->next_content)
   {
        if ((IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE))
	&&  !IS_OBJ_STAT(obj,ITEM_NOUNCURSE) && !IS_OBJ_STAT(obj,ITEM_QUEST))
        {   /* attempt to remove curse */
            if (!saves_dispel(level+2,obj->level,0))
            {
                found = TRUE;
                REMOVE_BIT(obj->extra_flags,ITEM_NODROP);
                REMOVE_BIT(obj->extra_flags,ITEM_NOREMOVE);
                act("Your $p glows blue.",victim,obj,NULL,TO_CHAR);
                act("$n's $p glows blue.",victim,obj,NULL,TO_ROOM);
            }
         }
    }
}

void spell_restore_mana( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    victim->mana = UMIN( victim->mana + 51, victim->max_mana );
    if (victim->max_mana == victim->mana)
        send_to_char("You feel fully focused!\n\r",victim);
    else
        send_to_char( "You feel more focused.\n\r", victim );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_sanctuary( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected(victim, skill_lookup("sever")))
    {
      if(victim==ch)
       send_to_char("You are currently severed from the gods' sanctuary.\n\r",ch);
      else
       act("$N is currently severed from the gods' sanctuary.",ch,NULL,victim,TO_CHAR);
      return;
    }

    if ( IS_SHIELDED(victim, SHD_SANCTUARY) )
    {
	if (victim == ch)
	  send_to_char("You are already in sanctuary.\n\r",ch);
	else
	  act("$N is already in sanctuary.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 6;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = SHD_SANCTUARY;
    affect_to_char( victim, &af );
    act( "$n is surrounded by a white aura.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You are surrounded by a white aura.\n\r", victim );
    return;
}



void spell_shield( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim->fighting )
    {
      if(victim == ch)
       send_to_char("A faint glow surrounds you for a brief moment, then vanishes.\n\r",ch);
      else
       act("A faint glow surrounds $N for a brief moment, then vanishes.",
        ch,NULL,victim,TO_CHAR);
      return;
    }

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already shielded from harm.\n\r",ch);
	else
	  act("$N is already protected by a shield.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 2+level/6;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = 0;
    af.subtype	 = number_range(1,level/9);
    affect_to_char( victim, &af );
    act( "$n is surrounded by a {yg{Yl{Wowi{Yn{yg{x shield.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You are surrounded by a {yg{Yl{Wowi{Yn{yg{x shield.\n\r", victim );
    return;
}

void spell_sleep( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
  
    if ( IS_AFFECTED(victim, AFF_SLEEP)
    ||   (IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD))
    ||   (level + 2) < victim->level
    ||   saves_spell( level-4, victim,DAM_CHARM) )
    {
       send_to_char("You failed.\n\r",ch);
       if(has_killed_flag(ch))
        remove_killed_flag(ch);
       multi_hit(victim,ch,TYPE_UNDEFINED);
       return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 1005 + (level/10);
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SLEEP;
    affect_join( victim, &af );

    if ( IS_AWAKE(victim) )
    {
	send_to_char( "You feel very sleepy ..... zzzzzz.\n\r", victim );
	act( "$n goes to sleep.", victim, NULL, NULL, TO_ROOM );
	victim->position = POS_SLEEPING;
    }
    return;
}

void spell_slow( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if ( is_affected( victim, sn ) || IS_AFFECTED(victim,AFF_SLOW))
    {
        if (victim == ch)
          send_to_char("You can't move any slower!\n\r",ch);
        else
          act("$N can't get any slower than that.",
              ch,NULL,victim,TO_CHAR);
        return;
    }
 
    if (saves_spell(level,victim,DAM_OTHER) 
    ||  IS_SET(victim->imm_flags,IMM_MAGIC))
    {
	if (victim != ch)
            send_to_char("Nothing seemed to happen.\n\r",ch);
        send_to_char("You feel momentarily lethargic.\n\r",victim);
        return;
    }
 
    if (IS_AFFECTED(victim,AFF_HASTE))
    {
        if (!check_dispel(level,victim,skill_lookup("haste")))
        {
	    if (victim != ch)
            	send_to_char("Spell failed.\n\r",ch);
            send_to_char("You feel momentarily slower.\n\r",victim);
            return;
        }
        
        act("$n is moving less quickly.",victim,NULL,NULL,TO_ROOM);
        return;
    }
 

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 1005+(level/5);
    af.location  = APPLY_DEX;
    af.modifier  = -1 - (level >= 18) - (level >= 25) - (level >= 32);
    af.bitvector = AFF_SLOW;
    affect_to_char( victim, &af );
    send_to_char( "You feel yourself slowing d o w n...\n\r", victim );
    act("$n starts to move in slow motion.",victim,NULL,NULL,TO_ROOM);
    return;
}

void spell_sever( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if(victim==ch)
    {
     send_to_char("Sever yourself? No way.\n\r",ch);
     return;
    }

    if ( is_affected( victim, sn ) )
    {
      act("{G$N{G's connection to the gods has already been severed.{x",
              ch,NULL,victim,TO_CHAR);
        return;
    }

    if (saves_spell(level,victim,DAM_OTHER))
    {
        send_to_char("You failed.\n\r",ch);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 1000+level/4;
    af.location  = APPLY_SAVES;
    af.modifier  = 5;
    af.bitvector = 0;
    af.dealer	 = ch;
    affect_to_char( victim, &af );

    act("{G$N{G's connection to the gods has been severed.{x",ch,NULL,victim,
     TO_CHAR);
    act("{G$N{G's connection to the gods has been severed.{x",ch,NULL,victim,TO_NOTVICT);
    send_to_char("{GYour connection to the gods has been severed.\n\r{x",victim);

    return;
}



void spell_stone_skin( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim->fighting )
    {
      if(victim == ch)
       send_to_char("A faint glow surrounds you for a brief moment, then vanishes.\n\r",ch);
      else
       act("A faint glow surrounds $N for a brief moment, then vanishes.",
        ch,NULL,victim,TO_CHAR);
      return;
    }

    if ( is_affected( ch, sn ) )
    {
	if (victim == ch)
	  send_to_char("Your skin is already as hard as a rock.\n\r",ch); 
	else
	  act("$N is already as hard as can be.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/7;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = 0;
    af.subtype	 = level/7;
    affect_to_char( victim, &af );
    act( "$n's skin turns into solid {Dr{xo{Wc{Dk{x.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "Your skin turns into solid {Dr{xo{Wc{Dk{x.\n\r", victim );
    return;
}

void spell_ward( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
   AFFECT_DATA af,*paf;
   int trig;

   if(target_name[0]=='\0')
   {
    send_to_char("You need to specify a minimum damage you want to be warded against.\n\r",ch);
    return;
   }
   trig = atoi(target_name);
   if(trig<0)
   {
    send_to_char("The specified number must be positive.\n\r",ch);
    return;
   }
   if(trig>3000)
   {
    send_to_char("Be reasonable.\n\r",ch);
    return;
   }
   if( is_affected(ch, sn) )
   {
     if((paf=affect_find(ch->affected,sn))==NULL)
     {
      send_to_char("Error in spell_ward.\n\r",ch);
      return;
     }
     paf->duration = level/5;
     paf->subtype  = trig;
     act("$n's ward is renewed.",ch,NULL,NULL,TO_ROOM);
     send_to_char("Your ward is renewed.\n\r",ch);
     return;
   }
   af.where    = TO_AFFECTS;
   af.type     = sn;
   af.level    = level;
   af.duration = level/5;
   af.location = APPLY_NONE;
   af.modifier = 0;
   af.bitvector = 0;
   af.subtype	= trig;
   affect_to_char( ch, &af );
   act("$n is warded.",ch, NULL, NULL, TO_ROOM);
   send_to_char( "You are warded.\n\r", ch );
   return;
}

void spell_summon( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim;
    
    if(has_pkflag(ch))
    {
     send_to_char("Your heart is pounding too fast.\n\r",ch);
     return;
    }
    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
    ||   victim == ch
    ||   victim->in_room == NULL
    ||   IS_SET(ch->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   (IS_NPC(victim) && IS_SET(victim->act,ACT_AGGRESSIVE))
    ||   victim->level >= level + 3
/*    ||   (is_clan(ch) && (is_clan(victim) && ((!is_same_clan(ch, victim)
    &&   (clan_table[ch->clan].pkill) && (clan_table[victim->clan].pkill))
    ||   clan_table[victim->clan].independent)))*/
    ||   (!IS_NPC(victim) && victim->level >= LEVEL_IMMORTAL)
    ||   victim->fighting != NULL
    ||   (IS_NPC(victim) && IS_SET(victim->imm_flags,IMM_SUMMON))
    ||	 (IS_NPC(victim) && victim->pIndexData->pShop != NULL)
    ||   (!IS_NPC(victim) && IS_SET(victim->act,PLR_NOSUMMON))
    ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_OTHER)) )

    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if(strstr(victim->in_room->area->builders,"Unlinked"))
    {
     send_to_char("You can't summon from areas that aren't linked yet!\n\r",ch);
     return;
    }

    act( "$n disappears suddenly.", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, ch->in_room );
    act( "$n arrives suddenly.", victim, NULL, NULL, TO_ROOM );
    act( "$n has summoned you!", ch, NULL, victim,   TO_VICT );
    do_look( victim, "auto" );
    return;
}



void spell_transport( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
	return;
}

void spell_teleport( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    ROOM_INDEX_DATA *pRoomIndex;

    if(has_pkflag(ch))
    {
     send_to_char("Your heart is pounding too fast.\n\r",ch);
     return;
    }
    if ( victim->in_room == NULL
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    || ( victim != ch && IS_SET(victim->imm_flags,IMM_SUMMON))
    || ( !IS_NPC(ch) && victim->fighting != NULL )
    || ( victim != ch
    && ( saves_spell( level - 5, victim,DAM_OTHER))))
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    pRoomIndex = get_random_room(victim);

    if (victim != ch)
	send_to_char("You have been teleported!\n\r",victim);

    act( "$n vanishes!", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, pRoomIndex );
    act( "$n slowly fades into existence.", victim, NULL, NULL, TO_ROOM );
    do_look( victim, "auto" );
    return;
}

void spell_weaken( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

/*    if ( is_affected( victim, sn ) || saves_spell( level, victim,DAM_OTHER) )
	return;*/
    if ( is_affected( victim, sn ) )
    {
      act("$N already looks weak.",ch,NULL,victim,TO_CHAR);
      return;
    }
    if(saves_spell(level,victim,DAM_OTHER))
    {
     send_to_char("Your attempt to weaken them failed.\n\r",ch);
     return;
    } 
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = 1005+(level/5);
    af.location  = APPLY_STR;
    af.modifier  = -1 * (level / 5);
    af.bitvector = AFF_WEAKEN;
    affect_to_char( victim, &af );
    send_to_char( "You feel your strength slip away.\n\r", victim );
    act("$n looks tired and weak.",victim,NULL,NULL,TO_ROOM);
    return;
}

void spell_leech( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, gsn_leech ) )
    {
      act("$N has already been infested by a leech.",ch,NULL,victim,TO_CHAR);
      return;
    }
    if(saves_spell(level,victim,DAM_NEGATIVE))
    {
     send_to_char("Your attempt to infest them failed.\n\r",ch);
     return;
    }
    af.where     = TO_AFFECTS;
    af.type      = gsn_leech;
    af.level     = level;
    af.duration  = 1005+(level/5);
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = 0;
    af.dealer	 = ch;
    affect_to_char( victim, &af );
    send_to_char( "You feel something moving beneath your skin...\n\r", victim );
    act("Something starts to move beneath $n's skin...",victim,NULL,NULL,TO_ROOM);
    return;
}

/*
 * NPC spells.
 */
void spell_acid_breath( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam,hp_dam,dice_dam,hpch;

    act("$n spits acid at $N.",ch,NULL,victim,TO_NOTVICT);
    act("$n spits a stream of corrosive acid at you.",ch,NULL,victim,TO_VICT);
    act("You spit acid at $N.",ch,NULL,victim,TO_CHAR);

    hpch = UMAX(12,ch->hit);
    hp_dam = number_range(hpch/11 + 1, hpch/6);
    dice_dam = dice(level,16);

    dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
    
    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }
    if (saves_spell(level,victim,DAM_ACID) || IS_SHIELDED(victim,SHD_SANCTUARY))
    {
	acid_effect(victim,level/2,dam/4,TARGET_CHAR);
	damage_old(ch,victim,dam/2,sn,DAM_ACID,TRUE);
    }
    else
    {
	acid_effect(victim,level,dam,TARGET_CHAR);
	damage_old(ch,victim,dam,sn,DAM_ACID,TRUE);
    }
}



void spell_fire_breath( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *vch, *vch_next;
    int dam,hp_dam,dice_dam;
    int hpch;

    act("$n breathes forth a cone of fire.",ch,NULL,victim,TO_NOTVICT);
    act("$n breathes a cone of hot fire over you!",ch,NULL,victim,TO_VICT);
    act("You breath forth a cone of fire.",ch,NULL,NULL,TO_CHAR);

    hpch = UMAX( 10, ch->hit );
    hp_dam  = number_range( hpch/9+1, hpch/5 );
    dice_dam = dice(level,20);

    dam = UMAX(hp_dam + dice_dam /10, dice_dam + hp_dam / 10);
    fire_effect(victim->in_room,level,dam/2,TARGET_ROOM);

    for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;

	if (is_safe_spell(ch,vch,TRUE) 
	||  (IS_NPC(vch) && IS_NPC(ch) 
	&&   (ch->fighting != vch || vch->fighting != ch)))
	    continue;

	if (vch == victim) /* full damage */
	{
	    if ( ( ch->fighting == NULL )
	    && ( !IS_NPC( ch ) )
	    && ( !IS_NPC( victim ) ) )
	    {
		ch->attacker = TRUE;
		victim->attacker = FALSE;
	    }
	    if (saves_spell(level,vch,DAM_FIRE) || IS_SHIELDED(vch,SHD_SANCTUARY))
	    {
		fire_effect(vch,level/2,dam/4,TARGET_CHAR);
		damage_old(ch,vch,dam/2,sn,DAM_FIRE,TRUE);
	    }
	    else
	    {
		fire_effect(vch,level,dam,TARGET_CHAR);
		damage_old(ch,vch,dam,sn,DAM_FIRE,TRUE);
	    }
	}
	else /* partial damage */
	{
	    if (saves_spell(level - 2,vch,DAM_FIRE) || IS_SHIELDED(vch,SHD_SANCTUARY))
	    {
		fire_effect(vch,level/4,dam/8,TARGET_CHAR);
		damage_old(ch,vch,dam/4,sn,DAM_FIRE,TRUE);
	    }
	    else
	    {
		fire_effect(vch,level/2,dam/4,TARGET_CHAR);
		damage_old(ch,vch,dam/2,sn,DAM_FIRE,TRUE);
	    }
	}
    }
}

void spell_frost_breath( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *vch, *vch_next;
    int dam,hp_dam,dice_dam, hpch;

    act("$n breathes out a freezing cone of frost!",ch,NULL,victim,TO_NOTVICT);
    act("$n breathes a freezing cone of frost over you!",
	ch,NULL,victim,TO_VICT);
    act("You breath out a cone of frost.",ch,NULL,NULL,TO_CHAR);

    hpch = UMAX(12,ch->hit);
    hp_dam = number_range(hpch/11 + 1, hpch/6);
    dice_dam = dice(level,16);

    dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
    cold_effect(victim->in_room,level,dam/2,TARGET_ROOM); 

    for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;

	if (is_safe_spell(ch,vch,TRUE)
	||  (IS_NPC(vch) && IS_NPC(ch) 
	&&   (ch->fighting != vch || vch->fighting != ch)))
	    continue;

	if (vch == victim) /* full damage */
	{
	    if ( ( ch->fighting == NULL )
	    && ( !IS_NPC( ch ) )
	    && ( !IS_NPC( victim ) ) )
	    {
		ch->attacker = TRUE;
		victim->attacker = FALSE;
	    }
	    if (saves_spell(level,vch,DAM_COLD) || IS_SHIELDED(vch,SHD_SANCTUARY))
	    {
		cold_effect(vch,level/2,dam/4,TARGET_CHAR);
		damage_old(ch,vch,dam/2,sn,DAM_COLD,TRUE);
	    }
	    else
	    {
		cold_effect(vch,level,dam,TARGET_CHAR);
		damage_old(ch,vch,dam,sn,DAM_COLD,TRUE);
	    }
	}
	else
	{
	    if (saves_spell(level - 2,vch,DAM_COLD) || IS_SHIELDED(vch,SHD_SANCTUARY))
	    {
		cold_effect(vch,level/4,dam/8,TARGET_CHAR);
		damage_old(ch,vch,dam/4,sn,DAM_COLD,TRUE);
	    }
	    else
	    {
		cold_effect(vch,level/2,dam/4,TARGET_CHAR);
		damage_old(ch,vch,dam/2,sn,DAM_COLD,TRUE);
	    }
	}
    }
}

void spell_tornado( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    CHAR_DATA *vch, *vch_next;
    int dam;

    act("$n conjures forth a devastating tornado!",ch,NULL,NULL,TO_ROOM);
    act("You conjure forth a devastating tornado!",ch,NULL,NULL,TO_CHAR);

    dam = dice(level,6)+number_range(level/2,level);
    for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;

	if (is_safe_spell(ch,vch,TRUE)
	||  (IS_NPC(vch) && IS_NPC(ch)
	&&   (ch->fighting != vch || vch->fighting != ch)))
	    continue;

	if (vch == victim) /* full damage */
	{
	    if ( ( ch->fighting == NULL )
	    && ( !IS_NPC( ch ) )
	    && ( !IS_NPC( victim ) ) )
	    {
		ch->attacker = TRUE;
		victim->attacker = FALSE;
	    }
	    if (saves_spell(level,vch,DAM_SOUND))
	    {
		damage_old(ch,vch,dam/2,sn,DAM_SOUND,TRUE);
	    }
	    else
	    {
		damage_old(ch,vch,dam,sn,DAM_COLD,TRUE);
	    }
	}
	else
	{
	    if (saves_spell(level - 2,vch,DAM_SOUND))
	    {
		damage_old(ch,vch,dam/4,sn,DAM_SOUND,TRUE);
	    }
	    else
	    {
		damage_old(ch,vch,dam/2,sn,DAM_SOUND,TRUE);
	    }
	}
    }
}

void spell_gas_breath( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam,hp_dam,dice_dam,hpch;

    act("$n breathes out a cloud of poisonous gas!",ch,NULL,NULL,TO_ROOM);
    act("You breath out a cloud of poisonous gas.",ch,NULL,NULL,TO_CHAR);

    hpch = UMAX(16,ch->hit);
    hp_dam = number_range(hpch/15+1,8);
    dice_dam = dice(level,12);

    dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
    poison_effect(ch->in_room,level,dam,TARGET_ROOM);

    for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;

	if (is_safe_spell(ch,vch,TRUE)
	||  (IS_NPC(ch) && IS_NPC(vch) 
	&&   (ch->fighting == vch || vch->fighting == ch)))
	    continue;

	    if ( ( ch->fighting == NULL )
	    && ( !IS_NPC( ch ) )
	    && ( !IS_NPC( vch ) ) )
	    {
		ch->attacker = TRUE;
		vch->attacker = FALSE;
	    }
	if (saves_spell(level,vch,DAM_POISON) || IS_SHIELDED(vch,SHD_SANCTUARY))
	{
	    poison_effect(vch,level/2,dam/4,TARGET_CHAR);
	    damage_old(ch,vch,dam/2,sn,DAM_POISON,TRUE);
	}
	else
	{
	    poison_effect(vch,level,dam,TARGET_CHAR);
	    damage_old(ch,vch,dam,sn,DAM_POISON,TRUE);
	}
    }
}

void spell_lightning_breath(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam,hp_dam,dice_dam,hpch;

    act("$n breathes a bolt of lightning at $N.",ch,NULL,victim,TO_NOTVICT);
    act("$n breathes a bolt of lightning at you!",ch,NULL,victim,TO_VICT);
    act("You breathe a bolt of lightning at $N.",ch,NULL,victim,TO_CHAR);

    hpch = UMAX(10,ch->hit);
    hp_dam = number_range(hpch/9+1,hpch/5);
    dice_dam = dice(level,20);

    dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);

    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }
    if (saves_spell(level,victim,DAM_LIGHTNING) || IS_SHIELDED(victim,SHD_SANCTUARY))
    {
	shock_effect(victim,level/2,dam/4,TARGET_CHAR);
	damage_old(ch,victim,dam/2,sn,DAM_LIGHTNING,TRUE);
    }
    else
    {
	shock_effect(victim,level,dam,TARGET_CHAR);
	damage_old(ch,victim,dam,sn,DAM_LIGHTNING,TRUE); 
    }
}

/*
 * Spells for mega1.are from Glop/Erkenbrand.
 */
void spell_general_purpose(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
 
    dam = number_range( 25, 100 );
    if ( saves_spell( level, victim, DAM_PIERCE) )
        dam /= 2;
    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }
    damage_old( ch, victim, dam, sn, DAM_PIERCE ,TRUE);
    return;
}

void spell_high_explosive(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
 
    dam = number_range( 30, 120 );
    if ( saves_spell( level, victim, DAM_PIERCE) )
        dam /= 2;
    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }
    damage_old( ch, victim, dam, sn, DAM_PIERCE ,TRUE);
    return;
}

/*void spell_grow(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
 CHAR_DATA *victim = (CHAR_DATA*) vo;
 int dam;
 int slev = level/20;
 AFFECT_DATA af;
 int i=0;

 if(ch==victim)
 {
  send_to_char("You don't want to cast this spell on yourself, do you?\n\r",ch);
  return;
 }
 if(saves_spell(level,victim,DAM_OTHER))
 {
  act("$n summons the force of nature but nothing happens.",ch,NULL,NULL,TO_ROOM);
  send_to_char("You summon the force of nature but nothing happens.\n\r",ch);
  return;
 }
 switch(number_range(0,slev-1))
 {

  case 0: act("$n summons a swarm of {Rf{ri{Yr{ye{Rf{rl{Yi{ye{Rs{x to distract $s opponent!",
          ch,NULL,victim,TO_NOTVICT);
          act("$n summons a swarm of {Rf{ri{Yr{ye{Rf{rl{Yi{ye{Rs{x to distract you!",ch,NULL,victim,TO_VICT);
          send_to_char("You summon a swarm of {Rf{ri{Yr{ye{Rf{rl{Yi{ye{Rs{x to distract your opponent!\n\r",ch);
          DAZE_STATE(victim,2*PULSE_VIOLENCE);
          break;
  case 1: act("$n summons vines to entangle and hinder $s opponent!",ch,NULL,victim,TO_NOTVICT);
          act("$n summons vines to entangle and hinder you!",ch,NULL,victim,TO_VICT);
          send_to_char("You summon vines to entangle and hinder your opponent!\n\r",ch);
          if(IS_AFFECTED(victim,AFF_HASTE))
          {
           if(check_dispel(level,victim,skill_lookup("haste")))
            act("$n is moving less quickly.",victim,NULL,NULL,TO_ROOM);
          }
          else if(!IS_AFFECTED(victim,AFF_SLOW))
          {
            af.where     = TO_AFFECTS;
            af.type      = sn;
            af.level     = level;
            af.duration  = 1005+(level/5);
            af.location  = APPLY_DEX;
            af.modifier  = -1 - (level >= 18) - (level >= 25) - (level >= 32);
            af.bitvector = AFF_SLOW;
            affect_to_char( victim, &af );
            send_to_char( "You feel yourself slowing d o w n...\n\r", victim );
            act("$n starts to move in slow motion.",victim,NULL,NULL,TO_ROOM);
          }
          else
          {
           af.where 	= TO_AFFECTS;
	   af.type	= sn;
 	   af.level	= level;
           af.duration	= 1005;
           af.location  = APPLY_HITROLL;
           af.modifier	= -10 + (level/10);
           af.bitvector = 0;
           affect_join( victim, &af);
           send_to_char("You have problems with hitting your opponent.\n\r",ch);
           act("$n has problems with hitting $s opponent.",victim,NULL,NULL,TO_ROOM);
          }
          break;
  case 2: act("A crack appears in the ground and $n trips!",victim,NULL,NULL,TO_ROOM);
          send_to_char("A crack appears in the ground and you trip!\n\r",victim);
          if((number_percent() < (level*3)/4))
          {
           send_to_char("You are stunned!\n\r",victim);
           act("$n appears to be stunned!",victim,NULL,NULL,TO_ROOM);
           victim->stunned=2;
          }
          break;
  case 3: act("$n summons blinding rays of light!",ch,NULL,NULL,TO_ROOM);
          send_to_char("You summon blinding rays of light!\n\r",ch);
          if(!IS_AFFECTED(victim,AFF_BLIND))
          {
           af.where	= TO_AFFECTS;
	   af.type	= sn;
           af.level	= level;
	   af.duration	= 1010;
	   af.location	= APPLY_NONE;
	   af.modifier	= 0;
           af.bitvector	= AFF_BLIND;
 	   affect_to_char(victim,&af);
           send_to_char("You scream in agony as your eyes are burned!\n\r",victim);
           act("$n screams in agony as $s eyes are burned!",victim,NULL,NULL,TO_ROOM);
          }
          break;
  case 4: act("$n summons the force of nature to corrupt $N's magical protection!",ch,NULL,victim,TO_NOTVICT);
          act("$n summons the force of nature to corrupt your magical protection!",ch,NULL,victim,TO_VICT);
          send_to_char("You summon the force of nature to corrupt your opponent's magical protection!\n\r",ch);
          for(i=0;i<3;i++)
          {
           if (check_dispel(level,victim,skill_lookup("sanctuary")))
           {
             act("The white aura around $n's body vanishes.",
               victim,NULL,NULL,TO_ROOM);
           }

           if (IS_SHIELDED(victim,SHD_SANCTUARY)
           && !saves_dispel(level, victim->level,-1)
	   && !is_affected(victim,skill_lookup("sanctuary")))
           {
	    REMOVE_BIT(victim->shielded_by,SHD_SANCTUARY);
            act("The white aura around $n's body vanishes.",
              victim,NULL,NULL,TO_ROOM);
           }

           if (IS_SHIELDED(victim,SHD_SANCTUARY)
	   && !saves_dispel(level, victim->level,-1)
	   && !is_affected(victim,skill_lookup("sanctuary")))
           {
	     REMOVE_BIT(victim->shielded_by,SHD_SANCTUARY);
             act("The white aura around $n's body vanishes.",
                victim,NULL,NULL,TO_ROOM);

           }
          }
          break;
  default:
 }
 damage(ch,victim,1,gsn_grow,DAM_OTHER,FALSE);
 return;
}*/

void spell_protective_force( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already protected by nature.\n\r",ch);
	else
	  act("$N is already protected by nature.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = gsn_protective_force;
    af.level     = level;
    af.duration  = level/4;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    act( "$n is surrounded by the protective force of nature.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You are surrounded by the protective force of nature.\n\r", victim );
    return;
}

void spell_resistance( int sn, int level, CHAR_DATA *ch, void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
	if (victim == ch)
	  send_to_char("You are already more resistant.\n\r",ch);
	else
	  act("$N is already more resistant.",ch,NULL,victim,TO_CHAR);
	return;
    }

    af.where     = TO_AFFECTS;
    af.type      = gsn_resistance;
    af.level     = level;
    af.duration  = level/4;
    af.location  = APPLY_STR;
    af.modifier  = 2;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    act( "$n looks stronger and more resistant.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You become stronger and more resistant.\n\r", victim );
    return;
}

void spell_cleansing( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    bool found = FALSE;

    if (saves_spell(level, victim,DAM_OTHER))
    {
	send_to_char( "A shiver runs down your spine.\n\r",victim);
	send_to_char( "You failed.\n\r", ch);
	return;
    }

    /* begin running through the spells */

    /*if (check_dispel(level,victim,skill_lookup("immaterial armor")))
        found = TRUE;*/

    if (check_dispel(level,victim,skill_lookup("bless")))
        found = TRUE;

    /*if (check_dispel(level,victim,skill_lookup("blindness")))
    {
        found = TRUE;
        act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
    }*/

    if (check_dispel(level,victim,skill_lookup("charm person")))
    {
        found = TRUE;
        act("$n regains $s free will.",victim,NULL,NULL,TO_ROOM);
    }

    /*if (check_dispel(level,victim,skill_lookup("curse")))
        found = TRUE;*/

    /*if (check_dispel(level,victim,skill_lookup("detect evil")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("detect good")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("detect hidden")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("detect invis")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("detect hidden")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("detect magic")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("faerie fire")))
    {
        act("$n's outline fades.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }*/

    if (check_dispel(level,victim,skill_lookup("fly")))
    {
        act("$n falls to the ground!",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("frenzy")))
    {
        act("$n no longer looks so wild.",victim,NULL,NULL,TO_ROOM);;
        found = TRUE;
    }

    /*if (check_dispel(level,victim,skill_lookup("vigor")))
    {
        act("$n no longer looks so vigorous.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }*/

    if (check_dispel(level,victim,skill_lookup("haste")))
    {
        act("$n is no longer moving so quickly.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    /*if (check_dispel(level,victim,skill_lookup("infravision")))
        found = TRUE;*/

    if (check_dispel(level,victim,skill_lookup("invis")))
    {
        act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (check_dispel(level,victim,skill_lookup("mass invis")))
    {
        act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    /*if (check_dispel(level,victim,skill_lookup("pass door")))
        found = TRUE;*/


    /*if (check_dispel(level,victim,skill_lookup("protection evil")))
        found = TRUE;

    if (check_dispel(level,victim,skill_lookup("protection good")))
        found = TRUE;*/

    if (check_dispel(level,victim,skill_lookup("protection neutral")))
       found = TRUE;

    if (check_dispel(level,victim,skill_lookup("sanctuary")))
    {
        act("The white aura around $n's body vanishes.",
            victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (IS_SHIELDED(victim,SHD_SANCTUARY)
	&& !saves_dispel(level, victim->level,-1)
	&& !is_affected(victim,skill_lookup("sanctuary")))
    {
	REMOVE_BIT(victim->shielded_by,SHD_SANCTUARY);
        act("The white aura around $n's body vanishes.",
            victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (IS_SHIELDED(victim,SHD_SANCTUARY)
	&& !saves_dispel(level, victim->level,-1)
	&& !is_affected(victim,skill_lookup("sanctuary")))
    {
	REMOVE_BIT(victim->shielded_by,SHD_SANCTUARY);
        act("The white aura around $n's body vanishes.",
            victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    /*if (check_dispel(level,victim,skill_lookup("force shield")))
    {
        act("The shield protecting $n vanishes.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }*/

    if (check_dispel(level,victim,skill_lookup("sleep")))
        found = TRUE;

    /*if (check_dispel(level,victim,skill_lookup("slow")))
    {
        act("$n is no longer moving so slowly.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }*/

    /*if (check_dispel(level,victim,skill_lookup("stone skin")))
    {
        act("$n's skin regains its normal texture.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }*/

    if (check_dispel(level,victim,skill_lookup("essence of mist")))
    {
        act("$n's body returns to its true form.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
    if (check_dispel(level,victim,skill_lookup("essence of bat")))
    {
        act("$n's body returns to its true form.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }
    if (check_dispel(level,victim,skill_lookup("essence of wolf")))
    {
        act("$n's body returns to its true form.",victim,NULL,NULL,TO_ROOM);
        found = TRUE;
    }

    if (found)
        send_to_char("Ok.\n\r",ch);
    else
        send_to_char("Spell failed.\n\r",ch);
	return;
}

void spell_grow(  int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

    act("$n summons vines to entangle and hinder $s opponent!",ch,NULL,victim,TO_NOTVICT);
    act("$n summons vines to entangle and hinder you!",ch,NULL,victim,TO_VICT);
    send_to_char("You summon vines to entangle and hinder your opponent!\n\r",ch);
    spell_slow(skill_lookup("slow"),level,ch,vo,target);
/*    if (saves_spell(level,victim,DAM_OTHER)
    ||  IS_SET(victim->imm_flags,IMM_MAGIC))
    {
        send_to_char("Your spell had no effect.\n\r",ch);
        return;
    }
    if(IS_AFFECTED(victim,AFF_HASTE))
    {
     if(check_dispel(level,victim,skill_lookup("haste")))
      act("$n is moving less quickly.",victim,NULL,NULL,TO_ROOM);
     else
      send_to_char("Your spell had no effect.\n\r",ch);
    }
    else if(!IS_AFFECTED(victim,AFF_SLOW))
    {
      af.where     = TO_AFFECTS;
      af.type      = sn;
      af.level     = level;
      af.duration  = 1005+(level/5);
      af.location  = APPLY_DEX;
      af.modifier  = -1 - (level >= 18) - (level >= 25) - (level >= 32);
      af.bitvector = AFF_SLOW;
      affect_to_char( victim, &af );
      send_to_char( "You feel yourself slowing d o w n...\n\r", victim );
      act("$n starts to move in slow motion.",victim,NULL,NULL,TO_ROOM);
    }
    else
    {
     act("$n can't get any slower.",victim,NULL,ch,TO_VICT);
    }*/
}

void spell_regeneration( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) || IS_AFFECTED(victim, AFF_REGENERATION))
    {
	if (victim == ch)
	  send_to_char("Your heart is already pumping as fast as possible.\n\r",ch);
	else
	  act("$N's heart is already pumping as fast as possible.",ch,NULL,victim,TO_CHAR);
	return;
    }
    af.where	 = TO_AFFECTS;
    af.type      = sn;
    af.level	 = level;
    af.duration  = level/4+5;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_REGENERATION;
    affect_to_char( victim, &af );
    send_to_char( "You feel your heart pumping faster.\n\r", victim );
    if ( ch != victim )
	act("$N's circulation is increased by your magic.",ch,NULL,victim,TO_CHAR);
    return;
}

void spell_blade_poison( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
 OBJ_DATA *obj;
 AFFECT_DATA af;

 obj = (OBJ_DATA *) vo;

 if (obj->item_type == ITEM_WEAPON)
 {
     if(affect_find(obj->affected,sn))
     {
	act("$p has already been treated with blade poison.",ch,obj,
         NULL,TO_CHAR);
        return;
     }
     af.where	 = TO_WEAPON;
     af.type	 = sn;
     af.level	 = level;
     af.duration	 = level/8;
     af.location	 = 0;
     af.modifier	 = 0;
     af.dealer = ch;
     af.bitvector = 0;
     affect_to_obj(obj,&af);

     act("$p is corrupted by your spell and glows malevolently.",
      ch,obj,NULL,TO_CHAR);
     return;
 }

 act("You can't treat $p with blade poison.",ch,obj,NULL,TO_CHAR);
 return;
}

void spell_renewal(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
 AFFECT_DATA *paf,*paf_next;
 CHAR_DATA *victim=(CHAR_DATA*)vo;

 for(paf=victim->affected;paf!=NULL;paf=paf_next)
 {
  paf_next=paf->next;
  if((paf->duration>=1000)||(paf->duration==-1))
   continue;
  paf->duration=level/2;
 }

 send_to_char("You are surrounded by a bright aura as your spells are renewed.\n\r",victim);
 act("$n is surrounded by a bright aura as $s spells are renewed.",victim,NULL,NULL,TO_ROOM);
 return;
}

void spell_divine_blessing(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
 AFFECT_DATA af;
 CHAR_DATA *victim=(CHAR_DATA*)vo;

 if(is_affected(victim,gsn_divine_blessing))
 {
  if(ch==victim)
     send_to_char("You are already divinely blessed.\n\r",victim);
  else
   act("$N is already divinely blessed.",ch,NULL,victim,TO_CHAR);
  return;
 }

 if(!IS_GOOD(victim))
 {
  if(ch==victim)
   send_to_char("Your alignment is not good enough to ask such a favour.\n\r",ch);
  else
   act("You pray for $N but the gods ignore your wish.",ch,NULL,victim,TO_CHAR);
  return;
 }
 af.where     = TO_AFFECTS;
 af.type      = gsn_divine_blessing;
 af.level     = level;
 af.duration  = level/3;
 af.location  = APPLY_HITROLL;
 af.modifier  = (level/8)+3;
 af.bitvector = 0;
 affect_to_char( victim, &af );
 af.location  = APPLY_DAMROLL;
 af.modifier   = (level/8)+3;
 affect_to_char( victim, &af );
 af.location  = APPLY_SAVING_SPELL;
 af.modifier  = -4 - (level / 6);
 affect_to_char( victim, &af );
 send_to_char("{xYou are surrounded by a {Ys{yh{Wi{xm{Ym{ye{Wr{xi{Yn{yg{x aura.\n\r",victim);
 act("{x$n{x is surrounded by a {Ys{yh{Wi{xm{Ym{ye{Wr{xi{Yn{yg{x aura.",victim,NULL,NULL,TO_ROOM);
 return;
}

void spell_infernal_offering(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
 AFFECT_DATA af;
 CHAR_DATA *victim=(CHAR_DATA*)vo;
 if(is_affected(victim,gsn_infernal_offering))
 {
  if(ch==victim)
     send_to_char("You have already received a reward for your offering.\n\r",victim);
  else
   act("$N has already received a reward for $S offering.",ch,NULL,victim,TO_CHAR);
  return;
 }

 if(!IS_EVIL(victim))
 {
  if(ch==victim)
   send_to_char("Your offering is rejected.\n\r",ch);
  else
   act("You offer $N but $E is rejected.",ch,NULL,victim,TO_CHAR);
  return;
 }
 damage(victim,victim,UMIN(level*5,victim->hit-2),TYPE_UNDEFINED,DAM_OTHER,FALSE);
 send_to_char("A terrible pain courses through your body.\n\r",victim);
 act("$n's body spasms for a brief moment.",victim,NULL,NULL,TO_ROOM);
 af.where     = TO_AFFECTS;
 af.type      = gsn_infernal_offering;
 af.level     = level;
 af.duration  = level/3;
 af.location  = APPLY_HITROLL;
 af.modifier  = (level/4)+5;
 af.bitvector = 0;
 affect_to_char( victim, &af );
 af.location  = APPLY_DAMROLL;
 af.modifier   = (level/4)+5;
 affect_to_char( victim, &af );
 af.location  = APPLY_SAVING_SPELL;
 af.modifier  = -3 - (level / 14);
 affect_to_char( victim, &af );
 send_to_char("{xYou are surrounded by a {Wg{xhostl{Wy{x light.\n\r",victim);
 act("{x$n{x is surrounded by a {Wg{xhostl{Wy{x light.",victim,NULL,NULL,TO_ROOM);
 return;
}

void spell_outrage(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
 AFFECT_DATA af;
 CHAR_DATA *victim=(CHAR_DATA*)vo;
 if(is_affected(victim,sn))
 {
  if(ch==victim)
     send_to_char("Rage is already coursing through your body.\n\r",victim);
  else
   act("Rage is already coursing through $N's body.",ch,NULL,victim,TO_CHAR);
  return;
 }

 af.where     = TO_AFFECTS;
 af.type      = sn;
 af.level     = level;
 af.duration  = level/4+5;
 af.location  = APPLY_HITROLL;
 af.modifier  = (level/6);
 af.bitvector = 0;
 affect_to_char( victim, &af );
 af.location  = APPLY_DAMROLL;
 af.modifier   = (level/6);
 affect_to_char( victim, &af );
 af.location  = APPLY_SAVING_SPELL;
 af.modifier  = -1 - (level / 10);
 affect_to_char( victim, &af );
 send_to_char("Your eyes turn {rr{Re{rd{x as rage courses through your body.\n\r",victim);
 act("{x$n{x's eyes turn {rr{Re{rd{x as rage courses through $s body.",victim,NULL,NULL,TO_ROOM);
 return;
}

void spell_cascade( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    AFFECT_DATA af;

    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }

    dam = dice(level/2+45,6);
    if ( saves_spell( level, victim, DAM_DROWNING ) )
	dam /= 2;
    damage_old( ch, victim, dam, sn,DAM_DROWNING,TRUE);
    if(victim==NULL || (victim->in_room!=ch->in_room)
          || is_affected(victim,gsn_cascade))
       return;

    if (IS_AFFECTED(victim,AFF_HASTE))
    {
       if(!saves_spell(level,victim,DAM_OTHER)&&!IS_SET(victim->imm_flags,IMM_MAGIC))
       {
        if (!check_dispel(level,victim,skill_lookup("haste")))
        {
	    if (victim != ch)
            	send_to_char("Waves wash over your opponent but nothing happens.\n\r",ch);
            send_to_char("Waves wash over you but nothing happens.\n\r",victim);
            return;
        }

        act("$n is slowing down as waves wash over $m.",victim,NULL,NULL,TO_ROOM);    }
     return;
    }

    af.where     = TO_AFFECTS;
    af.type      = gsn_cascade;
    af.level     = level;
    af.duration  = 1002+(level/10);
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char("Waves wash over you and hinder your movements.\n\r",victim);
    act("Waves wash over $n and hinder $s movements.",victim,NULL,NULL,TO_ROOM);
    return;
}

void spell_nourish(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
 CHAR_DATA *victim=(CHAR_DATA*)vo;

 if(IS_NPC(victim))
 {
  send_to_char("You failed.\n\r",ch);
  return;
 }
 if((victim->pcdata->condition[COND_HUNGER]>=48)
      && (victim->pcdata->condition[COND_THIRST]>=48))
 {
  if(victim==ch)
   send_to_char("You are neither hungry nor thirsty.\n\r",ch);
  else
   act("$N seems neither hungry nor thirsty.\n\r",ch,NULL,victim,TO_CHAR);
  return;
 }
 victim->pcdata->condition[COND_HUNGER]=48;
 victim->pcdata->condition[COND_THIRST]=48;
 send_to_char("A strange magic courses through your body and alleviates your desire for sustenance.\n\r",victim);
 act("$n is surrounded by a faint glow as $s desire for sustenance is alleviated.",victim,NULL,NULL,TO_ROOM);
 return;
}

void spell_essence_of_mist(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
 AFFECT_DATA af;
 CHAR_DATA *victim = (CHAR_DATA*)vo;

 if(ch!=victim)
 {
  send_to_char("Bug in spell_essence_of_mist: ch!=victim\n\r",ch); 
  return;
 }
 
 if(is_affected(ch,gsn_essence_of_mist) || is_affected(ch,gsn_essence_of_bat)
               ||is_affected(ch,gsn_essence_of_wolf))
 {
  send_to_char("The form of your body has already been changed.\n\r",ch);
  return;
 }
 /* res weapon */
 af.where     = TO_RESIST;
 af.type      = gsn_essence_of_mist;
 af.level     = level;
 af.duration  = -1;
 af.location  = APPLY_NONE;
 af.modifier  = 0;
 af.bitvector = RES_WEAPON;
 if(!IS_SET(ch->res_flags,RES_WEAPON))
  affect_to_char( victim, &af );

 /* vuln magic */
 af.where     = TO_VULN;
 af.bitvector = VULN_MAGIC;
 if(!IS_SET(ch->vuln_flags,VULN_MAGIC))
  affect_to_char(victim, &af);

 /* racial dex max + 4 */
 af.where     = TO_AFFECTS;
 af.location  = APPLY_DEX_P;
 af.modifier  = 4;
 af.bitvector = 0;
 affect_to_char( victim, &af);

 /* dex + 4 */
 af.location  = APPLY_DEX;
 affect_to_char(victim, &af);

 /* racial str max -4 */
 af.location  = APPLY_STR_P;
 af.modifier  = -4;
 affect_to_char( victim, &af);

 /* change size to tiny */
 af.location  = APPLY_SIZE;
 af.modifier = SIZE_TINY;
 if(ch->size!=SIZE_TINY)
  affect_to_char( victim, &af );

 send_to_char("{WM{ci{Cs{xt{Ws{x start to swirl as your body is transformed.\n\r",ch);
 act("{WM{ci{Cs{xt{Ws{x start to swirl as $n's body is transformed.",ch,NULL,NULL,TO_ROOM);

 return;
}

void spell_essence_of_bat(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
 AFFECT_DATA af;
 CHAR_DATA *victim = (CHAR_DATA*)vo;

 if(ch!=victim)
 {
  send_to_char("Bug in spell_essence_of_bat: ch!=victim\n\r",ch);
  return;
 }

 if(is_affected(ch,gsn_essence_of_mist)||is_affected(ch,gsn_essence_of_bat)
             ||is_affected(ch,gsn_essence_of_wolf))
 {
  send_to_char("The form of your body has already been changed.\n\r",ch);
  return;
 }
 /* res magic */
 af.where     = TO_RESIST;
 af.type      = gsn_essence_of_bat;
 af.level     = level;
 af.duration  = -1;
 af.location  = APPLY_NONE;
 af.modifier  = 0;
 af.bitvector = RES_MAGIC;
 if(!IS_SET(ch->res_flags,RES_MAGIC))
  affect_to_char( victim, &af );

 /* sanctuary */
 af.where = TO_SHIELDS;
 af.bitvector = SHD_SANCTUARY;
 if(!IS_SHIELDED(ch,SHD_SANCTUARY))
  affect_to_char(victim, &af);

 /* vuln holy */
 af.where     = TO_VULN;
 af.bitvector = VULN_HOLY;
 if(!IS_SET(ch->vuln_flags,VULN_HOLY))
  affect_to_char(victim, &af);

 /* vuln fire */
 af.where     = TO_VULN;
 af.bitvector = VULN_FIRE;
 if(!IS_SET(ch->vuln_flags,VULN_FIRE))
  affect_to_char(victim, &af);

 /* race int max + 4 */
 af.where     = TO_AFFECTS;
 af.location  = APPLY_INT_P;
 af.modifier  = 4;
 af.bitvector = 0;
 affect_to_char( victim, &af);

 /* int + 4 */
 af.location  = APPLY_INT;
 affect_to_char(victim, &af);
 af.where     = TO_AFFECTS;

 /* race wis max + 4 */
 af.location  = APPLY_WIS_P;
 af.modifier  = 4;
 af.bitvector = 0;
 affect_to_char( victim, &af);

 /* wis + 4 */
 af.location  = APPLY_WIS;
 affect_to_char(victim, &af);

 send_to_char("Your voice turns to a piercing screech as your body is transformed.\n\r",ch);
 act("$n's voice turns to a piercing screech as $s body is transformed.",ch,NULL,NULL,TO_ROOM);

 return;
}

void spell_essence_of_wolf(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
 AFFECT_DATA af;
 CHAR_DATA *victim = (CHAR_DATA*)vo;

 if(ch!=victim)
 {
  send_to_char("Bug in spell_essence_of_wolf: ch!=victim\n\r",ch);
  return;
 }

 if(is_affected(ch,gsn_essence_of_mist)||is_affected(ch,gsn_essence_of_bat)
      ||is_affected(ch,gsn_essence_of_wolf))
 {
  send_to_char("The form of your body has already been changed.\n\r",ch);
  return;
 }
 /* res bash */
 af.where     = TO_RESIST;
 af.type      = gsn_essence_of_wolf;
 af.level     = level;
 af.duration  = -1;
 af.location  = APPLY_NONE;
 af.modifier  = 0;
 af.bitvector = RES_BASH;
 if(!IS_SET(ch->res_flags,RES_BASH))
  affect_to_char( victim, &af );

 /* res pierce */
 af.bitvector = RES_PIERCE;
 if(!IS_SET(ch->res_flags,RES_PIERCE))
  affect_to_char(victim, &af);

 /* vuln silver */
 af.where     = TO_VULN;
 af.bitvector = VULN_SILVER;
 if(!IS_SET(ch->vuln_flags,VULN_SILVER))
  affect_to_char(victim, &af);

 /* race str max + 4 */
 af.where     = TO_AFFECTS;
 af.location  = APPLY_STR_P;
 af.modifier  = 4;
 af.bitvector = 0;
 affect_to_char( victim, &af);

 /* str + 4 */
 af.location  = APPLY_STR;
 affect_to_char(victim, &af);
 af.where     = TO_AFFECTS;

 /* race con max + 4 */
 af.location  = APPLY_CON_P;
 af.modifier  = 4;
 af.bitvector = 0;
 affect_to_char( victim, &af);

 /* con + 4 */
 af.location  = APPLY_CON;
 affect_to_char(victim, &af);

 /* hp + level*5 */
 af.location  = APPLY_HIT;
 af.modifier  = level*5;
 affect_to_char(victim, &af);

 /* change size to huge */
 af.location  = APPLY_SIZE;
 af.modifier = SIZE_HUGE;
 if(ch->size!=SIZE_HUGE)
  affect_to_char( victim, &af );

 send_to_char("You growl menacingly as your body is transformed.\n\r",ch);
 act("$n growls menacingly as $s body is transformed.",ch,NULL,NULL,TO_ROOM);

 return;
}

void do_revert(CHAR_DATA *ch,char *argument)
{
 if(!is_affected(ch,gsn_essence_of_mist)
     && !is_affected(ch,gsn_essence_of_bat)
     && !is_affected(ch,gsn_essence_of_wolf))
 {
  send_to_char("You are already in your true form.\n\r",ch);
  return;
 }
 affect_strip(ch,gsn_essence_of_mist);
 affect_strip(ch,gsn_essence_of_bat);
 affect_strip(ch,gsn_essence_of_wolf); 
 send_to_char("A sharp pain runs down your spine as you return to your true form.\n\r",ch);
 act("$n returns to his true form.",ch,NULL,NULL,TO_ROOM);
 return;
}
     
void spell_shrink(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
 CHAR_DATA *victim=(CHAR_DATA*)vo;
 AFFECT_DATA af;

 if(victim->size==SIZE_TINY)
 {
  if(ch==victim)
   send_to_char("You can't get any smaller.\n\r",ch);
  else
   act("$N can't get any smaller.",ch,NULL,victim,TO_CHAR);
  return;
 }

 if(!IS_NPC(victim)&&(victim->size!=pc_race_table[victim->race].size))
 {
  if(ch==victim)
   send_to_char("Your size has already been changed.\n\r",ch);
  else
   act("$N's size has already been changed.",ch,NULL,victim,TO_CHAR);
  return;
 }

 if (saves_spell(level,victim,DAM_OTHER)
 ||  IS_SET(victim->imm_flags,IMM_MAGIC))
 {
   if (victim != ch)
     send_to_char("Your spell failed.\n\r",ch);
   send_to_char("You feel a sharp pain but it subsides quickly.\n\r",victim);
   return;
 }

 af.where = TO_AFFECTS;
 af.level = level;
 af.type  = sn;
 af.duration = 1002+(level/10);
 af.location = APPLY_SIZE;
 af.modifier = SIZE_TINY;
 af.bitvector = 0;
 affect_to_char(victim,&af);
 send_to_char("Your head jerks in pain as your body shrinks.\n\r",victim);
 act("$n's head jerks in pain as $s body shrinks.",victim,NULL,NULL,TO_ROOM);
 return;
}

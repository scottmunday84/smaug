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
#include "tables.h"


extern char *target_name;
void scribe_func(CHAR_DATA *ch, char *argument,const char *verb);

int count_shields(CHAR_DATA *ch)
{
  int count=0;
 
  if(IS_SHIELDED(ch, SHD_ICE)) count++;
  if(IS_SHIELDED(ch, SHD_FIRE)) count++;
  if(IS_SHIELDED(ch, SHD_SHOCK)) count++;
  if(IS_SHIELDED(ch, SHD_SHADOW)) count++;
/*  if(IS_SHIELDED(ch, SHD_BLADE)) count++;*/
  if(IS_SHIELDED(ch, SHD_PRISMATIC)) count++;
/*  if(IS_SHIELDED(ch, SHD_ACID)) count++;*/
  if(IS_SHIELDED(ch, SHD_EARTH)) count++;

  return count;
}

void spell_farsight(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, AFF_FARSIGHT) )
    {
        if (victim == ch)
          send_to_char("Your eyes are already as good as they get.\n\r",ch);
        else
          act("$N can see just fine.",ch,NULL,victim,TO_CHAR);
        return;
    }
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_FARSIGHT;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes jump into focus.\n\r", victim );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_protection_voodoo(int sn,int level,CHAR_DATA *ch,void *vo,int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED(victim, SHD_PROTECT_VOODOO) )
    {
	return;
    }
    af.where	= TO_SHIELDS;
    af.type	= sn;
    af.level	= level;
    af.duration	= level;
    af.location	= APPLY_NONE;
    af.modifier	= 0;
    af.bitvector = SHD_PROTECT_VOODOO;
    affect_to_char( victim, &af );
    return;
}

void spell_empower( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    OBJ_DATA *object;
    char buf[MAX_STRING_LENGTH];
    char *name;
    int new_sn;
    int mana;
    int newmana;
    int newtarget;

    if ( ( new_sn = find_spell( ch,target_name ) ) < 0
    || ( !IS_NPC(ch) && (ch->level < skill_table[new_sn].skill_level[ch->class]
    ||                   ch->pcdata->learned[new_sn] == 0))
    || (skill_table[new_sn].spell_fun == spell_null))
    {
        send_to_char( "What spell do you wish to bind?\n\r", ch );
        return;
    }

    name = skill_table[new_sn].name;
    if ( !can_empower(name) )
    {
	send_to_char( "This spell can not be empowered.\n\r", ch );
	return;
    }
    newtarget = skill_table[new_sn].target;
    if (ch->level + 2 == skill_table[sn].skill_level[ch->class])
	mana = 50;
    else
    	mana = UMAX(
	    skill_table[sn].min_mana,
	    100 / ( 2 + ch->level - skill_table[sn].skill_level[ch->class] ) );

    if (ch->level + 2 == skill_table[new_sn].skill_level[ch->class])
	newmana = 50;
    else
    	newmana = UMAX(
	    skill_table[new_sn].min_mana,
	    100 / ( 2 + ch->level - skill_table[new_sn].skill_level[ch->class] ) );

    if ( (ch->mana - mana - newmana) < 0)
    {
        send_to_char( "You do not have enough mana.\n\r", ch );
        return;
    }
    ch->mana -= newmana;

    if ((newtarget == TAR_CHAR_DEFENSIVE) || (newtarget == TAR_CHAR_SELF))
    {
	object = create_object(get_obj_index(OBJ_VNUM_POTION), 0);
    }
    else
    {
	object = create_object(get_obj_index(OBJ_VNUM_SCROLL), 0);
    }
    object->value[0] = ch->level;
    object->value[1] = new_sn;
    object->level = ch->level-5;

    sprintf( buf, "%s%s", object->short_descr, name);
    free_string(object->short_descr);
    object->short_descr = str_dup(buf);
    sprintf( buf, "%s %s", object->name, name);
    free_string(object->name);
    object->name = str_dup(buf);

    if ((newtarget == TAR_CHAR_DEFENSIVE) || (newtarget == TAR_CHAR_SELF))
    {
	sprintf( buf, "$n has created a potion of %s!",name);
	act(buf,ch,object,NULL,TO_ROOM);
	sprintf( buf, "You create a potion of %s!\n\r",name);
	send_to_char(buf,ch);
    }
    else
    {
	sprintf( buf, "$n has created a scroll of %s!",name);
	act(buf,ch,object,NULL,TO_ROOM);
	sprintf( buf, "You create a scroll of %s!\n\r",name);
	send_to_char(buf,ch);
    }
    obj_to_char(object,ch);
    return;
}

void spell_resurrect( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    OBJ_DATA *obj;
    OBJ_DATA *cobj;
    OBJ_DATA *obj_next;
    CHAR_DATA *pet;
    int	length;

    if ( ( obj = get_obj_here( ch, "corpse" ) ) == NULL )
    {
	send_to_char( "There's no corpse here.\n\r", ch );
	return;
    }
    if ( ch->pet != NULL )
    {
	send_to_char("You failed.\n\r",ch);
	return;
    }
    pMobIndex = get_mob_index( MOB_VNUM_CORPSE );
    pet = create_mobile( pMobIndex );
    if (!IS_SET(pet->act, ACT_PET) ) 
        SET_BIT(pet->act, ACT_PET);
    if (!IS_SET(pet->affected_by, AFF_CHARM) ) 
        SET_BIT(pet->affected_by, AFF_CHARM);
    pet->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
    sprintf( buf, "%s{GThe mark of %s is on it's forehead.{x.\n\r",
	pet->description, ch->name );
    free_string( pet->description );
    pet->description = str_dup( buf );
    free_string( pet->short_descr );
    pet->short_descr = str_dup( str_replace(obj->short_descr, "corpse", "zombie") );
    sprintf( buf, "%s", str_replace(obj->description, "corpse", "zombie") );
    length = strlen(buf)-12;
    strncpy( arg, buf, length);
    arg[length] = '\0';
    sprintf( buf, "%s standing here.\n\r", arg);
    free_string( pet->long_descr );
    pet->long_descr = str_dup( buf );
    char_to_room( pet, ch->in_room );
    add_follower( pet, ch );
    pet->leader = ch;
    ch->pet = pet;
    pet->alignment = ch->alignment;
    pet->level = UMAX(1, UMIN(109, ((ch->level/2)+(obj->level/2))));
    pet->max_hit = pet->level * 15;
    pet->hit = pet->max_hit;
    pet->armor[0] = pet->level/6;
    pet->armor[1] = pet->level/6;
    pet->armor[2] = pet->level/6;
    pet->armor[3] = pet->level/8;
    for ( cobj = obj->contains; cobj != NULL; cobj = obj_next )
    {
	obj_next = cobj->next_content;
	obj_from_obj( cobj );
	obj_to_room( cobj, ch->in_room );
    }
    extract_obj( obj );
    sprintf( buf, "%s stands up and starts following you.\n\r", pet->short_descr);
    send_to_char( buf, ch);
    sprintf( buf, "%s stands up and starts following $n.", pet->short_descr);
    act( buf, ch, NULL, NULL, TO_ROOM);
    return;
}

void spell_conjure( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    char buf[MAX_STRING_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    OBJ_DATA *stone;
    CHAR_DATA *pet;
 
    if (IS_NPC(ch))
	return;

    stone = get_eq_char(ch,WEAR_HOLD);
    if (!IS_IMMORTAL(ch) 
    &&  (stone == NULL || stone->item_type != ITEM_DEMON_STONE))
    {
        send_to_char("You lack the proper component for this spell.\n\r",ch);
        return;
    }

    if ( ch->pet != NULL )
    {
	send_to_char("You failed.\n\r",ch);
	return;
    }

    pMobIndex = get_mob_index( MOB_VNUM_DEMON );
    pet = create_mobile( pMobIndex );
    if (!IS_SET(pet->act, ACT_PET) )
        SET_BIT(pet->act, ACT_PET);
    if (!IS_SET(pet->affected_by, AFF_CHARM) )
        SET_BIT(pet->affected_by, AFF_CHARM);
    pet->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
    sprintf( buf, "%s{GThe mark of %s is on it's forehead.{x\n\r",
        pet->description, ch->name );
    free_string( pet->description );
    pet->description = str_dup( buf );
    char_to_room( pet, ch->in_room );
    if (stone != NULL && stone->item_type == ITEM_DEMON_STONE)
    {
        act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR);
        act("$n draws upon the power of $p.",ch,stone,NULL,TO_ROOM);
        act("It flares brightly and $N appears.",ch,stone,pet,TO_CHAR);
        act("It flares brightly and $N appears.",ch,stone,pet,TO_ROOM);
        obj_from_char(stone);
        extract_obj(stone);
    } else
    {  
        act("$N suddenly appears in the room.",ch,NULL,pet,TO_CHAR);
        act("$N suddenly appears in the room.",ch,NULL,pet,TO_ROOM);
    }
    add_follower( pet, ch );
    pet->leader = ch;
    ch->pet = pet;
    pet->alignment = ch->alignment;
    pet->level = ch->level;
    pet->max_hit = pet->level * 30;
    pet->hit = pet->max_hit;
    pet->armor[0] = pet->level/2;
    pet->armor[1] = pet->level/2;
    pet->armor[2] = pet->level/2;
    pet->armor[3] = pet->level/3;

    return;
}



void spell_animate( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    CHAR_DATA *pet;
    int	length;

    if  ((obj->pIndexData->vnum > 17)
	|| (obj->pIndexData->vnum < 12))
    {
	send_to_char( "That's not a body part!\n\r", ch );
	return;
    }

    pMobIndex = get_mob_index( MOB_VNUM_ANIMATE );
    pet = create_mobile( pMobIndex );
    SET_BIT(pet->affected_by, AFF_CHARM);
    pet->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
    sprintf( buf, "%s{GIt's branded with the mark of %s.{x.\n\r",
	obj->description, ch->name );
    free_string( pet->description );
    pet->description = str_dup( buf );
    free_string( pet->short_descr );
    pet->short_descr = str_dup( obj->short_descr );
    free_string( pet->name );
    pet->name = str_dup( obj->name );
    sprintf( buf, "%s", obj->description);
    length = strlen(buf)-12;
    strncpy( arg, buf, length);
    arg[length] = '\0';
    sprintf( buf, "%s floating here.\n\r", arg);
    free_string( pet->long_descr );
    pet->long_descr = str_dup( buf );
    char_to_room( pet, ch->in_room );
    add_follower( pet, ch );
    pet->leader = ch;
    obj_from_char( obj );
    sprintf( buf, "%s floats up and starts following you.\n\r", pet->short_descr);
    send_to_char( buf, ch);
    sprintf( buf, "%s floats up and starts following $n.", pet->short_descr);
    act( buf, ch, NULL, NULL, TO_ROOM);
    return;
}

void spell_iceshield( int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_SHIELDED(victim, SHD_ICE))
    {
	if(victim == ch)
	    send_to_char("You are already surrounded by an {Cicy{x shield.\n\r", ch);
	else
	    act("$N is already surrounded by an {Cicy{x shield.",ch,NULL,victim,TO_CHAR);
	return;
    }
    if (count_shields(victim)>=3 && (victim->class!=class_lookup("sorcerer")) &&
        (victim->class!=class_lookup("wizard")))
    {
       if(victim == ch)
        send_to_char("You couldn't stand the strain of opposing elements.\n\r",ch);
       else
        act("$N couldn't stand the strain of opposing elements.",ch,NULL,victim,TO_CHAR);
       return;
    }

    if (IS_NPC(victim))
    {
	send_to_char("You failed.\n\r", ch);
	return;
    }

/*
    if ( (skill_table[sn].skill_level[victim->class] > LEVEL_HERO)
	&& (victim->level < LEVEL_IMMORTAL) )
    {
	send_to_char("You are surrounded by an {Cicy{x shield.\n\r", victim);
	act("$n is surrounded by an {Cicy{x shield.",victim, NULL,NULL,TO_ROOM);
	send_to_char("Your {Cicy{x shield quickly melts away.\n\r", victim);
	act("$n's {Cicy{x shield quickly melts away.",victim, NULL,NULL,TO_ROOM);
	return;
    }
*/

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/6;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = SHD_ICE;

   affect_to_char(victim, &af);
   send_to_char("You are surrounded by an {Cicy{x shield.\n\r", victim);
   act("$n is surrounded by an {Cicy{x shield.",victim, NULL,NULL,TO_ROOM);
   return;
}

void spell_fireshield(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{
   
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_SHIELDED(victim, SHD_FIRE))
    {
	if (victim == ch)
	    send_to_char("You are already surrounded by a {Rfirey{x shield.\r\n", ch);
	else
	    act("$N is already surrounded by a {Rfiery{x shield.",ch,NULL,victim,TO_CHAR);
	return;
    }
    if (count_shields(victim)>=3 && (victim->class!=class_lookup("sorcerer")) &&
        (victim->class!=class_lookup("wizard")))
    {
       if(victim == ch)
        send_to_char("You couldn't stand the strain of opposing elements.\n\r",ch);
       else
        act("$N couldn't stand the strain of opposing elements.",ch,NULL,victim,TO_CHAR);
       return;
    }
    if (IS_NPC(victim))
    {
	send_to_char("You failed.\n\r", ch);
	return;
    }

/*
    if ( (skill_table[sn].skill_level[victim->class] > LEVEL_HERO)
	&& (victim->level < LEVEL_IMMORTAL) )
    {
	send_to_char("You are surrounded by a {Rfiery{x shield.\n\r", victim);
	act("$n is surrounded by a {Rfiery{x shield.",victim, NULL,NULL,TO_ROOM);
	send_to_char("Your {Rfirey{x shield gutters out.\n\r", victim);
	act("$n's {Rfirey{x shield gutters out.",victim, NULL,NULL,TO_ROOM);
	return;
    }
*/

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/6;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = SHD_FIRE;

    affect_to_char(victim, &af);
    send_to_char("You are surrounded by a {Rfiery{x shield.\n\r", victim);
    act("$n is surrounded by a {Rfiery{x shield.",victim, NULL,NULL,TO_ROOM);
    return;

}

void spell_shockshield(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{

    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_SHIELDED(victim, SHD_SHOCK))
    {
	if (victim == ch)
	    send_to_char("You are already surrounded by a {Bcrackling{x shield.\n\r", ch);
	else
	    act("$N is already surrounded by a {Bcrackling{x shield.",ch, NULL, victim, TO_CHAR);
	return;
    }
    if (count_shields(victim)>=3 && (victim->class!=class_lookup("sorcerer")) &&
        (victim->class!=class_lookup("wizard")))
    {
       if(victim == ch)
        send_to_char("You couldn't stand the strain of opposing elements.\n\r",ch);
       else
        act("$N couldn't stand the strain of opposing elements.",ch,NULL,victim,TO_CHAR);
       return;
    }
    if (IS_NPC(victim))
    {
	send_to_char("You failed.\n\r", ch);
	return;
    }

/*
    if ( (skill_table[sn].skill_level[victim->class] > LEVEL_HERO)
	&& (victim->level < LEVEL_IMMORTAL) )
    {
	send_to_char("You are surrounded by a {Bcrackling{x shield.\n\r", victim);
	act("$n is surrounded by a {Bcrackling{x shield.",victim, NULL,NULL,TO_ROOM);
	send_to_char("Your {Bcrackling{x shield sizzles and fades.\n\r", victim);
	act("$n's {Bcrackling{x shield sizzles and fades.",victim, NULL,NULL,TO_ROOM);
	return;
    }
*/

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/6;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = SHD_SHOCK;

    affect_to_char(victim, &af);
    send_to_char("You are surrounded by a {Bcrackling{x field.\n\r",victim);
    act("$n is surrounded by a {Bcrackling{x shield.",victim, NULL,NULL, TO_ROOM);
    return;
}

void spell_quest_pill( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if (IS_NPC(victim))
	return;

    victim->qps++;
    send_to_char( "{YYou've gained a {RQuest Point{Y!{x\n\r", victim );
    printf_log("QP: %s gained a quest point from %s at %d.",
	victim->name,ch->name,victim->in_room->vnum);	
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_voodoo( int sn, int level, CHAR_DATA *ch, void *vo,int target )
{
    char name[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *bpart;
    OBJ_DATA *doll;

    bpart = get_eq_char(ch,WEAR_HOLD);
    if  ((bpart == NULL)
    ||   (bpart->pIndexData->vnum < 12)
    ||   (bpart->pIndexData->vnum > 17))
    {
	send_to_char("You are not holding a body part.\n\r",ch);
	return;
    }
    if (bpart->value[4] == 0)
    {
	send_to_char("This body part is from a mobile.\n\r",ch);
	return;
    }
    one_argument(bpart->name, name);
    doll = create_object(get_obj_index(OBJ_VNUM_VOODOO), 0);
    sprintf( buf, doll->short_descr, name );
    free_string( doll->short_descr );
    doll->short_descr = str_dup( buf );
    sprintf( buf, doll->description, name );
    free_string( doll->description );
    doll->description = str_dup( buf );
    sprintf( buf, doll->name, name );
    free_string( doll->name );
    doll->name = str_dup( buf );
    act( "$p morphs into a voodoo doll",ch,bpart,NULL,TO_CHAR);
    obj_from_char( bpart );
    obj_to_char(doll,ch);
    equip_char(ch,doll,WEAR_HOLD);
    act( "$n has created $p!", ch, doll, NULL, TO_ROOM );
    return;
}

void spell_shadowshield(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{

    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_SHIELDED(victim, SHD_SHADOW))
    {
	if (victim == ch)
	    send_to_char("You are already surrounded by the {Dshadows{x.\n\r", ch);
	else
	    act("$N is already surrounded by the {Dshadows{x.",ch, NULL, victim, TO_CHAR);
	return;
    }
    if (count_shields(victim)>=3 && (victim->class!=class_lookup("sorcerer")) &&
        (victim->class!=class_lookup("wizard")))
    {
       if(victim == ch)
        send_to_char("You couldn't stand the strain of opposing elements.\n\r",ch);
       else
        act("$N couldn't stand the strain of opposing elements.",ch,NULL,victim,TO_CHAR);
       return;
    }
    if (IS_NPC(victim))
    {
	send_to_char("You failed.\n\r", ch);
	return;
    }

    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/6;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = SHD_SHADOW;

    affect_to_char(victim, &af);
    send_to_char("You are surrounded by the {Dshadows{x.\n\r",victim);
    act("$n is surrounded by the {Dshadows{x.",victim, NULL,NULL, TO_ROOM);
    return;
}

void spell_bladebarrier(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{

    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_SHIELDED(victim, SHD_BLADE))
    {
	if (victim == ch)
	    send_to_char("You are already surrounded by a bladebarrier.\n\r", ch);
	else
	    act("$N is already surrounded by a bladebarrier.",ch, NULL, victim, TO_CHAR);
	return;
    }
    if (count_shields(victim)>=3 && (victim->class!=class_lookup("sorcerer")) &&
        (victim->class!=class_lookup("wizard")))
    {
       if(victim == ch)
        send_to_char("You couldn't stand the strain of opposing elements.\n\r",ch);
       else
        act("$N couldn't stand the strain of opposing elements.",ch,NULL,victim,TO_CHAR);
       return;
    }
    if (IS_NPC(victim))
    {
	send_to_char("You failed.\n\r", ch);
	return;
    }


    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/6;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = SHD_BLADE;

    affect_to_char(victim, &af);
    send_to_char("You are surrounded by a deadly bladebarrier.\n\r",victim);
    act("$n is surrounded by a deadly bladebarrier.",victim, NULL,NULL, TO_ROOM);
    return;
}
void spell_prismaticshield(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{

    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_SHIELDED(victim, SHD_PRISMATIC))
    {
	if (victim == ch)
	    send_to_char("You are already surrounded by a prismaticshield.\n\r", ch);
	else
	    act("$N is already surrounded by a prismaticshield.",ch, NULL, victim, TO_CHAR);
	return;
    }
    if (count_shields(victim)>=3 && (victim->class!=class_lookup("sorcerer")) &&
        (victim->class!=class_lookup("wizard")))
    {
       if(victim == ch)
        send_to_char("You couldn't stand the strain of opposing elements.\n\r",ch);
       else
        act("$N couldn't stand the strain of opposing elements.",ch,NULL,victim,TO_CHAR);
       return;
    }
    if (IS_NPC(victim))
    {
	send_to_char("You failed.\n\r", ch);
	return;
    }


    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/6;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = SHD_PRISMATIC;

    affect_to_char(victim, &af);
    send_to_char("You are surrounded by a {Mp{mr{Ci{cs{Wm{xa{Dtic{xs{Wh{ci{Ce{ml{Md{x.\n\r",victim);
    act("$n is surrounded by a {Mp{mr{Ci{cs{Wm{xa{Dtic{xs{Wh{ci{Ce{ml{Md{x.",victim, NULL,NULL, TO_ROOM);
    return;
}
void spell_acidshield(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{

    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_SHIELDED(victim, SHD_ACID))
    {
	if (victim == ch)
	    send_to_char("You are already surrounded by an {yacidic{x shield.\n\r", ch);
	else
	    act("$N is already surrounded by an {yacidic{x shield.",ch, NULL, victim, TO_CHAR);
	return;
    }
    if (count_shields(victim)>=3 && (victim->class!=class_lookup("sorcerer")) &&
        (victim->class!=class_lookup("wizard")))
    {
       if(victim == ch)
        send_to_char("You couldn't stand the strain of opposing elements.\n\r",ch);
       else
        act("$N couldn't stand the strain of opposing elements.",ch,NULL,victim,TO_CHAR);
       return;
    }
    if (IS_NPC(victim))
    {
	send_to_char("You failed.\n\r", ch);
	return;
    }


    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/6;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = SHD_ACID;

    affect_to_char(victim, &af);
    send_to_char("You are surrounded by an {yacidic{x shield.\n\r",victim);
    act("$n is surrounded by an {yacidic{x shield.",victim, NULL,NULL, TO_ROOM);
    return;
}

void spell_earthshield(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{

    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_SHIELDED(victim, SHD_EARTH))
    {
	if (victim == ch)
	    send_to_char("You are already surrounded by an earthshield.\n\r", ch);
	else
	    act("$N is already surrounded by an earthshield.",ch, NULL, victim, TO_CHAR);
	return;
    }
    if (count_shields(victim)>=3 && (victim->class!=class_lookup("sorcerer")) &&
        (victim->class!=class_lookup("wizard")))
    {
       if(victim == ch)
        send_to_char("You couldn't stand the strain of opposing elements.\n\r",ch);
       else
        act("$N couldn't stand the strain of opposing elements.",ch,NULL,victim,TO_CHAR);
       return;
    }
    if (IS_NPC(victim))
    {
	send_to_char("You failed.\n\r", ch);
	return;
    }


    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/6;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = SHD_EARTH;

    affect_to_char(victim, &af);
    send_to_char("You are surrounded by an earthshield.\n\r",victim);
    act("$n is surrounded by an earthshield.",victim, NULL,NULL, TO_ROOM);
    return;
}

void spell_mana_shield(int sn, int level, CHAR_DATA *ch, void *vo, int target)
{

    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (IS_SHIELDED(victim, SHD_MANA))
    {
	if (victim == ch)
	    send_to_char("You are already surrounded by a mana shield.\n\r", ch);
	else
	    act("$N is already surrounded by a mana shield.",ch, NULL, victim, TO_CHAR);
	return;
    }

    if (IS_NPC(victim))
    {
	send_to_char("You failed.\n\r", ch);
	return;
    }


    af.where     = TO_SHIELDS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level/6;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = SHD_MANA;

    affect_to_char(victim, &af);
    send_to_char("You are surrounded by a mana shield.\n\r",victim);
    act("$n is surrounded by a mana shield.",victim, NULL,NULL, TO_ROOM);
    return;
}

void do_scribe(CHAR_DATA *ch,char *argument)
{
 scribe_func(ch,argument,"scribe");
 return;
}

void do_brew(CHAR_DATA *ch,char *argument)
{
 scribe_func(ch,argument,"brew");
 return;
}

void scribe_func(CHAR_DATA *ch, char *argument,const char *verb)
{
 char arg[4][MAX_INPUT_LENGTH];
 char buf[MAX_STRING_LENGTH],number[21];
 int skill;
 OBJ_DATA *object=NULL,*obj=NULL,*obj_next=NULL;
 int mana=0;
 int sn[3],lev[3],i;
 OBJ_DATA *comp[3];

 for(i=0;i<3;i++)
 {
  sn[i]=-1;
  lev[i]=-1;
  comp[i]=NULL;
 }
 for(i=0;i<4;i++)
  argument=one_argument(argument,arg[i]);

 if(IS_NPC(ch))
  return;
 if(!strcmp(verb,"scribe"))
 {
  if((skill=get_skill(ch,gsn_scribe))==0)
  {
   send_to_char("You don't know how to scribe.\n\r",ch);
   return;
  }
 }
 else if(!strcmp(verb,"brew"))
 {
  if((skill=get_skill(ch,gsn_brew))==0)
  {
   send_to_char("You don't know how to brew.\n\r",ch);
   return;
  }
 }
 else
 {
  send_to_char("Scribe_func called with illegal verb; notify an immortal, please.\n\r",ch);
  return;
 }
 if(arg[0][0]=='\0')
 {
  sprintf(buf,"You need to specify a name for your %s.\n\r",
    (!strcmp(verb,"scribe") ? "scroll" : "potion"));
  send_to_char(buf,ch);
  return;
 }
 if(arg[1][0]=='\0')
 {
  sprintf(buf,"You need to specify at least one spell to %s.\n\r",verb);
  return;
 }
 for(i=1;i<4;i++)
 {
  if(arg[i][0]=='\0')
    break;
  if((i==3) && (ch->pcdata->tier==0))
  {
    send_to_char("You can not imbue more than two spells on first tier.\n\r",ch);
    return;
  }
  switch(i)
  {
   case 1: sprintf(number,"first"); break;
   case 2: sprintf(number,"second"); break;
   case 3: sprintf(number,"third"); break;
  }
  if ( ( sn[i-1] = find_spell( ch,arg[i] ) ) < 0
    || ( !IS_NPC(ch) && (ch->level < skill_table[sn[i-1]].skill_level[ch->class]
    ||                   ch->pcdata->learned[sn[i-1]] == 0))
    || (skill_table[sn[i-1]].spell_fun==spell_null))
    {

        sprintf(buf,"Which spell do you wish to bind as %s spell?\n\r",number);
        send_to_char( buf, ch );
        return;
    }

  if(!can_empower(skill_table[sn[i-1]].name))
  {
   sprintf(buf,"The %s spell you specified can not be empowered.\n\r",number);
   send_to_char( buf, ch);
   return;
  }
  lev[i-1]=UMIN(skill_table[sn[i-1]].skill_level[ch->class],101);
 }

 for(i=0;i<3;i++)
 {
  if(lev[i]==-1)
   break;
  switch(i+1)
  {
   case 1: sprintf(number,"first"); break;
   case 2: sprintf(number,"second"); break;
   case 3: sprintf(number,"third"); break;
  }
  for(obj=ch->carrying; obj!=NULL;obj=obj_next)
  {
   obj_next=obj->next_content;
   if(obj->wear_loc==WEAR_NONE
       && (can_see_obj(ch, obj))
       && (obj->item_type == ITEM_COMPONENT)
       && (obj->level>=lev[i])
       && ((comp[i]==NULL)||(comp[i]->level>obj->level))
       && ((i==0) 
          || ((i==1) && (obj!=comp[i-1]))
          || ((i==2) && (obj!=comp[i-1]) && (obj!=comp[i-2]))) )
      comp[i]=obj;
  }
  if(comp[i]==NULL)
  {
   sprintf(buf,"You don't have the necessary component for %sing the %s spell.\n\r",verb,number);
   send_to_char(buf,ch);
   return;
  }
 }

 mana = skill_table[sn[0]].min_mana*3 +
        (sn[1]!=-1?skill_table[sn[1]].min_mana*5 : 0),
        (sn[2]!=-1?skill_table[sn[2]].min_mana*7 : 0);
 if((ch->mana - mana) < 0)
 {
  send_to_char("You don't have enough mana.\n\r",ch);
  return;
 }
 ch->mana-=mana;
 for(i=0;i<3;i++)
 {
  if(comp[i]==NULL)
   break;

   sprintf(buf,"You draw upon the power of %s!\n\r",comp[i]->short_descr);
   send_to_char(buf,ch);
   extract_obj(comp[i]);
 }
 WAIT_STATE(ch,skill_table[gsn_scribe].beats);

 if(!strcmp(verb,"scribe"))
 {
  if(number_percent()<skill)
  {
   object = create_object(get_obj_index(OBJ_VNUM_SCROLL), 0);

   object->value[0] = ch->level;
   object->value[1] = sn[0];
   object->value[2] = (sn[1]!=-1?sn[1]:0);
   object->value[3] = (sn[2]!=-1?sn[2]:0);
   object->level = ch->level;

   sprintf( buf, "%s's scroll of %s", capitalize(ch->name), arg[0]);
   free_string(object->short_descr);
   object->short_descr = str_dup(buf);
   sprintf( buf, "%s's scroll %s", ch->name, arg[0]);
   free_string(object->name);
   object->name = str_dup(buf);
   SET_BIT(object->extra_flags,ITEM_NODROP);
   SET_BIT(object->extra_flags,ITEM_NOUNCURSE);
   obj_to_char(object,ch);
   act("{G$n{G scribbles some lines on an ancient parchment.{x",ch,NULL,NULL,TO_ROOM);
   send_to_char("{GYou scribble some lines on an ancient parchment.{x\n\r",ch);
   check_improve(ch,gsn_scribe,TRUE,2);
  }
  else
  {
     act("{G$n{G scribbles some lines on an ancient parchment, only to notice a mistake.{x",ch,NULL,NULL,TO_ROOM);
     send_to_char("{GYou scribble some lines on an ancient parchment, only to notice a mistake.{x\n\r",ch);
     check_improve(ch,gsn_scribe,FALSE,2);
  }
 }
 else if(!strcmp(verb,"brew"))
 {
  if(number_percent()<skill)
  {
   object = create_object(get_obj_index(OBJ_VNUM_POTION), 0);

   object->value[0] = ch->level;
   object->value[1] = sn[0];
   object->value[2] = (sn[1]!=-1?sn[1]:0);
   object->value[3] = (sn[2]!=-1?sn[2]:0);
   object->level = ch->level;

   sprintf( buf, "%s's potion of %s", capitalize(ch->name), arg[0]);
   free_string(object->short_descr);
   object->short_descr = str_dup(buf);
   sprintf( buf, "%s's potion %s", ch->name, arg[0]);
   free_string(object->name);
   object->name = str_dup(buf);
   SET_BIT(object->extra_flags,ITEM_NODROP);
   SET_BIT(object->extra_flags,ITEM_NOUNCURSE);
   obj_to_char(object,ch);
   act("{GWith considerable effort, $n{G brews a strange potion.{x",ch,NULL,NULL,TO_ROOM);
   send_to_char("{GWith considerable effort, you brew a strange potion.{x\n\r",ch);
   check_improve(ch,gsn_brew,TRUE,2);
  }
  else
  {
     act("{G$n{G tries to brew a potion and fails miserably.{x",ch,NULL,NULL,TO_ROOM);
     send_to_char("{GYou try to brew a potion and fail miserably.{x\n\r",ch);
     check_improve(ch,gsn_brew,FALSE,2);
  }
 }
 return;
}

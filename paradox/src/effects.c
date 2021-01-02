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
#include "recycle.h"

void acid_effect(void *vo, int level, int dam, int target)
{
    if (target == TARGET_ROOM) /* nail objects on the floor */
     {
	ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
	OBJ_DATA *obj, *obj_next;

	for (obj = room->contents; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    acid_effect(obj,level,dam,TARGET_OBJ);
	}
	return;
    }

    if (target == TARGET_CHAR)  /* do the effect on a victim */
    {
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj, *obj_next;
	
	/* let's toast some gear */
	for (obj = victim->carrying; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    acid_effect(obj,level,dam,TARGET_OBJ);
	}
	return;
    }

    if (target == TARGET_OBJ) /* toast an object */
    {
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	OBJ_DATA *t_obj,*n_obj;
	int chance;
	char *msg;

	if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF)
	||  IS_OBJ_STAT(obj,ITEM_NOPURGE)
        ||  IS_OBJ_STAT(obj,ITEM_QUEST)
	||  number_range(0,4) == 0)
	    return;

	chance = level / 4 + dam / 10;

	if (chance > 25)
	    chance = (chance - 25) / 2 + 25;
	 if (chance > 50)
	    chance = (chance - 50) / 2 + 50;

	if (IS_OBJ_STAT(obj,ITEM_BLESS))
	    chance -= 5;

	chance -= obj->level * 2;

	switch (obj->item_type)
	{
	    default:
		return;
	    case ITEM_CONTAINER:
	    case ITEM_PIT:
	    case ITEM_CORPSE_PC:
	    case ITEM_CORPSE_NPC:
		msg = "$p fumes and dissolves.";
		break;
	    case ITEM_ARMOR:
		msg = "$p is pitted and etched.";
		break;
	    case ITEM_CLOTHING:
		msg = "$p is corroded into scrap.";
	 	break;
	    case ITEM_STAFF:
	    case ITEM_WAND:
		chance -= 10;
		msg = "$p corrodes and breaks.";
		break;
	    case ITEM_SCROLL:
		chance += 10;
		msg = "$p is burned into waste.";
		break; 
	}

	chance = URANGE(5,chance,95);

	if (number_percent() > chance)
	    return;

	if (obj->carried_by != NULL)
	    xact_new(msg,obj->carried_by,obj,NULL,TO_ALL,POS_RESTING,SUPPRESS_BURN);
	else if (obj->in_room != NULL && obj->in_room->people != NULL)
	    xact_new(msg,obj->in_room->people,obj,NULL,TO_ALL,POS_RESTING,SUPPRESS_BURN);

	if (obj->item_type == ITEM_ARMOR)  /* etch it */
	{
	    AFFECT_DATA *paf;
	    bool af_found = FALSE;
	    int i;

 	    affect_enchant(obj);

	    for ( paf = obj->affected; paf != NULL; paf = paf->next)
            {
                if ( paf->location == APPLY_AC)
                {
                    af_found = TRUE;
                    paf->type = -1;
                    paf->modifier += 1;
                    paf->level = UMAX(paf->level,level);
		    break;
                }
            }

            if (!af_found)
            /* needs a new affect */
            {
		paf = new_affect();

                paf->type       = -1;
                paf->level      = level;
                paf->duration   = -1;
                paf->location   = APPLY_AC;
                paf->modifier   =  1;
                paf->bitvector  = 0;
                paf->next       = obj->affected;
                obj->affected   = paf;
            }

            if (obj->carried_by != NULL && obj->wear_loc != WEAR_NONE)
                for (i = 0; i < 4; i++)
                    obj->carried_by->armor[i] += 1;
            return;
	}

	/* get rid of the object */
	if (obj->contains)  /* dump contents */
	{
	    for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj)
	    {
		n_obj = t_obj->next_content;
		obj_from_obj(t_obj);
		if (obj->in_room != NULL)
		    obj_to_room(t_obj,obj->in_room);
		else if (obj->carried_by != NULL)
		    obj_to_room(t_obj,obj->carried_by->in_room);
		else
		{
		    extract_obj(t_obj);
		    continue;
		}

		acid_effect(t_obj,level/2,dam/2,TARGET_OBJ);
	    }
 	}

	extract_obj(obj);
	return;
    }
}


void cold_effect(void *vo, int level, int dam, int target)
{
    if (target == TARGET_ROOM) /* nail objects on the floor */
    {
        ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

        for (obj = room->contents; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;
            cold_effect(obj,level,dam,TARGET_OBJ);
        }
        return;
    }

    if (target == TARGET_CHAR) /* whack a character */
    {
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj, *obj_next;
	
	/* chill touch effect */
	if (!saves_spell(level + dam / 20, victim, DAM_COLD))
	{
	    AFFECT_DATA af;

            xact_new("$n turns blue and shivers.",victim,NULL,NULL,TO_ROOM,POS_RESTING,SUPPRESS_FLAG_EFFECTS);
	    xact_new("A chill sinks deep into your bones.",victim,NULL,NULL,TO_CHAR,POS_RESTING,SUPPRESS_FLAG_EFFECTS);
            af.where     = TO_AFFECTS;
            af.type      = skill_lookup("chill touch");
            af.level     = level;
            af.duration  = 1004;
            af.location  = APPLY_AC;
            af.modifier  = 10;
            af.bitvector = 0;
            affect_join( victim, &af );
	}

	/* hunger! (warmth sucked out */
	if (!IS_NPC(victim))
	    gain_condition(victim,COND_HUNGER,dam/20);

	/* let's toast some gear */
	for (obj = victim->carrying; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    cold_effect(obj,level,dam,TARGET_OBJ);
	}
	return;
   }

   if (target == TARGET_OBJ) /* toast an object */
   {
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int chance;
	char *msg;

	if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF)
	||  IS_OBJ_STAT(obj,ITEM_NOPURGE)
        ||  IS_OBJ_STAT(obj,ITEM_QUEST)
	||  (number_range(0,10) < 9))
	    return;

	chance = level / 4 + dam / 10;

	if (chance > 25)
	    chance = (chance - 25) / 2 + 25;
	if (chance > 50)
	    chance = (chance - 50) / 2 + 50;

	if (IS_OBJ_STAT(obj,ITEM_BLESS))
	    chance -= 5;

 	chance -= obj->level * 2;

	switch(obj->item_type)
	{
	    default:
		return;
	    case ITEM_POTION:
		msg = "$p freezes and shatters!";
		chance += 25;
		break;
	    case ITEM_DRINK_CON:
		msg = "$p freezes and shatters!";
		chance += 5;
		break;
	}

	chance = URANGE(5,chance,95);

	if (number_percent() > chance)
	    return;

	if (obj->carried_by != NULL)
	    xact_new(msg,obj->carried_by,obj,NULL,TO_ALL,POS_RESTING,SUPPRESS_BURN);
	else if (obj->in_room != NULL && obj->in_room->people != NULL)
	    xact_new(msg,obj->in_room->people,obj,NULL,TO_ALL,POS_RESTING,SUPPRESS_BURN);

	extract_obj(obj);
	return;
    }
}



void fire_effect(void *vo, int level, int dam, int target)
{
    if (target == TARGET_ROOM)  /* nail objects on the floor */
    {
	ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
	OBJ_DATA *obj, *obj_next;

	for (obj = room->contents; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    fire_effect(obj,level,dam,TARGET_OBJ);
	}
	return;
    }

    if (target == TARGET_CHAR)   /* do the effect on a victim */
    {
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj, *obj_next;

	/* chance of blindness */
	if (!IS_AFFECTED(victim,AFF_BLIND)
	&&  !saves_spell(level + dam / 20, victim,DAM_FIRE))
	{
            AFFECT_DATA af;
            xact_new("$n is blinded by smoke!",victim,NULL,NULL,TO_ROOM,POS_RESTING,SUPPRESS_FLAG_EFFECTS);
            xact_new("Your eyes tear up from smoke...you can't see a thing!",
		victim,NULL,NULL,TO_CHAR,POS_RESTING,SUPPRESS_FLAG_EFFECTS);
	
            af.where        = TO_AFFECTS;
            af.type         = skill_lookup("fire breath");
            af.level        = level;
            af.duration     = /*number_range(0,level/10);*/ 1004;
            af.location     = APPLY_HITROLL;
            af.modifier     = -4;
            af.bitvector    = AFF_BLIND;

            affect_to_char(victim,&af);
	}

	/* getting thirsty */
	if (!IS_NPC(victim))
	    gain_condition(victim,COND_THIRST,dam/20);

	/* let's toast some gear! */
	for (obj = victim->carrying; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;

	    fire_effect(obj,level,dam,TARGET_OBJ);
        }
	return;
    }

    if (target == TARGET_OBJ)  /* toast an object */
    {
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	OBJ_DATA *t_obj,*n_obj;
	int chance;
	char *msg;

    	if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF)
        ||  IS_OBJ_STAT(obj,ITEM_NOPURGE)
        || IS_OBJ_STAT(obj,ITEM_QUEST)
	||  (number_range(0,10) < 9))
            return;

        chance = level / 4 + dam / 10;

        if (chance > 25)
            chance = (chance - 25) / 2 + 25;
        if (chance > 50)
            chance = (chance - 50) / 2 + 50;

        if (IS_OBJ_STAT(obj,ITEM_BLESS))
            chance -= 5;
        chance -= obj->level * 2;

        switch ( obj->item_type )
        {
        default:
	    return;
        case ITEM_CONTAINER:
        case ITEM_PIT:
            msg = "$p ignites and burns!";
            break;
        case ITEM_POTION:
            chance += 25;
            msg = "$p bubbles and boils!";
            break;
        case ITEM_SCROLL:
            chance += 50;
            msg = "$p crackles and burns!";
            break;
        case ITEM_STAFF:
            chance += 10;
            msg = "$p smokes and chars!";
            break;
        case ITEM_WAND:
            msg = "$p sparks and sputters!";
            break;
        case ITEM_FOOD:
            msg = "$p blackens and crisps!";
            break;
        case ITEM_PILL:
            msg = "$p melts and drips!";
            break;
        }

        chance = URANGE(5,chance,95);

        if (number_percent() > chance)
            return;

	if (obj->carried_by != NULL)
            xact_new( msg, obj->carried_by, obj, NULL, TO_ALL ,POS_RESTING,SUPPRESS_BURN);
	else if (obj->in_room != NULL && obj->in_room->people != NULL)
	    xact_new(msg,obj->in_room->people,obj,NULL,TO_ALL,POS_RESTING,SUPPRESS_BURN);

        if (obj->contains)
        {
            /* dump the contents */

            for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj)
            {
                n_obj = t_obj->next_content;
                obj_from_obj(t_obj);
		if (obj->in_room != NULL)
                    obj_to_room(t_obj,obj->in_room);
		else if (obj->carried_by != NULL)
		    obj_to_room(t_obj,obj->carried_by->in_room);
		else
		{
		    extract_obj(t_obj);
		    continue;
		}
		fire_effect(t_obj,level/2,dam/2,TARGET_OBJ);
            }
        }

        extract_obj( obj );
	return;
    }
}

void poison_effect(void *vo,int level, int dam, int target)
{
    if (target == TARGET_ROOM)  /* nail objects on the floor */
    {
        ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

        for (obj = room->contents; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;
            poison_effect(obj,level,dam,TARGET_OBJ);
        }
        return;
    }

    if (target == TARGET_CHAR)   /* do the effect on a victim */
    {
        CHAR_DATA *victim = (CHAR_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

	/* chance of poisoning */
        if (!saves_spell(level + dam / 20,victim,DAM_POISON))
        {
	    AFFECT_DATA af;

            xsend_to_char("{xYou feel {ypoison{x coursing through your veins.{x\n\r",
                victim,SUPPRESS_FLAG_EFFECTS);
            xact_new("$n looks very ill.",victim,NULL,NULL,TO_ROOM,POS_RESTING,SUPPRESS_FLAG_EFFECTS);

            af.where     = TO_AFFECTS;
            af.type      = gsn_poison;
            af.level     = level;
            af.duration  = 1004;
            af.location  = APPLY_STR;
            af.modifier  = -1;
            af.bitvector = AFF_POISON;
            affect_join( victim, &af );
        }

	/* equipment */
	for (obj = victim->carrying; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    poison_effect(obj,level,dam,TARGET_OBJ);
	}
	return;
    }

    if (target == TARGET_OBJ)  /* do some poisoning */
    {
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int chance;
	

	if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF)
  	||  IS_OBJ_STAT(obj,ITEM_BLESS)
        || IS_OBJ_STAT(obj,ITEM_QUEST)
	||  number_range(0,4) == 0)
	    return;

	chance = level / 4 + dam / 10;
	if (chance > 25)
	    chance = (chance - 25) / 2 + 25;
	if (chance > 50)
	    chance = (chance - 50) / 2 + 50;

	chance -= obj->level * 2;

	switch (obj->item_type)
	{
	    default:
		return;
	    case ITEM_FOOD:
		break;
	    case ITEM_DRINK_CON:
		if (obj->value[0] == obj->value[1])
		    return;
		break;
	}

	chance = URANGE(5,chance,95);

	if (number_percent() > chance)
	    return;

	obj->value[3] = 1;
	return;
    }
}


void shock_effect(void *vo,int level, int dam, int target)
{
    if (target == TARGET_ROOM)
    {
	ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
	OBJ_DATA *obj, *obj_next;

	for (obj = room->contents; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    shock_effect(obj,level,dam,TARGET_OBJ);
	}
	return;
    }

    if (target == TARGET_CHAR)
    {
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj, *obj_next;

	/* daze and confused? */
	if (!saves_spell(level + dam/20,victim,DAM_LIGHTNING))
	{
	    xsend_to_char("Your muscles stop responding.\n\r",victim,SUPPRESS_FLAG_EFFECTS);
	    DAZE_STATE(victim,UMAX(12,level/4 + dam/80));
	}

	/* toast some gear */
	for (obj = victim->carrying; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    shock_effect(obj,level,dam,TARGET_OBJ);
	}
	return;
    }

    if (target == TARGET_OBJ)
    {
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int chance;
	char *msg;

	if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF)
	||  IS_OBJ_STAT(obj,ITEM_NOPURGE)
        || IS_OBJ_STAT(obj,ITEM_QUEST)
	||  number_range(0,4) == 0)
	    return;

	chance = level / 4 + dam / 10;

	if (chance > 25)
	    chance = (chance - 25) / 2 + 25;
	if (chance > 50)
	    chance = (chance - 50) /2 + 50;

	if (IS_OBJ_STAT(obj,ITEM_BLESS))
	    chance -= 5;

 	chance -= obj->level * 2;

	switch(obj->item_type)
	{
	    default:
		return;
	   case ITEM_WAND:
	   case ITEM_STAFF:
		chance += 10;
		msg = "$p overloads and explodes!";
		break;
	   case ITEM_JEWELRY:
		chance -= 10;
		msg = "$p is fused into a worthless lump.";
	}
	
	chance = URANGE(5,chance,95);

	if (number_percent() > chance)
	    return;

	if (obj->carried_by != NULL)
	    xact_new(msg,obj->carried_by,obj,NULL,TO_ALL,POS_RESTING,SUPPRESS_BURN);
	else if (obj->in_room != NULL && obj->in_room->people != NULL)
	    xact_new(msg,obj->in_room->people,obj,NULL,TO_ALL,POS_RESTING,SUPPRESS_BURN);

	extract_obj(obj);
	return;
    }
}

void rotting_effect(void *vo,int level, int dam, int target)
{
    if (target == TARGET_ROOM)  /* nail objects on the floor */
    {
        ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

        for (obj = room->contents; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;
            rotting_effect(obj,level,dam,TARGET_OBJ);
        }
        return;
    }

    if (target == TARGET_CHAR)   /* do the effect on a victim */
    {
        CHAR_DATA *victim = (CHAR_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

	/* chance of infection with plague */
        if (!saves_spell(level + dam / 20,victim,DAM_DISEASE))
        {
	    AFFECT_DATA af;

            xsend_to_char("{xYou scream as {gplague sores erupt from your skin.{x\n\r",
                victim,SUPPRESS_FLAG_EFFECTS);
            xact_new("$n screams as {gplague sores erupt from $s skin.{x",victim,NULL,NULL,TO_ROOM,POS_RESTING,SUPPRESS_FLAG_EFFECTS);

            af.where     = TO_AFFECTS;
            af.type      = gsn_plague;
            af.level     = level;
            af.duration  = 1004;
            af.location  = APPLY_DEX;
            af.modifier  = -1;
            af.bitvector = AFF_PLAGUE;
            affect_join( victim, &af );
        }

	/* equipment */
	for (obj = victim->carrying; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    rotting_effect(obj,level,dam,TARGET_OBJ);
	}
	return;
    }

    if (target == TARGET_OBJ)
    {
	
	return;
    }
}

void light_effect(void *vo,int level, int dam, int target)
{
    if (target == TARGET_ROOM)  /* nail objects on the floor */
    {
        ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

        for (obj = room->contents; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;
            light_effect(obj,level,dam,TARGET_OBJ);
        }
        return;
    }

    if (target == TARGET_CHAR)   /* do the effect on a victim */
    {
        CHAR_DATA *victim = (CHAR_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

	/* chance of blindness */
        if (!saves_spell(level + dam / 20,victim,DAM_OTHER))
        {
	    AFFECT_DATA af;

            xsend_to_char("{xYou are blinded by {Yr{ya{Yy{ys{x of {Wlight{x!{x\n\r",
                victim,SUPPRESS_FLAG_EFFECTS);
            xact_new("$n is blinded by {Yr{ya{Yy{ys{x of {Wlight{x.{x",victim,NULL,NULL,TO_ROOM,POS_RESTING,SUPPRESS_FLAG_EFFECTS);

            af.where     = TO_AFFECTS;
            af.type      = gsn_blindness;
            af.level     = level;
            af.duration  = 1004;
            af.location  = APPLY_HITROLL;
            af.modifier  = -2;
            af.bitvector = AFF_BLIND;
            affect_join( victim, &af );
        }

	/* equipment */
	for (obj = victim->carrying; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    rotting_effect(obj,level,dam,TARGET_OBJ);
	}
	return;
    }

    if (target == TARGET_OBJ)
    {
	
	return;
    }
}

void drowning_effect(void *vo,int level, int dam, int target)
{
    if (target == TARGET_ROOM)  /* nail objects on the floor */
    {
        ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

        for (obj = room->contents; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;
            drowning_effect(obj,level,dam,TARGET_OBJ);
        }
        return;
    }

    if (target == TARGET_CHAR)   /* do the effect on a victim */
    {
        CHAR_DATA *victim = (CHAR_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

	/* chance of slowing */
        if (!saves_spell(level + dam / 20,victim,DAM_OTHER)
           && !IS_AFFECTED(victim,AFF_SLOW)
           && !IS_SET(victim->imm_flags,IMM_MAGIC))
        {
	    AFFECT_DATA af;

            if(IS_AFFECTED(victim,AFF_HASTE))
            {
             if(check_dispel(level,victim,skill_lookup("haste")))
              xact_new("$n is moving less quickly.",victim,NULL,NULL,TO_ROOM,POS_RESTING,SUPPRESS_FLAG_EFFECTS);
            }
            else
            {
             af.where     = TO_AFFECTS;
             af.type      = skill_lookup("slow");
             af.level     = level;
             af.duration  = 1004;
             af.location  = APPLY_DEX;
             af.modifier  = -2;
             af.bitvector = AFF_SLOW;
             affect_to_char( victim, &af );
             xsend_to_char("You feel yourself slowing d o w n...\n\r",victim,SUPPRESS_FLAG_EFFECTS);
             xact_new("$n starts to move in slow motion.",victim,NULL,NULL,TO_ROOM,POS_RESTING,SUPPRESS_FLAG_EFFECTS);
            }
        }

	/* equipment */
	for (obj = victim->carrying; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    drowning_effect(obj,level,dam,TARGET_OBJ);
	}
	return;
    }

    if (target == TARGET_OBJ)
    {
	
	return;
    }
}

void unholy_effect(void *vo,int level, int dam, int target)
{
    if (target == TARGET_ROOM)  /* nail objects on the floor */
    {
        ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

        for (obj = room->contents; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;
            unholy_effect(obj,level,dam,TARGET_OBJ);
        }
        return;
    }

    if (target == TARGET_CHAR)   /* do the effect on a victim */
    {
        CHAR_DATA *victim = (CHAR_DATA *) vo;
        OBJ_DATA *obj, *obj_next;

	/* chance of cursing */
        if (!saves_spell(level + dam / 20,victim,DAM_NEGATIVE))
        {
	    AFFECT_DATA af;

            xsend_to_char("{xYou feel unclean.\n\r",victim,SUPPRESS_FLAG_EFFECTS);
            xact_new("$n looks very uncomfortable.",victim,NULL,NULL,TO_ROOM,POS_RESTING,SUPPRESS_FLAG_EFFECTS);

            af.where     = TO_AFFECTS;
            af.type      = gsn_curse;
            af.level     = level;
            af.duration  = 1004;
            af.location  = APPLY_SAVING_SPELL;
            af.modifier  = 2;
            af.bitvector = AFF_CURSE;
            affect_join( victim, &af );
        }

	/* equipment */
	for (obj = victim->carrying; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    unholy_effect(obj,level,dam,TARGET_OBJ);
	}
	return;
    }

    if (target == TARGET_OBJ)
    {
	
	return;
    }
}

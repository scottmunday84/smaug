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
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"

/* command procedures needed */
DECLARE_DO_FUN(do_assassinate 	);
DECLARE_DO_FUN(do_backstab	);
DECLARE_DO_FUN(do_emote		);
DECLARE_DO_FUN(do_berserk	);
DECLARE_DO_FUN(do_bash		);
DECLARE_DO_FUN(do_trip		);
DECLARE_DO_FUN(do_earthbind	);
DECLARE_DO_FUN(do_dirt		);
DECLARE_DO_FUN(do_flee		);
DECLARE_DO_FUN(do_kick		);
DECLARE_DO_FUN(do_disarm	);
DECLARE_DO_FUN(do_get		);
DECLARE_DO_FUN(do_recall	);
DECLARE_DO_FUN(do_yell		);
DECLARE_DO_FUN(do_sacrifice	);
DECLARE_DO_FUN(do_circle	);
DECLARE_DO_FUN(do_feed		);
DECLARE_DO_FUN(do_gouge		);
DECLARE_DO_FUN(do_vdpi		);
DECLARE_DO_FUN(do_vdtr		);
DECLARE_DO_FUN(do_vdth		);
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_rub		);
DECLARE_DO_FUN(do_blackjack	);
DECLARE_DO_FUN(do_charge	);
DECLARE_DO_FUN(do_shield_smash	);
DECLARE_DO_FUN(do_strike	);
DECLARE_DO_FUN(do_whirlwind	);

DECLARE_SPELL_FUN(spell_null	);

/*
 * Local functions.
 */
void	check_assist	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_dodge	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_parry	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    check_shield_block     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    check_critical_hit args((CHAR_DATA *ch, CHAR_DATA *victim));
bool	check_phase	args((CHAR_DATA *ch,	CHAR_DATA *victim));
bool	check_force	args((CHAR_DATA *ch,	CHAR_DATA *victim));
bool	check_armor	args((CHAR_DATA *ch, CHAR_DATA *victim, int dam));
bool 	check_shield	args((CHAR_DATA *ch, CHAR_DATA *victim, int dam));
bool	check_stone_skin args((CHAR_DATA *ch, CHAR_DATA *victim, int dam));
void	counter_attack	args((CHAR_DATA *ch,	CHAR_DATA *victim));

void    dam_message 	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
                            int dt, bool immune, int suppress, bool singular ) );
void	death_cry	args( ( CHAR_DATA *ch ) );
void	group_gain	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
int	xp_compute	args( ( CHAR_DATA *gch, CHAR_DATA *victim, 
			    int total_levels, int members ) );
bool	is_safe		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	is_voodood	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	make_corpse	args( ( CHAR_DATA *ch, CHAR_DATA *killer, int lootpoint ) );
void	one_hit	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool secondary ) );
void    mob_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void	raw_kill	args( ( CHAR_DATA *victim, CHAR_DATA *killer, int lootpoint ) );
void	set_fighting	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	disarm		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    process_shields args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
int     monk_style      args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt, int dam ));
void	check_arena	args( ( CHAR_DATA *ch, CHAR_DATA *victim));
void	check_quest	args( ( CHAR_DATA *ch, CHAR_DATA *victim));
void	check_leech	args( ( CHAR_DATA *ch, CHAR_DATA *victim));
void	iquest_announce	args((const char *buf));

TEAM_DATA *team_list = NULL;

/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */
void violence_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *victim;

    for ( ch = char_list; ch != NULL; ch = ch->next )
    {
	AFFECT_DATA *paf;
        AFFECT_DATA *paf_next;

        ch_next	= ch->next;
        
        for( paf = ch->affected; paf!=NULL; paf=paf_next)
        {
          paf_next = paf->next;
          if( paf->duration>1000)
           paf->duration--;
          else if( paf->duration<1000 )
               ;
          else
          {
           if(paf_next==NULL
            || paf_next->type != paf->type
            || ((paf_next->duration > 0)&&(paf_next->duration!=1000)))
           {
            if( paf->type > 0 && skill_table[paf->type].msg_off)
            {
              send_to_char(skill_table[paf->type].msg_off,ch);
              send_to_char("\n\r",ch);
            }
           }
          paf->duration=0;
          affect_remove(ch,paf);
        } 
      }              
        if(ch->fighting==NULL && !IS_NPC(ch) && (ch->pcdata->pkflag>0))
        {
         if(--ch->pcdata->pkflag==0)
          send_to_char("Your heart stops pounding.\n\r",ch);
        }
         
         
	if ( ( victim = ch->fighting ) == NULL || ch->in_room == NULL )
	    continue;

	if ( IS_AWAKE(ch) && ch->in_room == victim->in_room )
        {
            multi_hit( ch, victim, TYPE_UNDEFINED );
            process_shields(ch, victim);
        }
	else
	    stop_fighting( ch, FALSE );

	if ( ( victim = ch->fighting ) == NULL )
	    continue;

	/*
	 * Fun for the whole family!
	 */
	check_assist(ch,victim);
    
	if ( IS_NPC( ch ) )
	{
	    if ( HAS_TRIGGER( ch, TRIG_FIGHT ) )
		mp_percent_trigger( ch, victim, NULL, NULL, TRIG_FIGHT );
	    if ( HAS_TRIGGER( ch, TRIG_HPCNT ) )
		mp_hprct_trigger( ch, victim );
	}
    }

    return;
}

/* for auto assisting */
void check_assist(CHAR_DATA *ch,CHAR_DATA *victim)
{
    CHAR_DATA *rch, *rch_next;

    for (rch = ch->in_room->people; rch != NULL; rch = rch_next)
    {
	rch_next = rch->next_in_room;
	
	if (IS_AWAKE(rch) && rch->fighting == NULL)
	{

	    /* quick check for ASSIST_PLAYER */
	    if (!IS_NPC(ch) && IS_NPC(rch) 
	    && IS_SET(rch->off_flags,ASSIST_PLAYERS)
	    &&  rch->level + 6 > victim->level)
	    {
		do_emote(rch,"{Rscreams and attacks!{x");
		multi_hit(rch,victim,TYPE_UNDEFINED);
		continue;
	    }

	    /* PCs next */
	    if (!IS_NPC(ch) || IS_AFFECTED(ch,AFF_CHARM))
	    {
		if ( ( (!IS_NPC(rch) && IS_SET(rch->act,PLR_AUTOASSIST))
		||     IS_AFFECTED(rch,AFF_CHARM)) 
		&&   is_same_group(ch,rch) 
		&&   !is_safe_spell(rch, victim,FALSE))
		    multi_hit (rch,victim,TYPE_UNDEFINED);
		
		continue;
	    }
  	
	    /* now check the NPC cases */
	    
 	    if (IS_NPC(ch) && !IS_AFFECTED(ch,AFF_CHARM))
	
	    {
		if ( (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALL))

		||   (IS_NPC(rch) && rch->group && rch->group == ch->group)

		||   (IS_NPC(rch) && rch->race == ch->race 
		   && IS_SET(rch->off_flags,ASSIST_RACE))

		||   (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALIGN)
		   &&   ((IS_GOOD(rch)    && IS_GOOD(ch))
		     ||  (IS_EVIL(rch)    && IS_EVIL(ch))
		     ||  (IS_NEUTRAL(rch) && IS_NEUTRAL(ch)))) 

		||   (rch->pIndexData == ch->pIndexData 
		   && IS_SET(rch->off_flags,ASSIST_VNUM)))

	   	{
		    CHAR_DATA *vch;
		    CHAR_DATA *target;
		    int number;

		    if (number_bits(1) == 0)
			continue;
		
		    target = NULL;
		    number = 0;
		    for (vch = ch->in_room->people; vch; vch = vch->next)
		    {
			if (can_see(rch,vch)
			&&  is_same_group(vch,victim)
			&&  number_range(0,number) == 0)
			{
			    target = vch;
			    number++;
			}
		    }

		    if (target != NULL)
		    {
			do_emote(rch,"{Rscreams and attacks!{x");
			multi_hit(rch,target,TYPE_UNDEFINED);
		    }
		}	
	    }
	}
    }
}


/*
 * Do one group of attacks.
 */
void multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    int     chance;
    int lastfailed=0;

    /* decrement the wait */
    if (ch->desc == NULL)
	ch->wait = UMAX(0,ch->wait - PULSE_VIOLENCE);

    if (ch->desc == NULL)
	ch->daze = UMAX(0,ch->daze - PULSE_VIOLENCE); 


    /* no attacks for stunnies -- just a check */
    if (ch->position < POS_RESTING)
	return;

    if (ch->stunned)
    {
	ch->stunned--;
	if (!ch->stunned)
	{
	    send_to_char( "You regain your equilibrium.\n\r", ch);
	    act( "$n regains $s equilibrium.", ch, NULL, NULL, TO_ROOM );
	}
        return;
    }

    if (IS_NPC(ch))
    {
	mob_hit(ch,victim,dt);
	return;
    }

    one_hit( ch, victim, dt, FALSE );
    
    if(ch->fighting != victim)
     return;
   
    if (get_eq_char (ch, WEAR_SECONDARY) && (dt!=gsn_charge))
    {
	chance = (get_skill(ch,gsn_dual_wield)*2)/3;
        if(IS_AFFECTED(ch, AFF_SLOW))
         chance/=2;
	if ( number_percent( ) < chance )
	{
	    one_hit( ch, victim, dt, TRUE );
	    if ( get_skill(ch,gsn_dual_wield) != 0 && (!IS_NPC(ch)
	    && ch->level >= skill_table[gsn_dual_wield].skill_level[ch->class]))
	    {
		check_improve(ch,gsn_dual_wield,TRUE,6);
	    }
	} else {
          lastfailed=1;
        }
        if((dt!=gsn_backstab)&&(dt!=gsn_circle)&&(dt!=gsn_assassinate)
              &&(dt!=gsn_strike))
        {
         if ( ch->fighting != victim )
             return;
         chance=(get_skill(ch,gsn_2nd_dual)*2)/3;
         if(IS_AFFECTED(ch, AFF_SLOW))
          chance/=3;
         if(lastfailed)
         {
          chance/=2;
          lastfailed=0;
         }
         if(number_percent()<chance)
         { 
          one_hit(ch, victim, dt, TRUE);
          check_improve(ch,gsn_2nd_dual,TRUE,6);
          if(ch->fighting!=victim)   
           return;
         } else {
          lastfailed=1;
         }
        chance=(get_skill(ch,gsn_3rd_dual)*2)/3;
        if(IS_AFFECTED(ch,AFF_SLOW))
          chance/=4;
        if(lastfailed)
        {
         chance/=2;
         lastfailed=0;
        }
        if(number_percent()<chance)
        {
         one_hit(ch, victim, dt, TRUE);
         check_improve(ch,gsn_3rd_dual,TRUE,6);
         if(ch->fighting!=victim)
          return;
        } else {
         lastfailed=1;
        }
        chance=(get_skill(ch,gsn_4th_dual)*2)/3;
        if(IS_AFFECTED(ch,AFF_SLOW))
         chance/=5;
        if(lastfailed)
        {
         chance/=2;
         lastfailed=0;
        }
        if(number_percent()<chance)
        {
         one_hit(ch,victim,dt,TRUE);
         check_improve(ch,gsn_4th_dual,TRUE,6);
         if(ch->fighting!=victim)
          return;
        } else {
         lastfailed=1;
        }
      }
    } /* end dual attacks */

    if (ch->fighting != victim)
	return;

    if(dt==gsn_strike)
     return;

    if(IS_AFFECTED(ch,AFF_HASTE))
      one_hit(ch,victim,dt,FALSE);

    if(ch->fighting != victim)
     return;

    if(dt==gsn_backstab || dt==gsn_circle || dt==gsn_assassinate)
    {
     bool success=FALSE;
     chance=get_skill(ch,gsn_offhand);
     if(!chance) return;
     if(IS_AFFECTED(ch,AFF_SLOW))
      chance/=2;
     chance=(chance*3)/4 + (ch->level-victim->level);
     if(number_percent()<chance){
      one_hit(ch,victim,dt,FALSE); success=TRUE; }
     if((number_percent()<chance) && (ch->fighting==victim)
       && get_eq_char(ch,WEAR_SECONDARY)){
      one_hit(ch,victim,dt,TRUE); success=TRUE; }
     if(success)
      check_improve(ch,gsn_offhand,TRUE,5); 
    }	

    if ( ch->fighting != victim || dt == gsn_backstab || dt == gsn_circle || dt == gsn_assassinate)
	return;

    chance = (get_skill(ch,gsn_second_attack)*2)/3;

    if (IS_AFFECTED(ch,AFF_SLOW))
	chance /= 2;

    if ( number_percent( ) < chance )
    {
	one_hit( ch, victim, dt, FALSE );
	check_improve(ch,gsn_second_attack,TRUE,5);
	if ( ch->fighting != victim )
	    return;
    } else {
	lastfailed=1;
    }

    chance = (get_skill(ch,gsn_third_attack)*2)/3;

    if (IS_AFFECTED(ch,AFF_SLOW))
	chance /= 2;
    if (lastfailed)
    {
     chance /= 2;
     lastfailed=0;
    }
    
    if ( number_percent( ) < chance )
    {
	one_hit( ch, victim, dt, FALSE );
	check_improve(ch,gsn_third_attack,TRUE,6);
	if ( ch->fighting != victim )
	    return;
    } else {
	lastfailed=1;
    }

    chance = (get_skill(ch, gsn_fourth_attack)*2)/3;

    if (IS_AFFECTED(ch, AFF_SLOW))
	chance /= 3;

    if (lastfailed)
    {
       chance /= 2;
       lastfailed=0;
    }
    if (number_percent( ) < chance)
    {
	one_hit( ch, victim, dt, FALSE );
	check_improve(ch, gsn_fourth_attack, TRUE, 6);
	if (ch->fighting != victim)
	    return;
    } else {
	lastfailed=1;
    }

    chance = (get_skill(ch, gsn_fifth_attack)*2)/3;

    if (IS_AFFECTED(ch, AFF_SLOW))
	chance /= 4 ;
    
    if (lastfailed)
    {
      chance /= 2;
      lastfailed=0;
    }
    if (number_percent( ) < chance)
    {
	one_hit( ch, victim, dt, FALSE );
	check_improve(ch, gsn_fifth_attack, TRUE, 6);
	if (ch->fighting != victim)
	    return;
    } else {
       lastfailed=1;
    }
    
    chance = (get_skill(ch, gsn_sixth_attack)*2)/3;

    if (IS_AFFECTED(ch, AFF_SLOW))
       chance /= 5;

    if (lastfailed)
    {
     chance /= 2;
     lastfailed=0;
    }
    if(number_percent() < chance)
    {
     one_hit(ch, victim, dt, FALSE);
     check_improve(ch, gsn_sixth_attack, TRUE, 6);
     if(ch->fighting != victim)
      return;
    } else {
      lastfailed=1;
    }
   
    chance = (get_skill(ch, gsn_seventh_attack)*2)/3;
    
    if(IS_AFFECTED(ch, AFF_SLOW))
       chance/=6;
    
    if(lastfailed)
    {
     chance /= 2;
     lastfailed=0;
    }
    if(number_percent() < chance)
    {
     one_hit(ch, victim, dt, FALSE);
     check_improve(ch, gsn_seventh_attack, TRUE, 6);
     if(ch->fighting != victim)
      return;
    } else {
      lastfailed=1;
    }
    if(dt==gsn_ambush)
     return;
    check_leech(ch,victim);
    counter_attack(ch,victim);
    return;
 
}

/* procedure for all mobile attacks */
void mob_hit (CHAR_DATA *ch, CHAR_DATA *victim, int dt)
{
    int chance,number;
    int lastfailed=0;
    CHAR_DATA *vch, *vch_next;

    one_hit(ch,victim,dt, FALSE);

    if (ch->fighting != victim)
	return;

    if (ch->stunned)
	return;

    /* Area attack -- BALLS nasty! */
 
    if (IS_SET(ch->off_flags,OFF_AREA_ATTACK))
    {
	for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
	{
	    vch_next = vch->next;
	    if ((vch != victim && vch->fighting == ch))
		one_hit(ch,vch,dt, FALSE);
	}
    }

    if (ch->fighting != victim)
	return;

    if (get_eq_char (ch, WEAR_SECONDARY) && (dt!=gsn_charge))
    {
	chance = (get_skill(ch,gsn_dual_wield)*2)/3;
        if(IS_AFFECTED(ch, AFF_SLOW)&& !IS_SET(ch->off_flags,OFF_FAST))
         chance/=2;
	if ( number_percent( ) < chance )
	{
	    one_hit( ch, victim, dt, TRUE );
	
	} else {
          lastfailed=1;
        }
        if((dt!=gsn_backstab)&&(dt!=gsn_circle)&&(dt!=gsn_assassinate)
          &&(dt!=gsn_strike))
        {
         if ( ch->fighting != victim )
             return;
         chance=(get_skill(ch,gsn_2nd_dual)*2)/3;
         if(IS_AFFECTED(ch, AFF_SLOW)&& !IS_SET(ch->off_flags,OFF_FAST))
          chance/=3;
         if(lastfailed)
         {
          chance/=2;
          lastfailed=0;
         }
         if(number_percent()<chance)
         {
          one_hit(ch, victim, dt, TRUE);
          if(ch->fighting!=victim)
           return;
         } else {
          lastfailed=1;
         }
        chance=(get_skill(ch,gsn_3rd_dual)*2)/3;
        if(IS_AFFECTED(ch,AFF_SLOW)&& !IS_SET(ch->off_flags,OFF_FAST))
         chance/=4;
        if(lastfailed)
        {
         chance/=2;
         lastfailed=0;
        }
        if(number_percent()<chance)
        {
         one_hit(ch, victim, dt, TRUE);
         if(ch->fighting!=victim)
          return;
        } else {
         lastfailed=1;
        }
        chance=(get_skill(ch,gsn_4th_dual)*2)/3;
        if(IS_AFFECTED(ch,AFF_SLOW)&& !IS_SET(ch->off_flags,OFF_FAST))
         chance/=5;
        if(lastfailed)
        {
         chance/=2;
         lastfailed=0;
        }
        if(number_percent()<chance)
        {
         one_hit(ch,victim,dt,TRUE);
         if(ch->fighting!=victim)
          return;
        } else {
         lastfailed=1;
        }
     }
    }

    if(dt==gsn_strike)
     return;

    if (IS_AFFECTED(ch,AFF_HASTE)
    ||  (IS_SET(ch->off_flags,OFF_FAST) && !IS_AFFECTED(ch,AFF_SLOW)))
	one_hit(ch,victim,dt, FALSE);

    if (ch->fighting != victim)
	return;

    if(dt==gsn_backstab || dt==gsn_circle || dt==gsn_assassinate)
    {
     chance=get_skill(ch,gsn_offhand);
     if(!chance) return;
     if(IS_AFFECTED(ch,AFF_SLOW)&& !IS_AFFECTED(ch,AFF_SLOW) )
      chance/=2;
     chance=(chance*3)/4 + (ch->level-victim->level);
     if(number_percent()<chance)
      one_hit(ch,victim,dt,FALSE);
     if((number_percent()<chance) && (ch->fighting==victim) &&
       get_eq_char(ch,WEAR_SECONDARY))
      one_hit(ch,victim,dt,TRUE);
     }

    if (ch->fighting != victim || dt == gsn_backstab || dt == gsn_circle || dt==gsn_assassinate)
	return;
    chance = (get_skill(ch,gsn_second_attack)*2)/3;

    if (IS_AFFECTED(ch,AFF_SLOW)&& !IS_AFFECTED(ch,AFF_SLOW))
	chance /= 2;

    if ( number_percent( ) < chance )
    {
	one_hit( ch, victim, dt, FALSE );
	if ( ch->fighting != victim )
	    return;
    } else {
	lastfailed=1;
    }

    chance = (get_skill(ch,gsn_third_attack)*2)/3;

    if (IS_AFFECTED(ch,AFF_SLOW)&& !IS_AFFECTED(ch,AFF_SLOW))
	chance /= 2;
    if (lastfailed)
    {
     chance /= 2;
     lastfailed=0;
    }

    if ( number_percent( ) < chance )
    {
	one_hit( ch, victim, dt, FALSE );
	if ( ch->fighting != victim )
	    return;
    } else {
	lastfailed=1;
    }

    chance = (get_skill(ch, gsn_fourth_attack)*2)/3;

    if (IS_AFFECTED(ch, AFF_SLOW)&& !IS_AFFECTED(ch,AFF_SLOW))
	chance /= 3;

    if (lastfailed)
    {
       chance /= 2;
       lastfailed=0;
    }
    if (number_percent( ) < chance)
    {
	one_hit( ch, victim, dt, FALSE );
	if (ch->fighting != victim)
	    return;
    } else {
	lastfailed=1;
    }

    chance = (get_skill(ch, gsn_fifth_attack)*2)/3;

    if (IS_AFFECTED(ch, AFF_SLOW)&& !IS_AFFECTED(ch,AFF_SLOW))
	chance /= 4 ;

    if (lastfailed)
    {
      chance /= 2;
      lastfailed=0;
    }
    if (number_percent( ) < chance)
    {
	one_hit( ch, victim, dt, FALSE );
	if (ch->fighting != victim)
	    return;
    } else {
       lastfailed=1;
    }

    chance = (get_skill(ch, gsn_sixth_attack)*2)/3;

    if (IS_AFFECTED(ch, AFF_SLOW)&& !IS_AFFECTED(ch,AFF_SLOW))
       chance /= 5;

    if (lastfailed)
    {
     chance /= 2;
     lastfailed=0;
    }
    if(number_percent() < chance)
    {
     one_hit(ch, victim, dt, FALSE);
     if(ch->fighting != victim)
      return;
    } else {
      lastfailed=1;
    }

    chance = (get_skill(ch, gsn_seventh_attack)*2)/3;

    if(IS_AFFECTED(ch, AFF_SLOW)&& !IS_AFFECTED(ch,AFF_SLOW))
       chance/=6;

    if(lastfailed)
    {
     chance /= 2;
     lastfailed=0;
    }
    if(number_percent() < chance)
    {
     one_hit(ch, victim, dt, FALSE);
     if(ch->fighting != victim)
      return;
    } else {
      lastfailed=1;
    }
    check_leech(ch,victim);
    counter_attack(ch,victim);
    /* oh boy!  Fun stuff! */

    if (ch->wait > 0)
	return;

    number = number_range(0,2);

    if (number == 1 && IS_SET(ch->act,ACT_MAGE))
    {
	/*  { mob_cast_mage(ch,victim); return; } */ ;
    }

    if (number == 2 && IS_SET(ch->act,ACT_CLERIC))
    {	
	/* { mob_cast_cleric(ch,victim); return; } */ ;
    }

    /* now for the skills */

    number = number_range(0,8);

    switch(number) 
    {
    case (0) :
	if (IS_SET(ch->off_flags,OFF_BASH))
	    do_bash(ch,"");
	break;

    case (1) :
	if (IS_SET(ch->off_flags,OFF_BERSERK) && !IS_AFFECTED(ch,AFF_BERSERK))
	    do_berserk(ch,"");
	break;


    case (2) :
	if (IS_SET(ch->off_flags,OFF_DISARM) 
	|| (get_weapon_sn(ch) != gsn_hand_to_hand 
	&& (IS_SET(ch->act,ACT_WARRIOR)
	||  IS_SET(ch->act,ACT_VAMPIRE)
   	||  IS_SET(ch->act,ACT_THIEF))))
	    do_disarm(ch,"");
	break;

    case (3) :
	if (IS_SET(ch->off_flags,OFF_KICK))
	    do_kick(ch,"");
	break;

    case (4) :
	if (IS_SET(ch->off_flags,OFF_KICK_DIRT))
	    do_dirt(ch,"");
	break;

    case (5) :
	if (IS_SET(ch->off_flags,OFF_CIRCLE))
           do_circle(ch,"");
        break; 

    case (6) :
	if (IS_SET(ch->off_flags,OFF_TRIP))
	    do_trip(ch,"");
	break;

    case (7) :
	if (IS_SET(ch->off_flags,OFF_FEED))
           do_feed(ch,"");
        break;
    case (8) :
	if (IS_SET(ch->off_flags,OFF_BACKSTAB))
	{
	    do_backstab(ch,"");
	}
    }
}
	

/*
 * Hit one guy once.
 */
void one_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool secondary )
{
    OBJ_DATA *wield;
    int victim_ac;
    int thac0;
    int thac0_00;
    int thac0_32;
    int dam;
    int diceroll;
    int sn,skill;
    int dam_type;
    int result;

    sn = -1;


    /* just in case */
    if (victim == ch || ch == NULL || victim == NULL)
	return;

    /*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     */
    if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return;

    /*
     * Figure out the type of damage message.
     * if secondary == true, use the second weapon.
     */
    if (!secondary)
        wield = get_eq_char( ch, WEAR_WIELD );
    else
        wield = get_eq_char( ch, WEAR_SECONDARY );
    if ( dt == TYPE_UNDEFINED )
    {
	dt = TYPE_HIT;
	if ( wield != NULL && wield->item_type == ITEM_WEAPON )
	    dt += wield->value[3];
        else if((ch->class == class_lookup("sensai")) ||
                 (ch->class == class_lookup("shaolin")))
        {
         int chance;
         int range=0;
         chance = get_skill(ch, gsn_tiger_style) * 3/4;
         if(chance < number_percent())
          dt = TYPE_HIT + 17; /* punch */
         else
         {
          range+=5;
          check_improve(ch, gsn_tiger_style, TRUE, 6);
          chance = get_skill(ch, gsn_dragon_style) *3/4;
          if(chance > number_percent())
          {
           range+=5;
           check_improve(ch, gsn_dragon_style, TRUE, 6);
           chance = get_skill(ch, gsn_chaos_style) *3/4;
           if(chance > number_percent())
           {
            range+=5;
            check_improve(ch, gsn_chaos_style, TRUE, 6);
            chance = get_skill(ch, gsn_elemental_style) *3/4;
            if(chance > number_percent())
            {
             range+=5;
             check_improve(ch, gsn_elemental_style, TRUE, 6);
            }
           }
          }
          dt = TYPE_HIT + 40 + number_range(1,range);
         }
        }
	else 
	    dt += ch->dam_type;
    }

    if (dt < TYPE_HIT)
    	if (wield != NULL)
    	    dam_type = attack_table[wield->value[3]].damage;
    	else
    	    dam_type = attack_table[ch->dam_type].damage;
    else
    	dam_type = attack_table[dt - TYPE_HIT].damage;

    if (dam_type == -1)
	dam_type = DAM_BASH;

    /* get the weapon skill */
    sn = get_weapon_sn(ch);
    skill = 20 + get_weapon_skill(ch,sn);

    /*
     * Calculate to-hit-armor-class-0 versus armor.
     */
    if ( IS_NPC(ch) )
    {
	thac0_00 = 20;
	thac0_32 = -4;   /* as good as a thief */ 
	if (IS_SET(ch->act,ACT_VAMPIRE))
	    thac0_32 = -30;
        else if (IS_SET(ch->act,ACT_DRUID))
            thac0_32 = 0;
        else if (IS_SET(ch->act,ACT_RANGER))
            thac0_32 = -4;
	else if (IS_SET(ch->act,ACT_WARRIOR))
	    thac0_32 = -10;
	else if (IS_SET(ch->act,ACT_THIEF))
	    thac0_32 = -4;
	else if (IS_SET(ch->act,ACT_CLERIC))
	    thac0_32 = 2;
	else if (IS_SET(ch->act,ACT_MAGE))
	    thac0_32 = 6;
    }
    else
    {
	thac0_00 = class_table[ch->class].thac0_00;
	thac0_32 = class_table[ch->class].thac0_32;
    }
    thac0  = interpolate( ch->level, thac0_00, thac0_32 );

    if (thac0 < 0)
        thac0 = thac0/2;

    if (thac0 < -5)
        thac0 = -5 + (thac0 + 5) / 2;

    thac0 -= GET_HITROLL(ch)/10 * skill/100;
    thac0 += 5 * (100 - skill) / 100;

    if (dt == gsn_backstab)
	thac0 -= 10 * (100 - get_skill(ch,gsn_backstab));

    if (dt == gsn_assassinate)
        thac0 -= 10 * (100 - get_skill(ch,gsn_assassinate));

    if (dt == gsn_circle)
	thac0 -= 10 * (100 - get_skill(ch,gsn_circle));

    if (dt == gsn_charge)
        thac0 -= 10 * (100 - get_skill(ch,gsn_charge));

    if (dt == gsn_strike)
        thac0 -= 10 * (100 - get_skill(ch,gsn_strike));

    if (dt == gsn_ambush)
        thac0 -= 10 * (100 - get_skill(ch,gsn_ambush));


    switch(dam_type)
    {
	case(DAM_PIERCE):victim_ac = GET_AC(victim,AC_PIERCE)/10;	break;
	case(DAM_BASH):	 victim_ac = GET_AC(victim,AC_BASH)/10;		break;
	case(DAM_SLASH): victim_ac = GET_AC(victim,AC_SLASH)/10;	break;
	default:	 victim_ac = GET_AC(victim,AC_EXOTIC)/10;	break;
    }; 
	
    if (victim_ac < -15)
	victim_ac = (victim_ac + 15) / 5 - 15;
     
    if ( !can_see( ch, victim ) )
	victim_ac -= 4;

    if ( victim->position < POS_FIGHTING)
	victim_ac += 4;
 
    if (victim->position < POS_RESTING)
	victim_ac += 6;

    /*
     * The moment of excitement!
     */
    while ( ( diceroll = number_bits( 5 ) ) >= 20 )
	;

    if ( diceroll == 0
    || ( diceroll != 19 && diceroll < thac0 - victim_ac ) )
    {
	/* Miss. */
	damage( ch, victim, 0, dt, dam_type, TRUE );
	tail_chain( );
	return;
    }

    /*
     * Hit.
     * Calc damage.
     */
    if ( IS_NPC(ch) && (!ch->pIndexData->new_format || wield == NULL))
	if (!ch->pIndexData->new_format)
	{
	    dam = number_range( ch->level / 2, ch->level * 3 / 2 );
	    if ( wield != NULL )
	    	dam += dam / 2;
	}
	else
	    dam = dice(ch->damage[DICE_NUMBER],ch->damage[DICE_TYPE]);
	
    else
    {
	if (sn != -1)
	    check_improve(ch,sn,TRUE,5);
	if ( wield != NULL )
	{
	    if (wield->clan)
	    {
		dam = dice(ch->level/3,3) * skill/100;
	    } else
	    {
		if (wield->pIndexData->new_format)
		    dam = dice(wield->value[1],wield->value[2]) * skill/100;
		else
	    	    dam = number_range( wield->value[1] * skill/100, 
				wield->value[2] * skill/100);
	    }

	    if (get_eq_char(ch,WEAR_SHIELD) == NULL)  /* no shield = more */
		dam = dam * 11/10;

	    /* sharpness! */
	    if (IS_WEAPON_STAT(wield,WEAPON_SHARP) || IS_WEAPON_STAT(wield,WEAPON_VORPAL))
	    {
		int percent;

		if ((percent = number_percent()) <= (skill / 8))
		    dam = /*2 **/ /*((dam*3)/2)*/ dam + (dam * 2 * percent / 100);
	    }
	}
	else
	    dam = number_range( 1 + 4 * skill/100, 2 * ch->level/3 * skill/100);
    }

    /*
     * Bonuses.
     */
    if ( get_skill(ch,gsn_enhanced_damage) > 0 )
    {
        diceroll = number_percent();
        if (diceroll <= get_skill(ch,gsn_enhanced_damage))
        {
            check_improve(ch,gsn_enhanced_damage,TRUE,6);
            dam += 2 * ( dam * diceroll/300);
        }
    }
    if ( !IS_AWAKE(victim) )
	dam *= 2;
     else if (victim->position < POS_FIGHTING)
	dam = dam * 3 / 2;

    if ( dt == gsn_backstab && wield != NULL) 
    	if ( wield->value[0] != 2 )
	    dam *=  (ch->level / 25); /* max * 4 */
	else 
	    dam *=  (ch->level / 20); /* max * 5 */

    if ( dt == gsn_circle && wield != NULL) 
    	if ( wield->value[0] != 2 )
	    dam *=  (ch->level / 33); /* max * 3 */
	else 
	    dam *=  (ch->level / 25); /* max * 4 */

    if ( dt == gsn_assassinate && wield != NULL)
       if ( wield->value[0] != 2 )
          dam *= (ch->level / 20); /* max * 5 */
       else
          dam *= (ch->level / 16); /* max * 6 */

    if( dt == gsn_charge && wield != NULL)
      dam*=2;
    if( dt == gsn_whirlwind && wield != NULL)
      dam*=3/2;

    if( dt == gsn_strike && wield != NULL)
      dam *= (ch->level / 33); /* max * 3 */

    if( dt == gsn_ambush && wield != NULL)
      dam *= 2 + (ch->level/50); /* max * 4 */

    dam += GET_DAMROLL(ch)/11 * UMIN(100,skill) /100;

    if ( dam <= 0 )
	dam = 1;

    result = damage( ch, victim, dam, dt, dam_type, TRUE );

    if(result && ((class_lookup("sensai")==ch->class)||
       (class_lookup("shaolin")==ch->class)) && !IS_NPC(ch) )
    {
     switch(ch->pcdata->stance)
     {
      case STANCE_NONE: break;
      case STANCE_FLAMING:
         if (ch->fighting == victim && !check_force(ch,victim))
         {
	    dam = number_range(1,ch->level / 4 + 1);
            xact_new("{BYou {rburn{B $N.{x",ch,NULL,victim,TO_CHAR,POS_RESTING, SUPPRESS_FLAGS);
            xact_new("{x$n {rburns{x $N.",ch,NULL,victim,TO_NOTVICT,POS_RESTING,SUPPRESS_FLAGS);
            xact_new("{c$n {rburns{c you.{x",ch,NULL,victim,TO_VICT,POS_RESTING,SUPPRESS_FLAGS);
	    fire_effect( (void *) victim,ch->level/2,dam,TARGET_CHAR);
	    damage(ch,victim,dam,0,DAM_FIRE,FALSE);
         }
         break;
       case STANCE_VAMPIRIC:
         if (ch->fighting == victim && !check_force(ch,victim))
	{
	    dam = number_range(1, ch->level / 5 + 1);
            xact_new("{BYou draw life from $N.{x",ch,NULL,victim,TO_CHAR,POS_RESTING, SUPPRESS_FLAGS);
            xact_new("{x$n draws life from $N.",ch,NULL,victim,TO_NOTVICT,POS_RESTING, SUPPRESS_FLAGS);
            xact_new("{cYou feel $n drawing your life away.{x",ch,NULL,victim,TO_VICT,POS_RESTING, SUPPRESS_FLAGS);
	    damage_old(ch,victim,dam,0,DAM_NEGATIVE,FALSE);
	    ch->alignment = UMAX(-1000,ch->alignment - 1);
	    if ( ch->pet != NULL )
		ch->pet->alignment = ch->alignment;
	    ch->hit += dam/2;
	}
        break;
       case STANCE_FROST:
           if (ch->fighting == victim && !check_force(ch,victim))
	{
	    dam = number_range(1,ch->level / 6 + 2);
            xact_new("{BYou {cfreeze{B $N.{x",ch,NULL,victim,TO_CHAR,POS_RESTING, SUPPRESS_FLAGS);
            xact_new("{x$n {cfreezes{x $N.",ch,NULL,victim,TO_NOTVICT,POS_RESTING, SUPPRESS_FLAGS);
            xact_new("{c$n {cfreezes{c you.{x",ch,NULL,victim,TO_VICT,POS_RESTING, SUPPRESS_FLAGS);
	    cold_effect(victim,ch->level/2,dam,TARGET_CHAR);
	    damage(ch,victim,dam,0,DAM_COLD,FALSE);
	}
        break;
       case STANCE_SHOCKING:
            if (ch->fighting == victim && !check_force(ch,victim))
	{
	    dam = number_range(1,ch->level/5 + 2);
            xact_new("{BYou {Yshock{B $N.{x",ch,NULL,victim,TO_CHAR,POS_RESTING, SUPPRESS_FLAGS);
            xact_new("{x$n {Yshocks{x $N.",ch,NULL,victim,TO_NOTVICT,POS_RESTING, SUPPRESS_FLAGS);
            xact_new("{c$n {Yshocks{c you.{x",ch,NULL,victim,TO_VICT,POS_RESTING, SUPPRESS_FLAGS);
	    shock_effect(victim,ch->level/2,dam,TARGET_CHAR);
	    damage(ch,victim,dam,0,DAM_LIGHTNING,FALSE);
	}
        break;
       case STANCE_POISON:
           if(ch->fighting == victim && !check_force(ch,victim))
           {
            poison_effect((void*)victim,ch->level/2,dam,TARGET_CHAR);
           }
            break;
       default: break;
      }/* switch ch->pcdata->stance */
     }

    /* but do we have a funky weapon? */
    if (result && wield != NULL)
    { 
	int dam=0;
        AFFECT_DATA *pAf,af;

        if (ch->fighting == victim && ((pAf=affect_find(wield->affected,
            gsn_blade_poison))!=NULL))
        {
         if(pAf->dealer==ch)
         {
          if(!is_affected(victim,gsn_blade_poison)
                && !saves_spell(pAf->level,victim,DAM_POISON))
          {
           af.where = TO_AFFECTS;
           af.type = gsn_blade_poison;
           af.level = pAf->level;
           af.location = APPLY_NONE;
           af.duration = 1001;
           af.modifier = 0;
           af.bitvector = 0;
           af.subtype = 0;
           switch(number_range(0,2))
           {
            case 0: af.duration=1004+pAf->level/33;
                    af.location = APPLY_HITROLL;
                    af.modifier = -pAf->level;
                    af.bitvector = AFF_BLIND;
                    act("{G$n{G is poisoned and stumbles around blindly.{x",
                      victim,NULL,NULL,TO_ROOM);
                    send_to_char("{GYou are poisoned and stumble around blindly.{x\n\r",victim);
                    break;
            case 1: af.duration=1010+pAf->level/20;
                    af.location = APPLY_DEX;
                    af.modifier = -4;
                    af.subtype = 1;
                    act("{G$n{G is poisoned and $s legs start to convulse.{x",
			victim,NULL,NULL,TO_ROOM);
                    send_to_char("{GYou are poisoned and your legs start to convulse.{x\n\r",
                        victim);
                    break;
            case 2: af.duration=1010;
                    victim->stunned=number_range(1,3);
                    act("{G$n{G is poisoned and slumps to the ground unconsciously.{x",
                       victim,NULL,NULL,TO_ROOM);
                    send_to_char("{GYou are poisoned and slump to the ground unconsciously.{x\n\r",victim);
                    break;
           }
           affect_to_char( victim, &af );
           if(number_range(0,1)==0)
           {
            affect_remove_obj(wield,pAf);
            act("The blade poison on $p vanishes.",ch,wield,NULL,TO_CHAR);
           }
          }
         }
        }
	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_POISON)
           && !check_force(ch,victim))
	{
	    int level;
	    AFFECT_DATA *poison;

	    if ((poison = affect_find(wield->affected,gsn_poison)) == NULL)
		level = wield->level;
	    else
		level = poison->level;
	
	    /*if (!saves_spell(level / 2,victim,DAM_POISON))
	    {
		send_to_char("{cYou feel {ypoison{c coursing through your veins.\n\r{x",
		    victim);
		act("$n is {ypoisoned{x by the venom on $p.",
		    victim,wield,ch,TO_VICT);

    		af.where     = TO_AFFECTS;
    		af.type      = gsn_poison;
    		af.level     = level * 3/4;
    		af.duration  = level / 2;
    		af.location  = APPLY_STR;
    		af.modifier  = -1;
    		af.bitvector = AFF_POISON;
    		affect_join( victim, &af );
	    }*/

            poison_effect((void*)victim,level/2,dam,TARGET_CHAR);
	    /* weaken the poison if it's temporary */
	    if (poison != NULL)
	    {
	    	poison->level = UMAX(0,poison->level - 2);
	    	poison->duration = UMAX(0,poison->duration - 1);
	
	    	if (poison->level == 0 || poison->duration == 0)
		    act("The {ypoison{x on $p has worn off.",ch,wield,NULL,TO_CHAR);
	    }
 	}

        if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_ROTTING)
          && !check_force(ch,victim))
        {
          int level = wield->level;
          rotting_effect((void*)victim, level/2,dam,TARGET_CHAR);
        }

        if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_LIGHT)
          && !check_force(ch,victim))
        {
          int level = wield->level;
          light_effect((void*)victim, level/2, dam, TARGET_CHAR);
        }

        if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_DROWNING)
           && !check_force(ch,victim))
        {
          int level = wield->level;
          drowning_effect((void*)victim, level/2,dam,TARGET_CHAR);
        }

        if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_UNHOLY)
          && !check_force(ch,victim))
        {
          int level = wield->level;
          unholy_effect((void*)victim, level/2, dam, TARGET_CHAR);
        }

    	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_VAMPIRIC)
          && !check_force(ch,victim))
	{
	    dam = number_range(1, wield->level / 5 + 1);
	    xact_new("{k$p draws life from $n.{x",victim,wield,ch,TO_VICT,POS_RESTING, SUPPRESS_FLAGS);
	    xact_new("{iYou feel $p drawing your life away.{x",
		victim,wield,NULL,TO_CHAR,POS_RESTING, SUPPRESS_FLAGS);
	    damage_old(ch,victim,dam,0,DAM_NEGATIVE,FALSE);
	    ch->alignment = UMAX(-1000,ch->alignment - 1);
	    if ( ch->pet != NULL )
		ch->pet->alignment = ch->alignment;
	    ch->hit += dam/2;
	}

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_FLAMING)
         && !check_force(ch,victim))
	{
	    dam = number_range(1,wield->level / 4 + 1);
	    xact_new("{k$n is {rburned{k by $p.{x",victim,wield,ch,TO_VICT,POS_RESTING, SUPPRESS_FLAGS);
	    xact_new("{i$p {rsears{i your flesh.{x",victim,wield,NULL,TO_CHAR,POS_RESTING, SUPPRESS_FLAGS);
	    fire_effect( (void *) victim,wield->level/2,dam,TARGET_CHAR);
	    damage(ch,victim,dam,0,DAM_FIRE,FALSE);
	}

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_FROST)
          && !check_force(ch,victim))
	{
	    dam = number_range(1,wield->level / 6 + 2);
	    xact_new("{k$p {cfreezes{k $n.{x",victim,wield,ch,TO_VICT,POS_RESTING, SUPPRESS_FLAGS);
	    xact_new("{iThe {Ccold{i touch of $p surrounds you with {Cice.{x",
		victim,wield,NULL,TO_CHAR,POS_RESTING, SUPPRESS_FLAGS);
	    cold_effect(victim,wield->level/2,dam,TARGET_CHAR);
	    damage(ch,victim,dam,0,DAM_COLD,FALSE);
	}

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_SHOCKING)
           && !check_force(ch,victim))
	{
	    dam = number_range(1,wield->level/5 + 2);
	    xact_new("{k$n is struck by {Ylightning{k from $p.{x",victim,wield,ch,TO_VICT,POS_RESTING, SUPPRESS_FLAGS);
	    xact_new("{iYou are {Yshocked{i by $p.{x",victim,wield,NULL,TO_CHAR,POS_RESTING, SUPPRESS_FLAGS);
	    shock_effect(victim,wield->level/2,dam,TARGET_CHAR);
	    damage(ch,victim,dam,0,DAM_LIGHTNING,FALSE);
	}
    }

    tail_chain();
    return;
}

void process_shields(CHAR_DATA *ch, CHAR_DATA *victim)
{	
  int dam, dt;
  int count=0,total=0;

	    if (IS_SHIELDED(victim, SHD_ICE))
	    {
		if(ch->fighting!=victim)
                 return;
                if (!IS_SHIELDED(ch, SHD_ICE) && !check_force(victim,ch))
		{
		    dt = skill_lookup("iceshield");
		    dam = number_range(20, 40);
		    total+=xdamage(victim, ch, dam, dt, DAM_COLD, TRUE, SUPPRESS_SHIELD);
                    count++;
		}
	    }
	    if (IS_SHIELDED(victim, SHD_FIRE))
	    {
		if(ch->fighting!=victim)
                 return;
                if (!IS_SHIELDED(ch, SHD_FIRE) && !check_force(victim,ch))
		{
		    dt = skill_lookup("fireshield");
		    dam = number_range(20, 40);
		    total+=xdamage(victim, ch, dam, dt, DAM_FIRE, TRUE, SUPPRESS_SHIELD);
                    count++;
		}
	    }
	    if (IS_SHIELDED(victim, SHD_SHOCK))
	    {
                 if(ch->fighting!=victim)
                 return;
		if (!IS_SHIELDED(ch, SHD_SHOCK) && !check_force(victim,ch))
		{
		    dt = skill_lookup("shockshield");
		    dam = number_range(20, 40);
		    total+=xdamage(victim, ch, dam, dt, DAM_LIGHTNING, TRUE, SUPPRESS_SHIELD);
		    count++;	
		}
	    }
            if (IS_SHIELDED(victim, SHD_SHADOW))
	    {
                if(ch->fighting!=victim)
                 return;
		if (!IS_SHIELDED(ch, SHD_SHADOW) && !check_force(victim,ch))
		{
		    dt = skill_lookup("shadowshield");
		    dam = number_range(50, 100);
		    total+=xdamage(victim, ch, dam, dt, DAM_NEGATIVE, TRUE,SUPPRESS_SHIELD);
                    count++;
		}
	    }
            if (IS_SHIELDED(victim, SHD_BLADE))
	    {
                if(ch->fighting!=victim)
                 return;
		if (!IS_SHIELDED(ch, SHD_BLADE) && !check_force(victim,ch))
		{
		    dt = skill_lookup("bladebarrier");
		    dam = number_range(50, 100);
		    total+=xdamage(victim, ch, dam, dt, DAM_PIERCE, TRUE,SUPPRESS_SHIELD);
                    count++;
		}
	    }
            if (IS_SHIELDED(victim, SHD_PRISMATIC))
	    {
                if(ch->fighting!=victim)
                 return;
		if (!IS_SHIELDED(ch, SHD_PRISMATIC) && !check_force(victim,ch))
		{
		    dt = skill_lookup("prismaticshield");
		    dam = number_range(60, 120);
		    total+=xdamage(victim, ch, dam, dt, DAM_LIGHT, TRUE,SUPPRESS_SHIELD);
                    count++;
		}
	    }
            if (IS_SHIELDED(victim, SHD_ACID))
	    {
                if(ch->fighting!=victim)
                 return;
		if (!IS_SHIELDED(ch, SHD_ACID) && !check_force(victim,ch))
		{
		    dt = skill_lookup("acidshield");
		    dam = number_range(60, 120);
		    total+=xdamage(victim, ch, dam, dt, DAM_ACID, TRUE,SUPPRESS_SHIELD);
		    count++;	
		}
	    }
            if (IS_SHIELDED(victim, SHD_EARTH))
	    {
                if(ch->fighting!=victim)
                 return;
		if (!IS_SHIELDED(ch, SHD_EARTH) && !check_force(victim,ch))
		{
		    dt = skill_lookup("earthshield");
		    dam = number_range(40, 80);
		    total+=xdamage(victim, ch, dam, dt, DAM_EARTH, TRUE,SUPPRESS_SHIELD);
                    count++;
		}
	    }
       if(ch->fighting!=victim)
        return;
       if(count>1)
	dam_message(victim,ch,total,-1,FALSE,SUPPRESS_SHIELD_COMP,FALSE);
       else if (count==1)
        dam_message(victim,ch,total,-1,FALSE,SUPPRESS_SHIELD_COMP,TRUE);
       return;
}


/*
 * Inflict damage from a hit.
 */
int xdamage(CHAR_DATA *ch,CHAR_DATA *victim,int dam,int dt,int dam_type,
	    bool show,int suppress)
{

    OBJ_DATA *corpse;
    bool immune;
    int lootpoint=-1;
    AFFECT_DATA *pAf=NULL;

    if ( victim->position == POS_DEAD )
	return 0;

    if ( victim != ch )
    {
	/*
	 * Certain attacks are forbidden.
	 * Most other attacks are returned.
	 */
	if ( is_safe( ch, victim ) )
	    return 0;

	if ( victim->position > POS_STUNNED )
	{
	    if ( victim->fighting == NULL )
            {
		set_fighting( victim, ch );
           if ( IS_NPC( victim ) && HAS_TRIGGER( victim, TRIG_KILL ) )
		    mp_percent_trigger( victim, ch, NULL, NULL, TRIG_KILL );
	    }
	    if (victim->timer <= 4)
	    	victim->position = POS_FIGHTING;
	}

	if ( victim->position > POS_STUNNED )
	{
	    if ( ch->fighting == NULL )
		set_fighting( ch, victim );

	}

	/*
	 * More charm stuff.
	 */
	if ( victim->master == ch )
	    stop_follower( victim );
    }

    /*
     * Inviso attacks ... not.
     */
    if ( IS_SHIELDED(ch, SHD_INVISIBLE) )
    {
	affect_strip( ch, gsn_invis );
	affect_strip( ch, gsn_mass_invis );
	REMOVE_BIT( ch->shielded_by, SHD_INVISIBLE );
	act( "$n fades into existence.", ch, NULL, NULL, TO_ROOM );
    }
    if ( IS_SHIELDED(ch, SHD_SWALK))
    {
	affect_strip( ch, gsn_shadow_walk); 
        REMOVE_BIT(ch->shielded_by, SHD_SWALK);
        send_to_char("You step from the shadows.\n\r",ch);
        act("$n steps from the shadows.",ch,NULL,NULL,TO_ROOM);
    }
    if ( is_affected(ch, gsn_ambush))
    {
	affect_strip( ch, gsn_ambush );
        send_to_char("You step from your hiding place.\n\r",ch);
        act("$n steps from $s hiding place.",ch,NULL,NULL,TO_ROOM);
    }
    /* strip sever if victim is attacked by someone else than
       the person who severed him/her */
    if( is_affected(victim,gsn_sever) &&
       (affect_find(victim->affected,gsn_sever)->dealer!=ch)
       && (dt!=gsn_poison) && (dt!=gsn_plague)
       && (!IS_NPC(ch)))
    {
	affect_strip(victim,gsn_sever);
        if(skill_table[gsn_sever].msg_off)
        {
	 send_to_char(skill_table[gsn_sever].msg_off,victim);
         send_to_char("\n\r",victim);
        }
        act("{G$n{G is no longer severed.{x",victim,NULL,NULL,TO_ROOM);
    }
         
    /*
     * Damage modifiers.
     */

    if ( dam > 1 && !IS_NPC(victim) 
    &&   victim->pcdata->condition[COND_DRUNK]  > 10 )
	dam = 9 * dam / 10;

    if ( dam > 1 && IS_SHIELDED(victim, SHD_SANCTUARY) )
	dam /= 2;

    if ( dam > 1 && ((IS_SHIELDED(victim, SHD_PROTECT_EVIL) && IS_EVIL(ch) )
    ||		     (IS_SHIELDED(victim, SHD_PROTECT_GOOD) && IS_GOOD(ch) )))
	dam -= dam / 4;

    if ( dam > 1 && is_affected(victim, gsn_assassinate)
       && (affect_find(victim->affected,gsn_assassinate)->dealer == ch))
      dam += dam / 6;

    if ( (dam > 1) && (dt>0) && (dt < TYPE_HIT) &&
       (skill_table[dt].spell_fun!=spell_null))
    {
     if(is_affected(victim,gsn_resistance)
      && (number_percent() < (50-(ch->level/5)+(victim->level/10))))
       dam= dam/5;
    }

    immune = FALSE;


    /*
     * Check for parry, and dodge.
     */
    if ( dt >= TYPE_HIT && ch != victim)
    {
        if ( check_parry( ch, victim ) )
	    return 0;
	if ( check_dodge( ch, victim ) )
	    return 0;
	if ( check_shield_block(ch,victim))
	    return 0;
        if ( check_phase(ch,victim))
            return 0;
        if ( check_armor(ch,victim,dam))
            return 0;
        if ( check_shield(ch, victim, dam))
            return 0;
        if ( check_stone_skin(ch, victim, dam))
            return 0;
    }
    
    if((dt >= TYPE_HIT) && show && dam>0)
    { 
     if(check_critical_hit(ch,victim))
     {
      dam += dam * ((number_percent()/2+get_skill(ch,gsn_critical_hit))/2) / 50;
     }
    }
    if((dt==gsn_circle||dt==gsn_backstab||dt==gsn_assassinate||dt==gsn_strike)
        && show && dam>0)
    {
      if(check_critical_hit(ch,victim))
      {
	/*dam += dam * number_range(0,get_skill(ch,gsn_critical_hit)/2)/100;*/
        dam += dam * ((number_percent()/2+get_skill(ch,gsn_critical_hit))/2) / 100;
      }
    }
    if((ch->class == class_lookup("sensai")) ||
      (ch->class == class_lookup("shaolin")) && ((dt>=(TYPE_HIT+40) ||
           (dt==(TYPE_HIT+17)))))
     dam = monk_style(ch,victim,dt,dam);   
    switch(check_immune(victim,dam_type,dt))
    {
	case(IS_IMMUNE):
	    immune = TRUE;
	    dam = 0;
	    break;
	case(IS_RESISTANT):	
	    dam -= dam/3;
	    break;
	case(IS_VULNERABLE):
	    dam += dam/2;
	    break;
    }
    /* damage reduction */
    if(dam>=1500)
     dam=(dam*5)/6;
    else if(dam>=1000)
     dam=(dam*6)/7;
    else if(dam>=750)
     dam=(dam*7)/8;
    else if(dam>=500)
     dam=(dam*8)/9;
    else if(dam<=100)
     dam=(dam*11)/9;
    
   if(((pAf=affect_find(victim->affected,skill_lookup("ward")))!=NULL) 
       && show && (dam>=pAf->subtype))
   {
    char buf[MAX_STRING_LENGTH];
    sprintf(buf,"{GYour ward absorbs the damage and vanishes in a bright flash! {D({WAbsorb{D:{W%d{D){x\n\r",dam);
    affect_strip(victim,skill_lookup("ward"));
    send_to_char(buf,victim);
    sprintf(buf,"{G$n{G's ward absorbs the damage and vanishes in a bright flash! {D({WAbsorb{D:{W%d{D){x",dam);
    act(buf, victim, NULL, NULL, TO_ROOM);
    return 0;
   }
   
    if(IS_SHIELDED(victim, SHD_MANA) && show && (dt>=TYPE_HIT) && (dam>0))
    {
     int chance=33-dam/10+(victim->level-ch->level);
     if(number_percent()<chance)
     {
      char buf[MAX_STRING_LENGTH];
      int absorb=(dam/10)*number_range(1,4);
      if((victim->mana-absorb>=0) && (absorb>0))
      {
       sprintf(buf,"Your mana shield absorbs some of the damage! {D({WAbsorb{D:{W%d{D){x\n\r",absorb);
       send_to_char(buf,victim);
       sprintf(buf,"$n's mana shield absorbs some of the damage! {D({WAbsorb{D:{W%d{D){x",absorb);
       act(buf,victim,NULL,NULL,TO_ROOM);
       dam-=absorb;
       victim->mana-=absorb;
      }
     }
    }

    if (show)
    	dam_message( ch, victim, dam, dt, immune, suppress, TRUE );

    if (dam == 0)
	return 0;

    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */
    victim->hit -= dam;
    if ( !IS_NPC(victim)
    &&   victim->level >= LEVEL_IMMORTAL
    &&   victim->hit < 1 )
	victim->hit = 1;
    update_pos( victim );
    if (dt == gsn_feed)
    {
	ch->hit = UMIN(ch->hit+((dam/3)*2), ch->max_hit);
	update_pos( ch );
    }

    switch( victim->position )
    {
    case POS_MORTAL:
	act( "{c$n is mortally wounded, and will die soon, if not aided.{x",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char( 
	    "{cYou are mortally wounded, and will die soon, if not aided.{x\n\r",
	    victim );
	break;

    case POS_INCAP:
	act( "{c$n is incapacitated and will slowly die, if not aided.{x",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char(
	    "{cYou are incapacitated and will slowly {z{Rdie{x{c, if not aided.{x\n\r",
	    victim );
	break;

    case POS_STUNNED:
	act( "{c$n is stunned, but will probably recover.{x",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char("{cYou are stunned, but will probably recover.{x\n\r",
	    victim );
	break;

    case POS_DEAD:
        if ((IS_NPC(victim)) && ( victim->die_descr[0] != '\0'))
	{
	    act( "{c$n $T{x", victim, 0, victim->die_descr, TO_ROOM );
	}
	else
	{
	    if (IS_SET(ch->in_room->room_flags,ROOM_ARENA)
            && IS_SET(victim->in_room->room_flags,ROOM_ARENA))
           {
              check_arena(ch,victim);
              return dam;
           }
           if (global_quest==QUEST_CLOSED && ch->on_quest &&
                victim->on_quest && IS_SET(quest_flags,QUEST_COMBAT))
           {
		check_quest(ch,victim);
                return dam;
           }
            act( "{c$n is {CDEAD!!{x", victim, 0, 0, TO_ROOM );
	}
	send_to_char( "{cYou have been {RKILLED!!{x\n\r\n\r", victim );
	break;

    default:
	if ( dam > victim->max_hit / 4 )
	    send_to_char( "{cThat really did {RHURT!{x\n\r", victim );
	if ( victim->hit < victim->max_hit / 4 )
	    send_to_char( "{cYou sure are {z{RBLEEDING!{x\n\r", victim );
	break;
    }

    /*
     * Sleep spells and extremely wounded folks.
     */
    if ( !IS_AWAKE(victim) )
	stop_fighting( victim, FALSE );

    /*
     * Payoff for killing things.
     */
    if ( victim->position == POS_DEAD )
    {
        int pkpgain=0,killers=0;
	group_gain( ch, victim );

	if ( !IS_NPC(victim))
	{
            DESCRIPTOR_DATA *d;
            BUFFER *output;
            char buf[MAX_STRING_LENGTH];
            CHAR_DATA *wch,*wch_next;
                
	    lootpoint=MAX_LOOT;
            output=new_buf();
            sprintf( log_buf, "%s killed by %s at %d",
		victim->name,
		(IS_NPC(ch) ? ch->short_descr : ch->name),
		ch->in_room->vnum );
	    log_string( log_buf );
            victim->pcdata->pkflag=0;
            /* award pkps */
            if(!IS_NPC(ch) && (victim!=ch))
            {
             OBJ_DATA *obj;
 
             if(ch->level>victim->level)
              pkpgain = victim->pcdata->pkp/5;
             else if(ch->level<victim->level)
              pkpgain = victim->pcdata->pkp/3;
             else if(ch->level==victim->level)
              pkpgain = victim->pcdata->pkp/4;

             ch->pcdata->pkp+=pkpgain;
             victim->pcdata->pkp-=pkpgain;
             ch->pcdata->kills[game_state]++;
             victim->pcdata->deaths[game_state]++;
             sprintf(buf,"{WYou have gained {R%d{x {RP{rK{W points!{x\n\r",pkpgain);
             send_to_char(buf,ch);
             obj = create_object(get_obj_index(20250),0);
             sprintf(buf,"A shrunken head of %s, harvested by %s",victim->name,ch->name);
             obj->short_descr=str_dup(buf);
             obj->description=str_dup(buf);
             sprintf(buf,"shrunken head %s",victim->name);
             obj->name=str_dup(buf);
             obj_to_char(obj,ch);
             if(victim->pcdata->bounty>0)
             {
              ch->platinum+=victim->pcdata->bounty;
              sprintf(buf,"{WYou receive {R%d{W platinum as a {Rb{rount{Ry{W!{x\n\r",victim->pcdata->bounty);
	      send_to_char(buf,ch); 
              victim->pcdata->bounty=0;
             } 
             affect_strip(ch,gsn_gouge); 
             ch->pcdata->pkflag=0;
            }
            sprintf( buf, "{D[{RD{reat{Rh{D]{W %s{W has just been killed by %s{W!{x\n\r",
              victim->name,(IS_NPC(ch) ? ch->short_descr : ch->name));
            add_buf(output,buf);
            killers++;
            if(!IS_NPC(ch))
              for(wch=char_list; wch!=NULL; wch=wch_next)
              {
               wch_next = wch->next;
               if(wch->fighting==victim && (!IS_NPC(wch)) && (wch!=ch))
               {
                sprintf(buf,"{D[{RD{reat{Rh{D]{W %s{W has just been killed by %s{W!{x\n\r", victim->name, wch->name);
                add_buf(output,buf);
                killers++;
                lootpoint--;
                wch->pcdata->pkflag=0;
               }
              }
            if(lootpoint<0)
             lootpoint=0;
            /* no looting during paradox */
            if(game_state == GS_PARADOX)
             lootpoint=0;
            for(d=descriptor_list; d; d=d->next)
            {
             if(d->connected==CON_PLAYING)
             {
              page_to_char(buf_string(output),d->character);
             }
            }
            free_buf(output); 
	    /*
	     * Dying penalty:
	     * 2/3 way back to previous level.
	     */
/*	    if ( victim->exp > exp_per_level(victim,victim->pcdata->points) 
			       * victim->level )
	gain_exp( victim, (5 * (exp_per_level(victim,victim->pcdata->points)
			         * victim->level - victim->exp)/6) + 50 );*/

	}
        sprintf( log_buf, "%s got toasted by %s at %s [room %d]",
            (IS_NPC(victim) ? victim->short_descr : victim->name),
            (IS_NPC(ch) ? ch->short_descr : ch->name),
            ch->in_room->name, ch->in_room->vnum);
 
        if (IS_NPC(victim))
            wiznet(log_buf,NULL,NULL,WIZ_MOBDEATHS,0,0);
        else
            wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);

     	/*
	 * Death trigger
	 */
	if ( IS_NPC( victim ) && HAS_TRIGGER( victim, TRIG_DEATH) )
	{
	    victim->position = POS_STANDING;
	    mp_percent_trigger( victim, ch, NULL, NULL, TRIG_DEATH );
	}
        
        if(IS_NPC(victim) || IS_NPC(ch) 
          || (!IS_NPC(victim)&&!IS_NPC(ch)&&(victim==ch)))
	 raw_kill( victim, ch, -1 );
        else
        {
         raw_kill( victim, ch, lootpoint );
         register_pk(ch->name,victim->name,game_state,pkpgain,killers);
         if(!IS_IMMORTAL(victim))
          register_rank(victim->name,victim->pcdata->pkp,victim->race,victim->class,victim->clan);
         if(!IS_IMMORTAL(ch))
          register_rank(ch->name,ch->pcdata->pkp,ch->race,
			ch->class,ch->clan);
	 save_pkills();
        }
        /* dump the flags */
        if (ch != victim && !IS_NPC(ch) && (!is_same_clan(ch,victim)
	|| clan_table[victim->clan].independent))
        {
            if (IS_SET(victim->act,PLR_TWIT))
                REMOVE_BIT(victim->act,PLR_TWIT);
        }
        if(!IS_NPC(victim))
          set_killed_flag(victim); 

        /* RT new auto commands */

	if ( !IS_NPC(ch) && IS_NPC(victim) )
	{
	    OBJ_DATA *coins;

	    corpse = get_obj_list( ch, "corpse", ch->in_room->contents ); 

	    if ( IS_SET(ch->act, PLR_AUTOLOOT) &&
		 corpse && corpse->contains) /* exists and not empty */
		do_get( ch, "all corpse" );

 	    if (IS_SET(ch->act,PLR_AUTOGOLD) &&
	        corpse && corpse->contains  && /* exists and not empty */
		!IS_SET(ch->act,PLR_AUTOLOOT))
		if ((coins = get_obj_list(ch,"gcash",corpse->contains))
		     != NULL)
	      	    do_get(ch, "all.gcash corpse");
            
	    if ( IS_SET(ch->act, PLR_AUTOSAC) )
       	      if ( IS_SET(ch->act,PLR_AUTOLOOT) && corpse && corpse->contains)
		return dam;  /* leave if corpse has treasure */
	      else
		do_sacrifice( ch, "corpse" );
	}

	return dam;
    }

    if ( victim == ch )
	return dam;

    /*
     * Take care of link dead people.
     */
    if ( !IS_NPC(victim) && victim->desc == NULL )
    {
	if ( number_range( 0, victim->wait ) == 0 )
	{
	    do_recall( victim, "" );
	    return dam;
	}
    }

    /*
     * Wimp out?
     */
    if ( IS_NPC(victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2)
    {
	if ( ( IS_SET(victim->act, ACT_WIMPY) && number_bits( 2 ) == 0
	&&   victim->hit < victim->max_hit / 5) 
	||   ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL
	&&     victim->master->in_room != victim->in_room ) )
	    do_flee( victim, "" );
    }

    if ( !IS_NPC(victim)
    &&   victim->hit > 0
    &&   victim->hit <= victim->wimpy
    &&   victim->wait < PULSE_VIOLENCE / 2 )
	do_flee( victim, "" );

    tail_chain( );
    return dam;
}

int monk_style(CHAR_DATA *ch, CHAR_DATA *victim, int dt, int dam)
{
 int newdam=dam;
 int through=0;

 if(!IS_SHIELDED(victim, SHD_SANCTUARY))
  newdam/=2;
 if(dt == (TYPE_HIT + 17))
   newdam+=(newdam*2)+number_range(0,newdam);
 else if(dt >= (TYPE_HIT + 40) && (dt <= (TYPE_HIT + 45)))
         
 {
  newdam+=(newdam*2)+number_range(0,newdam);
  if(dt == (TYPE_HIT + 44))
  {
   act("{xYou deliver a powerful palm strike to $N's chin.{x",ch,NULL,
      victim, TO_CHAR);
   act("{x$n delivers a powerful palm strike to your chin.{x",ch,NULL,
      victim, TO_VICT);
   act("{x$n delivers a powerful palm strike to $N's chin.{x",ch,NULL,
     victim, TO_NOTVICT);
  }
  if(number_percent()<=10)
   through=1;
 }
 else if((dt >= (TYPE_HIT + 46)) && (dt <= (TYPE_HIT + 50)))
 {
  newdam+=(newdam*3)+number_range(0,newdam);
  if(dt == (TYPE_HIT + 47))
  {
   act("{xYou unleash the power of a dragon!{x",ch,NULL,victim, TO_CHAR);
   act("{x$n unleashes the power of a dragon!{x",ch,NULL,victim, TO_ROOM);
  }
  if(number_percent()<=25)
   through=1;
 }
 else if((dt >= (TYPE_HIT + 51)) && (dt <= (TYPE_HIT + 55)))
 {
  newdam+=(newdam*3)+number_range(0,newdam);
  if(dt == (TYPE_HIT + 51))
  {
   act("{xYou spin around like a whirlwind.{x",ch,NULL,victim,TO_CHAR);
   act("{x$n spins around like a whirlwind.{x",ch,NULL,victim,TO_ROOM);
  }
  if(number_percent()<=40)
   through=1;
 }
 else if((dt >= (TYPE_HIT + 56)) && (dt <= (TYPE_HIT + 60)))
 {
  newdam+=(newdam*3)+number_range(0,newdam*2);
  switch(dt-TYPE_HIT)
  {
   case 56: act("{xYou reach out with your mental powers and strike $N.{x",
             ch,NULL,victim,TO_CHAR);
            act("{x$n reaches out with $s mental powers and strikes you.{x",
             ch,NULL,victim,TO_VICT);
            act("{x$n reaches out with $s mental powers and strikes $N.{x",
              ch,NULL,victim,TO_NOTVICT);
            break;
   case 58: act("{xYou hit $N with lightning speed!{x",ch,NULL,victim,TO_CHAR);
            act("{x$n hits you with lightning speed!{x",ch,NULL,victim,TO_VICT);
            act("{x$n hits $N with lightning speed!{x",ch,NULL,victim,TO_NOTVICT);
            break;
   case 60: act("{xFlames appear from nowhere and burn $N.{x",ch,NULL,victim,
             TO_CHAR);
            act("{xFlames appear from nowhere and burn you.{x",ch,NULL,victim,
             TO_VICT);
            act("{xFlames appear from nowhere and burn $N.{x",ch,NULL,victim,
             TO_NOTVICT);
            break;
  }
  if(number_percent()<=45)
   through=1;
 }
 if(through && IS_SHIELDED(victim, SHD_SANCTUARY))
 {
  newdam*=2;
  act("{xYou {Wpunch{x through $N's sanctuary!{x",ch,NULL,victim,TO_CHAR);
  act("{x$n {Wpunches{x through your sanctuary!{x",ch,NULL,victim,TO_VICT);
  act("{x$n {Wpunches{x through $N's sanctuary!{x",ch,NULL,victim,TO_NOTVICT);
 }
 if(!IS_SHIELDED(victim, SHD_SANCTUARY)) 
  newdam*=2;
 if(newdam<=50)
  newdam=(newdam*5)/3;
 else if(newdam<=100)
  newdam=(newdam*4)/3;
 else if(newdam>=750)
  newdam=(newdam*11)/16;
 else if(newdam>=500)
  newdam=(newdam*3)/4;
 else if(newdam>=300)
  newdam=(newdam*4)/5;
 else if(newdam>=200)
  newdam=(newdam*5)/6; 
 return newdam;       
}

int damage(CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, int dam_type,bool show)
{
 return xdamage(ch,victim,dam,dt,dam_type,show,SUPPRESS_STD);
}

int damage_old(CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, int dam_type,bool show)
{
  return xdamage(ch,victim,dam,dt,dam_type,show,SUPPRESS_STD);
}

bool in_range(CHAR_DATA *ch, CHAR_DATA *victim)
{
 if(ch==victim)
  return TRUE;
 
 if(IS_NPC(ch) || IS_NPC(victim))
  return TRUE;

 if((ch->level<=20) || (victim->level<=20))
  return FALSE;
 
 if((ch->level+10)<victim->level)
  return FALSE;
 
 if((ch->level-10)>victim->level)
  return FALSE;
 
 if(is_same_clan(ch,victim))
  return FALSE;
 
 
 return TRUE;
}

bool is_safe(CHAR_DATA *ch, CHAR_DATA *victim)
{
    if (victim->in_room == NULL || ch->in_room == NULL)
	return TRUE;

    if (victim->fighting == ch || victim == ch /*||
        ch->fighting==victim*/)

	return FALSE;

/*    if (!IS_NPC(ch) && IS_IMMORTAL(ch))
	return FALSE;*/
    
    /* killing mobiles */
    if (IS_NPC(victim))
    {

	/* safe room? */
	if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))

	{
	    send_to_char("Not in this room.\n\r",ch);
	    return TRUE;
	}

	if (victim->pIndexData->pShop != NULL)
	{
	    send_to_char("The shopkeeper wouldn't like that.\n\r",ch);
	    return TRUE;
	}
        
        if (victim->spec_fun == spec_lookup("spec_questmaster"))
        {
            send_to_char("The questmaster wouldn't like that.\n\r",ch);
            return TRUE;
        }
	/* no killing healers, trainers, etc */
	if (IS_SET(victim->act,ACT_TRAIN)
	||  IS_SET(victim->act,ACT_PRACTICE)
	||  IS_SET(victim->act,ACT_IS_HEALER)
	||  IS_SET(victim->act,ACT_IS_CHANGER)
	||  IS_SET(victim->act,ACT_IS_SATAN)
	||  IS_SET(victim->act,ACT_IS_PRIEST))
	{
	    act("I don't think $G would approve.",ch,NULL,NULL,TO_CHAR);
	    return TRUE;
	}

	if (!IS_NPC(ch))
	{
	    /* no pets */
	    if (IS_SET(victim->act,ACT_PET))
	    {
		act("But $N looks so cute and cuddly...",
		    ch,NULL,victim,TO_CHAR);
		return TRUE;
	    }

	    /* no charmed creatures unless owner */
	    if (IS_AFFECTED(victim,AFF_CHARM) && ch != victim->master)
	    {
		send_to_char("You don't own that monster.\n\r",ch);
		return TRUE;
	    }
	}
    }
    /* killing players */
    else
    {
	/* NPC doing the killing */
	if (IS_NPC(ch))
	{
	    /* safe room check */
	    if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))
	    {
		send_to_char("Not in this room.\n\r",ch);
		return TRUE;
	    }

	    /* charmed mobs and pets cannot attack players while owned */
	    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master != NULL
	    &&  ch->master->fighting != victim)
	    {
		send_to_char("Players are your friends!\n\r",ch);
		return TRUE;
	    }

            if(has_killed_flag(victim))
            {
             send_to_char("But they have just been killed!\n\r",ch);
             return TRUE;
            }
/*            if(victim->level>101)
            {
             send_to_char("You can't kill immortals out of avatar mode.\n\r",ch);
             return TRUE;
            }*/
	}
	/* player doing the killing */
	else
	{
            DESCRIPTOR_DATA *d;
            int ccount=0,vcount=0;
	    if (IS_SET(victim->act,PLR_TWIT) )
		return FALSE;
            
            if(IS_SET(victim->in_room->room_flags,ROOM_ARENA))
             return FALSE;

            if(is_voodood(ch,victim))
		return FALSE;

            if(global_quest==QUEST_CLOSED && victim->on_quest &&
                ch->on_quest && IS_SET(quest_flags,QUEST_COMBAT)
               && victim->in_room->vnum!=516)
              return FALSE;

            if (IS_SET(victim->in_room->room_flags,ROOM_SAFE)
              && (game_state!=GS_PARADOX))
            {
                send_to_char("Not in this room.\n\r",ch);
                return TRUE;
            }
	    if (ch->on_quest)
	    {
		send_to_char("Not while you are on a quest.\n\r",ch);
		return TRUE;
	    }
	    if (victim->on_quest)
	    {
		send_to_char("They are on a quest, leave them alone.\n\r",ch);
		return TRUE;
	    }

            if (is_same_clan(ch,victim) )
	    {
		send_to_char("You can't fight your own clan members.\n\r",ch);
		return TRUE;
	    }

            if(!in_range(ch,victim))
            {
              send_to_char("They are not in your range.\n\r",ch);
              return TRUE;
            }

/*            if (victim->fighting != NULL && !IS_NPC(victim->fighting))
            {
               send_to_char("Kill stealing is not permitted.\n\r",ch);
               return TRUE;
            }*/

            if(has_killed_flag(victim))
            {
              send_to_char("But they have just been killed!\n\r",ch);
              return TRUE;
            } 
/*            if(victim->level>101)
            {
              send_to_char("You can't kill immortals out of avatar mode.\n\r",ch);
	      return TRUE;
            }*/
            for(d=descriptor_list;d!=NULL;d=d->next)
            {
             if((d->connected==CON_PLAYING)
                && (d->character != ch)
                && (d->character->fighting!=NULL)
                && (d->character->fighting==victim)
                && (d->character->in_room==victim->in_room))
                   vcount++;

             if((d->connected==CON_PLAYING)
                && (d->character != victim)
                && (d->character->fighting!=NULL)
                && (d->character->fighting==ch)
                && (d->character->in_room==ch->in_room))
                   ccount++;
            }
            switch(game_state)
            {
             default:
             case GS_CHAOS:
             case GS_PARADOX: return FALSE;
             case GS_SOLO: if(vcount>=1)
                           {
                            send_to_char("They are already fighting another player.\n\r",ch);
                            return TRUE;
                           }
                           if(ccount>=1)
                           {
                            send_to_char("You are already fighting another player.\n\r",ch);
                            return TRUE;
                           }
                           return FALSE;
             case GS_1VS2: if(vcount>=2)
                           {
                            send_to_char("They are already fighting two other players.\n\r",ch);
                            return TRUE;
                           }
                           if(ccount>=2)
                           {
                            send_to_char("You are already fighting two other players.\n\r",ch);
                            return TRUE;
                           }
                           return FALSE;
            }


	}
    }
    return FALSE;
}


bool is_voodood(CHAR_DATA *ch, CHAR_DATA *victim)
{
    OBJ_DATA *object;
    bool found;

    if ( ch->level > HERO )
	return FALSE;

    found = FALSE;
    for ( object = victim->carrying; object != NULL; object = object->next_content )
    {
	if (object->pIndexData->vnum == OBJ_VNUM_VOODOO)
	{
	    char arg[MAX_INPUT_LENGTH];

	    one_argument(object->name, arg);
	    if (!str_cmp(arg, ch->name))
	    {
		return TRUE;
	    }
	}
    }
    return FALSE;
}

bool is_safe_spell(CHAR_DATA *ch, CHAR_DATA *victim, bool area )
{
    if (victim->in_room == NULL || ch->in_room == NULL)
        return TRUE;

    if (victim == ch && area)
	return TRUE;

    if (victim->fighting == ch || victim == ch)
	return FALSE;

/*    if (!IS_NPC(ch) && IS_IMMORTAL(ch))
	return FALSE;*/

    /* killing mobiles */
    if (IS_NPC(victim))
    {
	/* safe room? */
	if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))
	    return TRUE;

	if (victim->pIndexData->pShop != NULL)
	    return TRUE;

        if (victim->spec_fun == spec_lookup("spec_questmaster"))
            return TRUE;

	/* no killing healers, trainers, etc */
	if (IS_SET(victim->act,ACT_TRAIN)
	||  IS_SET(victim->act,ACT_PRACTICE)
	||  IS_SET(victim->act,ACT_IS_HEALER)
	||  IS_SET(victim->act,ACT_IS_CHANGER)
	||  IS_SET(victim->act,ACT_IS_SATAN)
	||  IS_SET(victim->act,ACT_IS_PRIEST))
	    return TRUE;

	if (!IS_NPC(ch))
	{
	    /* no pets */
	    if (IS_SET(victim->act,ACT_PET))
	   	return TRUE;

	    /* no charmed creatures unless owner */
	    if (IS_AFFECTED(victim,AFF_CHARM) && (area || ch != victim->master))
		return TRUE;

	    /* legal kill? -- cannot hit mob fighting non-group member */
/*	    if (victim->fighting != NULL && !is_same_group(ch,victim->fighting))
		return TRUE;*/
	}
	else
	{
	    /* area effect spells do not hit other mobs */
	    if (area && !is_same_group(victim,ch->fighting))
		return TRUE;
	}
    }
    /* killing players */
    else
    {
	if (area && IS_IMMORTAL(victim) && victim->level >= LEVEL_IMMORTAL)
	    return TRUE;

	/* NPC doing the killing */
	if (IS_NPC(ch))
	{
	    /* charmed mobs and pets cannot attack players while owned */
	    if (((IS_AFFECTED(ch,AFF_CHARM)) & (ch->master != NULL))
	    &&  (ch->master->fighting != victim))
		return TRUE;
	
	    /* safe room? */
	    if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))
		return TRUE;

	    /* legal kill? -- mobs only hit players grouped with opponent*/
	    if (ch->fighting != NULL && !is_same_group(ch->fighting,victim))
		return TRUE;

            if(has_killed_flag(victim))
                return TRUE;
/*            if(victim->level>101)
                return TRUE;*/

	}

	/* player doing the killing */
	else
	{
	    DESCRIPTOR_DATA *d;
	    int ccount=0,vcount=0;
            if (IS_SET(victim->act,PLR_TWIT) )
		return FALSE;
           
            if (IS_SET(victim->in_room->room_flags,ROOM_ARENA))
               return FALSE;

            if(is_voodood(ch,victim))
		return FALSE;

            if(global_quest==QUEST_CLOSED && victim->on_quest &&
              ch->on_quest && IS_SET(quest_flags,QUEST_COMBAT)
              && victim->in_room->vnum!=516)
              return FALSE;

            if(is_same_group(ch,victim)&&area)
               return TRUE;

            if (IS_SET(victim->in_room->room_flags,ROOM_SAFE)
                && (game_state!=GS_PARADOX))
                return TRUE;

	    if (ch->on_quest)
	    	return TRUE;
	
	    if (victim->on_quest)
	    	return TRUE;
	

	    if (is_same_clan(ch,victim))
		return TRUE;

            if (!in_range(ch,victim))
                return TRUE;

/*            if (victim->fighting != NULL && !IS_NPC(victim->fighting))
               return TRUE;*/

            if(has_killed_flag(victim))
             return TRUE;
/*            if(victim->level>101)
             return TRUE;*/
             for(d=descriptor_list;d!=NULL;d=d->next)
            {
             if((d->connected==CON_PLAYING)
                && (d->character != ch)
                && (d->character->fighting!=NULL)
                && (d->character->fighting==victim)
                && (d->character->in_room==victim->in_room))
                   vcount++;
	     if((d->connected==CON_PLAYING)
                && (d->character != victim)
                && (d->character->fighting!=NULL)
                && (d->character->fighting==ch)
                && (d->character->in_room==ch->in_room))
                   ccount++;
            }
            switch(game_state)
            {
             default:
             case GS_CHAOS:
             case GS_PARADOX: return FALSE;
             case GS_SOLO: if(vcount>=1)
                            return TRUE;
                           if(ccount>=1)
                            return TRUE;
                           return FALSE;
             case GS_1VS2: if(vcount>=2)
                            return TRUE;
                           if(ccount>=2)
                            return TRUE;
                           return FALSE;
            }

	}
    }
    return FALSE;
}

/*
 * Check for parry.
 */
bool check_parry( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    chance = get_skill(victim,gsn_parry) / 4;

    if ( get_eq_char( victim, WEAR_WIELD ) == NULL )
    {
	if (IS_NPC(victim))
	    chance /= 2;
	else
	    return FALSE;
    }

    if (victim->stunned)
        return FALSE;

    if (!can_see(ch,victim))
	chance /= 2;

    if ( number_percent( ) >= chance + victim->level - ch->level )
	return FALSE;

    xact_new( "{iYou parry $n's attack.{x",  ch, NULL, victim, TO_VICT,POS_RESTING,SUPPRESS_DODGE);
    xact_new( "{h$N parries your attack.{x", ch, NULL, victim, TO_CHAR,POS_RESTING,SUPPRESS_DODGE);
    check_improve(victim,gsn_parry,TRUE,6);
    return TRUE;
}

/*
 * Check for shield block.
 */
bool check_shield_block( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;
    bool levitate;

    if ( !IS_AWAKE(victim) )
        return FALSE;

    levitate = (get_skill(victim,gsn_shield_levitation)!=0)
      && (skill_table[gsn_shield_levitation].skill_level[ch->class]<=ch->level)
      && (get_eq_char(victim,WEAR_SHIELD)!=NULL)
      && ((get_eq_char(victim,WEAR_SECONDARY)!=NULL)
         || ((get_eq_char(victim,WEAR_WIELD)!=NULL)
             && (get_eq_char(victim,WEAR_SECONDARY)==NULL)
             && IS_WEAPON_STAT(get_eq_char(victim,WEAR_WIELD),WEAPON_TWO_HANDS)));

    chance = get_skill(victim,gsn_shield_block) / 5 + 3;
    if(levitate)
      chance = (chance+(get_skill(victim,gsn_shield_levitation)/5+3))/2;

    if ( get_eq_char( victim, WEAR_SHIELD ) == NULL )
        return FALSE;

    if ( number_percent( ) >= chance + victim->level - ch->level )
        return FALSE;

    if (victim->stunned)
        return FALSE;

    xact_new( "{iYou block $n's attack with your shield.{x",  ch, NULL, victim,
TO_VICT,POS_RESTING,SUPPRESS_DODGE    );
    xact_new( "{h$N blocks your attack with a shield.{x", ch, NULL, victim,
TO_CHAR,POS_RESTING,SUPPRESS_DODGE    );
    check_improve(victim,gsn_shield_block,TRUE,6);
    if(levitate)
     check_improve(victim,gsn_shield_levitation,TRUE,6);
    return TRUE;
}


/*
 * Check for dodge.
 */
bool check_dodge( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    chance = get_skill(victim,gsn_dodge) / 4;
   
    if(is_affected(victim,gsn_essence_of_mist))
      chance *= 2;

    if (!can_see(victim,ch))
	chance /= 2;

    if ( number_percent( ) >= chance + victim->level - ch->level )
        return FALSE;

    if (victim->stunned)
        return FALSE;

    xact_new( "{iYou dodge $n's attack.{x", ch, NULL, victim, TO_VICT,POS_RESTING,SUPPRESS_DODGE    );
    xact_new( "{h$N dodges your attack.{x", ch, NULL, victim, TO_CHAR,POS_RESTING,SUPPRESS_DODGE    );
    check_improve(victim,gsn_dodge,TRUE,6);
    return TRUE;
}

bool check_armor(CHAR_DATA *ch,CHAR_DATA *victim, int dam)
{
 AFFECT_DATA *pAf;

 if((pAf=affect_find(victim->affected,skill_lookup("immaterial armor")))!=NULL)
 {
  if((dam<(pAf->level/2)) && ((number_percent()<(pAf->level/2-dam))) && (number_percent()<40))
  {
   xact_new( "{iYour immaterial armor absorbs $n's attack.{x",ch,NULL,victim,TO_VICT,POS_RESTING,SUPPRESS_DODGE);
   xact_new( "{h$N's immaterial armor absorbs your attack.{x",ch,NULL,victim,TO_CHAR,POS_RESTING,SUPPRESS_DODGE);
   pAf->subtype-=dam;
   if(pAf->subtype<=0)
   {
    send_to_char("{xYour suit of immaterial armor overloads and vanishes!\n\r",victim);
    affect_strip(victim,skill_lookup("immaterial armor"));
   }
   return TRUE;
  }
 }
 return FALSE;
}

bool check_shield(CHAR_DATA *ch,CHAR_DATA *victim, int dam)
{
 AFFECT_DATA *pAf;
 static bool bFire=FALSE;
 
 if(ch->fighting!=victim)
  return FALSE;
 
 if((pAf=affect_find(victim->affected,skill_lookup("force shield")))!=NULL)
 {
  if((dam<(pAf->level)) && ((number_percent()<(pAf->level-dam))) && (number_percent()<40))
  {
   xact_new( "{iYour force shield deflects $n's attack.{x",ch,NULL,victim,TO_VICT,POS_RESTING,SUPPRESS_DODGE);
   xact_new( "{h$N's force shield deflects your attack.{x",ch,NULL,victim,TO_CHAR,POS_RESTING,SUPPRESS_DODGE);
   if(number_percent()<10)
   {
    if(!bFire)
    {
     bFire=TRUE;
     act("{iYour force shield backfires!{x",ch,NULL,victim,TO_VICT);
     act("{hN's force shield backfires!{x",ch,NULL,victim,TO_CHAR);
     damage(victim,ch,dam,gsn_shield,
	    DAM_OTHER,TRUE);
    } 
   }
   else
    bFire=FALSE;
   pAf->subtype--;
   if(pAf->subtype<=0)
   {
    send_to_char("{xYour force shield flashes brightly and vanishes!\n\r",victim);
    affect_strip(victim,skill_lookup("force shield"));
   }
   return TRUE;
  }
 }
 return FALSE;
}

bool check_stone_skin(CHAR_DATA *ch,CHAR_DATA *victim, int dam)
{
 AFFECT_DATA *pAf;

 if((pAf=affect_find(victim->affected,skill_lookup("stone skin")))!=NULL)
 {
  if((dam<(pAf->level*2)) && ((number_percent()<((pAf->level*2)-dam))) && (number_percent()<40))
  {
   xact_new( "{i$n's attack bounces off your stone skin.{x",ch,NULL,victim,TO_VICT,POS_RESTING,SUPPRESS_DODGE);
   xact_new( "{hYour attack bounces off $N's stone skin.{x",ch,NULL,victim,TO_CHAR,POS_RESTING,SUPPRESS_DODGE);
   pAf->subtype--;
   if(pAf->subtype<=0)
   {
    send_to_char("{xYour stone skin crumbles away.\n\r",victim);
    affect_strip(victim,skill_lookup("stone skin"));
   }
   return TRUE;
  }
 }
 return FALSE;
}

bool check_phase(CHAR_DATA *ch,CHAR_DATA *victim)
{
 int chance;

 if(IS_NPC(victim))
  return FALSE;
 
 if(!IS_AWAKE(victim))
  return FALSE;
 
 if(victim->level<skill_table[gsn_phase].skill_level[victim->class])
  return FALSE;

 chance = get_skill(victim,gsn_phase)/4;
 
 if(!can_see(victim,ch))
  chance/=2;
 
 if(number_percent() >= (chance + victim->level - ch->level))
  return FALSE;
 
 if(victim->stunned)
  return FALSE;

 xact_new("{iYou phase to avoid $n's attack.{x",ch,NULL,victim,TO_VICT,POS_RESTING,SUPPRESS_DODGE);
 xact_new("{h$N phases to avoid your attack.{x",ch,NULL,victim,TO_CHAR,POS_RESTING,SUPPRESS_DODGE);
 check_improve(victim,gsn_phase,TRUE,6);

 return TRUE;
}

bool check_force(CHAR_DATA *ch,	CHAR_DATA *victim)
{
 int chance;

 if(IS_NPC(victim))
  return FALSE;
 if(!is_affected(victim,gsn_protective_force))
  return FALSE;
 chance = 50 + victim->level - ch->level;
 if(number_percent() < chance)
 {
  send_to_char("You are protected by the force of nature.\n\r",victim);
  act("$n is protected by the force of nature.",victim,NULL,NULL,TO_ROOM);
  return TRUE;
 }
 return FALSE;
}

bool check_critical_hit(CHAR_DATA *ch, CHAR_DATA *victim)
{
 int chance;

 if(IS_NPC(ch))
  return FALSE;

 chance = get_skill(ch, gsn_critical_hit) / 6;
 if(!chance)
  return FALSE;
  
 if(!can_see(ch,victim))
   chance /= 2;

 if( number_percent() >= chance + ch->level - victim->level )
  return FALSE; 
 
 send_to_char("{BYou score a {R-{D={W* {RCRITICAL HIT {W*{D={R-{B!{x\n\r",ch);
 act( "{c$n scores a {R-{D={W* {RCRITICAL HIT {W*{D={R-{c!{x",
    ch,NULL,victim,TO_VICT);
 act( "{x$n scores a {R-{D={W* {RCRITICAL HIT {W*{D={R-{x!",
    ch,NULL,victim,TO_NOTVICT);
 check_improve(ch,gsn_critical_hit,TRUE,6); 
 return TRUE;    
}

/*
 * Set position of a victim.
 */
void update_pos( CHAR_DATA *victim )
{
    if ( victim->hit > 0 )
    {
    	if ( victim->position <= POS_STUNNED )
	    victim->position = POS_STANDING;
	return;
    }

    if ( IS_NPC(victim) && victim->hit < 1 )
    {
	victim->position = POS_DEAD;
	return;
    }

    if ( victim->hit <= -11 )
    {
	victim->position = POS_DEAD;
	return;
    }

         if ( victim->hit <= -6 ) victim->position = POS_MORTAL;
    else if ( victim->hit <= -3 ) victim->position = POS_INCAP;
    else                          victim->position = POS_STUNNED;

    return;
}



/*
 * Start fights.
 */
void set_fighting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->fighting != NULL )
    {
	bug( "Set_fighting: already fighting", 0 );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_SLEEP) )
    {
      if( is_affected(ch,gsn_sleep))
	affect_strip( ch, gsn_sleep );
      else if (is_affected(ch,gsn_blackjack))
        affect_strip(ch, gsn_blackjack );
    }

    ch->fighting = victim;
    ch->position = POS_FIGHTING;
    ch->stunned = 0;

    if(has_killed_flag(ch))
     remove_killed_flag(ch);

    if(!IS_NPC(ch)&&!IS_NPC(victim))
    {
     ch->pcdata->pkflag=10;
     victim->pcdata->pkflag=10;
    }

    return;
}



/*
 * Stop fights.
 */
void stop_fighting( CHAR_DATA *ch, bool fBoth )
{
    CHAR_DATA *fch;
    char buf[MAX_STRING_LENGTH];

    for ( fch = char_list; fch != NULL; fch = fch->next )
    {
	if ( fch == ch || ( fBoth && fch->fighting == ch ) )
	{
	    fch->fighting	= NULL;
	    fch->position	= IS_NPC(fch) ? fch->default_pos : POS_STANDING;
	    fch->stunned	= 0;
	    update_pos( fch );
	    if (IS_SET(fch->comm,COMM_STORE))
		if(fch->tells)
		{
		    sprintf( buf, "You have {R%d{x tells waiting.\n\r", fch->tells );
		    send_to_char( buf, fch );
		    send_to_char("Type 'replay' to see tells.\n\r",fch);
		}
	}
    }

    return;
}



/*
 * Make a corpse out of a character.
 */
void make_corpse( CHAR_DATA *ch, CHAR_DATA *killer, int lootpoint )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    char *name;

    if ( IS_NPC(ch) )
    {
	if (IS_SET(ch->act, ACT_NO_BODY) )
	{
	    if (IS_SET(ch->act, ACT_NB_DROP) )
	    {
		for ( obj = ch->carrying; obj != NULL; obj = obj_next )
		{
		    obj_next = obj->next_content;
		    obj_from_char( obj );
		    if (obj->item_type == ITEM_POTION)
			obj->timer = number_range(500,1000);
		    if (obj->item_type == ITEM_SCROLL)
			obj->timer = number_range(1000,2500);
		    if (IS_SET(obj->extra_flags,ITEM_ROT_DEATH))
		    {
			obj->timer = number_range(5,10);
			REMOVE_BIT(obj->extra_flags,ITEM_ROT_DEATH);
		    }
		    REMOVE_BIT(obj->extra_flags,ITEM_VIS_DEATH);

		    if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
			extract_obj( obj );
		    act("$p falls to the floor.",ch,obj,NULL,TO_ROOM);
		    obj_to_room(obj,ch->in_room);
		}
	    }
	    return;
	}
	name		= ch->short_descr;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);
	corpse->timer	= number_range( 3, 6 );
	if ( ch->gold > 0 || ch->platinum > 0 )
	{
	    obj_to_obj( create_money( ch->platinum, ch->gold, ch->silver ), corpse );
	    ch->platinum = 0;
	    ch->gold = 0;
	    ch->silver = 0;
	}
	corpse->cost = 0;
    }
    else
    {
	name		= ch->name;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
	corpse->timer	= /*number_range( 25, 40 )*/30;
	REMOVE_BIT(ch->act,PLR_CANLOOT);
	/*if (!is_clan(ch))
	{
	    corpse->owner = str_dup(ch->name);
	    corpse->killer = NULL;
	}
	else
	{*/
	    corpse->owner = str_dup(ch->name);
	    corpse->killer = str_dup(killer->name);
	    if (ch->platinum > 1 || ch->gold > 1 || ch->silver > 1)
	    {
		obj_to_obj(create_money(ch->platinum/2, ch->gold / 2, ch->silver/2), corpse);
		ch->platinum -= ch->platinum/2;
		ch->gold -= ch->gold/2;
		ch->silver -= ch->silver/2;
	    }
/*	}*/
		
	corpse->cost = 0;
    }

    corpse->level = ch->level;

    sprintf( buf, corpse->short_descr, name );
    free_string( corpse->short_descr );
    corpse->short_descr = str_dup( buf );

    sprintf( buf, corpse->description, name );
    free_string( corpse->description );
    corpse->description = str_dup( buf );

    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
	bool floating = FALSE;

	obj_next = obj->next_content;
	if (obj->wear_loc == WEAR_FLOAT)
	    floating = TRUE;
	obj_from_char( obj );
	if (obj->item_type == ITEM_POTION)
	    obj->timer = number_range(500,1000);
	if (obj->item_type == ITEM_SCROLL)
	    obj->timer = number_range(1000,2500);
	if (IS_SET(obj->extra_flags,ITEM_ROT_DEATH) && !floating)
	{
	    obj->timer = number_range(5,10);
	    REMOVE_BIT(obj->extra_flags,ITEM_ROT_DEATH);
	}
	REMOVE_BIT(obj->extra_flags,ITEM_VIS_DEATH);

	if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
	    extract_obj( obj );
	else if (floating && !IS_OBJ_STAT(obj,ITEM_QUEST) && !IS_NPC(killer))
	{
	    if (IS_OBJ_STAT(obj,ITEM_ROT_DEATH)) /* get rid of it! */
	    { 
		if (obj->contains != NULL)
		{
		    OBJ_DATA *in, *in_next;

		    act("$p evaporates,scattering its contents.",
			ch,obj,NULL,TO_ROOM);
		    for (in = obj->contains; in != NULL; in = in_next)
		    {
			in_next = in->next_content;
			obj_from_obj(in);
			obj_to_room(in,ch->in_room);
		    }
		 }
		 else
		    act("$p evaporates.",
			ch,obj,NULL,TO_ROOM);
		 extract_obj(obj);
	    }
	    else
	    {
		act("$p falls to the floor.",ch,obj,NULL,TO_ROOM);
		obj_to_room(obj,ch->in_room);
	    }
	}
	else
	    obj_to_obj( obj, corpse );
    }
    corpse->lootpoint=lootpoint;
    obj_to_room( corpse, ch->in_room );
    return;
}



/*
 * Improved Death_cry contributed by Diavolo.
 */
void death_cry( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA *was_in_room;
    char *msg;
    int door;
    int vnum;

    vnum = 0;
    msg = "You hear $n's death cry.";
    if (IS_NPC(ch))
    {
	if (!IS_SET(ch->act, ACT_NO_BODY) )
	{
	    switch ( number_bits(4))
	    {
	    case  0:
		msg  = "$n hits the ground ... DEAD.";
		vnum = OBJ_VNUM_BLOOD;
		break;
	    case  1:
		msg  = "$n splatters blood on your armor.";
		vnum = OBJ_VNUM_BLOOD;
		break;
	    case  2:
		if (IS_SET(ch->parts,PART_GUTS))
		{
		    msg = "$n spills $s guts all over the floor.";
		    vnum = OBJ_VNUM_GUTS;
		}
		break;
	    case  3: 
		if (IS_SET(ch->parts,PART_HEAD))
		{
		    msg  = "$n's severed head plops on the ground.";
		    vnum = OBJ_VNUM_SEVERED_HEAD;
		}
		break;
	    case  4:
		if (IS_SET(ch->parts,PART_HEART))
		{
		    msg  = "$n's heart is torn from $s chest.";
		    vnum = OBJ_VNUM_TORN_HEART;
		}
		break;
	    case  5:
		if (IS_SET(ch->parts,PART_ARMS))
		{
		    msg  = "$n's arm is sliced from $s dead body.";
		    vnum = OBJ_VNUM_SLICED_ARM;
		}
		break;
	    case  6:
		if (IS_SET(ch->parts,PART_LEGS))
		{
		    msg  = "$n's leg is sliced from $s dead body.";
		    vnum = OBJ_VNUM_SLICED_LEG;				
		}
		break;
	    case 7:
		if (IS_SET(ch->parts,PART_BRAINS))
		{
		    msg = "$n's head is shattered, and $s brains splash all over you.";
		    vnum = OBJ_VNUM_BRAINS;
		}
		break;
	    case  8:
		msg  = "$n hits the ground ... DEAD.";
		vnum = OBJ_VNUM_BLOOD;
		break;
	    case  9:
		msg  = "$n hits the ground ... DEAD.";
		vnum = OBJ_VNUM_BLOOD;
	    }
	}
    } else if (ch->level > 19)
    {
	switch ( number_bits(4))
	{
	    case  0:
		msg  = "$n hits the ground ... DEAD.";
		vnum = OBJ_VNUM_BLOOD;
		break;
	    case  1:
		msg  = "$n splatters blood on your armor.";
		vnum = OBJ_VNUM_BLOOD;
		break;
	    case  2:
		if (IS_SET(ch->parts,PART_GUTS))
		{
		    msg = "$n spills $s guts all over the floor.";
		    vnum = OBJ_VNUM_GUTS;
		}
		break;
	    case  3:
		if (IS_SET(ch->parts,PART_HEAD))
		{
		    msg  = "$n's severed head plops on the ground.";
		    vnum = OBJ_VNUM_SEVERED_HEAD;
		}
		break;
	    case  4:
		if (IS_SET(ch->parts,PART_HEART))
		{
		    msg  = "$n's heart is torn from $s chest.";
		    vnum = OBJ_VNUM_TORN_HEART;
		}
		break;
	    case  5:
		if (IS_SET(ch->parts,PART_ARMS))
		{
		    msg  = "$n's arm is sliced from $s dead body.";
		    vnum = OBJ_VNUM_SLICED_ARM;
		}
		break;
	    case  6:
		if (IS_SET(ch->parts,PART_LEGS))
		{
		    msg  = "$n's leg is sliced from $s dead body.";
		    vnum = OBJ_VNUM_SLICED_LEG;
		}
		break;
	    case 7:
		if (IS_SET(ch->parts,PART_BRAINS))
		{
		    msg = "$n's head is shattered, and $s brains splash all over you.";
		    vnum = OBJ_VNUM_BRAINS;
		}
		break;
	    case  8:
		msg  = "$n hits the ground ... DEAD.";
		vnum = OBJ_VNUM_BLOOD;
		break;
	    case  9:
		msg  = "$n hits the ground ... DEAD.";
		vnum = OBJ_VNUM_BLOOD;
		break;
	    case  10:
		if (IS_SET(ch->parts,PART_HEAD))
		{
		    msg  = "$n's severed head plops on the ground.";
		    vnum = OBJ_VNUM_SEVERED_HEAD;
		}
		break;
	    case  11:
		if (IS_SET(ch->parts,PART_HEART))
		{
		    msg  = "$n's heart is torn from $s chest.";
		    vnum = OBJ_VNUM_TORN_HEART;
		}
		break;
	    case  12:
		if (IS_SET(ch->parts,PART_ARMS))
		{
		    msg  = "$n's arm is sliced from $s dead body.";
		    vnum = OBJ_VNUM_SLICED_ARM;
		}
		break;
	    case  13:
		if (IS_SET(ch->parts,PART_LEGS))
		{
		    msg  = "$n's leg is sliced from $s dead body.";
		    vnum = OBJ_VNUM_SLICED_LEG;
		}
		break;
	    case 14:
		if (IS_SET(ch->parts,PART_BRAINS))
		{
		    msg = "$n's head is shattered, and $s brains splash all over you.";
		    vnum = OBJ_VNUM_BRAINS;
		}
	}
    }

    act( msg, ch, NULL, NULL, TO_ROOM );

    if (( vnum == 0 ) && !IS_SET(ch->act, ACT_NO_BODY) )
    {
	switch ( number_bits(4))
	{
	case  0:
		vnum = 0;
		break;
	case  1: 
		vnum = OBJ_VNUM_BLOOD;
		break;
	case  2: 							
		vnum = 0;
		break;
	case  3: 
		vnum = OBJ_VNUM_BLOOD;				
		break;
	case  4: 
		vnum = 0;				
		break;
	case  5: 
		vnum = OBJ_VNUM_BLOOD;				
		break;
	case  6: 
		vnum = 0;				
		break;
	case 7:
		vnum = OBJ_VNUM_BLOOD;
	}
    }

    if ( vnum != 0 )
    {
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	char *name;

	name		= IS_NPC(ch) ? ch->short_descr : ch->name;
	obj		= create_object( get_obj_index( vnum ), 0 );
	obj->timer	= number_range( 4, 7 );
	if (!IS_NPC(ch))
	{
	    obj->timer = number_range( 12, 18 );
	}
	if (vnum == OBJ_VNUM_BLOOD)
	{
	    obj->timer = number_range( 1, 4 );
	}

	sprintf( buf, obj->short_descr, name );
	free_string( obj->short_descr );
	obj->short_descr = str_dup( buf );

	sprintf( buf, obj->description, name );
	free_string( obj->description );
	obj->description = str_dup( buf );

        sprintf( buf, obj->name, name );
        free_string( obj->name );
        obj->name = str_dup( buf );

	if (obj->item_type == ITEM_FOOD)
	{
	    if (IS_SET(ch->form,FORM_POISON))
		obj->value[3] = 1;
	    else if (!IS_SET(ch->form,FORM_EDIBLE))
		obj->item_type = ITEM_TRASH;
	}

	if (IS_NPC(ch))
	{
	    obj->value[4] = 0;
	} else {
	    obj->value[4] = 1;
	}

	obj_to_room( obj, ch->in_room );
    }

    if ( IS_NPC(ch) )
	msg = "You hear something's death cry.";
    else
	msg = "You hear someone's death cry.";

    was_in_room = ch->in_room;
    for ( door = 0; door <= 5; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = was_in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   pexit->u1.to_room != was_in_room )
	{
	    ch->in_room = pexit->u1.to_room;
	    act( msg, ch, NULL, NULL, TO_ROOM );
	}
    }
    ch->in_room = was_in_room;

    return;
}



void raw_kill( CHAR_DATA *victim, CHAR_DATA *killer, int lootpoint )
{
    int i;

    death_cry( victim );
    stop_fighting( victim, TRUE );
    make_corpse( victim, killer, lootpoint );

    if ( IS_NPC(victim) )
    {
	victim->pIndexData->killed++;
	kill_table[URANGE(0, victim->level, MAX_LEVEL-1)].killed++;
	extract_char( victim, TRUE );
	return;
    }

    extract_char( victim, FALSE );
    while ( victim->affected )
	affect_remove( victim, victim->affected );
    victim->affected_by	= race_table[victim->race].aff;
    victim->shielded_by	= race_table[victim->race].shd;
    for (i = 0; i < 4; i++)
    	victim->armor[i]= 100;
    victim->position	= POS_RESTING;
    victim->hit		= UMAX( 1, victim->hit  );
    victim->mana	= UMAX( 1, victim->mana );
    victim->move	= UMAX( 1, victim->move );
    WAIT_STATE(victim,24);
/*  save_char_obj( victim ); we're stable enough to not need this :) */
    return;
}



void group_gain( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;
    CHAR_DATA *lch;
    int xp;
    int members;
    int group_levels;

    /*
     * Monsters don't get kill xp's or alignment changes.
     * P-killing doesn't help either.
     * Dying of mortal wounds or poison doesn't give xp to anyone!
     */
    if ( victim == ch )
	return;
    
    members = 0;
    group_levels = 0;
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ) )
        {
	    members++;
	    group_levels += IS_NPC(gch) ? gch->level / 2 : gch->level;
	}
    }

    if ( members == 0 )
    {
	bug( "Group_gain: members.", members );
	members = 1;
	group_levels = ch->level ;
    }

    lch = (ch->leader != NULL) ? ch->leader : ch;

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	if ( !is_same_group( gch, ch ) || IS_NPC(gch))
	    continue;

/*	if ( gch->level - lch->level >= 5 )
	{
	    send_to_char( "You are too high for this group.\n\r", gch );
	    continue;
	}

	if ( gch->level - lch->level <= -5 )
	{
	    send_to_char( "You are too low for this group.\n\r", gch );
	    continue;
	}*/

	xp = xp_compute( gch, victim, group_levels,members );  
	sprintf( buf, "{BYou receive {W%d{B experience points.{x\n\r", xp );
	send_to_char( buf, gch );
	gain_exp( gch, xp );

	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( obj->wear_loc == WEAR_NONE )
		continue;

	    if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)    )
	    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)    )
	    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch) ) )
	    {
		act( "{cYou are {Wzapped{c by $p.{x", ch, obj, NULL, TO_CHAR );
		act( "$n is {Wzapped{x by $p.",   ch, obj, NULL, TO_ROOM );
		obj_from_char( obj );
		obj_to_room( obj, ch->in_room );
	    }
	}
        if(is_affected(ch,gsn_divine_blessing)&&!IS_GOOD(ch))
        {
         affect_strip(ch,gsn_divine_blessing);
         send_to_char("You have angered the gods and your shimmering aura vanishes.\n\r",ch);
        }
        if(is_affected(ch,gsn_infernal_offering)&&!IS_EVIL(ch))
        {
         affect_strip(ch,gsn_infernal_offering);
         send_to_char("You have angered the gods and the infernal power leaves you.\n\r",ch);
        }
        if(IS_SET(ch->exbit1,PLR_QUESTOR) && IS_NPC(victim))
        {
         if(ch->questmob==victim->pIndexData->vnum)
         {
          send_to_char("You have almost completed your quest!\n\r",ch);
          send_to_char("Return to the questmaster before your time runs out!\n\r",ch);
         ch->questmob=-1;
        }
       }
    }

    return;
}



/*
 * Compute xp for a kill.
 * Also adjust alignment of killer.
 * Edit this function to change xp computations.
 */
int xp_compute( CHAR_DATA *gch, CHAR_DATA *victim, int total_levels, int members )
{
    int xp,base_exp;
    int align,level_range;
    int change;
    int time_per_level;

    level_range = victim->level - gch->level;
   /* if (!IS_NPC(gch))
    {
	if (gch->class >= MAX_CLASS/2)
	    level_range -= 5;
    }*/
 
    if (!IS_NPC(gch) && !IS_NPC(victim))
    {
	xp = 1;
	return xp;
    }
    /* compute the base exp */
    switch (level_range)
    {
 	default : 	base_exp =   0;		break;
	case -9 :	base_exp =   1;		break;
	case -8 :	base_exp =   2;		break;
	case -7 :	base_exp =   5;		break;
	case -6 : 	base_exp =   9;		break;
	case -5 :	base_exp =  11;		break;
	case -4 :	base_exp =  22;		break;
	case -3 :	base_exp =  33;		break;
	case -2 :	base_exp =  50;		break;
	case -1 :	base_exp =  66;		break;
	case  0 :	base_exp =  83;		break;
	case  1 :	base_exp =  99;		break;
	case  2 :	base_exp = 121;		break;
	case  3 :	base_exp = 143;		break;
	case  4 :	base_exp = 165;		break;
    } 
    
    if (level_range > 4)
	base_exp = 160 + 20 * (level_range - 4);

    /* do alignment computations */
   
    align = victim->alignment - gch->alignment;

    if (IS_SET(victim->act,ACT_NOALIGN))
    {
	/* no change */
    }

    else if (align > 500) /* monster is more good than slayer */
    {
	change = (align - 500) * base_exp / 500 * gch->level/total_levels; 
	change = UMAX(1,change);
        gch->alignment = UMAX(-1000,gch->alignment - change);
	if ( gch->pet != NULL )
	    gch->pet->alignment = gch->alignment;
    }

    else if (align < -500) /* monster is more evil than slayer */
    {
	change =  ( -1 * align - 500) * base_exp/500 * gch->level/total_levels;
	change = UMAX(1,change);
	gch->alignment = UMIN(1000,gch->alignment + change);
        if ( gch->pet != NULL )
            gch->pet->alignment = gch->alignment;
    }

    else /* improve this someday */
    {
	change =  gch->alignment * base_exp/500 * gch->level/total_levels;  
	gch->alignment -= change;
        if ( gch->pet != NULL )
            gch->pet->alignment = gch->alignment;
    }
    
    /* calculate exp multiplier */
    if (IS_SET(victim->act,ACT_NOALIGN))
	xp = base_exp;

    else if (gch->alignment > 500)  /* for goodie two shoes */
    {
	if (victim->alignment < -750)
	    xp = (base_exp *4)/3;
   
 	else if (victim->alignment < -500)
	    xp = (base_exp * 5)/4;

        else if (victim->alignment < -250)
	    xp = (base_exp * 3)/4; 

        else if (victim->alignment > 750)
	    xp = base_exp / 4;

   	else if (victim->alignment > 500)
	    xp = base_exp / 2;

	else
	    xp = base_exp;
    }

    else if (gch->alignment < -500) /* for baddies */
    {
	if (victim->alignment > 750)
	    xp = (base_exp * 5)/4;
	
  	else if (victim->alignment > 500)
	    xp = (base_exp * 11)/10; 

   	else if (victim->alignment < -750)
	    xp = base_exp/2;

	else if (victim->alignment < -500)
	    xp = (base_exp * 3)/4;

	else if (victim->alignment < -250)
	    xp = (base_exp * 9)/10;

	else
	    xp = base_exp;
    }

    else if (gch->alignment > 200)  /* a little good */
    {

	if (victim->alignment < -500)
	    xp = (base_exp * 6)/5;

 	else if (victim->alignment > 750)
	    xp = base_exp/2;

	else if (victim->alignment > 0)
	    xp = (base_exp * 3)/4; 
	
	else
	    xp = base_exp;
    }

    else if (gch->alignment < -200) /* a little bad */
    {
	if (victim->alignment > 500)
	    xp = (base_exp * 6)/5;
 
	else if (victim->alignment < -750)
	    xp = base_exp/2;

	else if (victim->alignment < 0)
	    xp = (base_exp * 3)/4;

	else
	    xp = base_exp;
    }

    else /* neutral */
    {

	if (victim->alignment > 500 || victim->alignment < -500)
	    xp = (base_exp * 4)/3;

	else if (victim->alignment < 200 && victim->alignment > -200)
	    xp = base_exp/2;

 	else
	    xp = base_exp;
    }

    /* more exp at the low levels */
    if (gch->level < 11)
    	xp = 15 * xp / (gch->level + 4);

    /* less at high */
/*    if (gch->level > 60 )
	xp =  15 * xp / (gch->level - 25 );*/

    /* reduce for playing time */

    
    {
	/* compute quarter-hours per level */
	time_per_level = 4 *
			 (gch->played + (int) (current_time - gch->logon))/3600
			 / gch->level;

	time_per_level = URANGE(2,time_per_level,12);
	if (gch->level < 25)  /* make it a curve */
	    time_per_level = UMAX(time_per_level,(25 - gch->level));
/*
 *	xp = xp * time_per_level / 12;
 */
    }
    xp = xp*.7;
   
    /* randomize the rewards */
    xp = number_range (xp * 3/4, xp * 5/4);

    /* adjust for grouping */
    /*xp*=members;
    xp = xp * gch->level/( UMAX(1,total_levels -1) );*/
    
    if(xp<=0)
     xp=1;
    if(gch->level<=62)
     xp*=2;
    return xp;
}

void dam_message( CHAR_DATA *ch, CHAR_DATA *victim,int dam,int dt,bool immune, int suppress, bool singular )
{
    char buf1[256], buf2[256], buf3[256];
    const char *vs;
    const char *vp;
    char *attack;
    char punct;

    if (ch == NULL || victim == NULL)
	return;

	 if ( dam ==   0 ) { vs = "miss";	vp = "misses";		}
    else if ( dam <=   10 ) { vs = "{x{gtickle{B";	vp = "{x{gtickles{B";	}
    else if ( dam <=   20 ) { vs = "{x{gscrape{B";	vp = "{x{gscrapes{B";		}
    else if ( dam <=  30 ) { vs = "{x{rscratch{B";	vp = "{x{rscratches{B";		}
    else if ( dam <=  40 ) { vs = "{x{bcut{B";	vp = "{x{bcuts{B";		}
    else if ( dam <=  50 ) { vs = "{x{cwound{B";	vp = "{x{cwounds{B";		}
    else if ( dam <=  60 ) { vs = "{x{Rm{yau{Rl{B";       vp = "{x{Rm{yaul{Rs{B";		}
    else if ( dam <=  70 ) { vs = "{x{bt{Boas{bt{B";	vp = "{x{bt{Boast{bs{B";	}
    else if ( dam <=  80 ) { vs = "{x{gp{Gumme{gl{B";	vp = "{x{gp{Gummel{gs{B";	}
    else if ( dam <=  100 ) { vs = "{x{mDemolish{B";	vp = "{x{mDemolishes{B";		}
    else if ( dam <=  120 ) { vs = "{x{RMUTILATE{B";	vp = "{x{RMUTILATES{B";	}
    else if ( dam <=  140 ) { vs = "{x{GWHACK{B";	vp = "{x{GWHACKS{B";	}
    else if ( dam <=  160 ) { vs = "{x{YDISMEMBER{B";	vp = "{x{YDISMEMBERS{B";	}
    else if ( dam <=  180 ) { vs = "{x{bSlice{x-N-{rDice{B";	vp = "{x{bSlices{x-N-{rDices{B";	}
    else if ( dam <=  200 ) { vs = "{x{yATOMIZE{B";	vp = "{x{yATOMIZES{B";		}
    else if ( dam <=  225 ) { vs = "{x{R<> {YLACERATE{R <>{B";
			     vp = "{x{R<> {YLACERATES{R <>{B";			}
    else if ( dam <=  250 ) { vs = "{x{r*{b-{r* {GGiNSu {r*{b-{r*{B";
			     vp = "{x{r*{b-{r* {GGiNSuS {r*{b-{r*{B";			}
    else if ( dam <= 275)  { vs = "{x{B<-> {YPULVERIZE {B<->{B";
			     vp = "{x{B<-> {YPULVERIZES {B<->{B";		}
    else if ( dam <= 300)  { vs = "{x{c*{b-{c* {BVoRPalIZE {c*{b-{c* {B";
			     vp = "{x{c*{b-{c* {BVoRPalIZEs {c*{b-{c* {B";		}
    else if ( dam <= 350)  { vs = "{x{M<{R-{M*{R-{M> {RDISINTEGRATE {M<{R-{M*{R-{M>{B";
			     vp = "{x{M<{R-{M*{R-{M> {RDISINTEGRATES {M<{R-{M*{R-{M>{B";			}
    else if ( dam <= 450)  { vs = "{x{B={D+{B= {rSLauGHter {B={D+{B={B";
                             vp = "{x{B={D+{B= {rSLauGHterS {B={D+{B={B"; }
    else if ( dam <= 500)  { vs = "{x{G<--> {YVAPORIZE {G<-->{B";
                             vp = "{x{G<--> {YVAPORIZES {G<-->{B"; }
    else if ( dam <= 550)  { vs = "{x{g>{R>{g> {WCASTRATE {g<{R<{g<{B";
                             vp = "{x{g>{R>{g> {WCASTRATES {g<{R<{g<{B"; }
    else if ( dam <= 600)  { vs = "{x{r<{R<{Y-{r*{Y-{R>{r> {RI{YN{RC{YI{RN{YE{RR{YA{RT{YE {r<{R<{Y-{r*{Y-{R>{r>{B";
                             vp = "{x{r<{R<{Y-{r*{Y-{R>{r> {RI{YN{RC{YI{RN{YE{RR{YA{RT{YE{RS {r<{R<{Y-{r*{Y-{R>{r>{B"; }  
                         
    else if ( dam <= 650) { vs = "{x{y>{x+{y> {RP{xileDrive {y<{x+{y<{B";
                             vp = "{x{y>{x+{y> {RP{xileDrives {y<{x+{y<{B"; }
    else if ( dam <= 700) { vs = "{x{m<<{R<{W+{R-{W*{G-{W+{R>{m>> {RP{WU{RN{WC{RT{WU{RR{WE {m<<{R<{W+{R-{W*{Y-{W+{R>{m>>{B";
                             vp = "{x{m<<{R<{W+{R-{W*{G-{W+{R>{m>> {RP{WU{RN{WC{RT{WU{RR{WE{RS {m<<{R<{W+{R-{W*{Y-{W+{R>{m>>{B"; }
    else if ( dam <= 750){ vs = "{x{Y<{B<{R<{W+-*{Y-{W+{R>{B>{Y> {gC{WR{gU{WC{gI{WF{gY {Y<{B<{R<{W+{B-{W*-+{R>{B>{Y>{B";
                             vp = "{x{Y<{B<{R<{W+-*{Y-{W+{R>{B>{Y> {gC{WR{gU{WC{gI{WF{gI{WE{gS {Y<{B<{R<{W+{B-{W*-+{R>{B>{Y>{B"; }
    else if ( dam <= 800){ vs = "{x{B<{x-=-{B><{m-=-{B> {BR{MU{BP{BT{BU{GR{BE {B<{M-=-{B><{Y-=-{B>{B";
                             vp = "{x{B<{x-=-{B><{m-=-{B> {BR{MU{BP{BT{BU{GR{BE{GS {B<{M-=-{B><{Y-=-{B>{B"; }
    else if ( dam <= 1000) { vs = "{x{m_{r\\{R|{r/{m__{r\\{R|{r/{m_ {mE{WX{mT{WE{mR{WM{mI{WN{mA{WT{mE {m_{r\\{R|{r/{m__{r\\{R|{r/{m_{B";
                             vp = "{x{m_{r\\{R|{r/{m__{r\\{R|{r/{m_ {mE{WX{mT{WE{mR{WM{mI{WN{mA{WT{mE{WS {m_{r\\{R|{r/{m__{r\\{R|{r/{m_{B"; }
    else if ( dam <= 1500) { vs = "{x{Bdo {YGODLIKE {Bdamage to{B";
                             vp = "{x{Bdoes {YGODLIKE {Bdamage to{B"; }

    else                   { vs = "{x{g>{R>{g> {ROUCH!! {g<{R<{g<{B";
			     vp = "{x{g>{R>{g> {ROUCH!! {g<{R<{g<{B";		}

    punct   = (dam <= 100) ? '.' : '!';

    if ( dt == TYPE_HIT )
    {
	if (ch  == victim)
	{
	    sprintf( buf1, "{k$n %s {k$melf%c{x {D({WDam{D:{W%d{D){x",vp,punct,dam);
	    sprintf( buf2, "{hYou %s {hyourself%c{x {D({WDam{D:{W%d{D){x",vs,punct,dam);
	}
	else
	{
	    sprintf( buf1, "{k$n %s {k$N%c{x {D({WDam{D:{W%d{D){x",  vp, punct,dam );
	    sprintf( buf2, "{hYou %s {h$N%c{x {D({WDam{D:{W%d{D){x", vs, punct,dam );
	    sprintf( buf3, "{i$n %s {iyou%c{x {D({WDam{D:{W%d{D){x", vp, punct,dam );
	}
    }
    else
    {
	if ( dt >= 0 && dt < MAX_SKILL )
	    attack	= skill_table[dt].noun_damage;
	else if ( dt >= TYPE_HIT
	&& dt < TYPE_HIT + MAX_DAMAGE_MESSAGE) 
	    attack	= attack_table[dt - TYPE_HIT].noun;
        else if (dt==-1 && suppress==SUPPRESS_SHIELD_COMP)
        {
         if(singular)
          attack = str_dup("shield");
         else
          attack = str_dup("shields");
        }
	else
	{
	    bug( "Dam_message: bad dt %d.", dt );
	    dt  = TYPE_HIT;
	    attack  = attack_table[0].name;
	}

	if (immune)
	{
	    if (ch == victim)
	    {
		sprintf(buf1,"{k$n is unaffected by $s own %s.{x",attack);
		sprintf(buf2,"{hLuckily, you are immune to that.{x");
	    } 
	    else
	    {
	    	sprintf(buf1,"{k$N is unaffected by $n's %s!{x",attack);
	    	sprintf(buf2,"{h$N is unaffected by your %s!{x",attack);
	    	sprintf(buf3,"{i$n's %s is powerless against you.{x",attack);
	    }
	}
	else
	{
	    if (ch == victim)
	    {
		sprintf( buf1, "{k$n's %s %s {k$m%c{x {D({WDam{D:{W%d{D){x",attack,singular?vp:vs,punct,dam);
		sprintf( buf2, "{hYour %s %s {hyou%c{x {D({WDam{D:{W%d{D){x",attack,singular?vp:vs,punct,dam);
	    }
	    else
	    {
	    	sprintf( buf1, "{k$n's %s %s {k$N%c{x {D({WDam{D:{W%d{D){x",  attack, singular?vp:vs, punct,dam );
	    	sprintf( buf2, "{hYour %s %s {h$N%c{x {D({WDam{D:{W%d{D){x",  attack, singular?vp:vs, punct,dam );
	    	sprintf( buf3, "{i$n's %s %s {iyou%c{x {D({WDam{D:{W%d{D){x", attack, singular?vp:vs, punct,dam );
	    }
	}
    }

    if (ch == victim)
    {
	xact_new(buf1,ch,NULL,NULL,TO_ROOM, POS_RESTING,suppress);
	xact_new(buf2,ch,NULL,NULL,TO_CHAR, POS_RESTING,suppress);
    }
    else
    {
    	xact_new( buf1, ch, NULL, victim, TO_NOTVICT, POS_RESTING,suppress );
    	xact_new( buf2, ch, NULL, victim, TO_CHAR, POS_RESTING,suppress );
    	xact_new( buf3, ch, NULL, victim, TO_VICT, POS_RESTING,suppress );
    }
    if(dt==-1 && suppress==SUPPRESS_SHIELD_COMP)
     free_string(attack);
    return;
}



/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void disarm( CHAR_DATA *ch, CHAR_DATA *victim )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
	return;

    if ( IS_OBJ_STAT(obj,ITEM_NOREMOVE))
    {
	act("{j$S weapon won't budge!{x",ch,NULL,victim,TO_CHAR);
	act("{j$n tries to disarm you, but your weapon won't budge!{x",
	    ch,NULL,victim,TO_VICT);
	act("{k$n tries to disarm $N, but fails.{x",ch,NULL,victim,TO_NOTVICT);
	return;
    }

    act( "{j$n DISARMS you and sends your weapon flying!{x", 
	 ch, NULL, victim, TO_VICT    );
    act( "{jYou disarm $N!{x",  ch, NULL, victim, TO_CHAR    );
    act( "{k$n disarms $N!{x",  ch, NULL, victim, TO_NOTVICT );

    obj_from_char( obj );
    if ( IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_INVENTORY) 
       || IS_OBJ_STAT(obj,ITEM_QUEST) || (IS_SET(victim->in_room->room_flags,
           ROOM_ARENA)))
	obj_to_char( obj, victim );
    else
    {
	obj_to_room( obj, victim->in_room );
	if (IS_NPC(victim) && victim->wait == 0 && can_see_obj(victim,obj))
	    get_obj(victim,obj,NULL);
    }
    if((obj = get_eq_char(victim, WEAR_SECONDARY)) != NULL)
    {
     send_to_char("You wield your second weapon as your first!\n\r",victim);
     act("$n wields $s second weapon as $s first!",victim,NULL,ch,TO_ROOM);
     obj_from_char(obj);
     obj_to_char(obj,victim);
     wear_obj(victim, obj, TRUE); 
    }
    return;
}

void do_berserk( CHAR_DATA *ch, char *argument)
{
    int chance, hp_percent;

    if ((chance = get_skill(ch,gsn_berserk)) == 0
    ||  (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BERSERK))
    ||  (!IS_NPC(ch)
    &&   ch->level < skill_table[gsn_berserk].skill_level[ch->class]))
    {
	send_to_char("{hYou turn {rred{h in the face, but nothing happens.{x\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_BERSERK) || is_affected(ch,gsn_berserk)
    ||  is_affected(ch,skill_lookup("frenzy")))
    {
	send_to_char("{hYou get a little madder.{x\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CALM))
    {
	send_to_char("{hYou're feeling to mellow to berserk.{x\n\r",ch);
	return;
    }

    if (ch->mana < 50)
    {
	send_to_char("{hYou can't get up enough energy.{x\n\r",ch);
	return;
    }

    /* modifiers */

    /* fighting */
    if (ch->position == POS_FIGHTING)
	chance += 10;

    /* damage -- below 50% of hp helps, above hurts */
    hp_percent = 100 * ch->hit/ch->max_hit;
    chance += 25 - hp_percent/2;

    if (number_percent() < chance)
    {
	AFFECT_DATA af;

	WAIT_STATE(ch,PULSE_VIOLENCE);
	ch->mana -= 50;
	ch->move /= 2;

	/* heal a little damage */
	ch->hit += ch->level * 2;
	ch->hit = UMIN(ch->hit,ch->max_hit);

	send_to_char("{hYour pulse races as you are consumed by {rrage!{x\n\r",ch);
	act("{k$n gets a {cw{gi{rl{yd{k look in $s eyes.{x",ch,NULL,NULL,TO_ROOM);
	check_improve(ch,gsn_berserk,TRUE,2);

	af.where	= TO_AFFECTS;
	af.type		= gsn_berserk;
	af.level	= ch->level;
	af.duration	= number_fuzzy(ch->level / 8);
	af.modifier	= UMAX(1,ch->level/5);
	af.bitvector 	= AFF_BERSERK;

	af.location	= APPLY_HITROLL;
	affect_to_char(ch,&af);

	af.location	= APPLY_DAMROLL;
	affect_to_char(ch,&af);

	af.modifier	= UMAX(10,10 * (ch->level/5));
	af.location	= APPLY_AC;
	affect_to_char(ch,&af);
    }

    else
    {
	WAIT_STATE(ch,3 * PULSE_VIOLENCE);
	ch->mana -= 25;
	ch->move /= 2;

	send_to_char("{hYour pulse speeds up, but nothing happens.{x\n\r",ch);
	check_improve(ch,gsn_berserk,FALSE,2);
    }
}

void do_rub( CHAR_DATA *ch, char *argument)
{
 int chance;

 if ((chance = get_skill(ch,gsn_rub)) == 0
    ||  (!IS_NPC(ch)
    &&   ch->level < skill_table[gsn_rub].skill_level[ch->class]))
    {
	send_to_char("{xYou don't know the first thing about rubbing your eyes.{x\n\r",ch);
	return;
    }

 if(!IS_AFFECTED(ch,AFF_BLIND))
 {
  send_to_char("{BBut you aren't blind!{x\n\r",ch);
  return;
 }
 if(!is_affected(ch,skill_lookup("fire breath")) && !is_affected(ch,gsn_dirt))
 {
  send_to_char("{xRubbing won't help that!{x\n\r",ch);
  return;
 }
 WAIT_STATE(ch,skill_table[gsn_rub].beats);
 chance=(chance*3)/4;
 if(number_percent()>chance)
 {
  send_to_char("{RYou failed to rub your eyes clear!{x\n\r",ch);
  check_improve(ch,gsn_rub,FALSE,2);
  return;
 }
 if(is_affected(ch,skill_lookup("fire breath")))
  affect_strip(ch,skill_lookup("fire breath"));
 if(is_affected(ch,gsn_dirt))
  affect_strip(ch,gsn_dirt);
 send_to_char("{xYou rubbed your eyes clear!{x\n\r",ch);
 check_improve(ch,gsn_rub,TRUE,2);
 return;
}


void do_voodoo( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *doll;
 
    if (IS_NPC(ch))
        return;
 
    doll = get_eq_char(ch,WEAR_HOLD);
    if  (doll == NULL || (doll->pIndexData->vnum != OBJ_VNUM_VOODOO))
    {
        send_to_char("You are not holding a voodoo doll.\n\r",ch);
        return;
    }

    one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Syntax: voodoo <action>\n\r",ch);
	send_to_char("Actions: pin trip throw\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"pin"))
    {
	do_vdpi(ch,doll->name);
	return;
    }

    if (!str_cmp(arg,"trip"))
    {
	do_vdtr(ch,doll->name);
	return;
    }

    if (!str_cmp(arg,"throw"))
    {
	do_vdth(ch,doll->name);
	return;
    }

    do_voodoo(ch,"");
}

void do_vdpi( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    AFFECT_DATA af;
    bool found = FALSE;

    argument = one_argument( argument, arg1 );

    for (d = descriptor_list; d != NULL; d = d->next)
    {
	CHAR_DATA *wch;

	if (d->connected != CON_PLAYING || !can_see(ch,d->character))
	    continue;

	wch = ( d->original != NULL ) ? d->original : d->character;

	if (!can_see(ch,wch))
	    continue;

	if (!str_cmp(arg1,wch->name) && !found)
	{
	    if (IS_NPC(wch))
		continue;

	    if (IS_IMMORTAL(wch) && (wch->level > ch->level))
	    {
		send_to_char( "That's not a good idea.\n\r", ch);
		return;
	    }

	    if ((wch->level < 20) && !IS_IMMORTAL(ch))
	    {
		send_to_char( "They are a little too young for that.\n\r",ch);
		return;
	    }

	    if (IS_SHIELDED(wch,SHD_PROTECT_VOODOO))
	    {
		send_to_char( "They are still realing from a previous voodoo.\n\r",ch);
		return;
	    }

	    found = TRUE;

	    send_to_char( "You stick a pin into your voodoo doll.\n\r",ch);
	    act( "$n sticks a pin into a voodoo doll.", ch, NULL, NULL, TO_ROOM );
	    send_to_char( "{RYou double over with a sudden pain in your gut!{x\n\r",wch);
	    act( "$n suddenly doubles over with a look of extreme pain!",wch, NULL,NULL,TO_ROOM);
	    af.where      = TO_SHIELDS;
	    af.type       = skill_lookup("protection voodoo");
	    af.level      = wch->level;
	    af.duration   = 1;
	    af.location   = APPLY_NONE;
	    af.modifier   = 0;
	    af.bitvector  = SHD_PROTECT_VOODOO;
	    affect_to_char(wch,&af);
	    return;
	}
    }
    send_to_char("Your victim doesn't seem to be in the realm.\n\r",ch);
    return;
}

void do_vdtr( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    AFFECT_DATA af;
    bool found = FALSE; 
 
    argument = one_argument( argument, arg1 );
 
    for (d = descriptor_list; d != NULL; d = d->next)
    { 
	CHAR_DATA *wch; 
 
	if (d->connected != CON_PLAYING || !can_see(ch,d->character)) 
	    continue; 
 
	wch = ( d->original != NULL ) ? d->original : d->character; 
 
	if (!can_see(ch,wch)) 
	    continue; 
 
	if (!str_cmp(arg1,wch->name) && !found) 
	{ 
	    if (IS_NPC(wch)) 
		continue; 
 
	    if (IS_IMMORTAL(wch) && (wch->level > ch->level)) 
	    {         
		send_to_char( "That's not a good idea.\n\r", ch); 
		return; 
	    } 
 
	    if ((wch->level < 20) && !IS_IMMORTAL(ch))
	    {
		send_to_char( "They are a little too young for that.\n\r",ch);
		return;
	    }

	    if (IS_SHIELDED(wch,SHD_PROTECT_VOODOO))
	    {
                send_to_char( "They are still realing from a previous voodoo.\n\r",ch);
		return;
	    }

	    found = TRUE;
 
	    send_to_char( "You slam your voodoo doll against the ground.\n\r",ch); 
	    act( "$n slams a voodoo doll against the ground.", ch, NULL, NULL, TO_ROOM );
	    send_to_char( "{RYour feet slide out from under you!{x\n\r",wch);
	    send_to_char( "{RYou hit the ground face first!{x\n\r",wch);
	    act( "$n trips over $s own feet, and does a nose dive into the ground!",wch, NULL,NULL,TO_ROOM);
            af.where      = TO_SHIELDS;
            af.type       = skill_lookup("protection voodoo");
            af.level      = wch->level;
            af.duration   = 1;
            af.location   = APPLY_NONE;
            af.modifier   = 0;
            af.bitvector  = SHD_PROTECT_VOODOO;
            affect_to_char(wch,&af);
	    return;
	}
    }
    send_to_char("Your victim doesn't seem to be in the realm.\n\r",ch);
    return;
}

void do_vdth( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    AFFECT_DATA af;
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    bool found = FALSE; 
    int attempt;
 
    argument = one_argument( argument, arg1 );
 
    for (d = descriptor_list; d != NULL; d = d->next)
    { 
	CHAR_DATA *wch; 
 
	if (d->connected != CON_PLAYING || !can_see(ch,d->character)) 
	    continue; 
 
	wch = ( d->original != NULL ) ? d->original : d->character; 
 
	if (!can_see(ch,wch)) 
	    continue; 
 
	if (!str_cmp(arg1,wch->name) && !found) 
	{ 
	    if (IS_NPC(wch)) 
		continue; 
 
	    if (IS_IMMORTAL(wch) && (wch->level > ch->level)) 
	    {         
		send_to_char( "That's not a good idea.\n\r", ch); 
		return; 
	    } 
 
	    if ((wch->level < 20) && !IS_IMMORTAL(ch))
	    {
		send_to_char("They are a little too young for that.\n\r",ch);
		return;
	    }

	    if (IS_SHIELDED(wch,SHD_PROTECT_VOODOO))
	    {
                send_to_char( "They are still realing from a previous voodoo.\n\r",ch);
		return;
	    }

	    found = TRUE;
 
	    send_to_char( "You toss your voodoo doll into the air.\n\r",ch); 
	    act( "$n tosses a voodoo doll into the air.", ch, NULL, NULL, TO_ROOM );
            af.where      = TO_SHIELDS;
            af.type       = skill_lookup("protection voodoo");
            af.level      = wch->level;
            af.duration   = 1;
            af.location   = APPLY_NONE;
            af.modifier   = 0;
            af.bitvector  = SHD_PROTECT_VOODOO;
            affect_to_char(wch,&af);
	    if ((wch->fighting != NULL) || (number_percent() < 25))
	    {
		send_to_char( "{RA sudden gust of wind throws you through the air!{x\n\r",wch);
		send_to_char( "{RYou slam face first into the nearest wall!{x\n\r",wch);
		act( "A sudden gust of wind picks up $n and throws $m into a wall!",wch, NULL,NULL,TO_ROOM);
		return;
	    }
	    wch->position = POS_STANDING;
	    was_in = wch->in_room;
	    for ( attempt = 0; attempt < 6; attempt++ )
	    {
		EXIT_DATA *pexit;
		int door;

		door = number_door( );
		if ( ( pexit = was_in->exit[door] ) == 0
		||   pexit->u1.to_room == NULL
		||   IS_SET(pexit->exit_info, EX_CLOSED)
		|| ( IS_NPC(wch)
		&&   IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB) ) )
		    continue;

		move_char( wch, door, FALSE, TRUE );
		if ( ( now_in = wch->in_room ) == was_in )
		    continue;

		wch->in_room = was_in;
		sprintf(buf, "A sudden gust of wind picks up $n and throws $m to the %s.", dir_name[door]);
		act( buf, wch, NULL, NULL, TO_ROOM );
		send_to_char( "{RA sudden gust of wind throws you through the air!{x\n\r",wch);
		wch->in_room = now_in;
		act( "$n sails into the room and slams face first into a wall!",wch,NULL,NULL,TO_ROOM);
		do_look( wch, "auto" );
		send_to_char( "{RYou slam face first into the nearest wall!{x\n\r",wch);
		return;
	    }
	    send_to_char( "{RA sudden gust of wind throws you through the air!{x\n\r",wch);
            send_to_char( "{RYou slam face first into the nearest wall!{x\n\r",wch);
            act( "A sudden gust of wind picks up $n and throws $m into a wall!",wch, NULL,NULL,TO_ROOM);
            return;
	}
    }
    send_to_char("Your victim doesn't seem to be in the realm.\n\r",ch);
    return;
}

void do_bash( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;
    bool fWolf=FALSE;

    one_argument(argument,arg);
    fWolf = is_affected(ch,gsn_essence_of_wolf);

    if ( ((chance = get_skill(ch,gsn_bash)) == 0
    ||	 (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BASH))
    ||	 (!IS_NPC(ch)
    &&	  ch->level < skill_table[gsn_bash].skill_level[ch->class]))
    && !fWolf)
    {	
	send_to_char("Bashing? What's that?\n\r",ch);
	return;
    }

    if(fWolf)
     chance = get_skill(ch,gsn_essence_of_wolf);

    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't fighting anyone!\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (victim->position < POS_FIGHTING)
    {
	act("You'll have to let $M get back up first.",ch,NULL,victim,TO_CHAR);
	return;
    } 

    if (victim == ch)
    {
	send_to_char("You try to bash your brains out, but fail.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (ch->stunned)
    {
        send_to_char("You're still a little woozy.\n\r",ch);
        return;
    }

    if ( !can_see( ch, victim ) )
    {
	send_to_char("You get a running start, and slam right into a wall.\n\r",ch);
	return;
    }

    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }

    /* modifiers */

    /* size  and weight */
    chance += ch->carry_weight / 250;
    chance -= victim->carry_weight / 200;

    if (ch->size < victim->size)
	chance += (ch->size - victim->size) * 10;
    else
	chance += (ch->size - victim->size) * 10; 


    /* stats */
    chance += get_curr_stat(ch,STAT_STR);
    chance -= (get_curr_stat(victim,STAT_DEX) * 4)/3;
    chance -= GET_AC(victim,AC_BASH) /50;
    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
        chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
        chance -= 20;

    /* level */
    chance += (ch->level - victim->level);

    if (!IS_NPC(victim) 
	&& chance < get_skill(victim,gsn_dodge) )
    {	/*
        act("{i$n tries to bash you, but you dodge it.{x",ch,NULL,victim,TO_VICT);
        act("{h$N dodges your bash, you fall flat on your face.{x",ch,NULL,victim,TO_CHAR);
        WAIT_STATE(ch,skill_table[gsn_bash].beats);
        return;*/
	chance -= 1 * (get_skill(victim,gsn_dodge) - chance);
    }

    /* now the attack */
    if (number_percent() < chance )
    {
    
	act("{i$n sends you sprawling with a powerful bash!{x",
		ch,NULL,victim,TO_VICT);
	act("{hYou slam into $N, and send $M flying!{x",ch,NULL,victim,TO_CHAR);
	act("{k$n sends $N sprawling with a powerful bash.{x",
		ch,NULL,victim,TO_NOTVICT);
	if(!fWolf)
         check_improve(ch,gsn_bash,TRUE,1);

	DAZE_STATE(victim, 3 * PULSE_VIOLENCE);
        FLEE_DAZE(victim,88);
	WAIT_STATE(ch,skill_table[gsn_bash].beats);
	victim->position = POS_RESTING;
	damage(ch,victim,number_range(2,2 + 2 * ch->size + chance/20),gsn_bash,
	    DAM_BASH,TRUE);
	chance = (get_skill(ch,gsn_stun)/5);
	if (number_percent() < chance )
	{
	    chance = (get_skill(ch,gsn_stun)/5);
	    if (number_percent() < chance )
	    {
		victim->stunned = 2;
	    } else {
		victim->stunned = 1;
	    }
	    act("{iYou are stunned, and have trouble getting back up!{x",
		ch,NULL,victim,TO_VICT);
	    act("{h$N is stunned by your bash!{x",ch,NULL,victim,TO_CHAR);
	    act("{k$N is having trouble getting back up.{x",
		ch,NULL,victim,TO_NOTVICT);
	}
    }
    else
    {
	damage(ch,victim,0,gsn_bash,DAM_BASH,FALSE);
	act("{hYou fall flat on your face!{x",
	    ch,NULL,victim,TO_CHAR);
	act("{k$n falls flat on $s face.{x",
	    ch,NULL,victim,TO_NOTVICT);
	act("{iYou evade $n's bash, causing $m to fall flat on $s face.{x",
	    ch,NULL,victim,TO_VICT);
        if(!fWolf) 
 	 check_improve(ch,gsn_bash,FALSE,1);
	ch->position = POS_RESTING;
	WAIT_STATE(ch,skill_table[gsn_bash].beats * 3/2); 
    }
}

void do_shield_smash( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument(argument,arg);

    if ( (chance = get_skill(ch,gsn_shield_smash)) == 0
    ||	 (IS_NPC(ch))
    ||	 (!IS_NPC(ch)
    &&	  ch->level < skill_table[gsn_shield_smash].skill_level[ch->class]))
    {	
	send_to_char("Shield smashing? What's that?\n\r",ch);
	return;
    }

    if(get_eq_char(ch,WEAR_SHIELD)==NULL)
    {
      send_to_char("You are not even wearing a shield.\n\r",ch);
      return;
    }

    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't fighting anyone!\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (victim->position < POS_FIGHTING)
    {
	act("You'll have to let $M get back up first.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim == ch)
    {
	send_to_char("You try to smash yourself, but fail.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (ch->stunned)
    {
        send_to_char("You're still a little woozy.\n\r",ch);
        return;
    }

    if ( !can_see( ch, victim ) )
    {
	send_to_char("You can not even see your opponent!\n\r",ch);
	return;
    }

    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }

    /* modifiers */

    /* size  and weight */
    chance += ch->carry_weight / 250;
    chance -= victim->carry_weight / 200;

    if (ch->size < victim->size)
	chance += (ch->size - victim->size) * 10;
    else
	chance += (ch->size - victim->size) * 10;


    /* stats */
    chance += get_curr_stat(ch,STAT_STR);
    chance -= (get_curr_stat(victim,STAT_DEX) * 4)/3;
    chance -= GET_AC(victim,AC_BASH) /50;
    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
        chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
        chance -= 20;

    /* level */
    chance += (ch->level - victim->level);

    if (!IS_NPC(victim)
	&& chance < get_skill(victim,gsn_dodge) )
    {	/*
        act("{i$n tries to bash you, but you dodge it.{x",ch,NULL,victim,TO_VICT);
        act("{h$N dodges your bash, you fall flat on your face.{x",ch,NULL,victim,TO_CHAR);
        WAIT_STATE(ch,skill_table[gsn_bash].beats);
        return;*/
	chance -= 1 * (get_skill(victim,gsn_dodge) - chance);
    }

    /* now the attack */
    if (number_percent() < chance )
    {

	act("{i$n smashes you with $s shield!{x",
		ch,NULL,victim,TO_VICT);
	act("{hYou smash $N with your shield!{x",ch,NULL,victim,TO_CHAR);
	act("{k$n smashes $N with his shield!{x",
		ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_shield_smash,TRUE,1);

	DAZE_STATE(victim, 2 * PULSE_VIOLENCE);
        FLEE_DAZE(victim, 85);
	WAIT_STATE(ch,skill_table[gsn_shield_smash].beats);
	victim->position = POS_RESTING;
	damage(ch,victim,number_range(ch->level*2+10,ch->level*4+10),gsn_shield_smash,
	    DAM_BASH,TRUE);
	chance = (get_skill(ch,gsn_stun)/5);
	if (number_percent() < chance )
	{
	    chance = (get_skill(ch,gsn_stun)/5);
	    if (number_percent() < chance )
	    {
		victim->stunned = 2;
	    } else {
		victim->stunned = 1;
	    }
	    act("{iYou are stunned, and have trouble getting back up!{x",
		ch,NULL,victim,TO_VICT);
	    act("{h$N is stunned by your shield smash!{x",ch,NULL,victim,TO_CHAR);
	    act("{k$N is having trouble getting back up.{x",
		ch,NULL,victim,TO_NOTVICT);
	    check_improve(ch,gsn_stun,TRUE,1);
	}
    }
    else
    {
	damage(ch,victim,0,gsn_shield_smash,DAM_BASH,TRUE);
	check_improve(ch,gsn_shield_smash,FALSE,1);
	WAIT_STATE(ch,skill_table[gsn_shield_smash].beats);
    }
}


void do_dirt( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument(argument,arg);

    if ( (chance = get_skill(ch,gsn_dirt)) == 0
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_KICK_DIRT))
    ||   (!IS_NPC(ch)
    &&    ch->level < skill_table[gsn_dirt].skill_level[ch->class]))
    {
	send_to_char("{hYou get your feet dirty.{x\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't in combat!\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(victim,AFF_BLIND))
    {
	act("{h$E's already been blinded.{x",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim == ch)
    {
	send_to_char("Very funny.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;

    if (IS_NPC(victim) &&
	 victim->fighting != NULL && 
	!is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is such a good friend!",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (ch->stunned)
    {
        send_to_char("You're still a little woozy.\n\r",ch);
        return;
    }

    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }

    /* modifiers */

    /* dexterity */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= 2 * get_curr_stat(victim,STAT_DEX);

    /* speed  */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
	chance -= 10;

    /* level */
    chance += (ch->level - victim->level) * 2;

    /* sloppy hack to prevent false zeroes */
    if (chance % 5 == 0)
	chance += 1;

    /* terrain */

/*    switch(ch->in_room->sector_type)
    {
	case(SECT_INSIDE):		chance -= 10;	break;
	case(SECT_CITY):		chance -=  5;	break;
	case(SECT_FIELD):		chance +=  5;	break;
	case(SECT_FOREST):				break;
	case(SECT_HILLS):				break;
	case(SECT_MOUNTAIN):		chance -= 10;	break;
	case(SECT_WATER_SWIM):		chance -= 10;	break;
	case(SECT_WATER_NOSWIM):	chance -= 10;	break;
	case(SECT_AIR):			chance -= 10;  	break;
	case(SECT_DESERT):		chance += 10;   break;
    }*/

    if (chance == 0)
    {
	send_to_char("{hThere isn't any dirt to kick.{x\n\r",ch);
	return;
    }

    /* now the attack */
    if (number_percent() < chance)
    {
	AFFECT_DATA af;
	act("{k$n is blinded by the dirt in $s eyes!{x",victim,NULL,NULL,TO_ROOM);
	act("{i$n kicks dirt in your eyes!{x",ch,NULL,victim,TO_VICT);
        damage(ch,victim,number_range(2,5),gsn_dirt,DAM_NONE,FALSE);
	send_to_char("{DYou can't see a thing!{x\n\r",victim);
	check_improve(ch,gsn_dirt,TRUE,2);
	WAIT_STATE(ch,skill_table[gsn_dirt].beats);

	af.where	= TO_AFFECTS;
	af.type 	= gsn_dirt;
	af.level 	= ch->level;
	af.duration	= 1005;
	af.location	= APPLY_HITROLL;
	af.modifier	= -4;
	af.bitvector 	= AFF_BLIND;

	affect_to_char(victim,&af);
    }
    else
    {
	damage(ch,victim,0,gsn_dirt,DAM_NONE,TRUE);
	check_improve(ch,gsn_dirt,FALSE,2);
	WAIT_STATE(ch,skill_table[gsn_dirt].beats);
    }
}

void do_gouge( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument(argument,arg);

    if ( (chance = get_skill(ch,gsn_gouge)) == 0
    ||   (!IS_NPC(ch)
    &&    ch->level < skill_table[gsn_gouge].skill_level[ch->class]))
    {
	send_to_char("Gouge?  What's that?{x\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't in combat!\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(victim,AFF_BLIND))
    {
	act("{h$E's already been blinded.{x",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim == ch)
    {
	send_to_char("Very funny.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;

    if (IS_NPC(victim) &&
	 victim->fighting != NULL && 
	!is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is such a good friend!",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (ch->stunned)
    {
        send_to_char("You're still a little woozy.\n\r",ch);
        return;
    }

    /* modifiers */

    /* dexterity */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= 2 * get_curr_stat(victim,STAT_DEX);

    /* speed  */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
	chance -= 10;

    /* level */
    chance += (ch->level - victim->level) * 2;

    /* sloppy hack to prevent false zeroes */
    if (chance % 5 == 0)
	chance += 1;

    /* now the attack */
    if (number_percent() < chance)
    {
	AFFECT_DATA af;
	act("{k$n is blinded by a poke in the eyes!{x",victim,NULL,NULL,TO_ROOM);
	act("{i$n gouges at your eyes!{x",ch,NULL,victim,TO_VICT);
        damage(ch,victim,number_range(2,5),gsn_gouge,DAM_NONE,FALSE);
	send_to_char("{DYou see nothing but stars!{x\n\r",victim);
	check_improve(ch,gsn_gouge,TRUE,2);
	WAIT_STATE(ch,skill_table[gsn_gouge].beats);

	af.where	= TO_AFFECTS;
	af.type 	= gsn_gouge;
	af.level 	= ch->level;
	af.duration	= 1000+5+ch->level/10;
	af.location	= APPLY_HITROLL;
	af.modifier	= -4;
	af.bitvector 	= AFF_BLIND;

	affect_to_char(victim,&af);
    }
    else
    {
	damage(ch,victim,0,gsn_gouge,DAM_NONE,TRUE);
	check_improve(ch,gsn_gouge,FALSE,2);
	WAIT_STATE(ch,skill_table[gsn_gouge].beats);
    }
}

void do_trip( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument(argument,arg);

    if ( (chance = get_skill(ch,gsn_trip)) == 0
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_TRIP))
    ||   (!IS_NPC(ch) 
	  && ch->level < skill_table[gsn_trip].skill_level[ch->class]))
    {
	send_to_char("Tripping?  What's that?\n\r",ch);
	return;
    }


    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't fighting anyone!\n\r",ch);
	    return;
 	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;

    if (IS_NPC(victim) &&
	 victim->fighting != NULL && 
	!is_same_group(ch,victim->fighting))
    {
	send_to_char("Kill stealing is not permitted.\n\r",ch);
	return;
    }
    
    if (ch->stunned)
    {
        send_to_char("You're still a little woozy.\n\r",ch);
        return;
    }

    if (IS_AFFECTED(victim,AFF_FLYING))
    {
	act("{h$S feet aren't on the ground.{x",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim->position < POS_FIGHTING)
    {
	act("{h$N is already down.{c",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim == ch)
    {
	send_to_char("{hYou fall flat on your face!{x\n\r",ch);
	WAIT_STATE(ch,2 * skill_table[gsn_trip].beats);
	act("{k$n trips over $s own feet!{x",ch,NULL,NULL,TO_ROOM);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("$N is your beloved master.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }

    /* modifiers */

    /* size */
    if (ch->size < victim->size)
        chance += (ch->size - victim->size) * 10;  /* bigger = harder to trip */

    /* dex */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= get_curr_stat(victim,STAT_DEX) * 3 / 2;

    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
	chance -= 20;

    /* level */
    chance += (ch->level - victim->level) * 2;


    /* now the attack */
    if (number_percent() < chance)
    {
	act("{i$n trips you and you go down!{x",ch,NULL,victim,TO_VICT);
	act("{hYou trip $N and $N goes down!{x",ch,NULL,victim,TO_CHAR);
	act("{k$n trips $N, sending $M to the ground.{x",ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_trip,TRUE,1);

	DAZE_STATE(victim,2 * PULSE_VIOLENCE);
        FLEE_DAZE(victim, 91);
        WAIT_STATE(ch,skill_table[gsn_trip].beats);
	victim->position = POS_RESTING;
	damage(ch,victim,number_range(2, 2 +  2 * victim->size),gsn_trip,
	    DAM_BASH,TRUE);
    }
    else
    {
	damage(ch,victim,0,gsn_trip,DAM_BASH,TRUE);
	WAIT_STATE(ch,skill_table[gsn_trip].beats*2/3);
	check_improve(ch,gsn_trip,FALSE,1);
    } 
}

void do_earthbind( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument(argument,arg);

    if ( (chance = get_skill(ch,gsn_earthbind)) == 0
    ||   (IS_NPC(ch))
    ||   (!IS_NPC(ch)
	  && ch->level < skill_table[gsn_earthbind].skill_level[ch->class]))
    {
	send_to_char("You have no idea how to earthbind someone.\n\r",ch);
	return;
    }


    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't fighting anyone!\n\r",ch);
	    return;
 	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;

    if (ch->stunned)
    {
        send_to_char("You're still a little woozy.\n\r",ch);
        return;
    }

    if (!IS_AFFECTED(victim,AFF_FLYING))
    {
	act("{h$S feet are already on the ground.{x",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim == ch)
    {
	send_to_char("{xVery smart.{x\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("$N is your beloved master.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }

    /* modifiers */

    /* size */
    if (ch->size < victim->size)
        chance += (ch->size - victim->size) * 10;  /* bigger = harder to earthbind */

    /* dex */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= get_curr_stat(victim,STAT_DEX) * 3 / 2;

    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
	chance -= 20;

    /* level */
    chance += (ch->level - victim->level) * 2;


    /* now the attack */
    if (number_percent() < chance)
    {
        check_improve(ch,gsn_earthbind,TRUE,1);
        WAIT_STATE(ch,skill_table[gsn_earthbind].beats);
        affect_strip(victim, gsn_fly);
        if(IS_AFFECTED(victim, AFF_FLYING))
        {
         /* racial fly affect */
         damage(ch,victim,0,gsn_earthbind,DAM_BASH,TRUE);
        }
        else
        {
	 damage(ch,victim,number_range(ch->level/2,ch->level*2),gsn_earthbind,
	     DAM_BASH,TRUE);
         if( skill_table[gsn_fly].msg_off )
         { 
          send_to_char(skill_table[gsn_fly].msg_off, victim);
          send_to_char("\n\r", victim);
         }
	 act("{x$n falls to the ground!{x",victim,NULL,ch,TO_ROOM);
        }

    }
    else
    {
	damage(ch,victim,0,gsn_earthbind,DAM_BASH,TRUE);
	WAIT_STATE(ch,skill_table[gsn_earthbind].beats*2/3);
	check_improve(ch,gsn_earthbind,FALSE,1);
    }
}


void do_kill( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Kill whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }
    if ( victim == ch )
    {
	send_to_char( "{hYou hit yourself.  {z{COuch!{x\n\r", ch );
	/*multi_hit( ch, ch, TYPE_UNDEFINED );*/
	return;
    }

    if ( is_safe( ch, victim ) )
	return;

/*    if ( !IS_NPC(victim) )
    {
        if ( !IS_SET(victim->act, PLR_TWIT) )
        {
            send_to_char( "You must MURDER a player.\n\r", ch );
            return;
        }
    }*/
/*
    if ( victim->fighting != NULL && 
	!is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }
*/
    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
	act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	/*send_to_char( "You do the best you can!\n\r", ch );
	return;*/
        ch->fighting=NULL;
        if(!IS_NPC(ch) && !IS_NPC(victim))
        {
         ch->attacker=TRUE;
         victim->attacker=FALSE;
        }
        set_fighting(ch,victim);
        send_to_char("Ok.\n\r",ch);
        WAIT_STATE(ch,1*PULSE_VIOLENCE);
        return;
    }

    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }

    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}

void do_backstab( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if (arg[0] == '\0')
    {
        send_to_char("Backstab whom?\n\r",ch);
        return;
    }

    if (ch->fighting != NULL)
    {
	send_to_char("{hYou're facing the wrong end.{x\n\r",ch);
	return;
    }
 
    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r",ch);
        return;
    }

    if ( victim == ch )
    {
	send_to_char( "How can you sneak up on yourself?\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim ) )
      return;

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
    {
	send_to_char( "{hYou need to wield a primary weapon to backstab.{x\n\r", ch );
	return;
    }

    if ( victim->hit < victim->max_hit / 4)
    {
	act( "$N is hurt and suspicious ... you can't sneak up.",
	    ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }

    WAIT_STATE( ch, skill_table[gsn_backstab].beats );
    if ( number_percent( ) < get_skill(ch,gsn_backstab)
    || ( get_skill(ch,gsn_backstab) >= 2 && !IS_AWAKE(victim) ) )
    {
	check_improve(ch,gsn_backstab,TRUE,1);
	multi_hit( ch, victim, gsn_backstab );
    }
    else
    {
	check_improve(ch,gsn_backstab,FALSE,1);
	damage( ch, victim, 0, gsn_backstab,DAM_NONE,TRUE);
    }

    return;
}

void do_charge ( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if (arg[0] == '\0')
    {
        send_to_char("Charge whom?\n\r",ch);
        return;
    }

    if (ch->fighting != NULL)
    {
	send_to_char("{hYou're facing the wrong end.{x\n\r",ch);
	return;
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r",ch);
        return;
    }

    if ( victim == ch )
    {
	send_to_char( "How can you charge yourself?\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim ) )
      return;

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
    {
	send_to_char( "{hYou need to wield a primary weapon to charge.{x\n\r", ch );
	return;
    }

    if ( victim->hit < victim->max_hit / 4)
    {
	act( "$N is hurt and suspicious ... you can't charge $M.",
	    ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }

    WAIT_STATE( ch, skill_table[gsn_charge].beats );
    if ( number_percent( ) < get_skill(ch,gsn_charge)
    || ( get_skill(ch,gsn_charge) >= 2 && !IS_AWAKE(victim) ) )
    {
	check_improve(ch,gsn_charge,TRUE,1);
	multi_hit( ch, victim, gsn_charge );
    }
    else
    {
	check_improve(ch,gsn_charge,FALSE,1);
	damage( ch, victim, 0, gsn_charge,DAM_NONE,TRUE);
    }

    return;
}

void do_assassinate ( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    one_argument( argument, arg );
    
    if(get_skill(ch,gsn_assassinate) == 0)
    {
 	send_to_char("You don't know the first thing about assassination.\n\r",
             ch);
        return;
    }

    if (arg[0] == '\0')
    {
        send_to_char("Assassinate whom?\n\r",ch);
        return;
    }

    if (ch->fighting != NULL)
    {
	send_to_char("{hYou're facing the wrong end.{x\n\r",ch);
	return;
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r",ch);
        return;
    }

    if ( victim == ch )
    {
	send_to_char( "How can you sneak up on yourself?\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim ) )
      return;

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
    {
	send_to_char( "{hYou need to wield a primary weapon to assassinate.{x\n\r", ch );
	return;
    }

    if ( victim->hit < victim->max_hit / 4)
    {
	act( "$N is hurt and suspicious ... you can't sneak up.",
	    ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }
    if( is_affected( victim, gsn_assassinate ))
    {
     act("{G$N{G's weaknesses have already been exposed.{x",ch,NULL,
      victim,TO_CHAR);
    }
    else
    {
     af.where = TO_AFFECTS;
     af.type = gsn_assassinate;
     af.level = ch->level;
     af.location = APPLY_NONE;
     af.modifier = 0;
     af.duration = 1;
     af.bitvector = 0;
     af.dealer	= ch;	
     affect_to_char(victim,&af);
     act("{G$N{G's weaknesses have been exposed.{x",ch,NULL,victim,TO_CHAR);
     act("{GYour weaknesses have been exposed.{x",ch,NULL,victim,TO_VICT);
     act("{G$N{G's weaknesses have been exposed.{x",ch,NULL,victim,TO_NOTVICT);
    }
    WAIT_STATE( ch, skill_table[gsn_assassinate].beats );
    if ( number_percent( ) < get_skill(ch,gsn_assassinate)
    || ( get_skill(ch,gsn_assassinate) >= 2 && !IS_AWAKE(victim) ) )
    {
	check_improve(ch,gsn_assassinate,TRUE,1);
	multi_hit( ch, victim, gsn_assassinate );
    }
    else
    {
	check_improve(ch,gsn_assassinate,FALSE,1);
	damage( ch, victim, 0, gsn_assassinate,DAM_NONE,TRUE);
    }

    return;
}

void do_blackjack ( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    AFFECT_DATA af;
    int chance;

    one_argument( argument, arg );

    if((chance = get_skill(ch,gsn_blackjack)) == 0)
    {
 	send_to_char("You don't know how to knock out your opponents.\n\r",
             ch);
        return;
    }

    if (arg[0] == '\0')
    {
        send_to_char("Blackjack whom?\n\r",ch);
        return;
    }

    if (ch->fighting != NULL)
    {
	send_to_char("{hYou're facing the wrong end.{x\n\r",ch);
	return;
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r",ch);
        return;
    }

    if ( victim == ch )
    {
	send_to_char( "You don't feel like blackjacking yourself, do you?\n\r", ch );
	return;
    }

    if( victim->fighting!=NULL )
    {
      send_to_char("They are fighting, you can't get close enough.\n\r",ch);
      return;
    }

    if ( is_safe( ch, victim ) )
      return;

    if( IS_AFFECTED( victim, AFF_SLEEP ))
    {
     act("$N has already been put to sleep.{x",ch,NULL,
      victim,TO_CHAR);
     return;
    }

    chance = chance + (ch->level - victim->level);
    WAIT_STATE( ch, skill_table[gsn_blackjack].beats );
    if(number_percent() < chance)
    {
     if(!IS_NPC(ch) && has_killed_flag(ch))
      remove_killed_flag(ch);
    /* if(!IS_NPC(ch) && IS_SHIELDED(ch,SHD_SWALK))
     {
      affect_strip(ch,gsn_shadow_walk);
      REMOVE_BIT(ch->shielded_by,SHD_SWALK);
      send_to_char("You step from the shadows.\n\r",ch);
      act("$n steps from the shadows.",ch,NULL,NULL,TO_ROOM);
     }*/
     af.where = TO_AFFECTS;
     af.type = gsn_blackjack;
     af.level = ch->level;
     af.location = APPLY_NONE;
     af.modifier = 0;
     af.duration = 1005+ch->level/10;
     af.bitvector = AFF_SLEEP;
     affect_join(victim,&af);
     if(IS_AWAKE(victim))
     {
      act("You put $N to sleep.{x",ch,NULL,victim,TO_CHAR);
      act("You feel a sudden pain erupt through your skull!{x",ch,NULL,victim,TO_VICT);
      act("$N has been put to sleep.{x",ch,NULL,victim,TO_NOTVICT);
      victim->position=POS_SLEEPING;
     }
     check_improve(ch,gsn_blackjack,TRUE,1);
    }
    else
    {
      if ( ( ch->fighting == NULL )
      && ( !IS_NPC( ch ) )
      && ( !IS_NPC( victim ) ) )
      {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
      }
      damage(ch,victim,0,gsn_blackjack,DAM_NONE,TRUE);
      check_improve(ch,gsn_blackjack,FALSE,2);
    }

    return;
}

void do_circle( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
  
    if ( get_skill(ch,gsn_circle) == 0
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_CIRCLE))
    ||	 (!IS_NPC(ch)
    &&	  ch->level < skill_table[gsn_circle].skill_level[ch->class]))
    {	
	send_to_char("Circle? What's that?\n\r",ch);
	return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }
 
    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
    {
        send_to_char( "You need to wield a primary weapon to circle.\n\r", ch );
        return;
    }

    if ( victim->hit < victim->max_hit / 7)
    {
	act( "$N is hurt and suspicious ... you can't sneak around.",
	    ch, NULL, victim, TO_CHAR );
	return;
    }
  
    if (ch->stunned)
    {
        send_to_char("You're still a little woozy.\n\r",ch);
        return;
    }

    if ( !can_see( ch, victim ) )
    {
	send_to_char("You stumble blindly into a wall.\n\r",ch);
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_circle].beats );
    if ( number_percent( ) < get_skill(ch,gsn_circle)
    || ( get_skill(ch,gsn_circle) >= 2 && !IS_AWAKE(victim) ) )
    {
        check_improve(ch,gsn_circle,TRUE,1);
	act( "{i$n circles around behind you.{x", ch, NULL, victim, TO_VICT);
	act( "{hYou circle around $N.{x", ch, NULL, victim, TO_CHAR);
	act( "{k$n circles around behind $N.{x", ch, NULL, victim, TO_NOTVICT);
        multi_hit( ch, victim, gsn_circle );
    }
    else
    {
        check_improve(ch,gsn_circle,FALSE,1);
	act( "{i$n tries to circle around you.{x", ch, NULL, victim, TO_VICT);
	act( "{h$N circles with you.{x", ch, NULL, victim, TO_CHAR);
	act( "{k$n tries to circle around $N.{x", ch, NULL, victim, TO_NOTVICT);
        damage( ch, victim, 0, gsn_circle,DAM_NONE,TRUE);
    }
 
    return;
}

void do_strike( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    if ( get_skill(ch,gsn_strike) == 0
    ||	 (!IS_NPC(ch)
    &&	  ch->level < skill_table[gsn_strike].skill_level[ch->class]))
    {	
	send_to_char("Strike? What's that?\n\r",ch);
	return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL)
    {
        send_to_char( "You need to wield a primary weapon to strike.\n\r", ch );
        return;
    }

    if (ch->stunned)
    {
        send_to_char("You're still a little woozy.\n\r",ch);
        return;
    }

    if ( !can_see( ch, victim ) )
    {
	send_to_char("You need to see your opponent in order to strike.\n\r",ch);
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_strike].beats );
    if ( number_percent( ) < get_skill(ch,gsn_strike)
    || ( get_skill(ch,gsn_strike) >= 2 && !IS_AWAKE(victim) ) )
    {
        check_improve(ch,gsn_circle,TRUE,1);
	act( "{i$n strikes you with speed and accuracy!{x", ch, NULL, victim, TO_VICT);
	act( "{hYou strike $N with speed and accuracy!{x", ch, NULL, victim, TO_CHAR);
	act( "{k$n strikes $N with speed and accuracy!{x", ch, NULL, victim, TO_NOTVICT);
        multi_hit( ch, victim, gsn_strike );
    }
    else
    {
        check_improve(ch,gsn_strike,FALSE,1);
	act( "{i$n fails to strike you.{x", ch, NULL, victim, TO_VICT);
	act( "{h$N dodges your strike.{x", ch, NULL, victim, TO_CHAR);
	act( "{k$n fails to strike $N.{x", ch, NULL, victim, TO_NOTVICT);
        damage( ch, victim, 0, gsn_strike,DAM_NONE,TRUE);
    }

    return;
}


void do_feed( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int dam;
  
    if ( get_skill(ch,gsn_feed) == 0
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_FEED))	
    ||	 (!IS_NPC(ch)
    &&	  ch->level < skill_table[gsn_feed].skill_level[ch->class]))
    {	
	send_to_char("Feed? What's that?\n\r",ch);
	return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }
 
    if(!can_see(ch,victim))
    {
        send_to_char("You can't see your opponent's neck!\n\r",ch);
        return;
    }

    if (ch->stunned)
    {
        send_to_char("You're still a little woozy.\n\r",ch);
        return;
    }
  
    WAIT_STATE( ch, skill_table[gsn_feed].beats );
    if ( number_percent( ) <  (get_skill(ch,gsn_feed) + (ch->level - victim->level))
    || ( get_skill(ch,gsn_feed) >= 2 && !IS_AWAKE(victim)))
    {
        check_improve(ch,gsn_feed,TRUE,1);
	act( "{i$n bites you.{x", ch, NULL, victim, TO_VICT);
	act( "{hYou bite $N.{x", ch, NULL, victim, TO_CHAR);
	act( "{k$n bites $N.{x", ch, NULL, victim, TO_NOTVICT);
	dam=number_range( ((ch->level/2)+(victim->level/2)),
		((ch->level)+(victim->level/2))*2 );
        if(is_affected(victim,gsn_bloodbath))
         dam*=2;
        damage( ch, victim, dam, gsn_feed,DAM_NEGATIVE,TRUE);
        if(is_affected(victim,gsn_bloodbath) && (victim!=NULL)  
              && (number_percent() < (get_skill(ch,gsn_feed)
                    + (ch->level - victim->level))))
        {
          dam=2*number_range(((ch->level/2)+(victim->level/2)),
            ((ch->level)+(victim->level/2))*2);
          damage(ch,victim,dam,gsn_feed,DAM_NEGATIVE,TRUE);
        }
        
         
    }
    else
    {
        check_improve(ch,gsn_feed,FALSE,1);
	act( "{i$n tries to bite you, but hits only air.{x", ch, NULL, victim, TO_VICT);
	act( "{hYou chomp a mouthfull of air.{x", ch, NULL, victim, TO_CHAR);
	act( "{k$n tries to bite $N.{x", ch, NULL, victim, TO_NOTVICT);
        damage( ch, victim, 0, gsn_feed,DAM_NEGATIVE,TRUE);
    }
 
    return;
}


void do_flee( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    CHAR_DATA *victim;
    int attempt;

    if ( ( victim = ch->fighting ) == NULL )
    {
        if ( ch->position == POS_FIGHTING )
            ch->position = POS_STANDING;
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    if (ch->stunned)
    {
        send_to_char("You're still a little woozy.\n\r",ch);
        return;
    }

    was_in = ch->in_room;
    for ( attempt = 0; attempt < 6; attempt++ )
    {
	EXIT_DATA *pexit;
	int door;

	door = number_door( );
	if ( ( pexit = was_in->exit[door] ) == 0
	||   pexit->u1.to_room == NULL
	||   IS_SET(pexit->exit_info, EX_CLOSED)
	||   (number_percent() < ch->flee_daze)
	|| ( IS_NPC(ch)
	&&   IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB) ) )
	    continue;

	move_char( ch, door, FALSE, TRUE );
	if ( ( now_in = ch->in_room ) == was_in )
	    continue;

	ch->in_room = was_in;
	act( "$n has {Yfled{x!", ch, NULL, NULL, TO_ROOM );
        if ( !IS_NPC(ch) )
        {
            send_to_char( "{BYou {Yflee{B from combat!{x\n\r", ch );
            if( ( (ch->class == 2) || (ch->class == (MAX_CLASS/2)+1) )
            && (number_percent() < 3*(ch->level/2) ) )
            {
                if ( IS_NPC(victim) || ch->attacker == FALSE)
                {
                    send_to_char( "You {Ysnuck away{x safely.\n\r", ch);
                } else
                {    
                    send_to_char( "You feel something singe your butt on the way out.\n\r", ch);                                               
                    act( "$n is nearly {Yzapped{x in the butt by a lightning bolt from above!", ch, NULL, NULL, TO_ROOM );
                    ch->hit -= (ch->hit/8);
                }
            } else
            {    
                if ( !IS_NPC(victim) && ch->attacker == TRUE)
                {
                    send_to_char( "The {RWrath of Thoth {YZAPS{x your butt on the way out!\n\r", ch);
                    act( "$n is {Yzapped{x in the butt by a lightning bolt from above!", ch, NULL, NULL, TO_ROOM );
                    ch->hit -= (ch->hit/4);
                }
                send_to_char( "You lost 10 exp.\n\r", ch);
                gain_exp( ch, -10 );
            }
        }    
	ch->in_room = now_in;
	stop_fighting( ch, TRUE );
        if(!IS_NPC(ch))
         do_look(ch,"auto");
	return;
    }

    send_to_char( "{z{CPANIC!{x{B You couldn't escape!{x\n\r", ch );
    WAIT_STATE(ch,3);
    return;
}



void do_rescue( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *fch;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Rescue whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "What about {Yfleeing{x instead?\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch) && IS_NPC(victim) )
    {
	send_to_char( "Doesn't need your help!\n\r", ch );
	return;
    }

    if ( ch->fighting == victim )
    {
	send_to_char( "Too late.\n\r", ch );
	return;
    }

    if ( ( fch = victim->fighting ) == NULL )
    {
	send_to_char( "That person is not fighting right now.\n\r", ch );
	return;
    }


    WAIT_STATE( ch, skill_table[gsn_rescue].beats );
    if ( number_percent( ) > get_skill(ch,gsn_rescue))
    {
	send_to_char( "You fail the rescue.\n\r", ch );
	check_improve(ch,gsn_rescue,FALSE,1);
	return;
    }

    act( "{yYou rescue $N!{x",  ch, NULL, victim, TO_CHAR    );
    act( "{y$n rescues you!{x", ch, NULL, victim, TO_VICT    );
    act( "{y$n rescues $N!{x",  ch, NULL, victim, TO_NOTVICT );
    check_improve(ch,gsn_rescue,TRUE,1);

    stop_fighting( fch, FALSE );
    stop_fighting( victim, FALSE );

    set_fighting( ch, fch );
    set_fighting( fch, ch );
    return;
}



void do_kick( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int dam;

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_kick].skill_level[ch->class] )
    {
	send_to_char(
	    "You better leave the martial arts to fighters.\n\r", ch );
	return;
    }

    if (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_KICK))
	return;

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    if (ch->stunned)
    {
        send_to_char("You're still a little woozy.\n\r",ch);
        return;
    }

    dam = number_range( 1, ch->level );
    WAIT_STATE( ch, skill_table[gsn_kick].beats );
    if ( get_skill(ch,gsn_kick) > number_percent())
    {
	damage(ch,victim,number_range( dam, (ch->level*1.5) ), gsn_kick,DAM_BASH,TRUE);
	check_improve(ch,gsn_kick,TRUE,1);
    }
    else
    {
	damage( ch, victim, 0, gsn_kick,DAM_BASH,TRUE);
	check_improve(ch,gsn_kick,FALSE,1);
    }
    return;
}




void do_disarm( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int chance,hth,ch_weapon,vict_weapon,ch_vict_weapon, adv;
    bool fadv=FALSE; /* advanced disarm used? */

    hth = 0;

    if ((chance = get_skill(ch,gsn_disarm)) == 0)
    {
	send_to_char( "You don't know how to disarm opponents.\n\r", ch );
	return;
    }

    if ( get_eq_char( ch, WEAR_WIELD ) == NULL 
    &&   ((hth = get_skill(ch,gsn_hand_to_hand)) == 0
    ||    (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_DISARM))))
    {
	send_to_char( "You must wield a weapon to disarm.\n\r", ch );
	return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    if (ch->stunned)
    {
        send_to_char("You're still a little woozy.\n\r",ch);
        return;
    }

    if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
    {
	send_to_char( "{hYour opponent is not wielding a weapon.{x\n\r", ch );
	return;
    }

    /* find weapon skills */
    ch_weapon = get_weapon_skill(ch,get_weapon_sn(ch));
    vict_weapon = get_weapon_skill(victim,get_weapon_sn(victim));
    ch_vict_weapon = get_weapon_skill(ch,get_weapon_sn(victim));
    adv = get_skill(ch,gsn_advanced_disarm);

    /* modifiers */

    /* skill */
    if ( get_eq_char(ch,WEAR_WIELD) == NULL)
	chance = chance * hth/150;
    else
	chance = chance * ch_weapon/100;

    chance += (ch_vict_weapon/2 - vict_weapon) / 2; 

    /* dex vs. strength */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= 2 * get_curr_stat(victim,STAT_STR);

    /* level */
    chance += (ch->level - victim->level) * 2;
 
    if(IS_NPC(ch))
     chance/=2;
    
    /* advanced disarm */
    if(number_percent() < adv)
    {
     chance += number_range(adv/10,adv/3);
     fadv=TRUE;
     check_improve(ch,gsn_advanced_disarm,TRUE,1);
    }
    else
    {
     check_improve(ch,gsn_advanced_disarm,FALSE,1);
     adv=0;
    }
    chance /= 2;
 
    /* and now the attack */
    if (number_percent() < chance)
    {
	if (((chance = get_skill(victim,gsn_grip)) == 0)
	||   (!IS_NPC(victim)
	&&    victim->level < skill_table[gsn_grip].skill_level[victim->class])
        || (number_percent()<((adv*2)/3)))
	{
	    WAIT_STATE( ch, skill_table[gsn_disarm].beats );
	    disarm( ch, victim );
	    check_improve(ch,gsn_disarm,TRUE,1);
	    return;
	}
	if (number_percent() > (chance/5)*4)
	{
	    WAIT_STATE( ch, skill_table[gsn_disarm].beats );
	    disarm( ch, victim );
	    check_improve(ch,gsn_disarm,TRUE,1);
	    check_improve(victim,gsn_grip,FALSE,1);
	    return;
	}
	check_improve(victim,gsn_grip,TRUE,1);
    }
    WAIT_STATE(ch,skill_table[gsn_disarm].beats);
    if(fadv)
    {
     act("{hYou fail to disarm $N, despite your advanced technique.{x",ch,NULL,victim,TO_CHAR);
     act("{i$n fails to disarm you, despite $s advanced technique.{x",ch,NULL,victim,TO_VICT);
     act("{k$n fails to disarm $N, despite $s advanced technique.{x",ch,NULL,victim,TO_NOTVICT);
    }
    else
    { 
     act("{hYou fail to disarm $N.{x",ch,NULL,victim,TO_CHAR);
     act("{i$n tries to disarm you, but fails.{x",ch,NULL,victim,TO_VICT);
     act("{k$n tries to disarm $N, but fails.{x",ch,NULL,victim,TO_NOTVICT);
    }
    check_improve(ch,gsn_disarm,FALSE,1);
    return;
}

void do_surrender( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *mob;
    if ( (mob = ch->fighting) == NULL )
    {
	send_to_char( "But you're not fighting!\n\r", ch );
	return;
    }
    act( "You surrender to $N!", ch, NULL, mob, TO_CHAR );
    act( "$n surrenders to you!", ch, NULL, mob, TO_VICT );
    act( "$n tries to surrender to $N!", ch, NULL, mob, TO_NOTVICT );
    stop_fighting( ch, TRUE );
 
    if ( !IS_NPC( ch ) && IS_NPC( mob ) 
    &&   ( !HAS_TRIGGER( mob, TRIG_SURR ) 
        || !mp_percent_trigger( mob, ch, NULL, NULL, TRIG_SURR ) ) )
    {
	act( "$N seems to ignore your cowardly act!", ch, NULL, mob, TO_CHAR );
	multi_hit( mob, ch, TYPE_UNDEFINED );
    }
}

void do_sla( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to {RSLAY{x, spell it out.\n\r", ch );
    return;
}



void do_slay( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Slay whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "Suicide is a mortal sin.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) && victim->level >= get_trust(ch) )
    {
	send_to_char( "{hYou failed.{c\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) || get_trust(ch) >= CREATOR )
    { 
	act( "{hYou slay $M in cold blood!{x",  ch, NULL, victim, TO_CHAR    );
	act( "{i$n slays you in cold blood!{x", ch, NULL, victim, TO_VICT    );
	act( "{k$n slays $N in cold blood!{x",  ch, NULL, victim, TO_NOTVICT );
	raw_kill( victim, ch, 0 );
    }
    else
    {
	act( "{i$N wields a sword called '{z{RGodSlayer{i'!{x",  ch, NULL, victim, TO_CHAR    );
	act( "{hYou wield a sword called '{z{RGodSlayer{h'!{x", ch, NULL, victim, TO_VICT    );
	act( "{k$N wields a sword called '{z{RGodSlayer{k'!{x",  ch, NULL, victim, TO_NOTVICT );
        act("{i$N's slice takes off your left arm!{x",ch,NULL,victim,TO_CHAR);
        act("{hYour slice takes off $n's left arm!{x",ch,NULL,victim,TO_VICT);
        act("{k$N's slice takes off $n's left arm!{x",ch,NULL,victim,TO_NOTVICT);
        act("{i$N's slice takes off your right arm!{x",ch,NULL,victim,TO_CHAR);
        act("{hYour slice takes off $n's right arm!{x",ch,NULL,victim,TO_VICT);
        act("{k$N's slice takes off $n's right arm!{x",ch,NULL,victim,TO_NOTVICT);
        act("{i$N's slice cuts off both of your legs!{x",ch,NULL,victim,TO_CHAR);
        act("{hYour slice cuts off both of $n's legs!{x",ch,NULL,victim,TO_VICT);
        act("{k$N's slice cuts off both of $n's legs!{x",ch,NULL,victim,TO_NOTVICT);
        act("{i$N's slice beheads you!{x",ch,NULL,victim,TO_CHAR);
        act("{hYour slice beheads $n!{x",ch,NULL,victim,TO_VICT);
        act("{k$N's slice beheads $n!{x",ch,NULL,victim,TO_NOTVICT);
        act("{iYou are DEAD!!!{x",ch,NULL,victim,TO_CHAR);
	act("{h$n is DEAD!!!{x",ch,NULL,victim,TO_VICT);
	act("{k$n is DEAD!!!{x", ch,NULL,victim,TO_NOTVICT);
	act("A sword called '{z{RGodSlayer{x' vanishes.",ch,NULL,victim,TO_VICT);
	act("A sword called '{z{RGodSlayer{x' vanishes.",ch,NULL,victim,TO_NOTVICT);
	raw_kill( ch, victim, 0 );
    }
    return;
}

void check_arena(CHAR_DATA *ch, CHAR_DATA *victim)
{
 char buf[MAX_STRING_LENGTH];

 sprintf(buf,"{W%s has {Rd{refeate{Rd{W %s!",ch->name,victim->name);
 comm_arena(ch,victim,buf);

 ch->pcdata->awins++;
 victim->pcdata->alosses++;
 
 stop_fighting(victim,TRUE);
 char_from_room(victim);
 char_to_room(victim,get_room_index(516));
 affect_strip(victim,gsn_gouge);
 affect_strip(victim,skill_lookup("slow"));
 affect_strip(victim,skill_lookup("weaken"));
 affect_strip(victim,gsn_plague);
 affect_strip(victim,gsn_poison);
 affect_strip(victim,gsn_curse);
 affect_strip(victim,gsn_blindness);
 affect_strip(victim,gsn_assassinate);
 affect_strip(victim,skill_lookup("sever")); 
 affect_strip(victim,skill_lookup("dirt kicking"));
 victim->hit = victim->max_hit;
 victim->mana = victim->max_mana;
 victim->move = victim->max_move;
 victim->pcdata->pkflag=0;
 update_pos(victim);
 
 stop_fighting(ch,TRUE);
 char_from_room(ch);
 char_to_room(ch,get_room_index(515));
 affect_strip(ch,gsn_gouge);
 affect_strip(ch,skill_lookup("slow"));
 affect_strip(ch,skill_lookup("weaken"));
 affect_strip(ch,gsn_plague);
 affect_strip(ch,gsn_poison);
 affect_strip(ch,gsn_curse);
 affect_strip(ch,gsn_blindness);
 affect_strip(ch,gsn_assassinate);
 affect_strip(ch,skill_lookup("sever"));
 affect_strip(ch,skill_lookup("dirt kicking"));
 ch->hit = ch->max_hit;
 ch->mana = ch->max_mana;
 ch->move = ch->max_move;
 update_pos(ch);
 do_look(victim,"auto");
 do_look(ch,"auto");
 send_to_char("{WYou have LOST!{x\n\r",victim);
 send_to_char("{WYou have WON!{x\n\r",ch);
 ch->pcdata->pkflag=0;
 
 if(IS_SET(ch->exbit1,EXBIT1_CHALLENGER))
  REMOVE_BIT(ch->exbit1,EXBIT1_CHALLENGER);
 if(IS_SET(victim->exbit1,EXBIT1_CHALLENGER))
  REMOVE_BIT(victim->exbit1,EXBIT1_CHALLENGER);
 if(IS_SET(ch->exbit1,EXBIT1_CHALLENGED))
  REMOVE_BIT(ch->exbit1,EXBIT1_CHALLENGED);
 if(IS_SET(victim->exbit1,EXBIT1_CHALLENGED))
  REMOVE_BIT(victim->exbit1,EXBIT1_CHALLENGED);
  
 ch->challenger = NULL;
 ch->challenged = NULL;
 victim->challenger = NULL;
 victim->challenged = NULL;
/* REMOVE_BIT(ch->comm,COMM_NOCHANNELS);*/
 REMOVE_BIT(ch->act,PLR_NORESTORE);
/* REMOVE_BIT(victim->comm,COMM_NOCHANNELS); */
 REMOVE_BIT(victim->act,PLR_NORESTORE);
 arena = FIGHT_OPEN;

 send_to_char("You have been restored.\n\r",ch);
 send_to_char("You have been restored.\n\r",victim);

 return;
}

void check_quest(CHAR_DATA *ch, CHAR_DATA *victim)
{
 char buf[MAX_STRING_LENGTH];

 sprintf(buf,"{x[{YQ{yUES{YT{x] {c%s has {Rk{rille{Rd{c %s!{x\n\r",ch->name,victim->name);
 iquest_announce(buf);

 stop_fighting(victim,TRUE);
 char_from_room(victim);
 char_to_room(victim,get_room_index(516));
 affect_strip(victim,gsn_gouge);
 affect_strip(victim,skill_lookup("slow"));
 affect_strip(victim,skill_lookup("weaken"));
 affect_strip(victim,gsn_plague);
 affect_strip(victim,gsn_poison);
 affect_strip(victim,gsn_curse);
 affect_strip(victim,gsn_blindness);
 affect_strip(victim,gsn_assassinate);
 affect_strip(victim,skill_lookup("sever"));
 affect_strip(victim,skill_lookup("dirt kicking"));
 victim->hit = victim->max_hit;
 victim->mana = victim->max_mana;
 victim->move = victim->max_move;
 victim->pcdata->pkflag=0;
 update_pos(victim);

 do_look(victim,"auto");
 ch->pcdata->pkflag=0;

 send_to_char("You have been restored.\n\r",victim);
 return;
}

void do_challenge(CHAR_DATA *ch,  char *argument)
{
 CHAR_DATA *victim;
 char buf[MAX_STRING_LENGTH];

 if(IS_NPC(ch))
  return;

 if((ch->in_room->vnum == ROOM_VNUM_CORNER)
    && (!IS_IMMORTAL(ch)))
 {
  send_to_char("Just keep your nose in the corner like a good little player.\n\r",ch);
  return;
 }
 
 if(IS_IMMORTAL(ch))
 {
  send_to_char("Immortals are not allowed to enter the arena.\n\r",ch);
  return;
 }
 if(arena == FIGHT_START)
 {
  send_to_char("Sorry, someone else has already started a challenge, please try later.\n\r",ch);
  return;
 }

 if(arena == FIGHT_BUSY)
 {
  send_to_char("Sorry, there is a fight in progress, please wait a few moments.\n\r",ch);
  return;
 }

 if(arena == FIGHT_LOCK)
 {
  send_to_char("Sorry, the arena is currently locked from use.\n\r",ch);
  return;
 }
 
 if(IS_SET(ch->exbit1, EXBIT1_CHALLENGED))
 {
  send_to_char("You have already been challenged, either ACCEPT or DECLINE first.\n\r",ch);
  return;
 }

 if(IS_SET(ch->exbit1,EXBIT1_CHALLENGER))
 {
  send_to_char("You have already sent a challenge.\n\r",ch);
  return;
 }

 if(IS_SHIELDED(ch,SHD_INVISIBLE)||IS_AFFECTED(ch,AFF_SNEAK)||
    IS_AFFECTED(ch,AFF_HIDE)||IS_SHIELDED(ch,SHD_SWALK)||is_affected(ch,gsn_ambush))
 {
  send_to_char("You can't challenge while invis, sneaking or hidden.\n\r",ch);
  return;
 }

 if(argument[0]=='\0')
 {
  send_to_char("Challenge whom?\n\r",ch);
  return;
 }

 if((victim=get_char_world(ch,argument))==NULL)
 {
  send_to_char("They are not playing.\n\r",ch);
  return;
 }

 if(victim->in_room->vnum == ROOM_VNUM_CORNER)
 {
  send_to_char("They are in the corner, leave them alone.\n\r",ch);
  return;
 }

 if(IS_IMMORTAL(victim))
 {
  send_to_char("Challenge an immortal? Fool.\n\r",ch);
  return;
 }

 if(IS_NPC(victim) || victim == ch)
 {
  send_to_char("You cannot challenge NPCs or yourself.\n\r",ch);
  return; 
 }
 if(!IS_SET(ch->in_room->room_flags,ROOM_SAFE))
 {
  send_to_char("Get to a safe room first.\n\r",ch);
  return;
 }

 if(!IS_SET(victim->in_room->room_flags,ROOM_SAFE))
 {
  send_to_char("Wait until they are in a safe room.\n\r",ch);
  return;
 }

 if(IS_AFFECTED(victim,AFF_BLIND))
 {
  send_to_char("That person is blind right now.\n\r",ch);
  return;
 }

 if(IS_SET(victim->exbit1, EXBIT1_CHALLENGED))
 {
  send_to_char("They have already challenged someone else.\n\r",ch);
  return;
 }

 if(IS_SET(victim->comm,COMM_AFK))
 {
  send_to_char("That player is AFK.\n\r",ch);
  return;
 }

 if(victim->desc == NULL)
 {
  send_to_char("That player is linkdead.\n\r",ch);
  return;
 }

 if(IS_SET(ch->comm,COMM_NOARENA))
 {
  send_to_char("Why should you be allowed to challenge if noone can challenge you?\n\r",ch);
  return;
 }

 if(IS_SET(victim->comm,COMM_NOARENA))
 { 
  send_to_char("That player is blocking all challenges.\n\r",ch);
  return;
 }

 if(victim->level <= 20)
 {
  send_to_char("That player is just a newbie.\n\r",ch);
  return;
 }

 if(ch->level > (victim->level+50))
 {
  send_to_char("That player would be no challenge at all.\n\r",ch);
  return;
 }

 ch->challenged = victim;
 SET_BIT(ch->exbit1,EXBIT1_CHALLENGER);
 victim->challenger=ch;
 SET_BIT(victim->exbit1,EXBIT1_CHALLENGED);
 arena=FIGHT_START;
 send_to_char("Your challenge has been sent.\n\r",ch);
 act("{x$n has {Yc{yhallenge{Yd{x you to a {Rdeath match{x.",ch,NULL,victim,TO_VICT);
 sprintf(buf,"{W%s has {Yc{yhallenge{Yd{W %s to a death match.",
    ch->name,victim->name);
 comm_arena(ch,victim,buf);
 send_to_char("Type {GA{gCCEP{GT{x to meet the challenge.{x\n\r",victim);
 send_to_char("Type {RD{rECLIN{RE{x to chicken out.{x\n\r",victim);
 return;
}

void do_accept(CHAR_DATA *ch, char *argument)
{
 CHAR_DATA *victim;
 char buf[MAX_STRING_LENGTH];

 if(IS_NPC(ch)) return;

 if(!IS_SET(ch->exbit1, EXBIT1_CHALLENGED))
 {
  send_to_char("You have not been challenged.\n\r",ch);
  return;
 }

 if(arena == FIGHT_BUSY)
 {
  send_to_char("Sorry, there is a fight in progress, please wait a few moment.\n\r",ch);
  return;
 }

 if(arena == FIGHT_LOCK)
 {
  send_to_char("Sorry, the area is currently locked from use.\n\r",ch);
  return;
 }
 
 if((victim = get_char_mortal(ch,ch->challenger->name)) == NULL)
 {
  send_to_char("They aren't logged in!\n\r",ch);
  return;
 }
 
 if(victim == ch)
 {
  send_to_char("You haven't challenged yourself...\n\r",ch);
  return;
 }

 if(!IS_SET(victim->exbit1, EXBIT1_CHALLENGER) ||
      victim != ch->challenger)
 {
  send_to_char("That player hasn't challenged you!\n\r",ch);
  return;
 }

 send_to_char("You have {Ga{gccepte{Gd{x the challenge!\n\r",ch);
 act("$n {Ga{gccept{Gs{x your challenge!",ch,NULL,victim,TO_VICT);
 sprintf(buf,"{W%s has {Ga{gccepte{Gd{W %s's challenge.",ch->name,victim->name);
 comm_arena(ch,victim,buf);
 
 send_to_char("You make your way to the arena.\n\r",ch);
 char_from_room(ch); 
 char_to_room(ch,get_room_index(500));
 ch->hit = ch->max_hit;
 ch->mana = ch->max_mana;
 ch->move = ch->max_move;
/* SET_BIT(ch->comm,COMM_NOCHANNELS);*/
 SET_BIT(ch->act,PLR_NORESTORE);
 send_to_char("You make your way to the arena.\n\r",victim);
 char_from_room(victim);
 char_to_room(victim,get_room_index(500));
 victim->hit = victim->max_hit;
 victim->mana = victim->max_mana;
 victim->move = victim->max_move;
 do_look(ch,"auto");
 do_look(victim,"auto");
/* SET_BIT(victim->comm,COMM_NOCHANNELS);*/
 SET_BIT(victim->act,PLR_NORESTORE);
 arena = FIGHT_BUSY;
 return;

}

void do_decline(CHAR_DATA *ch, char *argument)
{
 CHAR_DATA *victim;
 char buf[MAX_STRING_LENGTH];

 if(IS_NPC(ch))
  return;
 
 if(!IS_SET(ch->exbit1,EXBIT1_CHALLENGED))
 {
  send_to_char("You have not been challenged.\n\r",ch);
  return;
 }

 if((victim = get_char_mortal(ch,ch->challenger->name)) == NULL)
 {
  send_to_char("They aren't logged in.\n\r",ch);
  return;
 }

 if(!IS_SET(victim->exbit1,EXBIT1_CHALLENGER) ||
     victim != ch->challenger)
 {
  send_to_char("That player hasn't challenged you.\n\r",ch);
  return;
 }

 victim->challenged = NULL;
 REMOVE_BIT(victim->exbit1, EXBIT1_CHALLENGER);
 ch->challenger = NULL;
 REMOVE_BIT(ch->exbit1, EXBIT1_CHALLENGED);

 arena = FIGHT_OPEN;
 send_to_char("Challenge {Rd{recline{Rd{x.\n\r",ch);
 act("$n has {Rd{recline{Rd{x your challenge.",ch,NULL,victim,TO_VICT);
 sprintf(buf,"{W%s has {Rd{recline{Rd{W %s's challenge.",ch->name,
    victim->name);
 comm_arena(ch,victim,buf);
 return;
}

void do_dismiss(CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim; 
  char buf[MAX_STRING_LENGTH];

  if(IS_NPC(ch))
   return;

 if( !IS_SET(ch->exbit1, EXBIT1_CHALLENGER))
 {
  send_to_char("You have not challenged anyone.\n\r",ch);
  return;
 }

 if((victim = get_char_mortal(ch,ch->challenged->name)) == NULL)
 {
  send_to_char("They aren't logged.\n\r",ch);
  return;
 }

 if(!IS_SET(victim->exbit1, EXBIT1_CHALLENGED) || victim != ch->challenged )
 {
   send_to_char("You haven't challenged that player.\n\r",ch);
   return;
 }


 /*== now actually decline == */
 ch->challenged = NULL;
 REMOVE_BIT(victim->exbit1, EXBIT1_CHALLENGED); 
 victim->challenger = NULL;
 REMOVE_BIT(ch->exbit1, EXBIT1_CHALLENGER);

 arena = FIGHT_OPEN;
 send_to_char("Challenge withdrawn.\n\r",ch);
 act("$n has withdrawn the challenge.",ch,NULL,victim,TO_VICT);
 sprintf(buf,"{W%s has withdrawn the challenge{x.",
        ch->name);
 comm_arena(ch,victim,buf);
 return;
}

void set_killed_flag(CHAR_DATA *ch)
{
 if(IS_NPC(ch))
  return;
 if(ch->pcdata->kflag==-1) 
  ch->pcdata->kflag = 9;
}

bool has_killed_flag(CHAR_DATA *ch)
{
 if(IS_NPC(ch))
  return FALSE;
 return (ch->pcdata->kflag>=0);
}

void remove_killed_flag(CHAR_DATA *ch)
{
 if(IS_NPC(ch))
  return;
 if(ch->pcdata->kflag!=-1)
 {
  ch->pcdata->kflag = -1;
  send_to_char("You just lost your {R(Killed){x-Flag!\n\r",ch);
 }
}

void counter_attack(CHAR_DATA *ch, CHAR_DATA *victim)
{
 int chance,atts,i;
 
 if(IS_NPC(victim))
  return;
 
 if(victim->fighting==NULL)
  return;
 
 if(victim->fighting!=ch)
  return;

 if((chance = get_skill(victim,gsn_counter))==0)
  return;

 atts = number_range(0,chance/33);
 i=0;
 chance = (chance*3)/4;
 for(;i<atts;i++)
 {
  if(victim->fighting!=NULL)
  {
   if(number_percent() < (chance + victim->level - ch->level))
   {
    /*damage(victim,ch,number_range((3*victim->level)/2,(7*victim->level)/2),
       gsn_counter,DAM_OTHER,TRUE);*/
    one_hit(victim,ch,gsn_counter,FALSE);
    check_improve(victim,gsn_counter,TRUE,6);
   }
   else
   {
    damage(victim,ch,0,gsn_counter,DAM_OTHER,TRUE);
    check_improve(victim,gsn_counter,FALSE,6);
   }
  }
 }
}

bool has_pkflag(CHAR_DATA *ch)
{
 if(IS_NPC(ch))
  return FALSE;
 if(ch->pcdata->pkflag>0)
  return TRUE;
 return FALSE;
}    

void do_chop( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument(argument,arg);

    if ( (chance = get_skill(ch,gsn_chop)) == 0
    ||   (!IS_NPC(ch)
    &&    ch->level < skill_table[gsn_chop].skill_level[ch->class]))
    {
	send_to_char("Chop?  What's that?{x\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't in combat!\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (victim == ch)
    {
	send_to_char("Very funny.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;


    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is such a good friend!",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (ch->stunned)
    {
        send_to_char("You're still a little woozy.\n\r",ch);
        return;
    }

    if ( ( ch->fighting == NULL )
    && ( !IS_NPC( ch ) )
    && ( !IS_NPC( victim ) ) )
    {
	ch->attacker = TRUE;
	victim->attacker = FALSE;
    }

    /* modifiers */

    /* dexterity */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= 2 * get_curr_stat(victim,STAT_DEX);

    /* speed  */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
	chance -= 10;

    /* level */
    chance += (ch->level - victim->level) * 2;

    /* sloppy hack to prevent false zeroes */
    if (chance % 5 == 0)
	chance += 1;

    /* now the attack */
    if (number_percent() < chance)
    {
	AFFECT_DATA af,*paf;
        act("You strike $N with a quick chop!",ch,NULL,victim,TO_CHAR);
        act("$n strikes $N with a quick chop!",ch,NULL,victim,TO_NOTVICT);
        act("$n strikes you with a quick chop!",ch,NULL,victim,TO_VICT);
	check_improve(ch,gsn_chop,TRUE,4);
	WAIT_STATE(ch,skill_table[gsn_chop].beats);
        damage(ch,victim,number_range(ch->level/4+10,ch->level+10),gsn_chop,DAM_MONK,TRUE);

        if(victim==NULL)
         return;
        send_to_char("Your hands feel weak and limp.\n\r",victim);
	af.where	= TO_AFFECTS;
	af.type 	= gsn_chop;
	af.level 	= ch->level;
	af.duration	= 1003;
	af.location	= APPLY_STR;
	af.modifier	= -5 - (ch->level/20);
	af.bitvector 	= 0;

	affect_join(victim,&af);
        if((paf=affect_find(victim->affected,gsn_chop))==NULL)
         return; 
        /* chop duration capped at six rounds */
        paf->duration = UMIN(1006,paf->duration);
    }
    else
    {
	damage(ch,victim,0,gsn_chop,DAM_NONE,TRUE);
	check_improve(ch,gsn_chop,FALSE,4);
	WAIT_STATE(ch,skill_table[gsn_chop].beats);
    }
}

void do_stance(CHAR_DATA *ch, char *argument)
{
 #define MAX_STANCES 6
 static char *stances[MAX_STANCES]=
 {"none","dragon","frost","bat","ray","scorpion"};
 int i;
 
 if(IS_NPC(ch))
  return;
 if((class_lookup("sensai")!=ch->class) &&
    (class_lookup("shaolin")!=ch->class))
 {
  send_to_char("Only sensai and shaolin can use stances.\n\r",ch);
  return;
 }
 if(argument[0]=='\0')
 {
  send_to_char("{CT{che following {Cs{ctance{Cs{c are availabl{Ce{D:\n\r",ch);
  for(i=0;i<MAX_STANCES;i++)
  {
   send_to_char(capitalize(stances[i]),ch);
   send_to_char("  ",ch); 
  }
  send_to_char("{x\n\r",ch);
  send_to_char("{CC{current stanc{Ce{D: ",ch);
  send_to_char(capitalize(stances[ch->pcdata->stance]),ch);
  send_to_char("{x\n\r",ch);
  return;
 }
 for(i=0;i<MAX_STANCES;i++)
 {
  if(!strcmp(stances[i],argument))
  {
   send_to_char("{CN{cew stanc{Ce{D: ",ch);
   send_to_char(capitalize(stances[i]),ch);
   send_to_char("{x\n\r",ch);
   ch->pcdata->stance=i;
   if(ch->fighting!=NULL)
    WAIT_STATE(ch,12);
   return;
  }
 }
 send_to_char("No such stance.\n\r",ch);
 do_stance(ch,"");
 return;
}

void check_leech(CHAR_DATA *ch, CHAR_DATA *victim)
{
 AFFECT_DATA *af;
 int dam;

 if(!is_affected(victim,gsn_leech))
  return;
 af = affect_find(victim->affected,gsn_leech);
 if(!af)
  return;
 if(af->dealer!=ch)
  return;
 dam = number_range(af->level/2,af->level);
 victim->mana = UMAX(0, victim->mana - dam);
 victim->move = UMAX(0, victim->move - dam);
 damage(ch,victim,dam,gsn_leech,DAM_NEGATIVE,TRUE);
 ch->hit = UMIN(ch->hit+((dam/3)*2), ch->max_hit);
 ch->mana = UMIN(ch->mana+((dam/3)*2), ch->max_mana);
 ch->move = UMIN(ch->move+((dam/3)*2), ch->max_move);
 update_pos( ch );
 return;
}

void do_bounty(CHAR_DATA *ch,char *argument)
{
 char arg1[MAX_INPUT_LENGTH];
 char arg2[MAX_INPUT_LENGTH];
 int bounty;
 CHAR_DATA *victim;
 char buf[MAX_STRING_LENGTH];
 
 argument = one_argument(argument,arg1);
 argument = one_argument(argument,arg2);

 if(IS_NPC(ch))
  return;
 
 if(arg1[0]=='\0')
 {
  send_to_char("Put a bounty on whose head?\n\r",ch);
  return;
 }
 if(arg2[0]=='\0')
 {
  send_to_char("Add how much to this person's bounty?\n\r",ch);
  return;
 }
 if((victim=get_char_world(ch,arg1))==NULL)
 {
  send_to_char("Noone with that name is online.\n\r",ch);
  return;
 }
 if(IS_NPC(victim))
 {
  send_to_char("You can't put a bounty on a mobile's head.\n\r",ch);
  return;
 }
 if((bounty=atoi(arg2))<0)
 {
  send_to_char("Specified bounty must be positive.\n\r",ch);
  return;
 }
 if(bounty<10)
 {
  send_to_char("You must add at least 10 platinum to the bounty.\n\r",ch);
  return;
 }
 if(bounty>ch->platinum)
 {
  send_to_char("You don't have that much platinum.\n\r",ch);
  return;
 }
 ch->platinum-=bounty;
 victim->pcdata->bounty+=bounty;
 sprintf(buf,"{cYou increase %s{c's bounty by {D%d{c platinum.{x\n\r",capitalize(victim->name),bounty); 
 send_to_char(buf,ch);
 sprintf(buf,"{cSomeone increases your bounty by {D%d{c platinum.{x\n\r",bounty);
 send_to_char(buf,victim);
 return;
}

void do_whirlwind(CHAR_DATA *ch,char *argument)
{
 int chance;
 CHAR_DATA *vch,*vch_next;

 if((chance=get_skill(ch,gsn_whirlwind))==0)
 {
  send_to_char("You spin around gracefully, but to no avail.\n\r",ch);
  return;
 }
 WAIT_STATE(ch,skill_table[gsn_whirlwind].beats);
 if(number_percent() < chance)
 {
  send_to_char("{BYou spin around like a whirlwind!{x\n\r",ch);
  act("$n spins around like a whirlwind!",ch,NULL,NULL,TO_ROOM);
  /*for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
	{
	    vch_next = vch->next;
	    if (!is_safe_spell(ch,vch,TRUE))
		one_hit(ch,vch,gsn_whirlwind, FALSE);
	}*/
  for(vch=char_list;vch!=NULL;vch=vch_next)
  {
   vch_next	= vch->next;
   if(vch->in_room == NULL)
	continue;
   if(vch->in_room == ch->in_room)
   {
    if(vch!=ch && !is_safe_spell(ch,vch,TRUE))
    {
      if((ch->fighting==NULL) && (!IS_NPC(ch))
         && (!IS_NPC(vch)))
      {
       ch->attacker=TRUE;
       vch->attacker=FALSE;
      }
      one_hit(ch,vch,gsn_whirlwind,FALSE);
    }
   }
  }
  check_improve(ch,gsn_whirlwind,TRUE,3);
 }
 else
 {
  send_to_char("{BYou try to spin around but fail miserably.{x\n\r",ch);
  act("$n tries to spin around but fails miserably.\n\r",ch,NULL,NULL,TO_ROOM);
  check_improve(ch,gsn_whirlwind,FALSE,3);
 }
}

void register_pk(char *killer, char *victim, int gs, int pkp, int killers)
{
 int count=0;
 PKILL_DATA *p=NULL,*p_next=NULL,*p_previous=NULL;

 if(!killer || !victim || (pkp<0) || (killers<0))
  return;
 
  p=alloc_perm(sizeof(*p));
  p->killer=str_dup(capitalize(killer));
  p->victim=str_dup(capitalize(victim));
  p->gs=gs;
  p->pkp=pkp;
  p->killers=killers;

 if(pkill_list==NULL)
 {
  pkill_list=p;
  pkill_list->next=NULL;
  return;
 }
 p->next=pkill_list;
 pkill_list=p;
 for(;p;p=p_next)
 {
  p_next = p->next;
  if((++count)>MAX_PKILL)
  {
   if(p==pkill_list)
   {
    free_string(p->killer); 
    free_string(p->victim);
    free_mem(p,sizeof(*p));
    pkill_list=NULL;
    return;
   }
   p_previous->next=NULL;
   return;
  }
  p_previous=p; 
 }
}

void register_rank(char *name, int pkp, int race, int class, int clan)
{
 RANK_DATA *p,*p_previous=NULL,*n;

 if(!name||(name==NULL))
  return;
 
 if(pkp<=100) /* 100 or less pkp? if so, we remove the char from
                 the ranking or ignore the request if (s)he isn't
                 included in the ranking */
 {
  for(p=rank_list;p;p=p->next)
  {
   if(!strcmp(p->name,capitalize(name)))
   {
    if(p_previous==NULL)
    {
     rank_list=p->next;
     free_string(p->name);
 /*    free_mem(p,sizeof(*p));*/
     return;
    }
    p_previous->next=p->next;
    free_string(p->name);
 /*   free_mem(p,sizeof(*p));*/
    return;
   } 
   p_previous=p;
  }
  return; 
 }

 /* remove the char from the ranking if (s)he is already ranked */
 register_rank(name,-1,-1,-1,-1);

 n = alloc_perm(sizeof(*p));
 n->name = str_dup(capitalize(name));
 n->pkp = pkp;
 n->race = race;
 n->class = class;
 n->clan = clan;
 n->next = NULL;

 if(rank_list==NULL)
 {
  rank_list=n;
  return;
 }
 for(p=rank_list;p;p=p->next)
 {
  if(n->pkp>=p->pkp)
  {
   if(p_previous==NULL)
   {
    n->next=p;
    rank_list=n;
    return;
   } 
   n->next=p;
   p_previous->next=n;
   return;
  }
  p_previous=p;
  if(p->next==NULL)
  {
   p->next=n;
   return;
  }
 }
 return; 
}

bool is_tlead(CHAR_DATA *ch)
{
 if(IS_NPC(ch))
  return FALSE;
 if(team_list==NULL)
  return FALSE;
 if(ch->pcdata->team==NULL)
  return FALSE;
 if(ch->pcdata->team->leader!=ch)
  return FALSE;
 return TRUE;
}

bool is_same_team(CHAR_DATA *ch,CHAR_DATA *victim)
{
 if(IS_NPC(ch)||IS_NPC(victim))
  return FALSE;
 if(team_list==NULL || ch->pcdata->team == NULL || victim->pcdata->team == NULL 
    || ch->pcdata->team!=victim->pcdata->team)
  return FALSE;
 return TRUE;
}

TEAM_DATA *get_team(CHAR_DATA *ch)
{
 if(IS_NPC(ch))
  return NULL;
 if(team_list==NULL)
  return NULL;
 return ch->pcdata->team;
}


void do_team( CHAR_DATA *ch,	char *argument )
{
 char arg1[MAX_INPUT_LENGTH];
 char arg2[MAX_INPUT_LENGTH];
 CHAR_DATA *victim;

 if(IS_NPC(ch))
  return;
 
 argument = one_argument( argument, arg1 );
 argument = one_argument( argument, arg2 );

 if(arg1[0]=='\0') 
 {
  send_to_char("Show team status.\n\r",ch);
  return;
 }
 else if(!str_prefix(arg1,"invite"))
 {
  if((ch->in_room->vnum == ROOM_VNUM_CORNER)
     && (!IS_IMMORTAL(ch)))
  {
   send_to_char("Just keep your nose in the corner like a good little player.\n\r",ch);
   return;
  }

  if(IS_IMMORTAL(ch))
  {
   send_to_char("Immortals are not allowed to lead teams.\n\r",ch);
   return;
  }
  if(get_team(ch)!=NULL && !is_tlead(ch))
  {
   send_to_char("Only the team leader can invite players.\n\r",ch);
   return;
  }
  if(ch->pcdata->tinvited!=NULL)
  {
   send_to_char("You have been invited by someone else, either TEAM ACCEPT or TEAM DECLINE first.\n\r",ch);
   return;
  }

  if(ch->pcdata->tinviting!=NULL)
  {
   send_to_char("You have already invited someone else.\n\r",ch);
   return;
  }

  if(IS_SHIELDED(ch,SHD_INVISIBLE)||IS_AFFECTED(ch,AFF_SNEAK)||
     IS_AFFECTED(ch,AFF_HIDE)||IS_SHIELDED(ch,SHD_SWALK)||is_affected(ch,gsn_ambush))
  {
   send_to_char("You can't invite people while invis, sneaking or hidden.\n\r",ch);
   return;
  }

  if(arg2[0]=='\0')
  {
   send_to_char("Invite whom?\n\r",ch);
   return;
  }

  if((victim=get_char_world(ch,arg2))==NULL)
  {
   send_to_char("They are not playing.\n\r",ch);
   return;
  }

  if(victim->in_room->vnum == ROOM_VNUM_CORNER)
  {
   send_to_char("They are in the corner, leave them alone.\n\r",ch);
   return;
  }

  if(IS_IMMORTAL(victim))
  {
   send_to_char("You can't invite immortals.\n\r",ch);
   return;
  }

  if(IS_NPC(victim) || victim == ch)
  {
   send_to_char("You cannot invite NPCs or yourself.\n\r",ch);
   return;
  }
  if(victim->fighting!=NULL)
  {
   send_to_char("That person is fighting right now.\n\r",ch);
   return;
  }
  if(IS_AFFECTED(victim,AFF_BLIND))
  {
   send_to_char("That person is blind right now.\n\r",ch);
   return;
  }
  if(get_team(victim)!=NULL && is_same_team(ch,victim))
  {
   send_to_char("They are already part of your team.\n\r",ch);
   return;
  }
  if(get_team(victim)!=NULL)
  {
   send_to_char("They are already part of another team.\n\r",ch);
   return;
  }
  if(victim->pcdata->tinvited!=NULL)
  {
   send_to_char("They have already been invited by someone else.\n\r",ch);
   return;
  }
  if(victim->pcdata->tinviting!=NULL)
  {
   send_to_char("They are currently inviting someone themselves.\n\r",ch);
   return;
  }
  if(IS_SET(victim->comm,COMM_AFK))
  {
   send_to_char("That player is AFK.\n\r",ch);
   return;
  }
  if(victim->desc == NULL)
  {
   send_to_char("That player is linkdead.\n\r",ch);
   return;
  }
  if(IS_SET(ch->comm,COMM_NOARENA))
  {
   send_to_char("Why should you be allowed to challenge if noone can challenge you?\n\r",ch);
   return;
  }
  if(IS_SET(victim->comm,COMM_NOARENA))
  {
   send_to_char("That player is blocking all challenges.\n\r",ch);
   return;
  }
  if(victim->level <= 20)
  {
   send_to_char("That player is just a newbie.\n\r",ch);
   return;
  }
  send_to_char("Ok.\n\r",ch);
  return;
 }
}

void do_ambush(CHAR_DATA *ch,	char *argument)
{
 AFFECT_DATA af;
 CHAR_DATA *victim;
 int skill;
 char arg[MAX_INPUT_LENGTH];

 argument = one_argument(argument, arg);

 if((skill=get_skill(ch,gsn_ambush))==0)
 {
  send_to_char("You don't know how to ambush.\n\r",ch);
  return;
 }

 if(has_pkflag(ch))
 {
  send_to_char("Your heart is pouding too fast.\n\r",ch);
  return;
 }
 
 if(arg[0]=='\0')
 {
  if(is_affected(ch,gsn_ambush))
  {
   send_to_char("You are already lying in ambush.\n\r",ch);
   return;
  }
  WAIT_STATE(ch,12);
  if(number_percent() < skill)
  {
   af.where = TO_AFFECTS;
   af.location = APPLY_NONE;
   af.subtype = 0;
   af.type = gsn_ambush;
   af.modifier = 0;
   af.bitvector = 0;
   af.duration = ch->level/5;
   af.level = ch->level;
   affect_to_char(ch,&af);
   check_improve(ch,gsn_ambush,TRUE,2);
   send_to_char("You hide and prepare an ambush.\n\r",ch);
   return;
  }
  send_to_char("You look around but can not find a suitable hiding place.\n\r",ch);
  check_improve(ch,gsn_ambush,FALSE,2);
  return;
 }
 if(!is_affected(ch,gsn_ambush))
 {
  send_to_char("You haven't even prepared an ambush yet.\n\r",ch);
  return;
 }
 if ((victim = get_char_room(ch,arg)) == NULL)
 {
  send_to_char("They aren't here.\n\r",ch);
  return;
 }
 if ( victim == ch )
 {
  send_to_char( "Ambush yourself? You'd probably notice.\n\r", ch );
  return;
 }

 if(is_safe(ch,victim))
  return;

 if ( get_eq_char( ch, WEAR_WIELD )  == NULL)
 {
  send_to_char( "{hYou need to wield a primary weapon to ambush.{x\n\r", ch );
  return;
 }

 if ( ( ch->fighting == NULL )
 && ( !IS_NPC( ch ) )
 && ( !IS_NPC( victim ) ) )
 {
  ch->attacker = TRUE;
  victim->attacker = FALSE;
 }

 WAIT_STATE( ch, 24 );
 if ( number_percent() < skill)
 {
   check_improve(ch,gsn_ambush,TRUE,1);
   multi_hit( ch, victim, gsn_ambush );
 }
 else
 {
   check_improve(ch,gsn_ambush,FALSE,1);
   damage( ch, victim, 0, gsn_ambush,DAM_NONE,TRUE);
 }
 return;
}


/***************************************************************************
*   Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
*   Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
*                                                                          *
*   Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
*   Chastain, Michael Quan, and Mitchell Tse.                              *
*                                                                          *
*   In order to use any part of this Merc Diku Mud, you must comply with   *
*   both the original Diku license in 'license.doc' as well the Merc       *
*   license in 'license.txt'.  In particular, you may not remove either of *
*   these copyright notices.                                               *
*                                                                          *
*   Much time and thought has gone into this software and you are          *
*   benefitting.  We hope that you share your changes too.  What goes      *
*   around, comes around.                                                  *
***************************************************************************/
/***************************************************************************
*       ROM 2.4 is copyright 1993-1995 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@pacinfo.com)                              *
*           Gabrielle Taylor (gtaylor@pacinfo.com)                         *
*           Brian Moore (rom@rom.efn.org)                                  *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file 'rom.license'                             *
***************************************************************************/
/***************************************************************************
*       ROT 2.0 is copyright 1996-1999 by Russ Walsh                       *
*       By using this code, you have agreed to follow the terms of the     *
*       ROT license, in the file 'rot.license'                             *
***************************************************************************/
/****************************************************************************
*  Automated Quest code written by Vassago of MOONGATE, moongate.ams.com    *
*  4000. Copyright (c) 1996 Ryan Addams, All Rights Reserved. Use of this   *
*  code is allowed provided you add a credit line to the effect of:         *
*  "Quest Code (c) 1996 Ryan Addams" to your logon screen with the rest     *
*  of the standard diku/rom credits. If you use this or a modified version  *
*  of this code, let me know via email: moongate@moongate.ams.com. Further  *
*  updates will be posted to the rom mailing list. If you'd like to get     *
*  the latest version of quest.c, please send a request to the above add-   *
*  ress. Quest Code v2.03. Please do not remove this notice from this file. *
****************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"
#include "tables.h"
#include "interp.h"

#define QUEST_OBJQUEST1 20200
#define QUEST_OBJQUEST2 20201
#define QUEST_OBJQUEST3 20202
#define QUEST_OBJQUEST4 20203
#define QUEST_OBJQUEST5 20204

struct quest_item 
{
 int vnum;
 int price;
};

#define MAX_QUEST_ITEMS 21 

struct quest_item quest_items[MAX_QUEST_ITEMS] =
{ {20206,169},
  {20207,150},
  {20208,302},
  {20209,129},
  {20210,119},
  {20211,177},
  {20212,184},
  {20213,160},   
  {20214,139},
  {20215,357},
  {20135,155},
  {20136,173},
  {20137,355},
  {20221,150},
  {20222,309},
  {20225,357},
  {20226,158},
  {20227,133},
  {20228,168},
  {20223,100},
  {20224,50},
 };

/*
 * Local functions.
 */
char *	find_quest_mob		args( ( CHAR_DATA *ch, bool lquest ) );
char *	make_lquest_obj		args( ( CHAR_DATA *ch ) );
bool	is_suitable_qmob	args( ( CHAR_DATA *ch,
				    CHAR_DATA *victim, bool lquest ) );
void generate_quest     args(( CHAR_DATA *ch, CHAR_DATA *questman ));
bool quest_level_diff   args(( int clevel, int mlevel));
bool chance             args(( int num ));
ROOM_INDEX_DATA         *find_location( CHAR_DATA *ch, char *arg );

/* CHANCE function. I use this everywhere in my code, very handy :> */

bool chance(int num)
{
    if (number_range(1,100) <= num) return TRUE;
    else return FALSE;
}

/* The main quest function */

void do_quest(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *questman;  // ,*nextquest
    OBJ_DATA *obj=NULL, *obj_next;
    OBJ_INDEX_DATA *questinfoobj;
    MOB_INDEX_DATA *questinfo;
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [9];
    int i=0;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);

    if (arg1[0] == '\0')
    {
        send_to_char("{CQ{cUES{CT{c commands: {DPOINTS INFO TIME REQUEST COMPLETE LIST\n\r",ch);
        send_to_char("                BUY IDENTIFY QUIT CONVERT REVERT SELL\n\r{x",ch);
        send_to_char("{cFor more information, type {D'{WHELP QUEST{D'{c.{x\n\r",ch);
        return;
    }
    if (!strcmp(arg1, "convert"))
    {
     if(ch->qps<=0)
     {
      send_to_char("You don't have any iquest points to convert.\n\r",ch);
      return;
     }
     ch->qps--;
     ch->questpoints+=20;
     send_to_char("One iquest point converted into 20 aquest points.\n\r",ch);
     return;
    }
    if (!strcmp(arg1,"revert"))
    {
     if(ch->questpoints<20)
     {
	send_to_char("You don't have enough aquest points to revert.\n\r",ch);
        return;
     }
     ch->qps++;
     ch->questpoints-=20;
     send_to_char("20 aquest points reverted into one iquest point.\n\r",ch);
     return;
    }
    if (!strcmp(arg1, "info"))
    {
        if (IS_SET(ch->exbit1, PLR_QUESTOR))
        {
            if (ch->questmob == -1 && ch->questgiver->short_descr != NULL)
            {
sprintf(buf,"Your quest is ALMOST complete!\n\rGet back to %s before your time runs out!\n\r",ch->questgiver->short_descr);
                send_to_char(buf, ch);
            }
            else if (ch->questobj > 0)
            {
                questinfoobj = get_obj_index(ch->questobj);
                if (questinfoobj != NULL)
                {
sprintf(buf, "You are on a quest to recover the fabled %s!\n\r",questinfoobj->name);
                    send_to_char(buf, ch);
                }
                else send_to_char("You aren't currently on a quest.\n\r",ch);
                return;
            }
            else if (ch->questmob > 0)
            {
                questinfo = get_mob_index(ch->questmob);
                if (questinfo != NULL)
                {
                    sprintf(buf, "You are on a quest to slay the dreaded %s!\n\r",questinfo->short_descr);
                    send_to_char(buf, ch);
                }
                else send_to_char("You aren't currently on a quest.\n\r",ch);
                return;
            }
        }
        else
            send_to_char("You aren't currently on a quest.\n\r",ch);
        return;
    }
    if (!strcmp(arg1, "points"))
    {
        sprintf(buf, "You have %d quest points.\n\r",ch->questpoints);
        send_to_char(buf, ch);
        return;
    }
    else if (!strcmp(arg1, "time"))
    {
        if (!IS_SET(ch->exbit1, PLR_QUESTOR))
        {
            send_to_char("You aren't currently on a quest.\n\r",ch);
            if (ch->nextquest > 1)
            {
                sprintf(buf, "There are %d minutes remaining until you can go on another quest.\n\r",ch->nextquest);
                send_to_char(buf, ch);
            }
            else if (ch->nextquest == 1)
            {
                sprintf(buf, "There is less than a minute remaining until you can go on another quest.\n\r");
                send_to_char(buf, ch);
            }
        }
        else if (ch->countdown > 0)
        {
            sprintf(buf, "Time left for current quest: %d\n\r",ch->countdown);
            send_to_char(buf, ch);
        }
        return;
    }

    for ( questman = ch->in_room->people; questman != NULL; questman = questman->next_in_room )
    {
        if (!IS_NPC(questman)) continue;
        if (questman->spec_fun == spec_lookup( "spec_questmaster" )) break;
    }

    if (questman == NULL || questman->spec_fun != spec_lookup( "spec_questmaster" ))
    {
        send_to_char("You can't do that here.\n\r",ch);
        return;
    }

    if ( questman->fighting != NULL)
    {
        send_to_char("Wait until the fighting stops.\n\r",ch);
        return;
    }

    ch->questgiver = questman;

    if (!strcmp(arg1, "list"))
    {
        act( "$n asks $N for a list of quest items.", ch, NULL, questman, TO_ROOM);
        act ("You ask $N for a list of quest items.",ch, NULL, questman, TO_CHAR);     
        send_to_char("{cNumber  Price ({DAQP{c)  Name{x\n\r",ch);
        for(i=0;i<MAX_QUEST_ITEMS;i++)
        {
         questinfoobj = get_obj_index(quest_items[i].vnum);
         sprintf(buf,"{D%-6d{c  {D%-11d{x  %-40s\n\r",
              i+1,quest_items[i].price,questinfoobj->short_descr);
         send_to_char(buf,ch);
        }        
        return;
    }
    else if (!strcmp(arg1, "identify"))
    {
     int number;
     if(arg2[0]=='\0')
     {
      send_to_char("Syntax: 'QUEST IDENTIFY <number>'\n\r",ch);
      return;
     }
     number = atoi(arg2);
     if(number<1 || number>MAX_QUEST_ITEMS)
     {
      sprintf(buf,"Number out of range (1 to %d).\n\r",MAX_QUEST_ITEMS);
      send_to_char(buf,ch);
      return; 
     }
     obj = create_object(get_obj_index(quest_items[number-1].vnum),-1);
     (*skill_table[skill_lookup("identify")].spell_fun)(skill_lookup("identify"),101,ch,(void*)obj,TAR_IGNORE);
     extract_obj(obj);
     return;
    } 
    
    else if (!strcmp(arg1, "buy"))
    {
       int number; 
        if (arg2[0] == '\0')
        {
            send_to_char("To buy an item, type 'QUEST BUY <number>'.\n\r",ch);
            return;
        }
       number = atoi(arg2);
     if(number<1||number>MAX_QUEST_ITEMS)
     {
      sprintf(buf,"Number out of range (1 to %d).\n\r",MAX_QUEST_ITEMS);
      send_to_char(buf,ch);
      return;
     }
     if(ch->questpoints-quest_items[number-1].price<0)
     {
      send_to_char("Not enough questpoints.\n\r",ch);
      return;
     }
     obj = create_object(get_obj_index(quest_items[number-1].vnum),-1);
     act("$n hands $N $p.",questman,obj,ch,TO_NOTVICT);
     act("$n hands you $p.",questman,obj,ch,TO_VICT);
     ch->questpoints -= quest_items[number-1].price;
     obj_to_char(obj,ch);
     printf_log("%s buys %s [Q] at %d.",
       ch->name,obj->short_descr,ch->in_room->vnum);
     return;
      
    }
    else if (!strcmp(arg1, "sell"))
    {
     int i;
     bool fIsQuest=FALSE;

     if(arg2[0]=='\0')
     {
      send_to_char("Quest sell what?\n\r",ch);
      return;
     }
     if((obj=get_obj_carry(ch,arg2))==NULL)
     {
      send_to_char("You are not carrying that.\n\r",ch);
      return;
     }
     if(!IS_OBJ_STAT(obj,ITEM_QUEST))
     {
      send_to_char("You can only sell quest items here.\n\r",ch);
      return;
     }
     for(i=0;i<MAX_QUEST_ITEMS;i++)
     {
      if(quest_items[i].vnum==obj->pIndexData->vnum)
      {
       fIsQuest=TRUE;
       break;
      }
     }
     if(!fIsQuest)
     {
      send_to_char("You can only sell quest items you could also buy here.\n\r",ch);
      return;
     }
     act("$n sells $p to $N.",ch,obj,questman,TO_NOTVICT);
     act("You sell $p to $N.",ch,obj,questman,TO_CHAR);
     sprintf(buf,"You receive %d questpoins.\n\r",quest_items[i].price/2);
     send_to_char(buf,ch);
     ch->questpoints+=(quest_items[i].price/2);
     printf_log("%s sells %s [Q] at %d.",
       ch->name,obj->short_descr,ch->in_room->vnum);
     obj_from_char(obj);
     extract_obj(obj);
     return; 
    } 
    else if (!strcmp(arg1, "request") && !IS_NPC(ch))
    {
        if(ch->level<21)
        {
         send_to_char("Get to level 21 first.\n\r",ch);
         return;
        }
        if(ch->on_quest==TRUE)
        {
         send_to_char("You are not allowed to aquest while iquesting.\n\r",ch);
         return;
        }
        act( "$n asks $N for a quest.", ch, NULL, questman, TO_ROOM);
        act ("You ask $N for a quest.",ch, NULL, questman, TO_CHAR);

        if (IS_SET(ch->exbit1, PLR_QUESTOR))
        {
            send_to_char("But you're already on a quest!\n\r",ch);
            return;
        }
	 

        if (ch->nextquest > 0)
        {
            sprintf(buf, "Now, don't be so greedy, %s, wait a little longer, will you?\n\r",ch->name);
            send_to_char( buf,ch);
            send_to_char("Come back later.\n\r",ch);
            return;
        }
        sprintf(buf, "Thank you, brave %s!\n\r",ch->name);
        send_to_char(buf,ch);
        ch->questmob = 0;
        ch->questobj = 0;
        generate_quest(ch, questman);
        if (ch->questmob > 0 || ch->questobj > 0)
        {
            ch->countdown = number_range(7,15);
            SET_BIT(ch->exbit1, PLR_QUESTOR);
            sprintf(buf, "You have %d minutes to complete this quest.\n\r",ch->countdown);
            send_to_char(buf,ch);
            send_to_char("May the gods go with you!\n\r",ch);
        }
        return;
    }
    else if (!strcmp(arg1, "complete"))
    {
act( "$n informs $N $e has completed $s quest.", ch, NULL, questman, TO_ROOM);
act ("You inform $N you have completed $s quest.",ch, NULL, questman, TO_CHAR);
        if (ch->questgiver != questman)
        {
	send_to_char(
"I never sent you on a quest! Perhaps you're thinking of someone else.\n\r",ch);
        return;
        }

        if (IS_SET(ch->exbit1, PLR_QUESTOR))
        {
            if(ch->on_quest==TRUE)
            {
             send_to_char("You are not allowed to aquest while iquesting.\n\r",ch);
             return;
            }
            if (ch->questmob == -1 && ch->countdown > 0)
            {
                int reward, pointreward, pracreward;
                reward = number_range(25,50);
                pointreward = number_range(1,10);
                sprintf(buf, "Congratulations on completing your quest!\n\r");
                send_to_char(buf,ch);
         	sprintf(buf, "As a reward, I am giving you %d aquest points, and %d platinum.\n\r",pointreward,reward); 
                send_to_char(buf,ch);
                if (chance(10))
                {
                    pracreward = number_range(2,6);
                    sprintf(buf, "You gain %d practices!\n\r",pracreward);
                    send_to_char(buf, ch);
                    ch->practice += pracreward;
                }

                REMOVE_BIT(ch->exbit1, PLR_QUESTOR);
                ch->questgiver = NULL;
                ch->countdown = 0;
                ch->questmob = 0;
                ch->questobj = 0;
                ch->nextquest = 15;  
                ch->platinum += reward;
                ch->questpoints += pointreward;

                return;
            }
            else if (ch->questobj > 0 && ch->countdown > 0)
            {
                bool obj_found = FALSE;

                for (obj = ch->carrying; obj != NULL; obj= obj_next)
                {
                    obj_next = obj->next_content;

                    if (obj != NULL && obj->pIndexData->vnum == ch->questobj)
                    {
                        obj_found = TRUE;
                        break;
                    }
                }
                if (obj_found == TRUE)
                {
                    int reward, pointreward, pracreward;
                    reward = number_range(25,50);
                    pointreward = number_range(1,10);
                    act("You hand $p to $N.",ch, obj, questman, TO_CHAR);
                    act("$n hands $p to $N.",ch, obj, questman, TO_ROOM);
                    sprintf(buf, "Congratulations on completing your quest!\n\r");
                    send_to_char(buf,ch);
                    sprintf(buf,"As a reward, I am giving you %d aquest points, and %d platinum.\n\r",pointreward,reward);
                    send_to_char(buf,ch);
                    if (chance(15))
                    {
                        pracreward = number_range(2,6);
                        sprintf(buf, "You gain %d practices!\n\r",pracreward);
                        send_to_char(buf, ch);
                        ch->practice += pracreward;
                    }
                    REMOVE_BIT(ch->exbit1, PLR_QUESTOR);
                    extract_obj(obj);
                    ch->questgiver = NULL;
                    ch->countdown = 0;
                    ch->questmob = 0;
                    ch->questobj = 0;
                    ch->nextquest = 15;
                    ch->platinum += reward;
                    ch->questpoints += pointreward;
                    return;
                }
                else
                {
  		sprintf(buf, "You haven't completed the quest yet, hurry there is still time!\n\r");
                send_to_char(buf,ch);
                    return;
                }
                return;
            }
        if ((ch->questmob > 0 || ch->questobj > 0) && ch->countdown > 0) 
            {
sprintf(buf, "You haven't completed the quest yet, but there is still time!\n\r");
               send_to_char(buf,ch); 
               return;
            }
        }
        if (ch->countdown > 0)
            sprintf(buf,"But you didn't complete your quest in time!\n\r");
        else if (!IS_SET(ch->exbit1, PLR_QUESTOR))
        sprintf(buf, "You have to REQUEST a quest first, %s.\n\r",ch->name);
        send_to_char( buf,ch);
        return;
    }
    else if(!strcmp(arg1,"quit"))
    {
     if(!IS_SET(ch->exbit1,PLR_QUESTOR))
     {
      send_to_char("But you aren't even on a quest!\n\r",ch);
      return;
     }
     act("$n informs $N that $e wants to quit $s quest.",ch,NULL,questman,TO_ROOM);
     act("You inform $N that you want to quit your quest.",ch,NULL,questman,TO_CHAR);
     ch->questgiver=NULL;
     ch->questmob=0;
     ch->questobj=0;
     ch->countdown=0;
     ch->nextquest=5;
     REMOVE_BIT(ch->exbit1,PLR_QUESTOR);
     send_to_char("Ok, quest quit, come back later and try again.\n\r",ch);
     return;
    }
    send_to_char("{CQ{cUES{CT{c commands: {DPOINTS INFO TIME REQUEST COMPLETE LIST\n\r",ch);
    send_to_char("                BUY IDENTIFY QUIT CONVERT REVERT SELL\n\r{x",ch);
    send_to_char("{cFor more information, type {D'{WHELP QUEST{D'{c.{x\n\r",ch);
    return;
}

void generate_quest(CHAR_DATA *ch, CHAR_DATA *questman)
{
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *room;
    OBJ_DATA *questitem;
    char buf [MAX_STRING_LENGTH];
for (victim = char_list; victim != NULL; victim = victim->next )
	{
	if (!IS_NPC(victim)) continue;
        if (quest_level_diff(ch->level, victim->level) == TRUE
            	&& victim->pIndexData != NULL
            	&& victim->pIndexData->pShop == NULL
            	&& !IS_SET(victim->imm_flags, IMM_SUMMON)
    		&& !IS_SET(victim->act, ACT_TRAIN)
    		&& !IS_SET(victim->act, ACT_PRACTICE)
    		&& !IS_SET(victim->act, ACT_IS_HEALER)
		&& !IS_SET(victim->act, ACT_PET)
		&& !IS_SET(victim->affected_by, AFF_CHARM)
		&& !IS_SET(victim->affected_by, SHD_INVISIBLE)
                && !strstr(victim->in_room->area->builders,"Unlinked")
                && !IS_SET(victim->in_room->room_flags,ROOM_NO_RECALL)
		&& chance(15) ) break; 
	
	}
	
    if ( victim == NULL  )
    {
        send_to_char("I'm sorry, but I don't have any quests for you at this time.\n\r",ch);
        send_to_char("Try again later.\n\r",ch);
        ch->nextquest = 1;
        return;
    }

    if ( ( room = find_location( ch, victim->name ) ) == NULL )
    {
sprintf(buf, "I'm sorry, but I don't have any quests for you at this time.\n\r");
        send_to_char(buf,ch);
        sprintf(buf, "Try again later.\n\r");
        send_to_char(buf,ch);
        ch->nextquest = 1; 
        return;
    }


    if (chance(40))
    {
        int objvnum = 0;

        switch(number_range(0,4))
        {
            case 0:
            objvnum = QUEST_OBJQUEST1;
            break;

            case 1:
            objvnum = QUEST_OBJQUEST2;
            break;

            case 2:
            objvnum = QUEST_OBJQUEST3;
            break;

            case 3:
            objvnum = QUEST_OBJQUEST4;
            break;

            case 4:
            objvnum = QUEST_OBJQUEST5;
            break;
        }

        questitem = create_object( get_obj_index(objvnum), ch->level );
        obj_to_room(questitem, room);
        ch->questobj = questitem->pIndexData->vnum;

        sprintf(buf, "Vile pilferers have stolen %s from the royal treasury!\n\r",questitem->short_descr);
        send_to_char(buf,ch);
        send_to_char("My court wizardess, with her magic mirror, has pinpointed its location.\n\r",ch);

        sprintf(buf, "Look in the general area of %s for %s!\n\r",room->area->name, room->name);
        send_to_char(buf,ch);
        return;
    }


    else
    {
    switch(number_range(0,1))
    {
        case 0:
sprintf(buf, "An enemy of mine, %s, is making vile threats against the crown.\n\r",victim->short_descr);
        send_to_char(buf,ch);
        send_to_char("This threat must be eliminated!\n\r",ch);
        break;

        case 1:
sprintf(buf,"One of the most heinous criminals, %s, has escaped from the prison!\n\r",victim->short_descr); 

        send_to_char(buf,ch);
        sprintf(buf, "Since the escape, %s has murdered %d civillians!\n\r",victim->short_descr, number_range(2,20));
        send_to_char( buf,ch);
        send_to_char("The penalty for this crime is death and you are to deliver the sentence!\n\r",ch);
        break;
    }

    if (room->name != NULL)
    {
        sprintf(buf, "Seek %s out somewhere in the vicinity of %s!\n\r",victim->short_descr,room->name);
        send_to_char( buf,ch);

        sprintf(buf, "That location is in the general area of %s.\n\r",room->area->name);
        send_to_char( buf,ch);
    }
    ch->questmob = victim->pIndexData->vnum;
    }
    return;
}


bool quest_level_diff(int clevel, int mlevel)
{

if (clevel < 5 && mlevel < 9) return TRUE;
else if 
(clevel > 4 && clevel < 11 && mlevel < 16) return TRUE;
else if 
(clevel > 10 && clevel < 15 && mlevel > 5 && mlevel < 20) return TRUE;
else if 
(clevel > 14 && clevel < 25 && mlevel > 10 && mlevel < 30) return TRUE;
else if
(clevel > 24 && clevel < 35 && mlevel > 15 && mlevel < 40) return TRUE;
else if
(clevel > 34 && clevel < 45 && mlevel > 20 && mlevel < 50) return TRUE;
else if
(clevel > 44 && clevel < 55 && mlevel > 25 && mlevel < 60) return TRUE;
else if
(clevel > 54 && clevel < 65 && mlevel > 30 && mlevel < 70) return TRUE;
else if
(clevel > 64 && clevel < 75 && mlevel > 35 && mlevel < 80) return TRUE;
else if
(clevel > 74 && clevel < 85 && mlevel > 40 && mlevel < 90) return TRUE;
else if
(clevel > 84 && clevel < 95 && mlevel > 45 && mlevel < 100) return TRUE;
else if
(clevel > 94 && clevel < 105 && mlevel >50 && mlevel < 110) return TRUE;
else if
(clevel > 104 && clevel < 115 && mlevel > 55 && mlevel < 120) return TRUE;
else return FALSE;

}


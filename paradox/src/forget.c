/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
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
#include "merc.h"
#include "tables.h"

void show_damlist(CHAR_DATA *ch);
bool show_help(CHAR_DATA *ch,char *argument);
int flag_value(const struct flag_type *flag_table,char *argument);
char *flag_string(const struct flag_type *flag_table, int bits);

void do_forge(CHAR_DATA *ch, char *argument)
{
 char arg1[MAX_INPUT_LENGTH]; /* weapon class */
 char arg2[MAX_INPUT_LENGTH]; /* damage type  */
 char flag[3][MAX_INPUT_LENGTH]; /* first/second/third flag */
 int wc=0,dt=0,flags=0,i;
 OBJ_DATA *obj;
 char buf[MAX_STRING_LENGTH];
 char numstr[31];

 argument = one_argument(argument,arg1);
 argument = one_argument(argument,arg2);
 argument = one_argument(argument,flag[0]);
 argument = one_argument(argument,flag[1]);
 argument = one_argument(argument,flag[2]);

 if(IS_NPC(ch))
 {
  send_to_char("You are an NPC. Go away.\n\r",ch);
  return;
 }

 if((ch->in_room==NULL ) || (ch->in_room->vnum!=20133))
 {
  send_to_char("Does this look like a forge to you?\n\r",ch);
  return;
 }
 if(arg1[0]=='\0'/*||arg2[0]=='\0'||flag[0][0]=='\0'||flag[1][0]=='\0'||
     flag[2][0]=='\0'*/)
 {
  send_to_char("{cSyntax{D:{x forge <weaponclass> <damagetype> <flag1> <flag2> <flag3>\n\r",ch);
  send_to_char("        forge wclass {D- {clist available weapon classes{x\n\r",ch);
  send_to_char("	forge dtype  {D- {clist available damage types{x\n\r",ch);
  send_to_char("	forge flags  {D- {clist available weapon flags{x\n\r",ch);
  return;
 }
 if(!strcmp(arg1,"wclass"))
 {
  send_to_char("{cValid weapon classes are{D:{x\n\r",ch);
  show_help(ch,"wclass");
  return;
 }
 else if(!strcmp(arg1,"dtype"))
 { 
  send_to_char("{cValid damage types are{D:{x\n\r",ch);
  show_damlist(ch);
  return;
 }
 else if(!strcmp(arg1,"flags"))
 {
  send_to_char("{cValid weapon flags are{D:{x\n\r",ch);
  show_help(ch,"wtype");
  return;
 }
 else if(arg2[0]=='\0'||flag[0][0]=='\0'||
     flag[1][0]=='\0'||flag[2][0]=='\0')
 {
   do_forge(ch,""); 
   return;
 }
 if(ch->questpoints-400<0)
 {
  send_to_char("Forging a weapon costs 400 aquest points. You don't have enough.\n\r",ch);
  return;
 }
 if((weapon_lookup(arg1)==-1)&&strcmp(arg1,"exotic"))
 {
  send_to_char("This weapon class is not available. Valid classes are:\n\r",ch);
  show_help(ch,"wclass");
  return;
 }
 wc=weapon_type(arg1);
 if(((dt=attack_lookup(arg2))==0)
     || ((dt>=40) && (dt<=60)))
 {
  send_to_char("This damage type is not available. Valid types are:\n\r",ch);
  show_damlist(ch);
  return;
 }
 for(i=0;i<3;i++)
 {
  switch(i)
  {
   case 0: sprintf(numstr,"first"); break;
   case 1: sprintf(numstr,"second"); break;
   case 2: sprintf(numstr,"third"); break;
  }
  if(flag_value(weapon_type2,flag[i])==NO_FLAG)
  {
   sprintf(buf,"The %s flag you have specified is not available. Valid flags are:\n\r",numstr);
   send_to_char(buf,ch);
   show_help(ch,"wtype");
   return;
  }
  if(IS_SET(flags,flag_value(weapon_type2,flag[i])))
  {
   send_to_char("You can't use a weapon flag twice.\n\r",ch);
   return;
  }
  flags^=flag_value(weapon_type2,flag[i]);
 }
 obj=create_object(get_obj_index(20254),-1);
 ch->questpoints-=400;
 free_string(obj->name);
 sprintf(buf,"forged %s",((wc==0)?("weapon"):(weapon_class[wc].name)));
 obj->name=str_dup(buf);
 free_string(obj->short_descr);
 sprintf(buf,"{DA {Rf{ro{Wr{xg{re{Rd {D%s{x",((wc==0)?("weapon"):(weapon_class[wc].name))); 
 obj->short_descr=str_dup(buf);
 free_string(obj->description);
 sprintf(buf,"%s{D is lying here.{x",buf);
 obj->description=str_dup(buf);
 obj->value[0]=wc;
 obj->value[3]=dt;
 obj->value[4]=flags;
 obj_to_char(obj,ch);
 act("$n forges $p.",ch,obj,NULL,TO_ROOM);
 act("You forge $p.",ch,obj,NULL,TO_CHAR);
 send_to_char("400 aquest points have been deducted.\n\r",ch);
 printf_log("%s forges a %s (%s/%s) at %d.",
   ch->name,weapon_class[wc].name,attack_table[dt].name,
    flag_string(weapon_type2,flags),ch->in_room->vnum);   
 return;
}

void do_forget(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *rch;
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int pos;
    bool found = FALSE;

    if (ch->desc == NULL)
	rch = ch;
    else
	rch = ch->desc->original ? ch->desc->original : ch;

    if (IS_NPC(rch))
	return;

    smash_tilde( argument );

    argument = one_argument(argument,arg);
    
    if (arg[0] == '\0')
    {
	if (rch->pcdata->forget[0] == NULL)
	{
	    send_to_char("You are not forgetting anyone.\n\r",ch);
	    return;
	}
	send_to_char("{mYou are currently forgetting{x:\n\r",ch);

	for (pos = 0; pos < MAX_FORGET; pos++)
	{
	    if (rch->pcdata->forget[pos] == NULL)
		break;

	    sprintf(buf,"{G-    {D%s{x\n\r",capitalize(rch->pcdata->forget[pos]));
	    send_to_char(buf,ch);
	}
	return;
    }

    for (pos = 0; pos < MAX_FORGET; pos++)
    {
	if (rch->pcdata->forget[pos] == NULL)
	    break;

	if (!str_cmp(arg,rch->pcdata->forget[pos]))
	{
	    send_to_char("You have already forgotten that person.\n\r",ch);
	    return;
	}
    }

    for (d = descriptor_list; d != NULL; d = d->next)
    {
	CHAR_DATA *wch;

 	if (d->connected != CON_PLAYING || !can_see(ch,d->character))
	    continue;
	
	wch = ( d->original != NULL ) ? d->original : d->character;

 	if (!can_see(ch,wch))
	    continue;

	if (!str_cmp(arg,wch->name))
	{
	    found = TRUE;
	    if (wch == ch)
	    {
		send_to_char("You forget yourself for a moment, but it passes.\n\r",ch);
		return;
	    }
	    if (wch->level >= LEVEL_IMMORTAL)
	    {
		send_to_char("That person is very hard to forget.\n\r",ch);
		return;
	    }
	}
    }

    if (!found)
    {
	send_to_char("No one by that name is playing.\n\r",ch);
	return;
    }

    for (pos = 0; pos < MAX_FORGET; pos++)
    {
	if (rch->pcdata->forget[pos] == NULL)
	    break;
     }

     if (pos >= MAX_FORGET)
     {
	send_to_char("Sorry, you have reached the forget limit.\n\r",ch);
	return;
     }
  
     /* make a new forget */
     rch->pcdata->forget[pos]		= str_dup(arg);
     sprintf(buf,"You are now deaf to %s.\n\r",capitalize(arg));
     send_to_char(buf,ch);
}

void do_remembe(CHAR_DATA *ch, char *argument)
{
    send_to_char("I'm sorry, remember must be entered in full.\n\r",ch);
    return;
}

void do_remember(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *rch;
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    int pos;
    bool found = FALSE;
 
    if (ch->desc == NULL)
	rch = ch;
    else
	rch = ch->desc->original ? ch->desc->original : ch;
 
    if (IS_NPC(rch))
	return;
 
    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	if (rch->pcdata->forget[0] == NULL)
	{
	    send_to_char("You are not forgetting anyone.\n\r",ch);
	    return;
	}
	send_to_char("{mYou are currently forgetting{x:\n\r",ch);

	for (pos = 0; pos < MAX_FORGET; pos++)
	{
	    if (rch->pcdata->forget[pos] == NULL)
		break;

	    sprintf(buf,"{G-    {D%s{x\n\r",capitalize(rch->pcdata->forget[pos]));
	    send_to_char(buf,ch);
	}
	return;
    }

    for (pos = 0; pos < MAX_FORGET; pos++)
    {
	if (rch->pcdata->forget[pos] == NULL)
	    break;

	if (found)
	{
	    rch->pcdata->forget[pos-1]		= rch->pcdata->forget[pos];
	    rch->pcdata->forget[pos]		= NULL;
	    continue;
	}

	if(!strcmp(arg,rch->pcdata->forget[pos]))
	{
	    send_to_char("Forget removed.\n\r",ch);
	    free_string(rch->pcdata->forget[pos]);
	    rch->pcdata->forget[pos] = NULL;
	    found = TRUE;
	}
    }

    if (!found)
	send_to_char("No one by that name is forgotten.\n\r",ch);
}

void do_view(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *rch;
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int pos;
    bool found = FALSE;

    if (ch->desc == NULL)
	rch = ch;
    else
	rch = ch->desc->original ? ch->desc->original : ch;

    if (IS_NPC(rch))
	return;

    smash_tilde( argument );

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	if (rch->pcdata->view[0] == NULL)
	{
	    send_to_char("You are not viewing anyone.\n\r",ch);
	    return;
	}
	send_to_char("{mYou are currently viewing{x:\n\r",ch);

	for (pos = 0; pos < MAX_VIEW; pos++)
	{
	    if (rch->pcdata->view[pos] == NULL)
		break;

	    sprintf(buf,"{G-    {D%s{x\n\r",capitalize(rch->pcdata->view[pos]));
	    send_to_char(buf,ch);
	}
	return;
    }

    for (pos = 0; pos < MAX_VIEW; pos++)
    {
	if (rch->pcdata->view[pos] == NULL)
	    break;

	if (!str_cmp(arg,rch->pcdata->view[pos]))
	{
          for (pos = 0; pos < MAX_VIEW; pos++)
          {
	    if (rch->pcdata->view[pos] == NULL)
	    break;

	    if (found)
	    {
	     rch->pcdata->view[pos-1]		= rch->pcdata->view[pos];
	     rch->pcdata->view[pos]		= NULL;
	     continue;
	    }

	    if(!strcmp(arg,rch->pcdata->view[pos]))
	    {
	     send_to_char("View removed.\n\r",ch);
	     free_string(rch->pcdata->view[pos]);
	     rch->pcdata->view[pos] = NULL;
	     found = TRUE;
	    }
           }

           return;
	  }
    }

    for (d = descriptor_list; d != NULL; d = d->next)
    {
	CHAR_DATA *wch;

 	if (d->connected != CON_PLAYING || !can_see(ch,d->character))
	    continue;
	
	wch = ( d->original != NULL ) ? d->original : d->character;

 	if (!can_see(ch,wch))
	    continue;

	if (!str_cmp(arg,wch->name))
	{
	    found = TRUE;
	    if (wch == ch)
	    {
		send_to_char("View yourself? Not really.\n\r",ch);
		return;
	    }
	    if (wch->level >= LEVEL_IMMORTAL)
	    {
		send_to_char("Immortals can see you anyway.\n\r",ch);
		return;
	    }
	}
    }

    if (!found)
    {
	send_to_char("No one by that name is playing.\n\r",ch);
	return;
    }

    for (pos = 0; pos < MAX_VIEW; pos++)
    {
	if (rch->pcdata->view[pos] == NULL)
	    break;
     }

     if (pos >= MAX_VIEW)
     {
	send_to_char("Sorry, you have reached the view limit.\n\r",ch);
	return;
     }

     /* make a new view */
     rch->pcdata->view[pos]		= str_dup(arg);
     sprintf(buf,"You now view %s.\n\r",capitalize(arg));
     send_to_char(buf,ch);
}















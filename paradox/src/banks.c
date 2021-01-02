/* Copyright by Gregor Stipicic, 2001 */

#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "recycle.h"

#define ROOM_VNUM_BANK	20117

void do_deposit(CHAR_DATA *ch,char *argument)
{
 char buf[MAX_STRING_LENGTH];
 int amount;
 
 if(IS_NPC(ch))
  return;

 if(!is_number(argument))
 {
   send_to_char("Deposit how much platinum?\n\r",ch);
   return;
 }
 if(ch->in_room->vnum!=ROOM_VNUM_BANK)
 {
  send_to_char("That's not a bank.\n\r",ch);
  return;
 }
 amount=atoi(argument); 
 if(amount<1)
 {
   send_to_char("That's not a positive number.\n\r",ch);
   return;
 }
 if((amount%10)!=0)
 {
   send_to_char("Deposits must be made in multiples of 10 platinum.\n\r",ch);
   return;
 }
 if(amount>ch->platinum)
 {
   send_to_char("You don't have that much platinum.\n\r",ch);
   return;
 }
 if((ch->balance+amount)>30000)
 {
  send_to_char("The bank can only hold 30,000 platinum coins.\n\r",ch);
  return;
 }
 ch->balance+=amount;
 ch->platinum-=amount;
 sprintf(buf,"You deposit %d platinum.\n\r",amount);
 send_to_char(buf,ch);
 sprintf(buf,"Your new balance is %d platinum.\n\r",ch->balance);
 send_to_char(buf,ch);
 return;

}

void do_withdraw(CHAR_DATA *ch,char *argument)
{
 char buf[MAX_STRING_LENGTH];
 int amount;

 if(IS_NPC(ch))
  return;

 if(!is_number(argument))
 {
  send_to_char("Withdraw how much platinum?\n\r",ch);
  return;
 }
 
 if(ch->in_room->vnum!=ROOM_VNUM_BANK)
 {
  send_to_char("That's not a bank.\n\r",ch);
  return;
 }

 amount = atoi(argument);
 if(amount<1)
 {
  send_to_char("That's not a positive number.\n\r",ch);
  return;
 }
 if(((amount%10)!=0) && (ch->balance>=10))
 {
  send_to_char("Withdrawals must be made in multiples of 10 platinum.\n\r",ch);
  return;
 }
 if(amount>ch->balance)
 {
  send_to_char("You don't have that much platinum on your account.\n\r",ch);
  return;
 }
 
 ch->platinum+=amount;
 ch->balance-=amount;
 sprintf(buf,"You withdraw %d platinum.\n\r",amount);
 send_to_char(buf,ch);
 sprintf(buf,"Your new balance is %d platinum.\n\r",ch->balance);
 send_to_char(buf,ch);
 return;
}

void do_balance(CHAR_DATA *ch,char *argument)
{
 char buf[MAX_STRING_LENGTH];

 if(IS_NPC(ch))
  return;
 if(ch->in_room->vnum!=ROOM_VNUM_BANK)
 {
  send_to_char("That's not a bank.\n\r",ch);
  return;
 }
 sprintf(buf,"Your current balance is %d platinum.\n\r",ch->balance);
 send_to_char(buf,ch);
 return;
}

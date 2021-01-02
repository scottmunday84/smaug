/***************************************************************************
 *  This file contains auction code developed by Brian Babey, and any      *
 *  communication regarding it should be sent to [bbabey@iname.com]        *
 *  Web Address: http://www.erols.com/bribe/                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*       ROM 2.4 is copyright 1993-1995 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@pacinfo.com)                              *
*           Gabrielle Taylor (gtaylor@pacinfo.com)                         *
*           Brian Moore (rom@rom.efn.org)                                  *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "merc.h"
#include "recycle.h"

DECLARE_DO_FUN( do_auction );

void show_obj_stats( CHAR_DATA *ch, OBJ_DATA *obj );
void auction_channel( char * msg );

void do_auction( CHAR_DATA *ch, char * argument )
{
    long platinum=0, gold=0;
    OBJ_DATA *	obj;
    char arg1[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    int i;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( ch == NULL || IS_NPC(ch) )
	return;

    if ( arg1[0] == '\0')
    {
	if ( IS_SET(ch->comm,COMM_NOAUCTION) )
	{
	    REMOVE_BIT(ch->comm,COMM_NOAUCTION );
	    send_to_char("Auction channel is now ON.\n\r",ch);
	    return;
	}

	SET_BIT(ch->comm,COMM_NOAUCTION);
	send_to_char("Auction channel is now OFF.\n\r",ch);
	return;
    }

    if ( !str_cmp( arg1, "list" ) )
    {

	if ( !auction_number )
	{
	    send_to_char("There is nothing up for auction right now.\n\r",ch);
	    return;
	}
        send_to_char("{CT{cicke{Ct  B{cidde{Cr           B{ci{Cd           I{cte{Cm{x\n\r",ch);
        for(i=0;i<MAX_AUCTION;i++)
        {
         if(auction_info[i].item!=NULL)
         {
          sprintf(buf,"{D#%5d  {c%-15s{D  %-12ld  {x%s{x\n\r",
             auction_info[i].ticket,
             (auction_info[i].high_bidder!=NULL?
              auction_info[i].high_bidder->name:"None"),
             auction_info[i].current_bid,
             auction_info[i].item->short_descr);
          send_to_char(buf,ch);
         }
        }  
	return;
    }

    if ( !str_cmp( arg1, "bid" ) )
    {
	long bid;
        sh_int ticket;
        int index;

        if ( !auction_number )
        {
            send_to_char("There is nothing up for auction right now.\n\r",ch);
            return;
        }

	if ( arg2[0] == '\0' )
	{
	    send_to_char("You must enter a ticket number.\n\r",ch);
	    return;
	}
        if ( arg3[0] == '\0' )
        {
	    send_to_char("You must enter an amount to bid.\n\r",ch);
            return;
        }
        
        ticket = atoi( arg2 );
	bid = atol( arg3 );
        
        index = -1;
        for(i=0;i<MAX_AUCTION;i++)
         if(auction_info[i].ticket==ticket)
         {
          index = i;
          break;
         }
        if((index==-1) || (ticket==0))
        {
         send_to_char("No such ticket.\n\r",ch);
         return;
        }
        if(bid<0)
        {
         send_to_char("Only positive bids are possible.\n\r",ch);
         return;
        }
        if(auction_info[i].high_bidder==NULL)
        {
         if( bid < auction_info[index].current_bid)
         {
          sprintf(buf,"You must bid at least %ld gold.\n\r",
             auction_info[index].current_bid);
          send_to_char(buf,ch);
         }
        }
	else if ( bid <= auction_info[index].current_bid )
	{
	    sprintf(buf, "You must bid above the current bid of %ld gold.\n\r",
		auction_info[index].current_bid );
            send_to_char(buf,ch);
	    return;
	}
        if(auction_info[i].high_bidder==NULL)
        {
         if(bid < MINIMUM_BID)
         {
          sprintf(buf,"The minimum bid is %d gold.\n\r",MINIMUM_BID);
          send_to_char(buf,ch);
          return;
         }
        }
        else if ( bid < (MINIMUM_BID + auction_info[index].current_bid))
	{
	    sprintf( buf, "The minimum bid is %ld gold.\n\r",MINIMUM_BID
                   +auction_info[index].current_bid);
	    send_to_char(buf,ch);
	    return;
	}

	if ( (ch->gold + 100 * ch->platinum) < bid )
	{
	    send_to_char("You can't cover that bid.\n\r",ch);
	    return;
	}

	sprintf(buf, "{cTicket {D%d{c: {D%ld{c gold has been offered for {x%s{c.\n\r",
	    ticket,bid, auction_info[index].item->short_descr);
	auction_channel( buf );

	if ( auction_info[index].high_bidder != NULL )
	{
	  auction_info[index].high_bidder->platinum += auction_info[index].platinum_held;
          auction_info[index].high_bidder->gold += auction_info[index].gold_held;
	}

	gold = UMIN( ch->gold, bid );

	if ( gold < bid )
	{
	    platinum = ((bid - gold + 99 ) / 100 );
	    gold = bid - 100* platinum;
	}

	ch->platinum -= platinum;
	ch->gold -= gold;

	auction_info[index].gold_held		= gold;
	auction_info[index].platinum_held	= platinum;
	auction_info[index].high_bidder	= ch;
	auction_info[index].current_bid	= bid;
	auction_info[index].status	 	= 0;

	return;	
    }

    if ( !strcmp(arg1,"submit" ))
    {
     long bid;
     int index;

     if(auction_number+1>=MAX_AUCTION)
     {
 	sprintf(buf,"Please wait, not more then %d items can be auctioned simultaneously.\n\r",MAX_AUCTION);
        send_to_char(buf,ch);
	return;
     }
 
     if(arg2[0]=='\0')
     {
       send_to_char("What do you want to auction?\n\r",ch);
       return;
     }
     if(arg3[0]=='\0')
     {
      bid=100;
     }
     else
     {
      bid = atol( arg3 );
      if(bid<MINIMUM_BID)
      {
       sprintf(buf,"Starting bid has to be higher than minimum bid (%d gold).\n\r",MINIMUM_BID);
       send_to_char(buf,ch);
       return;
      }
     }
 
    if ( (obj = get_obj_carry( ch, arg2 )) == NULL )
    {
	send_to_char("You aren't carrying that item.\n\r",ch);
	return;
    }
    if	( (obj->cost<=0))
    {
	send_to_char("You can't auction trash.\n\r",ch);
        return;
    }
    index=-1;
    for(i=0;i<MAX_AUCTION;i++)
     if(auction_info[i].item==NULL)
     {
      index = i;
      break;
     }
    if(index==-1)
    {
     send_to_char("Error: No free auction slot.\n\r",ch);
     bug("auction submit: no free slot",0);
     return;
    }
     
    auction_number++;
    if(auction_ticket == MAX_TICKET)
     auction_ticket = 0;
    else
     auction_ticket++;
    
    auction_info[index].owner		= ch;
    auction_info[index].item		= obj;
    auction_info[index].current_bid	= bid;
    auction_info[index].status		= 0;
    auction_info[index].ticket 		= auction_ticket;

    sprintf(buf,"{cTicket {D%d{c: {cNow taking bids on {x%s{c for {D%ld{c gold.\n\r", auction_ticket, obj->short_descr,bid );
    auction_channel( buf );

    obj_from_char( obj );

    return;
  }
 
  if(!strcmp(arg1,"info"))
  {
   int ticket;
   int index;
  
   if(arg2[0]=='\0')
   {
    send_to_char("You need to supply a ticket number.\n\r",ch);
    return;
   }
   ticket = atoi( arg2 );
   index = -1;
   for(i=0;i<MAX_AUCTION;i++)
    if(auction_info[i].ticket == ticket)
    {
     index = i;
     break;
    }
   if((index==-1) || (ticket==0))
   {
    send_to_char("Invalid ticket number.\n\r",ch); 
    return;
   }
   show_obj_stats(ch,auction_info[index].item);   
   return;
 }
  do_help(ch,"auction");
  return;
}

void auction_update( )
{
    char buf[MAX_STRING_LENGTH], temp[MAX_STRING_LENGTH],
	temp1[MAX_STRING_LENGTH];
    int i;

    if ( !auction_number )
	return;

    for(i=0;i<MAX_AUCTION;i++)
    {
     if(auction_info[i].item==NULL)
      continue;
     auction_info[i].status++;

     if ( auction_info[i].status == AUCTION_LENGTH )
     {
	sprintf(buf,"{cTicket {D%d{c: {x%s {RS{rOL{RD{c to {c%s{c for {D%ld{c gold.\n\r",
            auction_info[i].ticket,
	    auction_info[i].item->short_descr,
	    auction_info[i].high_bidder->name,
	    auction_info[i].current_bid );
	auction_channel( buf );

	auction_info[i].owner->platinum += auction_info[i].platinum_held;
	auction_info[i].owner->gold += auction_info[i].gold_held;

	sprintf(temp1, "%ld platinum ", auction_info[i].platinum_held );
	sprintf(temp,  "%ld gold ", auction_info[i].gold_held );
	sprintf(buf, "You receive %s%s%scoins.\n\r",
		auction_info[i].platinum_held > 0 ? temp1 : "",
		((auction_info[i].platinum_held > 0) &&
		 (auction_info[i].gold_held > 0)) ? "and " : "",
		auction_info[i].gold_held > 0 ? temp : "" );
	send_to_char( buf, auction_info[i].owner );
		
	obj_to_char( auction_info[i].item, auction_info[i].high_bidder );

	sprintf(buf, "%s{x appears in your hands.\n\r",
		auction_info[i].item->short_descr );
	send_to_char( buf, auction_info[i].high_bidder );
	
	auction_info[i].item		= NULL;
	auction_info[i].owner		= NULL;
	auction_info[i].high_bidder	= NULL;
	auction_info[i].current_bid	= 0;
	auction_info[i].status		= 0;
	auction_info[i].gold_held	= 0;
	auction_info[i].platinum_held	= 0;
        auction_info[i].ticket		= 0;
        auction_number--;
       
        continue;
    }

    if ( auction_info[i].status == AUCTION_LENGTH - 1 )
    {
	sprintf(buf, "{cTicket {D%d{c: {x%s{c - going twice at {D%ld{c gold.\n\r",
		auction_info[i].ticket,
                auction_info[i].item->short_descr,
		auction_info[i].current_bid );
	auction_channel( buf );
        continue;
    }

    if ( auction_info[i].status == AUCTION_LENGTH - 2 )
    {
	if ( auction_info[i].high_bidder == NULL )
	{
	    sprintf(buf, "{cTicket {D%d{c: {cNo bids on {x%s{c - item removed.\n\r",
	      auction_info[i].ticket,	
              auction_info[i].item->short_descr);
	    auction_channel( buf );

	    obj_to_char( auction_info[i].item, auction_info[i].owner );

	sprintf(buf, "{x%s{x is returned to you.\n\r",
		auction_info[i].item->short_descr );
	send_to_char( buf, auction_info[i].owner );
	
	    auction_info[i].item           = NULL;
	    auction_info[i].owner          = NULL;
	    auction_info[i].current_bid    = 0;
	    auction_info[i].status         = 0;
            auction_info[i].ticket	   = 0;
            auction_number--;

            continue;
	}

        sprintf(buf, "{cTicket {D%d{c: {x%s{c - going once at {D%ld{c gold.\n\r",
                auction_info[i].ticket,
                auction_info[i].item->short_descr,
                auction_info[i].current_bid );
        auction_channel( buf );
        continue;
    }
  }
    return;

}

void auction_channel( char * msg )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    sprintf(buf, "\n\r{D[{CA{cUCTIO{CN{D]{W %s{x", msg ); /* Add color if you wish */

      for ( d = descriptor_list; d != NULL; d = d->next )
      {
	CHAR_DATA *victim;

	victim = d->original ? d->original : d->character;

	if ( d->connected == CON_PLAYING &&
	     !IS_SET(victim->comm,COMM_NOAUCTION) &&
	     !IS_SET(victim->comm,COMM_QUIET) )
	      {
		send_to_char( buf, victim );
	      }
      }

    return;
}

void show_obj_stats( CHAR_DATA *ch, OBJ_DATA *obj )
{
    BUFFER *outlist;
    (*skill_table[skill_lookup("identify")].spell_fun)
      (skill_lookup("identify"),101,ch,(void*)obj,TAR_IGNORE);
    if(obj->item_type==ITEM_CONTAINER)
    {
      act("$p holds:",ch,obj,NULL,TO_CHAR);
      outlist=show_list_to_char(obj->contains,ch,TRUE,TRUE);
      page_to_char(buf_string(outlist),ch);
      free_buf(outlist);
    }
     
    return;
}

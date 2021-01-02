From: ferric@UWYO.EDU

Ok, the following code was inspired by Erwin Andreasons code which does a
similiar thing for merc, I believe.  I also used a couple variable names
he did, but for the most part, the actual code is my own (besides that
ripped from rom, of course).

This code is freely distributable, although I appreciate knowing that what
I'm doing is making a difference, so email me if you use this, please.
---------------------------------------------------------------------------

Lets get started.

In merc.h find:

#if defined(unix)
#define PLAYER_DIR      "../player/"            /* Player files */
#define GOD_DIR         "../gods/"              /* list of gods */
#define NULL_FILE       "/dev/null"             /* To reserve one stream*/
#endif

and change it to:

#if defined(unix)
#define LAST_COMMAND    "../last_command.txt"  /*For the signal handler.*/
#define PLAYER_DIR      "../player/"            /* Player files */
#define GOD_DIR         "../gods/"              /* list of gods */
#define NULL_FILE       "/dev/null"             /* To reserve one stream*/
#endif


then find:
extern          WEATHER_DATA            weather_info;

and add this under it:

extern          char                    last_command[MAX_STRING_LENGTH];


At the top of comm.c, under the declares add:

void init_signals   args( (void) );
void do_auto_shutdown args( (void) );

then find:

#if defined(unix)
    control = init_socket( port );

and under it add:

    init_signals(); /* For the use of the signal handler. -Ferric */

then add:

void init_signals()
{
  signal(SIGBUS,sig_handler);
  signal(SIGTERM,sig_handler);
  signal(SIGABRT,sig_handler);
  signal(SIGSEGV,sig_handler);
}

and:

void sig_handler(int sig)
{
switch(sig)
  {
  case SIGBUS:
    bug("Sig handler SIGBUS.",0);
    do_auto_shutdown();
    break;
  case SIGTERM:
    bug("Sig handler SIGTERM.",0);
    do_auto_shutdown();
    break;
  case SIGABRT:
    bug("Sig handler SIGABRT",0);  
    do_auto_shutdown();
    break;
  case SIGSEGV:
    bug("Sig handler SIGSEGV",0);
    do_auto_shutdown();
    break;
  }
}


Then, in act_wiz.c add:

void do_auto_shutdown()
{

/*This allows for a shutdown without somebody in-game actually calling it.
		-Ferric*/
    FILE *fp;
    extern bool merc_down;
    DESCRIPTOR_DATA *d,*d_next;
    merc_down = TRUE;

    /* This is to write to the file. */
    fclose(fpReserve);
    if((fp = fopen(LAST_COMMAND,"a")) == NULL)
      bug("Error in do_auto_save opening last_command.txt",0);
   
      fprintf(fp,"Last Command: %s\n",
            last_command);

    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );

    for ( d = descriptor_list; d != NULL; d = d_next)
      {
      if(d->character)
	  do_save (d->character, "");
      d_next = d->next;
      close_socket(d);
      }
    return;
}

next, in interp.c

near the top add:

char    last_command[MAX_STRING_LENGTH];

then find:

    /*
     * Grab the command word.
     * Special parsing so ' can be a command,
     *   also no spaces needed after punctuation.
     */
    strcpy( logline, argument );

under it add:

    /*Lets see who is doing what? -Ferric*/
    strcpy( buf, argument);
    sprintf(last_command,"%s in room[%d]: %s.",ch->name,
            ch->in_room->vnum,
            buf);


--------------------------------------------------------------------------
Ok, a few things.  One, I always feel like I've forgotten something in
this, so read over it carefully...if it doesn't seem complete, just let me
know and I'll go over it again to see what I missed.

Two, for gods sake, make sure you know what this is doing.  It should work
if you just plug it in, but it is highly configurable (I'm curious if it
can be used coupled with hot-reboot..that's on my to-do list).  Try 'man
signals' if you are on linux/bsd, and read them over.  If you are familiar
with signal handlers, then naturally this probably doesn't apply to you.

Three, I'm %98 sure this works.  Since I'm not live right now, I've not
tested it on a 2nd/3rd party (that's what you guys are for ;) ).  Again,
if there is a flaw in this, let me know.

Anyways, enjoy, and drop me a line if anybody uses this.

			-Ferric
			ferric@uwyo.edu		
			MelmothMUD

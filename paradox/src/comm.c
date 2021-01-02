/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku vMud improvments copyright (C) 1992, 1993 by Michael         *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Thanks to abaddon for proof-reading our comm.c and pointing out bugs.  *
 *  Any remaining bugs are, of course, our work, not his.  :)              *
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

/*
 * This file contains all of the OS-dependent stuff:
 *   startup, signals, BSD sockets for tcp/ip, i/o, timing.
 *
 * The data flow for input is:
 *    Game_loop ---> Read_from_descriptor ---> Read
 *    Game_loop ---> Read_from_buffer
 *
 * The data flow for output is:
 *    Game_loop ---> Process_Output ---> Write_to_descriptor -> Write
 *
 * The OS-dependent functions are Read_from_descriptor and Write_to_descriptor.
 * -- Furey  26 Jan 1993
 */

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#include "merc.h"
#include "recycle.h"
#include "tables.h"

/* command procedures needed */
DECLARE_DO_FUN(do_help		);
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_skills	);
DECLARE_DO_FUN(do_outfit	);
DECLARE_DO_FUN(do_unread	);
DECLARE_DO_FUN(do_announce      );
DECLARE_DO_FUN(do_pack		);
DECLARE_DO_FUN(do_autoall	);

/*
 * Malloc debugging stuff.
 */
#if defined(sun)
#undef MALLOC_DEBUG
#endif

#if defined(MALLOC_DEBUG)
#include <malloc.h>
extern	int	malloc_debug	args( ( int  ) );
extern	int	malloc_verify	args( ( void ) );
#endif



/*
 * Signal handling.
 * Apollo has a problem with __attribute(atomic) in signal.h,
 *   I dance around it.
 */
#if defined(apollo)
#define __attribute(x)
#endif

#if defined(unix)
#include <signal.h>
#endif

#if defined(apollo)
#undef __attribute
#endif



/*
 * Socket and TCP/IP stuff.
 */
#if	defined(macintosh) || defined(MSDOS)
const	char	echo_off_str	[] = { '\0' };
const	char	echo_on_str	[] = { '\0' };
const	char 	go_ahead_str	[] = { '\0' };
#endif

#if	defined(unix)
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "telnet.h"
const	char	echo_off_str	[] = { IAC, WILL, TELOPT_ECHO, '\0' };
const	char	echo_on_str	[] = { IAC, WONT, TELOPT_ECHO, '\0' };
const	char 	go_ahead_str	[] = { IAC, GA, '\0' };
#endif



/*
 * OS-dependent declarations.
 */
#if	defined(_AIX)
#include <sys/select.h>
/*int	accept		args( ( int s, struct sockaddr *addr, int *addrlen
) );*/
/*int	bind		args( ( int s, struct sockaddr *name, int namelen
) );*/
void	bzero		args( ( char *b, int length ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
#endif

#if	defined(apollo)
#include <unistd.h>
void	bzero		args( ( char *b, int length ) );
#endif

#if	defined(__hpux)
/*int	accept		args( ( int s, void *addr, int *addrlen ) );*/
/*int	bind		args( ( int s, const void *addr, int addrlen )
);*/
void	bzero		args( ( char *b, int length ) );
int	getpeername	args( ( int s, void *addr, int *addrlen ) );
int	getsockname	args( ( int s, void *name, int *addrlen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	setsockopt	args( ( int s, int level, int optname,
 				const void *optval, int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
#endif

#if	defined(interactive)
#include <net/errno.h>
#include <sys/fnctl.h>
#endif

#if	defined(linux)
/*int	accept		args( ( int s, struct sockaddr *addr, int *addrlen
) );*/
/*int	bind		args( ( int s, struct sockaddr *name, int namelen
) );*/
int	close		args( ( int fd ) );
/*int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );*/
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
/*int	listen		args( ( int s, int backlog ) );*/
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if	defined(macintosh)
#include <console.h>
#include <fcntl.h>
#include <unix.h>
struct	timeval
{
	time_t	tv_sec;
	time_t	tv_usec;
};
#if	!defined(isascii)
#define	isascii(c)		( (c) < 0200 )
#endif
static	long			theKeys	[4];

int	gettimeofday		args( ( struct timeval *tp, void *tzp ) );
#endif

#if	defined(MIPS_OS)
extern	int		errno;
#endif

#if	defined(MSDOS)
int	gettimeofday	args( ( struct timeval *tp, void *tzp ) );
int	kbhit		args( ( void ) );
#endif

#if	defined(NeXT)
int	close		args( ( int fd ) );
int	fcntl		args( ( int fd, int cmd, int arg ) );
#if	!defined(htons)
u_short	htons		args( ( u_short hostshort ) );
#endif
#if	!defined(ntohl)
u_long	ntohl		args( ( u_long hostlong ) );
#endif
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if	defined(sequent)
/*int	accept		args( ( int s, struct sockaddr *addr, int *addrlen
) );*/
/*int	bind		args( ( int s, struct sockaddr *name, int namelen
) );*/
int	close		args( ( int fd ) );
int	fcntl		args( ( int fd, int cmd, int arg ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
#if	!defined(htons)
u_short	htons		args( ( u_short hostshort ) );
#endif
int	listen		args( ( int s, int backlog ) );
#if	!defined(ntohl)
u_long	ntohl		args( ( u_long hostlong ) );
#endif
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	setsockopt	args( ( int s, int level, int optname, caddr_t optval,
			    int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

/* This includes Solaris Sys V as well */
#if defined(sun)
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	close		args( ( int fd ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
#if defined(SYSV)
int setsockopt		args( ( int s, int level, int optname,
			    const char *optval, int optlen ) );
#else
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
#endif
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if defined(ultrix)
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	close		args( ( int fd ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

void init_signals 	args( (void) );
void do_auto_shutdown	args( (void) );

/*
 * Global variables.
 */
DESCRIPTOR_DATA *   descriptor_list;	/* All open descriptors		*/
DESCRIPTOR_DATA *   d_next;		/* Next descriptor in loop	*/
FILE *		    fpReserve;		/* Reserved file handle		*/
bool		    god;		/* All new chars are gods!	*/
bool		    merc_down;		/* Shutdown			*/
bool		    wizlock = FALSE;		/* Game is no longer wizlocked		*/
bool		    newlock;		/* Game is newlocked		*/
char		    str_boot_time[MAX_INPUT_LENGTH];
time_t		    current_time;	/* time of this pulse */	
char		    clcode[ MAX_INPUT_LENGTH ];
bool		    MOBtrigger = TRUE;  /* act() switch                 */
ALLOW_DATA	    *allow_list=NULL;
PKILL_DATA	    *pkill_list=NULL;
RANK_DATA	    *rank_list=NULL;



/*
 * OS-dependent local functions.
 */
#if defined(macintosh) || defined(MSDOS)
void	game_loop_mac_msdos	args( ( void ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
bool	write_to_descriptor	args( ( int desc, char *txt, int length ) );
#endif

#if defined(unix)
void	game_loop_unix		args( ( int control, int wwwcontrol ) );
int	init_socket		args( ( int port ) );
void	init_descriptor		args( ( int control ) );
void	init_descriptor_www	args( ( int wwwcontrol ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
bool	write_to_descriptor	args( ( int desc, char *txt, int length ) );
#endif




/*
 * Other local functions (OS-independent).
 */
bool	check_parse_name	args( ( char *name ) );
bool	check_reconnect		args( ( DESCRIPTOR_DATA *d, char *name,
				    bool fConn ) );
bool	check_playing		args( ( DESCRIPTOR_DATA *d, char *name ) );
int	main			args( ( int argc, char **argv ) );
void	nanny			args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	process_output		args( ( DESCRIPTOR_DATA *d, bool fPrompt ) );
void	read_from_buffer	args( ( DESCRIPTOR_DATA *d ) );
void	stop_idling		args( ( CHAR_DATA *ch ) );
void    bust_a_prompt           args( ( CHAR_DATA *ch ) );

int port,control,wwwport,wwwcontrol;

int main( int argc, char **argv )
{
    struct timeval now_time;
    bool fCopyOver = FALSE;
/*    int port;*/
/*    int wwwport;*/

#if defined(unix)
/*    int control;*/
/*    int wwwcontrol;*/
#endif

    /*
     * Memory debugging if needed.
     */
#if defined(MALLOC_DEBUG)
    malloc_debug( 2 );
#endif

    /*
     * Init time.
     */
    gettimeofday( &now_time, NULL );
    current_time 	= (time_t) now_time.tv_sec;
    strcpy( str_boot_time, ctime( &current_time ) );

    /*
     * Macintosh console initialization.
     */
#if defined(macintosh)
    console_options.nrows = 31;
    cshow( stdout );
    csetmode( C_RAW, stdin );
    cecho2file( "log file", 1, stderr );
#endif

    /*
     * Reserve one channel for our use.
     */
    if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
    {
	perror( NULL_FILE );
	exit( 1 );
    }

    /*
     * Get the port number.
     */
    port = 8080;
    wwwport = 8081;
    if (argv[1] && argv[1][0])
    {
 	fCopyOver = TRUE;
 	control = atoi(argv[2]);
        wwwcontrol = atoi(argv[3]);
    }
    else
  	fCopyOver = FALSE;

/*    if ( argc > 1 )
    {
	if ( !is_number( argv[1] ) )
	{
	    fprintf( stderr, "Usage: %s [port #] [wwwport #]\n", argv[0] );
	    exit( 1 );
	}
	else if ( ( port = atoi( argv[1] ) ) <= 1024 )
	{
	    fprintf( stderr, "Port numbers must be above 1024.\n" );
	    exit( 1 );
	}
    }
    if ( argc > 2 )
    {
	if ( !is_number( argv[2] ) )
	{
	    fprintf( stderr, "Usage: %s [port #] [wwwport #]\n", argv[0] );
	    exit( 1 );
	}
	else if ( ( wwwport = atoi( argv[2] ) ) <= 1024 )
	{
	    fprintf( stderr, "Port numbers must be above 1024.\n" );
	    exit( 1 );
	}
    }*/

    /*
     * Run the game.
     */
#if defined(macintosh) || defined(MSDOS)
    boot_db( );
    log_string( "Merc is ready to rock." );
    game_loop_mac_msdos( );
#endif

#if defined(unix)
 
   if (!fCopyOver) /* We have already the port if copyover'ed */
   {  
     control = init_socket (port);
   
     wwwcontrol = init_socket( wwwport );
   }
   /* init_signals(); */
    boot_db( fCopyOver );
    sprintf( log_buf, "Paradox MUD is ready to rock on ports %d and %d.", port, wwwport );
    log_string( log_buf );
    game_loop_unix( control, wwwcontrol );
    close (wwwcontrol);
    close (control);
#endif

    /*
     * That's all, folks.
     */
    log_string( "Normal termination of game." );
    exit( 0 );
    return 0;
}



#if defined(unix)
int init_socket( int port )
{
    static struct sockaddr_in sa_zero;
    struct sockaddr_in sa;
    int x = 1;
    int fd;

    if ( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
    {
	perror( "Init_socket: socket" );
	exit( 1 );
    }

    if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR,
    (char *) &x, sizeof(x) ) < 0 )
    {
	perror( "Init_socket: SO_REUSEADDR" );
	close(fd);
	exit( 1 );
    }

#if defined(SO_DONTLINGER) && !defined(SYSV)
    {
	struct	linger	ld;

	ld.l_onoff  = 1;
	ld.l_linger = 1000;

	if ( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER,
	(char *) &ld, sizeof(ld) ) < 0 )
	{
	    perror( "Init_socket: SO_DONTLINGER" );
	    close(fd);
	    exit( 1 );
	}
    }
#endif

    sa		    = sa_zero;
    sa.sin_family   = AF_INET;
    sa.sin_port	    = htons( port );

    if ( bind( fd, (struct sockaddr *) &sa, sizeof(sa) ) < 0 )
    {
	perror("Init socket: bind" );
	close(fd);
	exit(1);
    }


    if ( listen( fd, 3 ) < 0 )
    {
	perror("Init socket: listen");
	close(fd);
	exit(1);
    }

    return fd;
}
#endif



#if defined(macintosh) || defined(MSDOS)
void game_loop_mac_msdos( void )
{
    struct timeval last_time;
    struct timeval now_time;
    static DESCRIPTOR_DATA dcon;

    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

    /*
     * New_descriptor analogue.
     */
    dcon.descriptor	= 0;
    dcon.connected	= CON_GET_NAME;
    dcon.host		= str_dup( "localhost" );
    dcon.outsize	= 2000;
    dcon.outbuf		= alloc_mem( dcon.outsize );
    dcon.next		= descriptor_list;
    dcon.showstr_head	= NULL;
    dcon.showstr_point	= NULL;
    dcon.pEdit		= NULL;			/* OLC */
    dcon.pString	= NULL;			/* OLC */
    dcon.editor		= 0;			/* OLC */
    descriptor_list	= &dcon;

    /*
     * Send the greeting.
     */
    {
	extern char * help_greetinga;
	extern char * help_greetingb;
	extern char * help_greetingc;
	extern char * help_greetingd;
	extern char * help_greetinge;
	extern char * help_authors;
	extern char * help_login;
	switch (number_range(0,4))
	{
	    default:
		if ( help_greetinga[0] == '.' )
		    write_to_buffer( &dcon, help_greetinga+1, 0 );
		else
		    write_to_buffer( &dcon, help_greetinga  , 0 );
		break;
	    case 0:
		if ( help_greetinga[0] == '.' )
		    write_to_buffer( &dcon, help_greetinga+1, 0 );
		else
		    write_to_buffer( &dcon, help_greetinga  , 0 );
		
		break;
	    case 1:
		if ( help_greetingb[0] == '.' )
		    write_to_buffer( &dcon, help_greetingb+1, 0 );
		else
		    write_to_buffer( &dcon, help_greetingb  , 0 );
		break;
	    case 2:
		if ( help_greetingc[0] == '.' )
		    write_to_buffer( &dcon, help_greetingc+1, 0 );
		else
		    write_to_buffer( &dcon, help_greetingc  , 0 );
		break;
	    case 3:
		if ( help_greetingd[0] == '.' )
		    write_to_buffer( &dcon, help_greetingd+1, 0 );
		else
		    write_to_buffer( &dcon, help_greetingd  , 0 );
		break;
	    case 4:
		if ( help_greetinge[0] == '.' )
		    write_to_buffer( &dcon, help_greetinge+1, 0 );
		else
		    write_to_buffer( &dcon, help_greetinge  , 0 );
		break;
	}
	if ( help_authors[0] == '.' )
	    write_to_buffer( &dcon, help_authors+1, 0 );
	else
	    write_to_buffer( &dcon, help_authors  , 0 );
	if ( help_login[0] == '.' )
	    write_to_buffer( &dcon, help_login+1, 0 );
	else
	    write_to_buffer( &dcon, help_login  , 0 );
    }

    /* Main loop */
    while ( !merc_down )
    {
	DESCRIPTOR_DATA *d;

	/*
	 * Process input.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next	= d->next;
	    d->fcommand	= FALSE;

#if defined(MSDOS)
	    if ( kbhit( ) )
#endif
	    {
		if ( d->character != NULL )
		    d->character->timer = 0;
		if ( !read_from_descriptor( d ) )
		{
		    if ( d->character != NULL)
			save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket( d );
		    continue;
		}
	    }

	    if (d->character != NULL && d->character->daze > 0)
		--d->character->daze;
            if (d->character != NULL && d->character->flee_daze > 0)
		--d->character->flee_daze;
	    if ( d->character != NULL && d->character->wait > 0 )
	    {
		--d->character->wait;
		continue;
	    }

	    read_from_buffer( d );
	    if ( d->incomm[0] != '\0' )
	    {
		d->fcommand	= TRUE;
		stop_idling( d->character );

	/* OLC */
	        if ( d->showstr_point )
	            show_string( d, d->incomm );
	        else
	        if ( d->pString )
	            string_add( d->character, d->incomm );
	        else
	            switch ( d->connected )
	            {
	                case CON_PLAYING:
			    if ( !run_olc_editor( d ) )
				substitute_alias( d, d->incomm );
			    break;
	                default:
			    nanny( d, d->incomm );
			    break;
	            }


		d->incomm[0]	= '\0';
	    }
	}



	/*
	 * Autonomous game motion.
	 */
	update_handler( );



	/*
	 * Output.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;

	    if ( ( d->fcommand || d->outtop > 0 ) )
	    {
		if ( !process_output( d, TRUE ) )
		{
		    if ( d->character != NULL && d->character->level > 1)
			save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket( d );
		}
	    }
	}



	/*
	 * Synchronize to a clock.
	 * Busy wait (blargh).
	 */
	now_time = last_time;
	for ( ; ; )
	{
	    int delta;

#if defined(MSDOS)
	    if ( kbhit( ) )
#endif
	    {
		if ( dcon.character != NULL )
		    dcon.character->timer = 0;
		if ( !read_from_descriptor( &dcon ) )
		{
		    if ( dcon.character != NULL && d->character->level > 1)
			save_char_obj( d->character );
		    dcon.outtop	= 0;
		    close_socket( &dcon );
		}
#if defined(MSDOS)
		break;
#endif
	    }

	    gettimeofday( &now_time, NULL );
	    delta = ( now_time.tv_sec  - last_time.tv_sec  ) * 1000 * 1000
		  + ( now_time.tv_usec - last_time.tv_usec );
	    if ( delta >= 1000000 / PULSE_PER_SECOND )
		break;
	}
	last_time    = now_time;
	current_time = (time_t) last_time.tv_sec;
    }

    return;
}
#endif



#if defined(unix)
void game_loop_unix( int control, int wwwcontrol )
{
    static struct timeval null_time;
    struct timeval last_time;

    signal( SIGPIPE, SIG_IGN );
    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

    init_signals();
    /* Main loop */
    while ( !merc_down )
    {
	fd_set in_set;
	fd_set out_set;
	fd_set exc_set;
	DESCRIPTOR_DATA *d;
	int maxdesc;

#if defined(MALLOC_DEBUG)
	if ( malloc_verify( ) != 1 )
	    abort( );
#endif

	/*
	 * Poll all active descriptors.
	 */
	FD_ZERO( &in_set  );
	FD_ZERO( &out_set );
	FD_ZERO( &exc_set );
	FD_SET( control, &in_set );
	maxdesc	= control;
	for ( d = descriptor_list; d; d = d->next )
	{
	    maxdesc = UMAX( maxdesc, d->descriptor );
	    FD_SET( d->descriptor, &in_set  );
	    FD_SET( d->descriptor, &out_set );
	    FD_SET( d->descriptor, &exc_set );
	}

	if ( select( maxdesc+1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
	{
	    perror( "Game_loop: select: poll" );
	    exit( 1 );
	}

	/*
	 * New connection?
	 */
	if ( FD_ISSET( control, &in_set ) )
	    init_descriptor( control );

	FD_ZERO( &in_set  );
	FD_ZERO( &out_set );
	FD_ZERO( &exc_set );
	FD_SET( wwwcontrol, &in_set );
	maxdesc	= wwwcontrol;
	for ( d = descriptor_list; d; d = d->next )
	{
	    maxdesc = UMAX( maxdesc, d->descriptor );
	    FD_SET( d->descriptor, &in_set  );
	    FD_SET( d->descriptor, &out_set );
	    FD_SET( d->descriptor, &exc_set );
	}

	if ( select( maxdesc+1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
	{
	    perror( "Game_loop: select: poll" );
	    exit( 1 );
	}

	if ( FD_ISSET( wwwcontrol, &in_set ) )
	    init_descriptor_www( wwwcontrol );

	/*
	 * Kick out the freaky folks.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;   
	    if ( FD_ISSET( d->descriptor, &exc_set ) )
	    {
		FD_CLR( d->descriptor, &in_set  );
		FD_CLR( d->descriptor, &out_set );
		if ( d->character && d->character->level > 1)
		    save_char_obj( d->character );
		d->outtop	= 0;
		close_socket( d );
	    }
	}

	/*
	 * Process input.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next	= d->next;
	    d->fcommand	= FALSE;

	    if ( FD_ISSET( d->descriptor, &in_set ) )
	    {
		if ( d->character != NULL )
		    d->character->timer = 0;
		if ( !read_from_descriptor( d ) )
		{
		    FD_CLR( d->descriptor, &out_set );
		    if ( d->character != NULL && d->character->level > 1)
			save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket( d );
		    continue;
		}
	    }

	    if (d->character != NULL && d->character->daze > 0)
		--d->character->daze;
            if (d->character != NULL && d->character->flee_daze > 0)
 		--d->character->flee_daze;

	    if ( d->character != NULL && d->character->wait > 0 )
	    {
		--d->character->wait;
		continue;
	    }

	    read_from_buffer( d );
	    if ( d->incomm[0] != '\0' )
	    {
		d->fcommand	= TRUE;
		stop_idling( d->character );

		/* OLC */
	if ( d->showstr_point )
	    show_string( d, d->incomm );
	else
	if ( d->pString )
	    string_add( d->character, d->incomm );
	else
	    switch ( d->connected )
	    {
	        case CON_PLAYING:
		    if ( !run_olc_editor( d ) )
    		        substitute_alias( d, d->incomm );
		    break;
	        default:

		    nanny( d, d->incomm );
                break;
          }

		d->incomm[0]	= '\0';
	    }
	}



	/*
	 * Autonomous game motion.
	 */
	update_handler( );



	/*
	 * Output.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;

	    if ( ( d->fcommand || d->outtop > 0 )
	    &&   FD_ISSET(d->descriptor, &out_set) )
	    {
		if ( !process_output( d, TRUE ) )
		{
		    if ( d->character != NULL && d->character->level > 1)
			save_char_obj( d->character );
		    d->outtop	= 0;
		    close_socket( d );
		}
	    }
	}



	/*
	 * Synchronize to a clock.
	 * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
	 * Careful here of signed versus unsigned arithmetic.
	 */
	{
	    struct timeval now_time;
	    long secDelta;
	    long usecDelta;

	    gettimeofday( &now_time, NULL );
	    usecDelta	= ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
			+ 1000000 / PULSE_PER_SECOND;
	    secDelta	= ((int) last_time.tv_sec ) - ((int) now_time.tv_sec );
	    while ( usecDelta < 0 )
	    {
		usecDelta += 1000000;
		secDelta  -= 1;
	    }

	    while ( usecDelta >= 1000000 )
	    {
		usecDelta -= 1000000;
		secDelta  += 1;
	    }

	    if ( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
	    {
		struct timeval stall_time;

		stall_time.tv_usec = usecDelta;
		stall_time.tv_sec  = secDelta;
		if ( select( 0, NULL, NULL, NULL, &stall_time ) < 0 )
		{
		    perror( "Game_loop: select: stall" );
		    exit( 1 );
		}
	    }
	}

	gettimeofday( &last_time, NULL );
	current_time = (time_t) last_time.tv_sec;
    }

    return;
}
#endif



#if defined(unix)
void init_descriptor( int control )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *dnew;
    struct sockaddr_in sock;
    struct hostent *from;
    int desc;
    int size;

    size = sizeof(sock);
    getsockname( control, (struct sockaddr *) &sock, &size );
    if ( ( desc = accept( control, (struct sockaddr *) &sock, &size) ) < 0 )
    {
	perror( "New_descriptor: accept" );
	return;
    }

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

    if ( fcntl( desc, F_SETFL, FNDELAY ) == -1 )
    {
	perror( "New_descriptor: fcntl: FNDELAY" );
	return;
    }

    /*
     * Cons a new descriptor.
     */
    dnew = new_descriptor();

    dnew->descriptor	= desc;
    dnew->connected	= CON_GET_NAME;
    dnew->showstr_head	= NULL;
    dnew->showstr_point = NULL;
    dnew->outsize	= 2000;
    dnew->pEdit		= NULL;			/* OLC */
    dnew->pString	= NULL;			/* OLC */
    dnew->editor	= 0;			/* OLC */
    dnew->outbuf	= alloc_mem( dnew->outsize );

    size = sizeof(sock);
    if ( getpeername( desc, (struct sockaddr *) &sock, &size ) < 0 )
    {
	perror( "New_descriptor: getpeername" );
	dnew->host = str_dup( "(unknown)" );
	dnew->ip   = str_dup( "(unknown)" );
    }
    else
    {
	/*
	 * Would be nice to use inet_ntoa here but it takes a struct arg,
	 * which ain't very compatible between gcc and system libraries.
	 */
	int addr;

	addr = ntohl( sock.sin_addr.s_addr );
	sprintf( buf, "%d.%d.%d.%d",
	    ( addr >> 24 ) & 0xFF, ( addr >> 16 ) & 0xFF,
	    ( addr >>  8 ) & 0xFF, ( addr       ) & 0xFF
	    );
	sprintf( log_buf, "Sock.sinaddr:  %s", buf );
	log_string( log_buf );
	from = gethostbyaddr( (char *) &sock.sin_addr,
	    sizeof(sock.sin_addr), AF_INET );
	dnew->host = str_dup( from ? from->h_name : buf );
        dnew->ip   = str_dup( buf );
    }
	
    /*
     * Swiftest: I added the following to ban sites.  I don't
     * endorse banning of sites, but Copper has few descriptors now
     * and some people from certain sites keep abusing access by
     * using automated 'autodialers' and leaving connections hanging.
     *
     * Furey: added suffix check by request of Nickel of HiddenWorlds.
     */
    if ( check_ban(dnew->host,BAN_ALL))
    {
	write_to_descriptor( desc,
	    "Your site has been banned from this mud.\n\r", 0 );
	close( desc );
	free_descriptor(dnew);
	return;
    }
    /*
     * Init descriptor data.
     */
    dnew->next			= descriptor_list;
    descriptor_list		= dnew;

    /*
     * Send the greeting.
     */
    {
	extern char * help_greetinga;
	extern char * help_greetingb;
	extern char * help_greetingc;
	extern char * help_greetingd;
	extern char * help_greetinge;
	extern char * help_authors;
	extern char * help_login;
	switch (number_range(0,4))
	{
	    default:
		if ( help_greetinga[0] == '.' )
		    write_to_buffer( dnew, help_greetinga+1, 0 );
		else
		    write_to_buffer( dnew, help_greetinga  , 0 );
		break;
	    case 0:
		if ( help_greetinga[0] == '.' )
		    write_to_buffer( dnew, help_greetinga+1, 0 );
		else
		    write_to_buffer( dnew, help_greetinga  , 0 );
		break;
	    case 1:
		if ( help_greetingb[0] == '.' )
		    write_to_buffer( dnew, help_greetingb+1, 0 );
		else
		    write_to_buffer( dnew, help_greetingb  , 0 );
		break;
	    case 2:
		if ( help_greetingc[0] == '.' )
		    write_to_buffer( dnew, help_greetingc+1, 0 );
		else
		    write_to_buffer( dnew, help_greetingc  , 0 );
		break;
	    case 3:
		if ( help_greetingd[0] == '.' )
		    write_to_buffer( dnew, help_greetingd+1, 0 );
		else
		    write_to_buffer( dnew, help_greetingd  , 0 );
		break;
	    case 4:
		if ( help_greetinge[0] == '.' )
		    write_to_buffer( dnew, help_greetinge+1, 0 );
		else
		    write_to_buffer( dnew, help_greetinge  , 0 );
		break;
	}
	if ( help_authors[0] == '.' )
	    write_to_buffer( dnew, help_authors+1, 0 );
	else
	    write_to_buffer( dnew, help_authors  , 0 );
	if ( help_login[0] == '.' )
	    write_to_buffer( dnew, help_login+1, 0 );
	else
	    write_to_buffer( dnew, help_login  , 0 );
    }
    return;
}
#endif

#if defined(unix)
void init_descriptor_www( int wwwcontrol )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *dnew;
    struct sockaddr_in sock;
    struct hostent *from;
    int desc;
    int size;
    char clandat[MAX_STRING_LENGTH];
    BUFFER *output;
    DESCRIPTOR_DATA *d;
    int iClass;
    int iRace;
    int iClan;
    int iLevelLower;
    int iLevelUpper;
    int nMatch;
    FILE *fg;
    bool rgfClass[MAX_CLASS];
    bool rgfRace[MAX_PC_RACE];
    bool rgfClan[MAX_CLAN];
    bool fClassRestrict = FALSE;
    bool fClanRestrict = FALSE;
    bool fClan = FALSE;
    bool fRaceRestrict = FALSE;
    bool fImmortalOnly = FALSE;

    size = sizeof(sock);
    getsockname( wwwcontrol, (struct sockaddr *) &sock, &size );
    if ( ( desc = accept( wwwcontrol, (struct sockaddr *) &sock, &size) ) < 0 )
    {
	perror( "New_descriptor: accept" );
	return;
    }

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

    if ( fcntl( desc, F_SETFL, FNDELAY ) == -1 )
    {
	perror( "New_descriptor: fcntl: FNDELAY" );
	return;
    }

    /*
     * Cons a new descriptor.
     */
    dnew = new_descriptor();

    dnew->descriptor	= desc;
    dnew->connected	= CON_GET_NAME;
    dnew->showstr_head	= NULL;
    dnew->showstr_point = NULL;
    dnew->outsize	= 2000;
    dnew->outbuf	= alloc_mem( dnew->outsize );

    size = sizeof(sock);
    if ( getpeername( desc, (struct sockaddr *) &sock, &size ) < 0 )
    {
	perror( "New_descriptor: getpeername" );
	dnew->host = str_dup( "(unknown)" );
    }
    else
    {
	/*
	 * Would be nice to use inet_ntoa here but it takes a struct arg,
	 * which ain't very compatible between gcc and system libraries.
	 */
	int addr;

	addr = ntohl( sock.sin_addr.s_addr );
	sprintf( buf, "%d.%d.%d.%d",
	    ( addr >> 24 ) & 0xFF, ( addr >> 16 ) & 0xFF,
	    ( addr >>  8 ) & 0xFF, ( addr       ) & 0xFF
	    );
	sprintf( log_buf, "Sock.sinaddr:  %s", buf );
	from = gethostbyaddr( (char *) &sock.sin_addr,
	    sizeof(sock.sin_addr), AF_INET );
	dnew->host = str_dup( from ? from->h_name : buf );
    }

    iLevelLower    = 0;
    iLevelUpper    = MAX_LEVEL;
    for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
        rgfClass[iClass] = FALSE;
    for ( iRace = 0; iRace < MAX_PC_RACE; iRace++ )
        rgfRace[iRace] = FALSE;
    for (iClan = 0; iClan < MAX_CLAN; iClan++)
	rgfClan[iClan] = FALSE;

    nMatch = 0;
    buf[0] = '\0';
    output = new_buf();
    sprintf(buf, WWW_WHO);
    if ((fg = fopen(buf,"w")) == NULL)
    {
	bug("WWW-Who: fopen",0);
	perror(buf);
    }
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *wch;
        char const *class;
 
	buf[0] = '\0';
        /*
         * Check for match against restrictions.
         * Don't use trust as that exposes trusted mortals.
         */
        if ( d->connected != CON_PLAYING || d->character->invis_level > 1 
	|| d->character->incog_level > 1)
            continue;
 
        wch   = ( d->original != NULL ) ? d->original : d->character;

	if (IS_SHIELDED(wch,SHD_INVISIBLE))
	    continue;

        if (wch->invis_level >= 2 || wch->incog_level >= 2 )
	    continue;

        if (wch->in_room->vnum == ROOM_VNUM_CORNER)
            continue;


        if ( wch->level < iLevelLower
        ||   wch->level > iLevelUpper
        || ( fImmortalOnly  && wch->level < LEVEL_IMMORTAL )
        || ( fClassRestrict && !rgfClass[wch->class] )
        || ( fRaceRestrict && !rgfRace[wch->race])
 	|| ( fClan && !is_clan(wch))
	|| ( fClanRestrict && !rgfClan[wch->clan]))
            continue;
 
        nMatch++;
 
        /*
         * Figure out what to print for class.
	 */
	class = class_table[wch->class].who_name;

	/*
	 * Format it up.
	 */
        if (!is_clan(wch)) 
        { 
	    sprintf(clandat, "X,3,3");
        } 
        else 
        { 
	    if (clan_table[wch->clan].independent)
	    {
		sprintf(clandat, "%s,0,0", clan_table[wch->clan].who_name);
	    }
	    else if (is_clead(wch) && is_pkill(wch))
	    {
		sprintf(clandat, "%s,1,1", clan_table[wch->clan].who_name);
	    }
	    else if (is_clead(wch) && !is_pkill(wch))
	    {
		sprintf(clandat, "%s,2,1", clan_table[wch->clan].who_name);
	    }
	    else if (is_pkill(wch))
	    {
		sprintf(clandat, "%s,1,0", clan_table[wch->clan].who_name);
	    }
	    else
	    {
		sprintf(clandat, "%s,2,0", clan_table[wch->clan].who_name);
	    }
        }
	sprintf(buf2, "%s", IS_NPC(wch) ? "" : wch->pcdata->title);
	str_replace(buf2, "{%", "{-");
	sprintf( buf, "%3d,%s,%s,%s,%s%s\n\r", wch->level,
	    wch->race < MAX_PC_RACE ? pc_race_table[wch->race].who_name : "      ",
	    class, clandat, wch->name, buf2);
	fprintf(fg,buf);
    }
    fclose(fg);
    close( desc );
    free_descriptor(dnew);
    return;
}
#endif


void close_socket( DESCRIPTOR_DATA *dclose )
{
    CHAR_DATA *ch;

    if ( dclose->outtop > 0 )
	process_output( dclose, FALSE );

    if ( dclose->snoop_by != NULL )
    {
	write_to_buffer( dclose->snoop_by,
	    "Your victim has left the game.\n\r", 0 );
    }

    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->snoop_by == dclose )
		d->snoop_by = NULL;
	}
    }

    if ( ( ch = dclose->character ) != NULL )
    {
	sprintf( log_buf, "Closing link to %s.", ch->name );
	log_string( log_buf );
        if(ch->pet && ch->pet->in_room == NULL)
        {
         char_to_room(ch->pet, get_room_index(ROOM_VNUM_LIMBO));
         extract_char(ch->pet, TRUE);
        }

	if ( dclose->connected == CON_PLAYING )
	{
	    act( "$n has lost $s link.", ch, NULL, NULL, TO_ROOM );
	    wiznet("Net death has claimed $N.",ch,NULL,WIZ_LINKS,0,0);
        /*    {
               char buf[MAX_STRING_LENGTH];
               act( buf, ch, NULL, NULL, TO_ALL );
            }*/
	    ch->desc = NULL;
            if(IS_SET(ch->in_room->room_flags,ROOM_ARENA))
            {
             CHAR_DATA *opp = NULL;
             char buf[MAX_STRING_LENGTH];
             
             char_from_room(ch);
             char_to_room(ch,get_room_index(ROOM_VNUM_LIMBO));
             if(IS_SET(ch->exbit1,EXBIT1_CHALLENGER))
             {
              REMOVE_BIT(ch->exbit1,EXBIT1_CHALLENGER);
              opp = ch->challenged;
              ch->challenged = NULL;
              ch->challenger = NULL;
             }
             if(IS_SET(ch->exbit1,EXBIT1_CHALLENGED))
             {
              REMOVE_BIT(ch->exbit1,EXBIT1_CHALLENGED);
              opp = ch->challenger;
              ch->challenger = NULL;
              ch->challenged = NULL;
             }
             if(IS_SET(ch->act,PLR_NORESTORE))
              REMOVE_BIT(ch->act,PLR_NORESTORE);
             if(opp)
             {
              char_from_room(opp);
              char_to_room(opp,get_room_index(ROOM_VNUM_ALTAR));
              if(IS_SET(opp->exbit1,EXBIT1_CHALLENGER))
               REMOVE_BIT(opp->exbit1,EXBIT1_CHALLENGER);
              if(IS_SET(opp->exbit1,EXBIT1_CHALLENGED))
               REMOVE_BIT(opp->exbit1,EXBIT1_CHALLENGED);
              if(IS_SET(opp->act,PLR_NORESTORE))
               REMOVE_BIT(opp->act,PLR_NORESTORE);
              opp->challenged = NULL;
              opp->challenger = NULL;
              do_look(opp,"auto");
             }
             arena = FIGHT_OPEN;
             if(opp)
             {
              sprintf(buf,"{W%s has gone linkdead. Arena reset.",
                ch->name);
              send_to_char("{xYour opponent has gone linkdead. Arena reset.\n\r",opp);
              send_to_char("{xThe arena was reset after you lost your link.\n\r",ch);
              comm_arena(ch,opp,buf);
             }
          }
             
	}
	else
	{
	    free_char( dclose->original ? dclose->original : dclose->character );
	}
    }

    if ( d_next == dclose )
	d_next = d_next->next;   

    if ( dclose == descriptor_list )
    {
	descriptor_list = descriptor_list->next;
    }
    else
    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d && d->next != dclose; d = d->next )
	    ;
	if ( d != NULL )
	    d->next = dclose->next;
	else
	    bug( "Close_socket: dclose not found.", 0 );
    }

    close( dclose->descriptor );
    free_descriptor(dclose);
#if defined(MSDOS) || defined(macintosh)
    exit(1);
#endif
    return;
}



bool read_from_descriptor( DESCRIPTOR_DATA *d )
{
    int iStart;

    /* Hold horses if pending command already. */
    if ( d->incomm[0] != '\0' )
	return TRUE;

    /* Check for overflow. */
    iStart = strlen(d->inbuf);
    if ( iStart >= sizeof(d->inbuf) - 10 )
    {
	sprintf( log_buf, "%s input overflow!", d->host );
	log_string( log_buf );
	write_to_descriptor( d->descriptor,
	    "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
	return FALSE;
    }

    /* Snarf input. */
#if defined(macintosh)
    for ( ; ; )
    {
	int c;
	c = getc( stdin );
	if ( c == '\0' || c == EOF )
	    break;
	putc( c, stdout );
	if ( c == '\r' )
	    putc( '\n', stdout );
	d->inbuf[iStart++] = c;
	if ( iStart > sizeof(d->inbuf) - 10 )
	    break;
    }
#endif

#if defined(MSDOS) || defined(unix)
    for ( ; ; )
    {
	int nRead;

	nRead = read( d->descriptor, d->inbuf + iStart,
	    sizeof(d->inbuf) - 10 - iStart );
	if ( nRead > 0 )
	{
	    iStart += nRead;
	    if ( d->inbuf[iStart-1] == '\n' || d->inbuf[iStart-1] == '\r' )
		break;
	}
	else if ( nRead == 0 )
	{
	    log_string( "EOF encountered on read." );
	    return FALSE;
	}
	else if ( errno == EWOULDBLOCK )
	    break;
	else
	{
	    perror( "Read_from_descriptor" );
	    return FALSE;
	}
    }
#endif

    d->inbuf[iStart] = '\0';
    return TRUE;
}



/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( DESCRIPTOR_DATA *d )
{
    int i, j, k;

    /*
     * Hold horses if pending command already.
     */
    if ( d->incomm[0] != '\0' )
	return;

    /*
     * Look for at least one new line.
     */
    for ( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( d->inbuf[i] == '\0' )
	    return;
    }

    /*
     * Canonical input processing.
     */
    for ( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( k >= MAX_INPUT_LENGTH - 2 )
	{
	    write_to_descriptor( d->descriptor, "Line too long.\n\r", 0 );

	    /* skip the rest of the line */
	    for ( ; d->inbuf[i] != '\0'; i++ )
	    {
		if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
		    break;
	    }
	    d->inbuf[i]   = '\n';
	    d->inbuf[i+1] = '\0';
	    break;
	}

	if ( d->inbuf[i] == '\b' && k > 0 )
	    --k;
	else if ( isascii(d->inbuf[i]) && isprint(d->inbuf[i]) )
	    d->incomm[k++] = d->inbuf[i];
    }

    /*
     * Finish off the line.
     */
    if ( k == 0 )
	d->incomm[k++] = ' ';
    d->incomm[k] = '\0';

    /*
     * Deal with bozos with #repeat 1000 ...
     */

    if ( k > 1 || d->incomm[0] == '!' )
    {
    	if ( d->incomm[0] != '!' && strcmp( d->incomm, d->inlast ) )
	{
	    d->repeat = 0;
	}
	else
	{
	    d->repeat++;
	    if ( d->repeat == 25 )
	    {
		sprintf( log_buf, "%s input spamming!", d->host );
		log_string( log_buf );
		wiznet("Spam spam spam $N spam spam spam spam spam!",
		       d->character,NULL,WIZ_SPAM,0,get_trust(d->character));
		if (d->incomm[0] == '!')
		    wiznet(d->inlast,d->character,NULL,WIZ_SPAM,0,
			get_trust(d->character));
		else
		    wiznet(d->incomm,d->character,NULL,WIZ_SPAM,0,
			get_trust(d->character));
	    } else if ( d->repeat == 35 )
	    {
                sprintf( log_buf, "%s still input spamming!", d->host );
                log_string( log_buf );
                wiznet("Spam spam spam $N spam spam spam spam spam!",
                       d->character,NULL,WIZ_SPAM,0,get_trust(d->character));
                if (d->incomm[0] == '!')
                    wiznet(d->inlast,d->character,NULL,WIZ_SPAM,0,
                        get_trust(d->character));
                else
                    wiznet(d->incomm,d->character,NULL,WIZ_SPAM,0,
                        get_trust(d->character));
                write_to_descriptor( d->descriptor,
                    "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
            } else if ( d->repeat >= 45 )
	    {
		d->repeat = 0;
		write_to_descriptor( d->descriptor,
		    "\n\r*** I WARNED YOU!!! ***\n\r", 0 );
		strcpy( d->incomm, "quit" );
	    }
	}
    }


    /*
     * Do '!' substitution.
     */
    if ( d->incomm[0] == '!' )
	strcpy( d->incomm, d->inlast );
    else
	strcpy( d->inlast, d->incomm );

    /*
     * Shift the input buffer.
     */
    while ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
	i++;
    for ( j = 0; ( d->inbuf[j] = d->inbuf[i+j] ) != '\0'; j++ )
	;
    return;
}



/*
 * Low level output function.
 */
bool process_output( DESCRIPTOR_DATA *d, bool fPrompt )
{
    extern bool merc_down;

    /*
     * Bust a prompt.
     */
    if ( !merc_down )
	if ( d->showstr_point )
	    write_to_buffer( d, "[Hit Return to continue]\n\r", 0 );
	else if ( fPrompt && d->pString && d->connected == CON_PLAYING )
	    write_to_buffer( d, "> ", 2 );
	else if ( fPrompt && d->connected == CON_PLAYING )
	{
	    CHAR_DATA *ch;

	CHAR_DATA *victim;

	ch = d->character;

        /* battle prompt */
        if ((victim = ch->fighting) != NULL && can_see(ch,victim)
          && (IS_NPC(victim) || victim->desc != NULL))
        {
            int percent;
	    char meter[100];
	    char buf[MAX_STRING_LENGTH];
 
            if (victim->max_hit > 0)
                percent = victim->hit * 100 / victim->max_hit;
            else
                percent = -1;
 
            if (percent >= 100)
                sprintf(meter,"looks strong and healthy.{x");
            else if (percent >= 90)
                sprintf(meter,"is laughing at your feeble attempts.{x");
            else if (percent >= 80)
                sprintf(meter,"has finally started to notice you.{x");
            else if (percent >= 70)
                sprintf(meter,"has some small scratches.{x");
            else if (percent >= 60)
                sprintf(meter,"has some minor wounds and bruises.{x");
            else if (percent >= 50)
                sprintf(meter,"is bleeding from some wounds and cuts.{x");
            else if (percent >= 40)
                sprintf(meter,"looks pretty battered.{x");
            else if (percent >= 30)
                sprintf(meter,"is bleeding profusely.{x");
            else if (percent >= 20)
                sprintf(meter,"is begging you for mercy.{x");
            else if (percent >= 10)
                sprintf(meter,"is bleeding from terrible wounds and gashes.{x");
            else if (percent >= 0)
                sprintf(meter,"is embracing death.{x");
            sprintf(buf,"{R%s %s{x\n\r", 
	            IS_NPC(victim) ?victim->short_descr : victim->name,meter);
	    buf[2] = UPPER(buf[2]);
	    send_to_char(buf, ch);
	    if (victim->stunned)
	    {
		sprintf(buf,"{f%s is stunned.{x\n\r", 
	            IS_NPC(victim) ? victim->short_descr : victim->name);
		send_to_char(buf, ch);
	    }
        }


	ch = d->original ? d->original : d->character;
	if (!IS_SET(ch->comm, COMM_COMPACT) )
	    write_to_buffer( d, "\n\r", 2 );


        if ( IS_SET(ch->comm, COMM_PROMPT) )
            bust_a_prompt( d->character );

	if (IS_SET(ch->comm,COMM_TELNET_GA))
	    write_to_buffer(d,go_ahead_str,0);
    }

    /*
     * Short-circuit if nothing to write.
     */
    if ( d->outtop == 0 )
	return TRUE;

    /*
     * Snoop-o-rama.
     */
    if ( d->snoop_by != NULL )
    {
	if (d->character != NULL)
	    write_to_buffer( d->snoop_by, d->character->name,0);
	write_to_buffer( d->snoop_by, "> ", 2 );
	write_to_buffer( d->snoop_by, d->outbuf, d->outtop );
    }

    /*
     * OS-dependent output.
     */
    if ( !write_to_descriptor( d->descriptor, d->outbuf, d->outtop ) )
    {
	d->outtop = 0;
	return FALSE;
    }
    else
    {
	d->outtop = 0;
	return TRUE;
    }
}

/*
 * Bust a prompt (player settable prompt)
 * coded by Morgenes for Aldara Mud
 * Completely overhauled for color by RW
 */
void bust_a_prompt( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char doors[MAX_INPUT_LENGTH];
    EXIT_DATA *pexit;
    bool found;
    bool round;
    const char *dir_name[] = {"N","E","S","W","U","D"};
    char *door_name[] = {"north","east","south","west","up","down"};
    int door, outlet;
 
    sprintf(buf2, "%s%s", 
     IS_NPC(ch)?"":((ch->pcdata->pkflag>0)?"{x<{RP{rK{x>":""), 
     ch->prompt);
    if (ch->prompt == NULL || ch->prompt[0] == '\0')
    {
        sprintf( buf, "%s<%dhp %dm %dmv> %s",
            IS_NPC(ch) ? "":((ch->pcdata->pkflag>0)?"{x<{RP{rK{x>":""),
	    ch->hit,ch->mana,ch->move,ch->prefix);
	send_to_char(buf,ch);
	return;
    }

   if (IS_SET(ch->comm,COMM_AFK))
   {
	send_to_char("<{rAFK{x> ",ch);
	return;
   }

    found = FALSE;
    doors[0] = '\0';
    for (door = 0; door < 6; door++)
    {
	round = FALSE;
	outlet = door;
	if ( ( ch->alignment < 0 )
	&&   ( pexit = ch->in_room->exit[door+6] ) != NULL)
	    outlet += 6;
	if ((pexit = ch->in_room->exit[outlet]) != NULL
	&&  pexit ->u1.to_room != NULL
	&&  (can_see_room(ch,pexit->u1.to_room)
	||   (IS_AFFECTED(ch,AFF_INFRARED) 
	&&    !IS_AFFECTED(ch,AFF_BLIND)))
	/*&&  !IS_SET(pexit->exit_info,EX_CLOSED)*/)
	{
	    found = TRUE;
	    round = TRUE;
            if(!IS_SET(pexit->exit_info,EX_CLOSED))
 	     strcat(doors,dir_name[door]);
            else
             sprintf(doors+strlen(doors),"(%s)",dir_name[door]);
	}
	if (!round)
	{
	    OBJ_DATA *portal;

            portal = get_obj_exit( door_name[door], ch->in_room->contents);
            if ((portal != NULL)
	    && !IS_AFFECTED(ch,AFF_BLIND))
            {
                found = TRUE;
                round = TRUE;
		strcat(doors,dir_name[door]);
	    }
	}
    }
    if (!found)
    {
	sprintf(buf,"none");
    } else {
	sprintf(buf,"%s",doors);
    }
    str_replace_c(buf2, "%e", buf);
    str_replace_c(buf2, "%c", "\n\r");
    sprintf(buf,"%d",ch->hit);
    str_replace_c(buf2, "%h", buf);
    sprintf(buf,"%d",ch->max_hit);
    str_replace_c(buf2, "%H", buf);
    sprintf(buf,"%d",ch->mana);
    str_replace_c(buf2, "%m", buf);
    sprintf(buf,"%d",ch->max_mana);
    str_replace_c(buf2, "%M", buf);
    sprintf(buf,"%d",ch->move);
    str_replace_c(buf2, "%v", buf);
    sprintf(buf,"%d",ch->max_move);
    str_replace_c(buf2, "%V", buf);
    sprintf(buf,"%ld",ch->exp);
    str_replace_c(buf2, "%x", buf);
    if (!IS_NPC(ch) )
	sprintf(buf,"%ld",(ch->level + 1) * exp_per_level(ch,ch->pcdata->points) - ch->exp);
    else
	sprintf(buf,"none");
    str_replace_c(buf2, "%X", buf);
    sprintf(buf,"%ld",ch->platinum);
    str_replace_c(buf2, "%p", buf);
    sprintf(buf,"%ld",ch->gold);
    str_replace_c(buf2, "%g", buf);
    sprintf(buf,"%ld",ch->silver);
    str_replace_c(buf2, "%s", buf);
    sprintf(buf,"%s",IS_SHIELDED(ch,SHD_SWALK)?"{C({cSW{C){x":" ");
    str_replace_c(buf2, "%S", buf);
    sprintf(buf,"%s",IS_AFFECTED(ch,AFF_SNEAK)?"{C({cSneak{C){x":" ");
    str_replace_c(buf2, "%n", buf); 
    sprintf(buf,"%s",is_affected(ch,gsn_ambush)?"{g({GAmb{g){x":" ");
    str_replace_c(buf2, "%A", buf);
    if( ch->level > 9 )
	sprintf(buf,"%d",ch->alignment);
    else
	sprintf( buf, "%s", IS_GOOD(ch) ? "good" : IS_EVIL(ch) ?
	"evil" : "neutral" );
    str_replace_c(buf2, "%a", buf);
    if( ch->in_room != NULL )
	sprintf( buf, "%s", 
	    ((!IS_NPC(ch) && IS_SET(ch->act,PLR_HOLYLIGHT)) ||
	    (!IS_AFFECTED(ch,AFF_BLIND) && !room_is_dark( ch->in_room )))
	    ? ch->in_room->name : "darkness");
    else
	sprintf( buf, " " );
    str_replace_c(buf2, "%r", buf);
    if( IS_IMMORTAL( ch ) && ch->in_room != NULL )
	sprintf( buf, "%d", ch->in_room->vnum );
    else
	sprintf( buf, " " );
    str_replace_c(buf2, "%o", buf);
        if( IS_IMMORTAL( ch ))
        sprintf( buf, "%s", olc_ed_name(ch) );
    str_replace_c(buf2, "%O", buf);
    if( IS_IMMORTAL( ch ))
        sprintf( buf, "%s", olc_ed_vnum(ch) );  
    str_replace_c(buf2, "%R", buf);
    if( IS_IMMORTAL( ch ) && ch->in_room != NULL )
	sprintf( buf, "%s", ch->in_room->area->name );
    else
	sprintf( buf, " " );
    str_replace_c(buf2, "%z", buf);

   send_to_char( buf2, ch );

   if (ch->prefix[0] != '\0')
        write_to_buffer(ch->desc,ch->prefix,0);
   return;
}



/*
 * Append onto an output buffer.
 */
void write_to_buffer( DESCRIPTOR_DATA *d, const char *txt, int length )
{
    /*
     * Find length in case caller didn't.
     */
    if ( length <= 0 )
	length = strlen(txt);

    /*
     * Initial \n\r if needed.
     */
    if ( d->outtop == 0 && !d->fcommand )
    {
	d->outbuf[0]	= '\n';
	d->outbuf[1]	= '\r';
	d->outtop	= 2;
    }

    /*
     * Expand the buffer as needed.
     */
    while ( d->outtop + length >= d->outsize )
    {
	char *outbuf;

        if (d->outsize >= 32000)
	{
	    bug("Buffer overflow. Closing.\n\r",0);
	    close_socket(d);
	    return;
 	}
	outbuf      = alloc_mem( 2 * d->outsize );
	strncpy( outbuf, d->outbuf, d->outtop );
	free_mem( d->outbuf, d->outsize );
	d->outbuf   = outbuf;
	d->outsize *= 2;
    }

    /*
     * Copy.
     */
    strncpy( d->outbuf + d->outtop, txt, length );
    d->outtop += length;
    return;
}



/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
bool write_to_descriptor( int desc, char *txt, int length )
{
    int iStart;
    int nWrite;
    int nBlock;

#if defined(macintosh) || defined(MSDOS)
    if ( desc == 0 )
	desc = 1;
#endif

    if ( length <= 0 )
	length = strlen(txt);

    for ( iStart = 0; iStart < length; iStart += nWrite )
    {
	nBlock = UMIN( length - iStart, 2048 );
	if ( ( nWrite = write( desc, txt + iStart, nBlock ) ) < 0 )
	    { perror( "Write_to_descriptor" ); return FALSE; }
    } 

    return TRUE;
}



/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny( DESCRIPTOR_DATA *d, char *argument )
{
    DESCRIPTOR_DATA *d_old, *d_next;
    char buf[MAX_STRING_LENGTH];
    char newbuf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char strsave[MAX_INPUT_LENGTH];
    CHAR_DATA *ch;
    CHAR_DATA *victim;
    char *pwdnew;
    char *p;
    int iClass,i,weapon;
    int pos;
    bool fOld;
    int num;
    static int race=0;

    while ( isspace(*argument) )
	argument++;

    ch = d->character;

    switch ( d->connected )
    {

    default:
	bug( "Nanny: bad d->connected %d.", d->connected );
	close_socket( d );
	return;

    case CON_GET_NAME:
	if ( argument[0] == '\0' )
	{
	    close_socket( d );
	    return;
	}

	argument[0] = UPPER(argument[0]);
	if ( !check_parse_name( argument ) )
	{
	    write_to_buffer( d, "Illegal name, try another.\n\r", 0 );
	    write_to_buffer( d, "(If you've used this name here before, and are no\n\r", 0 );
	    write_to_buffer( d, " longer able to, it may be because we've added a\n\r", 0 );
	    write_to_buffer( d, " new mobile that uses the same name. Log in with\n\r", 0 );
	    write_to_buffer( d, " a new name, and let an IMM know, and we will fix it.)\n\r", 0 );
	    write_to_buffer( d, "\n\rName: ", 0 );
	    return;
	}

	fOld = load_char_obj( d, argument );
	ch   = d->character;

	if (IS_SET(ch->act, PLR_DENY))
	{
	    sprintf( log_buf, "Denying access to %s@%s.", argument, d->host );
	    log_string( log_buf );
	    write_to_buffer( d, "You are denied access.\n\r", 0 );
	    close_socket( d );
	    return;
	}

	if (check_ban(d->host,BAN_PERMIT) && !IS_SET(ch->act,PLR_PERMIT))
	{
	    write_to_buffer(d,"Your site has been banned from this mud.\n\r",0);
	    close_socket(d);
	    return;
	}
/*
	if (check_adr(d->host,BAN_PERMIT) && (ch->level > 101) )
	{
	    write_to_buffer(d,"Immortals are not allowed to connect from your site.\n\r",0);
	    close_socket(d);
	    return;
	}
*/
	if (IS_SET(ch->comm, COMM_WIPED ) )
	{
	    close_socket( d );
	    return;
	}
        /* Gregor Stipicic, 2001*/
        if (IS_IMMORTAL(ch) && !is_allowed(ch->name,d->host) && !is_allowed(ch->name,d->ip))
        {
           write_to_buffer(d,"You can not connect to this character from your current site (ISC failure).\n\r",0);
           close_socket(d);
           return;
        }  
	if ( check_reconnect( d, argument, FALSE ) )
	{
	    fOld = TRUE;
	}
	else
	{
	    if ( wizlock && !IS_IMMORTAL(ch) && !is_allowed(ch->name,"playtester"))
	    {
		write_to_buffer( d, "The game is wizlocked.\n\r", 0 );
		close_socket( d );
		return;
	    }
	}

	if ( fOld )
	{
	    /* Old player */
	    write_to_buffer( d, "Password: ", 0 );
	    write_to_buffer( d, echo_off_str, 0 );
	    d->connected = CON_GET_OLD_PASSWORD;
	    return;
	}
	else
	{
	    /* New player */
 	    if (newlock)
	    {
                write_to_buffer( d, "The game is newlocked.\n\r", 0 );
                close_socket( d );
                return;
            }

	    if (check_ban(d->host,BAN_NEWBIES))
	    {
		write_to_buffer(d,
		    "New players are not allowed from your site.\n\r",0);
		close_socket(d);
		return;
	    }
            write_to_buffer(d,"\n\r"\
" Please note that Paradox MUD has a very lenient naming policy.	We do\n\r"\
" not allow racist, sexist or otherwisely offensive names though.\n\r"\
" Names like Jewboy, Faggot, etc. won't be tolerated.\n\r"\
" Although it is not explicitly forbidden to use names like Batman,\n\r"\
" we advise you to use a little bit of creativity.\n\r"\
" The excessive use of caps is also forbidden (e.g CoOlDuDe).\n\r\n\r",0);
	    sprintf( buf, "Did I get that right, %s (Y/N)? ", argument );
	    write_to_buffer( d, buf, 0 );
	    d->connected = CON_CONFIRM_NEW_NAME;
	    return;
	}
	break;

    case CON_GET_OLD_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ))
	{
	    write_to_buffer( d, "Wrong password.\n\r", 0 );
	    close_socket( d );
	    return;
	}
 
	write_to_buffer( d, echo_on_str, 0 );

	if (check_playing(d,ch->name))
	    return;

	ch->pcdata->socket = str_dup( d->host );
	if ( check_reconnect( d, ch->name, TRUE ) )
	    return;

	sprintf( log_buf, "%s@%s has connected.", ch->name, d->host );
	log_string( log_buf );
	wiznet(log_buf,NULL,NULL,WIZ_SITES,0,get_trust(ch));
	ch->pcdata->socket = str_dup( d->host );
        add_socket(ch->pcdata->socket,ch->name);
	d->connected=CON_CHECK_REROLL;
	/*break;*/

    case CON_CHECK_REROLL:
	if (IS_SET(ch->act, PLR_REROLL ) )
	{
    	    int plat,bounty,age,pos,iqp,aqp;
            int lno,lid,lpe,lne,lch,lcl,clan;
            bool col;
            FILE *fp; 
            char *prompt;
            sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( ch->name ) );
	    sprintf(newbuf, "%s", str_dup( ch->pcdata->pwd ));
	    sprintf( argument, "%s", capitalize( ch->name ) );
            plat=ch->balance;
            bounty=ch->pcdata->bounty;
            age=ch->played;
            iqp=ch->qps;
            aqp=ch->questpoints;
            col=IS_SET(ch->act,PLR_COLOUR);
            lno = ch->pcdata->last_note;
            lid = ch->pcdata->last_idea;
            lpe = ch->pcdata->last_penalty;
            lne = ch->pcdata->last_news;
            lch = ch->pcdata->last_changes;
            lcl = ch->pcdata->last_claninfo;
	    clan = ch->clan;
            prompt = str_dup(ch->prompt);
            fclose( fpReserve );
            if((fp=fopen("alias.tmp","w"))!=NULL)
            { 
             for(pos=0;pos<MAX_ALIAS;pos++)
             {
	        if(ch->pcdata->alias[pos]==NULL
                  || ch->pcdata->alias_sub[pos]==NULL)
                break;
                fprintf(fp,"%s~\n%s~\n",
                     ch->pcdata->alias[pos],
                     ch->pcdata->alias_sub[pos]);
             }
             fprintf(fp,"End~\n\n");  
             fclose(fp);
            } 
            else
             bug("Nanny, CON_CHECK_REROLL: couldn't open alias.tmp for writing.",0);        
            fpReserve=fopen(NULL_FILE,"r");
	    extract_char(d->character,TRUE);
	    d->character = NULL;
	    fOld = load_char_reroll( d, argument );
	    ch   = d->character;
	    free_string( ch->pcdata->pwd );
	    ch->pcdata->pwd	= str_dup( newbuf );
	    newbuf[0] = '\0';
	    ch->pcdata->tier = 1;
	    ch->pcdata->socket = str_dup( d->host );
            ch->balance=plat;
            ch->pcdata->bounty=bounty;
            ch->played=age;
            ch->qps=iqp;
            ch->questpoints=aqp;
            ch->pcdata->last_note = lno;
            ch->pcdata->last_idea = lid;
            ch->pcdata->last_penalty = lpe;
            ch->pcdata->last_news = lne;
            ch->pcdata->last_changes = lch;
            ch->pcdata->last_claninfo = lcl;
            ch->clan = clan;
            free_string( ch->prompt );
            ch->prompt = prompt;
            fclose(fpReserve);
            if((fp=fopen("alias.tmp","r"))!=NULL)
            {
             char *word;
             for(pos=0;pos<MAX_ALIAS;pos++)
             { 
              if(feof(fp))
               break;
              word=fread_string(fp);
              if(!strcmp(word,"End"))
               break;
              ch->pcdata->alias[pos]=word;
              ch->pcdata->alias_sub[pos]=fread_string(fp);
             }
             fclose(fp);
            }
            else
             bug("Nanny, CON_CHECK_REROLL: couldn't open alias.tmp for reading.",0);
            fpReserve=fopen(NULL_FILE,"r");
            if(col) SET_BIT(ch->act,PLR_COLOUR);
	    write_to_buffer( d, echo_on_str, 0 );
            send_to_char("{cThe following {Craces{c are available:{x\n\r\n\r",ch);
	    pos = 0;
	    for ( race = 1; race_table[race].name != NULL; race++ )
	    {
		if (!race_table[race].pc_race)
		    break;
		sprintf(newbuf, "{Y({y%2d{Y){x %3s%-24s", race, " ",  race_table[race].name);
		send_to_char(newbuf,ch);
		pos++;
		if (pos >= 2) {
		    write_to_buffer(d,"\n\r",1);
		    pos = 0;
		}
	    }
	    newbuf[0] = '\0';
	    write_to_buffer(d,"\n\r\n\r",0);
	    write_to_buffer(d,"What is your race (help <race> for more information)? ",0);
	    d->connected = CON_GET_NEW_RACE;
	    break;
	}

	if ( IS_IMMORTAL(ch) )
	{
	    do_help( ch, "imotd" );
	    d->connected = CON_READ_IMOTD;
 	}
	else
	{
	    do_help( ch, "motd" );
	    d->connected = CON_READ_MOTD;
	}

	for (pos = 0; pos < MAX_DUPES; pos++)
	{
	    if (ch->pcdata->dupes[pos] == NULL)
		break;

	    if ( ( victim = get_char_mortal( ch, str_dup(ch->pcdata->dupes[pos]) ) ) != NULL )
		force_quit(victim, "");
	}

	break;

/* RT code for breaking link */
 
    case CON_BREAK_CONNECT:
	switch( *argument )
	{
	case 'y' : case 'Y':
            for ( d_old = descriptor_list; d_old != NULL; d_old = d_next )
	    {
		d_next = d_old->next;
		if (d_old == d || d_old->character == NULL)
		    continue;

		if (str_cmp(ch->name,d_old->original ?
		    d_old->original->name : d_old->character->name))
		    continue;

		close_socket(d_old);
	    }
	    ch->pcdata->socket = str_dup( d->host );
	    if (check_reconnect(d,ch->name,TRUE))
	    	return;
	    write_to_buffer(d,"Reconnect attempt failed.\n\rName: ",0);
            if ( d->character != NULL )
            {
                free_char( d->character );
                d->character = NULL;
            }
	    d->connected = CON_GET_NAME;
	    break;

	case 'n' : case 'N':
	    write_to_buffer(d,"Name: ",0);
            if ( d->character != NULL )
            {
                free_char( d->character );
                d->character = NULL;
            }
	    d->connected = CON_GET_NAME;
	    break;

	default:
	    write_to_buffer(d,"Please type Y or N? ",0);
	    break;
	}
	break;

    case CON_CONFIRM_NEW_NAME:
	switch ( *argument )
	{
	case 'y': case 'Y':
            write_to_buffer(d,"\n\rNew character.\n\r\n\r"\
" Please choose a password which is not easy to guess and is not\n\r"\
" contained in dictionaries. We recommend using combinations of\n\r"\
" numbers, regular and special characters to ensure maximum\n\r"\
" security. Please note that we don't reimburse people in case\n\r"\
" of hacked characters.\n\r"\
" Paradox MUD also uses encrypted passwords; that means we will\n\r"\
" not be able to tell you your password if you happen to forget\n\r"\
" it.\n\r\n\r",0);
	    sprintf( buf, "Give me a password for %s: %s",
		ch->name, echo_off_str );
	    ch->pcdata->socket = str_dup( d->host );
	    write_to_buffer( d, buf, 0 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    break;

	case 'n': case 'N':
	    write_to_buffer( d, "Ok, what IS it, then? ", 0 );
	    free_char( d->character );
	    d->character = NULL;
	    d->connected = CON_GET_NAME;
	    break;

	default:
	    write_to_buffer( d, "Please type Yes or No? ", 0 );
	    break;
	}
	break;

    case CON_GET_NEW_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strlen(argument) < 5 )
	{
	    write_to_buffer( d,
		"Password must be at least five characters long.\n\rPassword: ",
		0 );
	    return;
	}
        if (!str_infix(ch->name,argument))
        {
	    write_to_buffer( d,
               "Your password can NOT contain your character's name.\n\rPassword: ",0);
            return;
        }
	pwdnew = crypt( argument, ch->name );
	for ( p = pwdnew; *p != '\0'; p++ )
	{
	    if ( *p == '~' )
	    {
		write_to_buffer( d,
		    "New password not acceptable, try again.\n\rPassword: ",
		    0 );
		return;
	    }
	}

	free_string( ch->pcdata->pwd );
	ch->pcdata->pwd	= str_dup( pwdnew );
	write_to_buffer( d, "Please retype password: ", 0 );
	d->connected = CON_CONFIRM_NEW_PASSWORD;
	break;

    case CON_CONFIRM_NEW_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
	{
	    write_to_buffer( d, "Passwords don't match.\n\rRetype password: ",
		0 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    return;
	}
        write_to_buffer(d,
"\n\r Please note that it is recommended to turn color on, if you have the\n\r"\
" possibility. Most parts of the game extensively use ANSI color.\n\r",0);
        write_to_buffer(d,"\n\rDo you want ANSI color? [Y/N] ",0);
        d->connected = CON_ANSI;
        break;
    case CON_ANSI:
       if(argument[0]=='\0'|| UPPER(argument[0])=='Y')
       {
        SET_BIT(ch->act,PLR_COLOUR);
        send_to_char("\n\r{RA{rnsi enable{Rd{x.\n\r",ch);
       }
       else if(UPPER(argument[0])=='N')
       {
        send_to_char("\n\rAnsi disabled.\n\r",ch);
       }
       else
       {
        write_to_buffer(d,"Do you want ANSI color? [Y/N] ",0);
        return;
       }
	
        ch->pcdata->socket = str_dup( d->host );
	write_to_buffer( d, echo_on_str, 0 );
	send_to_char("{cThe following {Craces{c are available:{x\n\r\n\r",ch);
	pos = 0;
	for ( race = 1; race_table[race].name != NULL; race++ )
	{
	    if (!race_table[race].pc_race)
		break;
	    sprintf(newbuf, "{Y({y%2d{Y){x %3s%-24s", race," ", race_table[race].name);
	    send_to_char(newbuf,ch);
	    pos++; 
	    if (pos >= 2) { 
		write_to_buffer(d,"\n\r",1);
		pos = 0;
	    } 
	}
	newbuf[0] = '\0';
	write_to_buffer(d,"\n\r\n\r",0);
	write_to_buffer(d,"What is your race (help <race> for more information)? ",0);
	d->connected = CON_GET_NEW_RACE;
	break;

    case CON_GET_NEW_RACE:
        one_argument(argument,arg);	

        if (!strcmp(arg,"help"))
	{
	    argument = one_argument(argument,arg);
	    if (argument[0] == '\0')
		do_help(ch,"races");
	    else
		do_help(ch,argument);
            write_to_buffer(d,
		"What is your race (help <race> for more information)? ",0);
	    break;
  	}
	
        if(isdigit(argument[0]))
        {
         if(((num=atoi(argument))<1) || (num>=race))
         {
          write_to_buffer(d,
            "That is not a valid number.\n\r",0);
          write_to_buffer(d,
            "What is your race? (help <race> for more information) ",0);
          break;
         }
         else
          race=num;
        }
        else
	 race = race_lookup(argument);

	if (race == 0 || !race_table[race].pc_race)
	{
	    write_to_buffer(d,"That is not a valid race.\n\r",0);
            send_to_char("{cThe following {Craces{c are available:{x\n\r  ",ch);
            for ( race = 1; race_table[race].name != NULL; race++ )
            {
            	if (!race_table[race].pc_race)
                    break;
            	write_to_buffer(d,race_table[race].name,0);
            	write_to_buffer(d," ",1);
            }
            write_to_buffer(d,"\n\r",0);
            write_to_buffer(d,
		"What is your race? (help <race> for more information) ",0);
	    break;
	}

        ch->race = race;
	/* initialize stats */
	for (i = 0; i < MAX_STATS; i++)
	    ch->perm_stat[i] = pc_race_table[race].stats[i];
	ch->affected_by = ch->affected_by|race_table[race].aff;
	ch->shielded_by = ch->shielded_by|race_table[race].shd;
	ch->imm_flags	= ch->imm_flags|race_table[race].imm;
	ch->res_flags	= ch->res_flags|race_table[race].res;
	ch->vuln_flags	= ch->vuln_flags|race_table[race].vuln;
	ch->form	= race_table[race].form;
	ch->parts	= race_table[race].parts;

	/* add skills */
	for (i = 0; i < 5; i++)
	{
	    if (pc_race_table[race].skills[i] == NULL)
	 	break;
	    group_add(ch,pc_race_table[race].skills[i],FALSE);
	}
	/* add cost */
	ch->pcdata->points = pc_race_table[race].points;
	ch->size = pc_race_table[race].size;
        write_to_buffer(d,
"\n\r Gender has no influence on stats or skills. It's just a cosmetic choice.\n\r\n\r",0);
        write_to_buffer( d, "What is your sex (M/F/N)? ", 0 );
        d->connected = CON_GET_NEW_SEX;
        break;
        

    case CON_GET_NEW_SEX:
	switch ( argument[0] )
	{
	case 'm': case 'M': ch->sex = SEX_MALE;    
			    ch->pcdata->true_sex = SEX_MALE;
			    break;
	case 'f': case 'F': ch->sex = SEX_FEMALE; 
			    ch->pcdata->true_sex = SEX_FEMALE;
			    break;
	case 'n': case 'N': ch->sex = SEX_NEUTRAL;
			    ch->pcdata->true_sex = SEX_NEUTRAL;
			    break;
	default:
	    write_to_buffer( d, "That's not a sex.\n\rWhat IS your sex? ", 0 );
	    return;
	}

	write_to_buffer( d, echo_on_str, 0 );
	send_to_char("{cThe following {Cclasses{c are available:{x\n\r\n\r",ch);
	if (ch->pcdata->tier != 1) {
	    ch->pcdata->tier = 0;
	    for ( iClass = 0; iClass < MAX_CLASS/2; iClass++ )
	    {
                sprintf(newbuf,   "{Y({y%2d{Y){x %3s%-25s\n\r",
                    iClass+1," ",class_table[iClass].name);
                send_to_char(newbuf,ch);
/*		write_to_buffer(d,"      ",0);
		write_to_buffer(d,class_table[iClass].name,0);
		write_to_buffer(d,"\n\r",1);*/
	    }
	} else {
	    for ( iClass = MAX_CLASS/2; iClass < MAX_CLASS; iClass++ )
	    {
                sprintf(newbuf,   "{Y({y%2d{Y){x %3s%-25s\n\r",
                      iClass+1," ",class_table[iClass].name);
                send_to_char(newbuf,ch);
/*		write_to_buffer(d,"      ",0);
		write_to_buffer(d,class_table[iClass].name,0);
		write_to_buffer(d,"\n\r",1);*/
	    }
	}
	write_to_buffer(d,"\n\r\n\r",0);
	write_to_buffer(d,"What is your class (help <class> for more information)? ",0);
	d->connected = CON_GET_NEW_CLASS;
	break;

    case CON_GET_NEW_CLASS:
        one_argument(argument,arg);

        if (!strcmp(arg,"help"))
	{
	    argument = one_argument(argument,arg);
	    if (argument[0] == '\0')
		do_help(ch,"tier");
	    else
		do_help(ch,argument);
            write_to_buffer(d,
		"What is your class (help <class> for more information)? ",0);
	    break;
  	}
        if(isdigit(argument[0]))
        {
         num=atoi(argument);
         num--;
         if((num<(ch->pcdata->tier==0?0:MAX_CLASS/2))
           ||(num>=(ch->pcdata->tier==0?MAX_CLASS/2:MAX_CLASS)))
         {
          write_to_buffer(d,
            "That's not a class.\n\rWhat IS your class (help <class> for more information)? ", 0);
          return;
         }
         else 
          iClass=num;
        }
        else
	 iClass = class_lookup(argument);

	if ( iClass == -1 )
	{
	    write_to_buffer( d,
		"That's not a class.\n\rWhat IS your class (help <class> for more information)? ", 0 );
	    return;
	}
	if (ch->pcdata->tier != 1) {
	    if (iClass >= (MAX_CLASS/2)) {
		write_to_buffer( d,
		    "That's not a first tier class.\n\rWhat IS your class (help <class> for more information)? ", 0 );
		return;
	    }
	} else {
	    if (iClass < (MAX_CLASS/2)) {
		write_to_buffer( d,
		    "That's not a second tier class.\n\rWhat IS your class (help <class> for more information)? ", 0 );
		return;
	    }
	}
        ch->class = iClass;

	sprintf( log_buf, "%s@%s new player.", ch->name, d->host );
	log_string( log_buf );
	wiznet("Newbie alert!  $N sighted.",ch,NULL,WIZ_NEWBIE,0,0);
        wiznet(log_buf,NULL,NULL,WIZ_SITES,0,get_trust(ch));
       
	write_to_buffer( d, "\n\r", 2 );
	write_to_buffer( d, "You may be good, neutral, or evil.\n\r",0);
	write_to_buffer( d, "Which alignment (G/N/E)? ",0);
	d->connected = CON_GET_ALIGNMENT;
	break;

case CON_GET_ALIGNMENT:
	switch( argument[0])
	{
	    case 'g' : case 'G' : ch->alignment = 750;  break;
	    case 'n' : case 'N' : ch->alignment = 0;	break;
	    case 'e' : case 'E' : ch->alignment = -750; break;
	    default:
		write_to_buffer(d,"That's not a valid alignment.\n\r",0);
		write_to_buffer(d,"Which alignment (G/N/E)? ",0);
		return;
	}

	write_to_buffer(d,"\n\r",0);

        group_add(ch,"rom basics",FALSE);
        group_add(ch,class_table[ch->class].base_group,FALSE);
        ch->pcdata->learned[gsn_recall] = 50;
	write_to_buffer(d,"Do you wish to customize this character?\n\r",0);
	write_to_buffer(d,"Customization takes time, but allows a wider range of skills and abilities.\n\r",0); 
        if(ch->pcdata->tier==1)
         write_to_buffer(d,"It is recommended to customize second tier characters.\n\r",0);
	write_to_buffer(d,"Customize (Y/N)? ",0);
	d->connected = CON_DEFAULT_CHOICE;
	break;

case CON_DEFAULT_CHOICE:
	write_to_buffer(d,"\n\r",2);
        switch ( argument[0] )
        {
        case 'y': case 'Y': 
	    ch->gen_data = new_gen_data();
	    ch->gen_data->points_chosen = ch->pcdata->points;
	    do_help(ch,"group header");
	    list_group_costs(ch);
	    write_to_buffer(d,"You already have the following skills:\n\r",0);
	    do_skills(ch,"");
	    do_help(ch,"menu choice");
	    d->connected = CON_GEN_GROUPS;
	    break;
        case 'n': case 'N': 
	    group_add(ch,class_table[ch->class].default_group,TRUE);
            if((ch->class != class_lookup("sensai")) &&
                (ch->class != class_lookup("shaolin")))
            {
             write_to_buffer( d, "\n\r", 2 );
	     write_to_buffer(d,
	 	 "Please pick a weapon from the following choices:\n\r",0);
	     buf[0] = '\0';
	     for ( i = 0; weapon_table[i].name != NULL; i++)
		 if (ch->pcdata->learned[*weapon_table[i].gsn] > 0)
		 {
		     strcat(buf,weapon_table[i].name);
		     strcat(buf," ");
	 	 }
	     strcat(buf,"\n\rYour choice? ");
	     write_to_buffer(d,buf,0);
             d->connected = CON_PICK_WEAPON;
            }
            else
            {
             write_to_buffer(d,"\n\r",2);
             do_help(ch,"motd");
             d->connected = CON_READ_MOTD;
            }
            break;
        default:
            write_to_buffer( d, "Please answer (Y/N)? ", 0 );
            return;
        }
	break;

    case CON_PICK_WEAPON:
	write_to_buffer(d,"\n\r",2);
	weapon = weapon_lookup(argument);
	if (weapon == -1 || ch->pcdata->learned[*weapon_table[weapon].gsn] <= 0)
	{
	    write_to_buffer(d,
		"That's not a valid selection. Choices are:\n\r",0);
            buf[0] = '\0';
            for ( i = 0; weapon_table[i].name != NULL; i++)
                if (ch->pcdata->learned[*weapon_table[i].gsn] > 0)
                {
                    strcat(buf,weapon_table[i].name);
		    strcat(buf," ");
                }
            strcat(buf,"\n\rYour choice? ");
            write_to_buffer(d,buf,0);
	    return;
	}

	ch->pcdata->learned[*weapon_table[weapon].gsn] = 40;
	write_to_buffer(d,"\n\r",2);
	do_help(ch,"motd");
	d->connected = CON_READ_MOTD;
	break;

    case CON_GEN_GROUPS:
	send_to_char("\n\r",ch);
       	if (!str_cmp(argument,"done"))
       	{
	    sprintf(buf,"Creation points: %d\n\r",ch->pcdata->points);
	    send_to_char(buf,ch);
	    sprintf(buf,"Experience per level: %ld\n\r",
	            (long)exp_per_level(ch,ch->gen_data->points_chosen));
	    if (ch->pcdata->points < 40)
		ch->train = (40 - ch->pcdata->points + 1) / 2;
	    free_gen_data(ch->gen_data);
	    ch->gen_data = NULL;
	    send_to_char(buf,ch);
            if((ch->class!=class_lookup("sensai")) &&
                  (ch->class!=class_lookup("shaolin")))
             {
             write_to_buffer( d, "\n\r", 2 );
             write_to_buffer(d,
                 "Please pick a weapon from the following choices:\n\r",0);
             buf[0] = '\0';
             for ( i = 0; weapon_table[i].name != NULL; i++)
                 if (ch->pcdata->learned[*weapon_table[i].gsn] > 0)
                 {
                     strcat(buf,weapon_table[i].name);
	       	     strcat(buf," ");
                 }
             strcat(buf,"\n\rYour choice? ");
             write_to_buffer(d,buf,0);
             d->connected = CON_PICK_WEAPON;
            }
            else
            {
             write_to_buffer(d,"\n\r",2);
             do_help(ch,"motd");
             d->connected = CON_READ_MOTD;
            }
            break;
        }

        if (!parse_gen_groups(ch,argument))
        send_to_char(
        "Choices are: list,learned,premise,add,drop,info,help,class and done.\n\r"
        ,ch);

        do_help(ch,"menu choice");
        break;

    case CON_READ_IMOTD:
	write_to_buffer(d,"\n\r",2);
        do_help( ch, "motd" );
        d->connected = CON_READ_MOTD;
	break;

    case CON_READ_MOTD:
        if ( ch->pcdata == NULL || ch->pcdata->pwd[0] == '\0')
        {
            write_to_buffer( d, "Warning! Null password!\n\r",0 );
            write_to_buffer( d, "Please report old password with bug.\n\r",0);
            write_to_buffer( d,
                "Type 'password null <new password>' to fix.\n\r",0);
        }

	send_to_char( 
    "\n\r{GWelcome to {GP{ga{Wr{Da{Wd{go{GX {GMUD.{x\n\r",
	    ch );
/*	do_count(ch,"");*/
  /*      if(!IS_SET(ch->act,PLR_REROLL))
        {*/
	 ch->next	= char_list;
	 char_list	= ch;
 /*        }
        else
         REMOVE_BIT(ch->act,PLR_REROLL);*/
	d->connected	= CON_PLAYING;
	reset_char(ch);
        
	if ( ch->level == 0 )
	{

	    ch->perm_stat[class_table[ch->class].attr_prime] += 3;

/* add this while initializing all the racial stuff for new characters */
	    ch->level	= 1;
	    ch->exp	= exp_per_level(ch,ch->pcdata->points);
	    ch->hit	= ch->max_hit;
	    ch->mana	= ch->max_mana;
	    ch->move	= ch->max_move;
	    ch->train	 = 20;
	    ch->practice = 25;
            ch->pcdata->pkp = 100;
	    /*sprintf( buf, "the %s",
		title_table [ch->class] [ch->level]
		[ch->sex == SEX_FEMALE ? 1 : 0] );
	    set_title( ch, buf );*/
	    do_pack(ch,"self");
	    do_autoall(ch,"");
	    /*obj_to_char(create_object(get_obj_index(OBJ_VNUM_MAP),0),ch);
	    obj_to_char(create_object(get_obj_index(OBJ_VNUM_WMAP),0),ch);
	    obj_to_char(create_object(get_obj_index(OBJ_VNUM_EMAP),0),ch);*/

	    char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
	    send_to_char("\n\r",ch);
	    /*do_help(ch,"NEWBIE INFO");
	    send_to_char("\n\r",ch);*/
	    add_socket(ch->pcdata->socket,ch->name);
	}
	else if ( ch->in_room != NULL )
	{
	    char_to_room( ch, ch->in_room );
	}
	else if ( IS_IMMORTAL(ch) )
	{
	    char_to_room( ch, get_room_index( ROOM_VNUM_CHAT ) );
	}
	else
	{
	    if ( ch->alignment <0 )
		char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLEB ) );
	    else
		char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
	}

/*	act( "$n has entered RoD.", ch, NULL, NULL, TO_ROOM );
	do_look( ch, "auto" );*/

         if (IS_IMMORTAL(ch) )
          {
	wiznet("$N has entered Paradox MUD.",ch,NULL,
		WIZ_LOGINS,WIZ_SITES,get_trust(ch));
        ch->position = POS_STANDING;
        do_look( ch, "auto" );
        }
        else
        {
/*        do_announce( ch, "has entered Paradox MUD." );*/
        act("$n has entered the game.",ch,NULL,NULL,TO_ROOM);
        ch->position=POS_STANDING;
        do_look( ch, "auto" );
        wiznet("$N has entered Paradox MUD.",ch,NULL,
                WIZ_LOGINS,WIZ_SITES,get_trust(ch));
        }

	if (ch->pet != NULL)
	{
	    char_to_room(ch->pet,ch->in_room);
	    act("$n has entered the game.",ch->pet,NULL,NULL,TO_ROOM);
	}
        conn_so_far++;
        if(!IS_IMMORTAL(ch))
        register_rank(ch->name,ch->pcdata->pkp,ch->race,
		ch->class,ch->clan);
	do_unread(ch,"");
	break;
    }

    return;
}



/*
 * Parse a name for acceptability.
 */
bool check_parse_name( char *name )
{
    int e;
    /*
     * Reserved words.
     */
    if ( is_name( name, 
	"all auto immortal immortals self someone something the you demise balance circle loner honor somone unlinked") )
	return FALSE;
	
    if (str_cmp(capitalize(name),"Alander") && (!str_prefix("Alan",name)
    || !str_suffix("Alander",name)))
	return FALSE;

    /*
     * Cursing
     */

    for ( e = 1; e < MAX_CLAN; e++)
    {
	if (!str_prefix(clan_table[e].name, name))
	    return FALSE;
    }
    if (!str_infix("immortal",name))
	return FALSE;
    if (!str_infix(" ", name))
	return FALSE;
    if (!str_infix("fuck",name))
	return FALSE;
    if (!str_infix("shit",name))
        return FALSE;
    if (!str_infix("asshole",name))
        return FALSE;
    if (!str_infix("pussy",name))
        return FALSE;
    /*
     * Length restrictions.
     */
     
    if ( strlen(name) <  2 )
	return FALSE;

#if defined(MSDOS)
    if ( strlen(name) >  8 )
	return FALSE;
#endif

#if defined(macintosh) || defined(unix)
    if ( strlen(name) > 12 )
	return FALSE;
#endif

    /*
     * Alphanumerics only.
     * Lock out IllIll twits.
     */
    {
	char *pc;
	bool fIll,adjcaps = FALSE,cleancaps = FALSE;
 	int total_caps = 0;

	fIll = TRUE;
	for ( pc = name; *pc != '\0'; pc++ )
	{
	    if ( !isalpha(*pc) )
		return FALSE;

	    if ( isupper(*pc)) /* ugly anti-caps hack */
	    {
		if (adjcaps)
		    cleancaps = TRUE;
		total_caps++;
		adjcaps = TRUE;
	    }
	    else
		adjcaps = FALSE;

	    if ( LOWER(*pc) != 'i' && LOWER(*pc) != 'l' )
		fIll = FALSE;
	}

	if ( fIll )
	    return FALSE;

	if (cleancaps || (total_caps > (strlen(name)) / 2 && strlen(name) < 3))
	    return FALSE;
    }

    /*
     * Prevent players from naming themselves after mobs.
     */
    {
	extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
	MOB_INDEX_DATA *pMobIndex;
	int iHash;

	for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
	    for ( pMobIndex  = mob_index_hash[iHash];
		  pMobIndex != NULL;
		  pMobIndex  = pMobIndex->next )
	    {
		if ( is_name( name, pMobIndex->player_name ) )
		    return FALSE;
	    }
	}
    }

    return TRUE;
}



/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect( DESCRIPTOR_DATA *d, char *name, bool fConn )
{
    CHAR_DATA *ch;
    char buf[MAX_STRING_LENGTH];

    for ( ch = char_list; ch != NULL; ch = ch->next )
    {
	if ( !IS_NPC(ch)
	&&   (!fConn || ch->desc == NULL)
	&&   !str_cmp( d->character->name, ch->name ) )
	{
	    if ( fConn == FALSE )
	    {
		free_string( d->character->pcdata->pwd );
		d->character->pcdata->pwd = str_dup( ch->pcdata->pwd );
	    }
	    else
	    {
		OBJ_DATA *obj;

		free_char( d->character );
		d->character = ch;
		ch->desc	 = d;
		ch->timer	 = 0;
		if(ch->tells)
		{
		    sprintf( buf, "Reconnecting.  You have {R%d{x tells waiting.\n\r",
			ch->tells );
		    send_to_char( buf, ch );
		    send_to_char("Type 'replay' to see tells.\n\r",ch);
		}
		else
		{
			send_to_char("Reconnecting.  You have no tells waiting.\n\r",ch);
		}
		act( "$n has reconnected.", ch, NULL, NULL, TO_ROOM );
		if ((obj = get_eq_char(ch,WEAR_LIGHT)) != NULL
		&&  obj->item_type == ITEM_LIGHT && obj->value[2] != 0)
		    --ch->in_room->light;

		sprintf( log_buf, "%s@%s reconnected.", ch->name, d->host );
		log_string( log_buf );
		wiznet("$N groks the fullness of $S link.",
		    ch,NULL,WIZ_LINKS,0,0);
		d->connected = CON_PLAYING;
	    }
	    return TRUE;
	}
    }

    return FALSE;
}



/*
 * Check if already playing.
 */
bool check_playing( DESCRIPTOR_DATA *d, char *name )
{
    DESCRIPTOR_DATA *dold;

    for ( dold = descriptor_list; dold; dold = dold->next )
    {
	if ( dold != d
	&&   dold->character != NULL
	&&   dold->connected != CON_GET_NAME
	&&   dold->connected != CON_GET_OLD_PASSWORD
	&&   !str_cmp( name, dold->original
	         ? dold->original->name : dold->character->name ) )
	{
	    write_to_buffer( d, "That character is already playing.\n\r",0);
	    write_to_buffer( d, "Do you wish to connect anyway (Y/N)?",0);
	    d->connected = CON_BREAK_CONNECT;
	    return TRUE;
	}
    }

    return FALSE;
}



void stop_idling( CHAR_DATA *ch )
{
    if ( ch == NULL
    ||   ch->desc == NULL
    ||   ch->desc->connected != CON_PLAYING
    ||   ch->was_in_room == NULL 
    ||   ch->in_room != get_room_index(ROOM_VNUM_LIMBO))
	return;

    ch->timer = 0;
    char_from_room( ch );
    char_to_room( ch, ch->was_in_room );
    ch->was_in_room	= NULL;
    act( "$n has returned from the void.", ch, NULL, NULL, TO_ROOM );
    return;
}



/*
 * Write to one char.
 */
void send_to_char_bw( const char *txt, CHAR_DATA *ch )
{
    if ( txt != NULL && ch->desc != NULL )
        write_to_buffer( ch->desc, txt, strlen(txt) );
    return;
}

/* Write to one char, check if (s)he wants to receive that output first */
void xsend_to_char(const char *txt, CHAR_DATA *ch, int suppress)
{
 if(IS_NPC(ch))
 {
  send_to_char(txt,ch);
  return;
 }
 if(!IS_SET(ch->pcdata->suppress,suppress))
  send_to_char(txt,ch);
 return;
}


/*
 * Write to one char, new colour version, by Lope.
 */
void send_to_char( const char *txt, CHAR_DATA *ch )
{
    const	char 	*point;
    		char 	*point2;
    		char 	buf[ MAX_STRING_LENGTH*4 ];

    buf[0] = '\0';
    point2 = buf;
    if( txt && ch->desc )
	{
	    if( IS_SET( ch->act, PLR_COLOUR ) )
	    {
		for( point = txt ; *point ; point++ )
	        {
		    if( *point == '{' )
		    {
			point++;
			strcat( buf, colour( *point, ch ) );
			for( point2 = buf ; *point2 ; point2++ )
			    ;
			continue;
		    }
		    *point2 = *point;
		    *++point2 = '\0';
		}			
		*point2 = '\0';
        	write_to_buffer( ch->desc, buf, point2 - buf );
	    }
	    else
	    {
		for( point = txt ; *point ; point++ )
	        {
		    if( *point == '{' )
		    {
			point++;
			if( *point != '{' )
			{
			    continue;
			}
		    }
		    *point2 = *point;
		    *++point2 = '\0';
		}
		*point2 = '\0';
        	write_to_buffer( ch->desc, buf, point2 - buf );
	    }
	}
    return;
}

/*
 * Send a page to one char.
 */
void page_to_char_bw( const char *txt, CHAR_DATA *ch )
{
    if ( txt == NULL || ch->desc == NULL)


    if (ch->lines == 0 )
    {
	send_to_char(txt,ch);
	return;
    }
	
#if defined(macintosh)
	send_to_char(txt,ch);
#else
    ch->desc->showstr_head = alloc_mem(strlen(txt) + 1);
    strcpy(ch->desc->showstr_head,txt);
    ch->desc->showstr_point = ch->desc->showstr_head;
    show_string(ch->desc,"");
#endif
}

/*
 * Page to one char, new colour version, by Lope.
 */
void page_to_char( const char *txt, CHAR_DATA *ch )
{
    const	char	*point;
    		char	*point2;
    		char	buf[ MAX_STRING_LENGTH * 4 ];

    buf[0] = '\0';
    point2 = buf;
    if( txt && ch->desc )
	{
	    if( IS_SET( ch->act, PLR_COLOUR ) )
	    {
		for( point = txt ; *point ; point++ )
	        {
		    if( *point == '{' )
		    {
			point++;
			strcat( buf, colour( *point, ch ) );
			for( point2 = buf ; *point2 ; point2++ )
			    ;
			continue;
		    }
		    *point2 = *point;
		    *++point2 = '\0';
		}			
		*point2 = '\0';
		free_string( ch->desc->showstr_head );
		ch->desc->showstr_head  = str_dup( buf );
		ch->desc->showstr_point = ch->desc->showstr_head;
		show_string( ch->desc, "" );
	    }
	    else
	    {
		for( point = txt ; *point ; point++ )
	        {
		    if( *point == '{' )
		    {
			point++;
			if( *point != '{' )
			{
			    continue;
			}
		    }
		    *point2 = *point;
		    *++point2 = '\0';
		}
		*point2 = '\0';
		free_string( ch->desc->showstr_head );
		ch->desc->showstr_head  = str_dup( buf );
		ch->desc->showstr_point = ch->desc->showstr_head;
		show_string( ch->desc, "" );
	    }
	}
    return;
}

/* string pager */
void show_string(struct descriptor_data *d, char *input)
{
    char buffer[4*MAX_STRING_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    register char *scan, *chk;
    int lines = 0, toggle = 1;
    int show_lines;

    one_argument(input,buf);
    if (buf[0] != '\0')
    {
	if (d->showstr_head)
	{
	    free_string(d->showstr_head);
	    d->showstr_head = 0;
	}
    	d->showstr_point  = 0;
	return;
    }

    if (d->character)
	show_lines = d->character->lines;
    else
	show_lines = 0;

    for (scan = buffer; ; scan++, d->showstr_point++)
    {
	if (((*scan = *d->showstr_point) == '\n' || *scan == '\r')
	    && (toggle = -toggle) < 0)
	    lines++;

	else if (!*scan || (show_lines > 0 && lines >= show_lines))
	{
	    *scan = '\0';
	    write_to_buffer(d,buffer,strlen(buffer));
	    for (chk = d->showstr_point; isspace(*chk); chk++);
	    {
		if (!*chk)
		{
		    if (d->showstr_head)
        	    {
            		free_string(d->showstr_head);
            		d->showstr_head = 0;
        	    }
        	    d->showstr_point  = 0;
    		}
	    }
	    return;
	}
    }
    return;
}
	

/* quick sex fixer */
void fix_sex(CHAR_DATA *ch)
{
    if (ch->sex < 0 || ch->sex > 2)
    	ch->sex = IS_NPC(ch) ? 0 : ch->pcdata->true_sex;
}

void act (const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2,
          int type)
{
    /* to be compatible with older code */
    xact_new(format,ch,arg1,arg2,type,POS_RESTING,SUPPRESS_STD);
}

void act_new (const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2,
	  int type, int min_pos )
{
    /* to be compatible with older code */
    xact_new(format,ch,arg1,arg2,type,min_pos,SUPPRESS_STD);
}

/*
 * The colour version of the act( ) function, -Lope
 */
/*
 * Extended act with optional suppression - Gregor Stipicic, 2001
 */
void xact_new( const char *format, CHAR_DATA *ch, const void *arg1,
	      const void *arg2, int type, int min_pos, int suppress )
{
    static char * const he_she  [] = { "it",  "he",  "she" };
    static char * const him_her [] = { "it",  "him", "her" };
    static char * const his_her [] = { "its", "his", "her" };
 
    CHAR_DATA 		*to;
    CHAR_DATA 		*vch = ( CHAR_DATA * ) arg2;
    OBJ_DATA 		*obj1 = ( OBJ_DATA  * ) arg1;
    OBJ_DATA 		*obj2 = ( OBJ_DATA  * ) arg2;
    const 	char 	*str;
    char 		*i;
    char 		*point;
    char 		*i2;
    char 		fixed[ MAX_STRING_LENGTH ];
    char 		buf[ MAX_STRING_LENGTH   ];
    char 		fname[ MAX_INPUT_LENGTH  ];
    bool		fColour = FALSE;
    int collen=-1,count=-1;
    if( !format || !*format )
        return;

    if( !ch || !ch->in_room )
	return;

    to = ch->in_room->people;
    if( type == TO_VICT )
    {
        if ( !vch )
        {
            bug( "Act: null vch with TO_VICT.", 0 );
            return;
        }

	if ( !vch->in_room )
	    return;

        to = vch->in_room->people;
    }
 
    for( ; to ; to = to->next_in_room )
    {
        	if ( (!IS_NPC(to) && to->desc == NULL )
	||   ( IS_NPC(to) && !HAS_TRIGGER(to, TRIG_ACT) )
	||    to->position < min_pos )
            continue;
 
        if( type == TO_CHAR && to != ch )
            continue;
        if( type == TO_VICT && ( to != vch || to == ch ) )
            continue;
        if( type == TO_ROOM && to == ch )
            continue;
        if( type == TO_NOTVICT && ( to == ch || to == vch ) )
            continue;
 
        point   = buf;
        str     = format;
        while( *str )
        {
            count++;
            if( *str != '$' && *str != '{' )
            {
                *point++ = *str++;
                continue;
            }

	    i = NULL;
	    switch( *str )
	    {
		case '$':
		    fColour = TRUE;
		    ++str;
		    i = " <@@@> ";
		    if ( !arg2 && *str >= 'A' && *str <= 'Z' && *str != 'G' )
		    {
			bug( "Act: missing arg2 for code %d.", *str );
			i = " <@@@> ";
		    }
		    else
		    {
			switch ( *str )
			{
			    default:  
				bug( "Act: bad code %d.", *str );
				i = " <@@@> ";                                
				break;

			    case 't': 
				i = (char *) arg1;                            
				break;

			    case 'T': 
				i = (char *) arg2;                            
				break;

			    case 'n': 
				i = PERS( ch,  to  );                         
				break;

			    case 'N': 
				i = PERS( vch, to  );                         
				break;
                            
			    case 'w':
				i = SWPERS( ch, to );
                                break;
			    case 'e': 
				i = he_she  [URANGE(0, ch  ->sex, 2)];        
				break;

			    case 'E': 
				i = he_she  [URANGE(0, vch ->sex, 2)];        
				break;

			    case 'm': 
				i = him_her [URANGE(0, ch  ->sex, 2)];        
				break;

			    case 'M': 
				i = him_her [URANGE(0, vch ->sex, 2)];        
				break;

			    case 's': 
				i = his_her [URANGE(0, ch  ->sex, 2)];        
				break;

			    case 'S': 
				i = his_her [URANGE(0, vch ->sex, 2)];        
				break;
 
			    case 'p':
				i = can_see_obj( to, obj1 )
				  ? obj1->short_descr
				  : "something";
				break;
 
			    case 'P':
				i = can_see_obj( to, obj2 )
				  ? obj2->short_descr
				  : "something";
				break;
 
			    case 'd':
				if ( !arg2 || ((char *) arg2)[0] == '\0' )
				{
				    i = "door";
				}
				else
				{
				    one_argument( (char *) arg2, fname );
				    i = fname;
				}
				break;

			    case 'G':
				if ( ch->alignment < 0 )
				{
				    i = "Fade";
				}
				else
				{
				    i = "Fade";
				}
				break;

			}
		    }
		    break;

		case '{':
		    fColour = FALSE;
		    ++str;
		    i = NULL;
		    if( IS_SET( to->act, PLR_COLOUR ) )
		    {
			i = colour( *str, to );
                        if((collen==-1) && (count==0))
                         collen=strlen(i);
		    }
		    break;

		default:
		    fColour = FALSE;
		    *point++ = *str++;
		    break;
	    }

            ++str;
	    if( fColour && i )
	    {
		fixed[0] = '\0';
		i2 = fixed;

		if( IS_SET( to->act, PLR_COLOUR ) )
		{
		    for( i2 = fixed ; *i ; i++ )
	            {
			if( *i == '{' )
			{
			    i++;
			    strcat( fixed, colour( *i, to ) );
			    for( i2 = fixed ; *i2 ; i2++ )
				;
			    continue;
			}
			*i2 = *i;
			*++i2 = '\0';
		    }			
		    *i2 = '\0';
		    i = &fixed[0];
		}
	        else
		{
		    for( i2 = fixed ; *i ; i++ )
	            {
			if( *i == '{' )
			{
			    i++;
			    if( *i != '{' )
			    {
				continue;
			    }
			}
			*i2 = *i;
			*++i2 = '\0';
		    }			
		    *i2 = '\0';
		    i = &fixed[0];
		}
	    }


	    if( i )
	    {
		while( ( *point = *i ) != '\0' )
		{
		    ++point;
		    ++i;
		}
	    }
        }
 
        *point++	= '\n';
        *point++	= '\r';
        *point		= '\0';

        if(collen)
         buf[collen]=UPPER(buf[collen]);
  	buf[0]=UPPER( buf[0] );
	if ( to->desc != NULL)
        {
          if(IS_NPC(to) || !IS_SET(to->pcdata->suppress,suppress))
	    write_to_buffer( to->desc, buf, point - buf );
          continue;
	}
	if ( MOBtrigger )
	    mp_act_trigger( buf, to, ch, arg1, arg2, TRIG_ACT );
    }
    return;
}

char *colour( char type, CHAR_DATA *ch )
{

    if( IS_NPC( ch ) )
	return ( "" );

    switch( type )
    {
	default:
	    sprintf(clcode, colour_clear( ch ));
	    break;
	case 'x':
	    sprintf(clcode, colour_clear( ch ));
	    break;
        case '0':
	    sprintf(clcode, colour_clear( ch ));
	    break;
	case 'z':
	    sprintf( clcode, BLINK );
	    break;
	case 'b':
	    sprintf( clcode, C_BLUE );
	    break;
        case '4':
            sprintf( clcode, C_BLUE );
            break;
	case 'c':
	    sprintf( clcode, C_CYAN );
	    break;
        case '6':
            sprintf( clcode, C_CYAN );
            break;
	case 'g':
	    sprintf( clcode, C_GREEN );
	    break;
        case '2':
            sprintf( clcode, C_GREEN );
            break;
	case 'm':
	    sprintf( clcode, C_MAGENTA );
	    break;
        case '5':
            sprintf( clcode, C_MAGENTA );
            break;
	case 'r':
	    sprintf( clcode, C_RED );
	    break;
        case '1':
            sprintf( clcode, C_RED );
            break;
	case 'w':
	    sprintf( clcode, C_WHITE );
	    break;
        case '7':
            sprintf( clcode, C_WHITE );
            break;
	case 'y':
	    sprintf( clcode, C_YELLOW );
	    break;
        case '3':
            sprintf( clcode, C_YELLOW );
            break;
	case 'B':
	    sprintf( clcode, C_B_BLUE );
	    break;
        case '$':
            sprintf( clcode, C_B_BLUE );
            break;
	case 'C':
	    sprintf( clcode, C_B_CYAN );
	    break;
        case '^':
            sprintf( clcode, C_B_CYAN );
            break;
	case 'G':
	    sprintf( clcode, C_B_GREEN );
	    break;
        case '@':
            sprintf( clcode, C_B_GREEN );
            break;
	case 'M':
	    sprintf( clcode, C_B_MAGENTA );
	    break;
        case '%':
            sprintf( clcode, C_B_MAGENTA );
            break;
	case 'R':
	    sprintf( clcode, C_B_RED );
	    break;
        case '!':
            sprintf( clcode, C_B_RED );
            break;
	case 'W':
	    sprintf( clcode, C_B_WHITE );
	    break;
        case '&':
            sprintf( clcode, C_B_WHITE );
            break;
	case 'Y':
	    sprintf( clcode, C_B_YELLOW );
	    break;
        case '#':
            sprintf( clcode, C_B_YELLOW );
            break;
	case 'D':
	    sprintf( clcode, C_D_GREY );
	    break;
        case '8':
            sprintf( clcode, C_D_GREY );
            break;
        case '*':
            switch(number_range(2,16))
            {
		case 2: sprintf(clcode, C_RED); break;
		case 3: sprintf(clcode, C_GREEN); break;
		case 4: sprintf(clcode, C_YELLOW); break;
		case 5: sprintf(clcode, C_BLUE); break;
		case 6: sprintf(clcode, C_MAGENTA); break;
		case 7: sprintf(clcode, C_CYAN); break;
		case 8: sprintf(clcode, C_WHITE); break;
		case 9: sprintf(clcode, C_D_GREY); break;
		case 10: sprintf(clcode, C_B_RED); break;
		case 11: sprintf(clcode, C_B_GREEN); break;
		case 12: sprintf(clcode, C_B_YELLOW); break;
		case 13: sprintf(clcode, C_B_BLUE); break;
		case 14: sprintf(clcode, C_B_MAGENTA); break;
		case 15: sprintf(clcode, C_B_CYAN); break;
		case 16: sprintf(clcode, C_B_WHITE); break;
            }
            break;

	case 'A':	/* Auction Channel */
	    if (ch->color_auc)
	    {
		sprintf( clcode, colour_channel(ch->color_auc, ch));
	    }
	    else
	    {
		sprintf( clcode, C_B_GREEN );
	    }
	    break;
	case 'E':	/* Clan Gossip Channel */
	    if (ch->color_cgo)
	    {
		sprintf( clcode, colour_channel(ch->color_cgo, ch));
	    }
	    else
	    {
		sprintf( clcode, C_B_RED );
	    }
	    break;
	case 'F':	/* Clan Talk Channel */
	    if (ch->color_cla)
	    {
		sprintf( clcode, colour_channel(ch->color_cla, ch));
	    }
	    else
	    {
		sprintf( clcode, C_B_MAGENTA );
	    }
	    break;
	case 'H':	/* Gossip Channel */
	    if (ch->color_gos)
	    {
		sprintf( clcode, colour_channel(ch->color_gos, ch));
	    }
	    else
	    {
		sprintf( clcode, C_B_BLUE );
	    }
	    break;
	case 'J':	/* Grats Channel */
	    if (ch->color_gra)
	    {
		sprintf( clcode, colour_channel(ch->color_gra, ch));
	    }
	    else
	    {
		sprintf( clcode, C_YELLOW );
	    }
	    break;
	case 'K':	/* Group Tell Channel */
	    if (ch->color_gte)
	    {
		sprintf( clcode, colour_channel(ch->color_gte, ch));
	    }
	    else
	    {
		sprintf( clcode, C_CYAN );
	    }
	    break;
	case 'L':	/* Immortal Talk Channel */
	    if (ch->color_imm)
	    {
		sprintf( clcode, colour_channel(ch->color_imm, ch));
	    }
	    else
	    {
		sprintf( clcode, C_B_WHITE );
	    }
	    break;
	case 'N':	/* Music Channel */
	    if (ch->color_mus)
	    {
		sprintf( clcode, colour_channel(ch->color_mus, ch));
	    }
	    else
	    {
		sprintf( clcode, C_MAGENTA );
	    }
	    break;
	case 'P':	/* Question+Answer Channel */
	    if (ch->color_que)
	    {
		sprintf( clcode, colour_channel(ch->color_que, ch));
	    }
	    else
	    {
		sprintf( clcode, C_B_YELLOW );
	    }
	    break;
	case 'Q':	/* Quote Channel */
	    if (ch->color_quo)
	    {
		sprintf( clcode, colour_channel(ch->color_quo, ch));
	    }
	    else
	    {
		sprintf( clcode, C_GREEN );
	    }
	    break;
	case 'S':	/* Say Channel */
	    if (ch->color_say)
	    {
		sprintf( clcode, colour_channel(ch->color_say, ch));
	    }
	    else
	    {
		sprintf( clcode, C_MAGENTA );
	    }
	    break;
	case 'T':	/* Shout+Yell Channel */
	    if (ch->color_sho)
	    {
		sprintf( clcode, colour_channel(ch->color_sho, ch));
	    }
	    else
	    {
		sprintf( clcode, C_B_RED );
	    }
	    break;
	case 'U':	/* Tell+Reply Channel */
	    if (ch->color_tel)
	    {
		sprintf( clcode, colour_channel(ch->color_tel, ch));
	    }
	    else
	    {
		sprintf( clcode, C_CYAN );
	    }
	    break;
	case 'V':	/* Wiznet Messages */
	    if (ch->color_wiz)
	    {
		sprintf( clcode, colour_channel(ch->color_wiz, ch));
	    }
	    else
	    {
		sprintf( clcode, C_WHITE );
	    }
	    break;
	case 'a':	/* Mobile Talk */
	    if (ch->color_mob)
	    {
		sprintf( clcode, colour_channel(ch->color_mob, ch));
	    }
	    else
	    {
		sprintf( clcode, C_MAGENTA );
	    }
	    break;
	case 'e':	/* Room Title */
	    if (ch->color_roo)
	    {
		sprintf( clcode, colour_channel(ch->color_roo, ch));
	    }
	    else
	    {
		sprintf( clcode, C_B_BLUE );
	    }
	    break;
	case 'f':	/* Opponent Condition */
	    if (ch->color_con)
	    {
		sprintf( clcode, colour_channel(ch->color_con, ch));
	    }
	    else
	    {
		sprintf( clcode, C_B_RED );
	    }
	    break;
	case 'h':	/* Fight Actions */
	    if (ch->color_fig)
	    {
		sprintf( clcode, colour_channel(ch->color_fig, ch));
	    }
	    else
	    {
		sprintf( clcode, C_B_BLUE );
	    }
	    break;
	case 'i':	/* Opponents Fight Actions */
	    if (ch->color_opp)
	    {
		sprintf( clcode, colour_channel(ch->color_opp, ch));
	    }
	    else
	    {
		sprintf( clcode, C_CYAN );
	    }
	    break;
	case 'j':	/* Disarm Messages */
	    if (ch->color_dis)
	    {
		sprintf( clcode, colour_channel(ch->color_dis, ch));
	    }
	    else
	    {
		sprintf( clcode, C_B_YELLOW );
	    }
	    break;
	case 'k':	/* Witness Messages */
	    if (ch->color_wit)
	    {
		sprintf( clcode, colour_channel(ch->color_wit, ch));
	    }
	    else
	    {
		sprintf( clcode, colour_clear( ch ));
	    }
	    break;
	case 'l':	/* Quest Gossip */
            if (ch->color_qgo)
            {
                sprintf( clcode, colour_channel(ch->color_qgo, ch));
            }
            else
            {
                sprintf( clcode, C_B_CYAN);
            }
            break;

	case '{':
	    sprintf( clcode, "%c", '{' );
	    break;
    }
    return clcode;
}

char *colour_clear( CHAR_DATA *ch )
{

    if (ch->color)
    {
	if (ch->color == 1)
	    sprintf( clcode, R_RED );
	else if (ch->color == 2)
	    sprintf( clcode, R_GREEN );
	else if (ch->color == 3)
	    sprintf( clcode, R_YELLOW );
	else if (ch->color == 4)
	    sprintf( clcode, R_BLUE );
	else if (ch->color == 5)
	    sprintf( clcode, R_MAGENTA );
	else if (ch->color == 6)
	    sprintf( clcode, R_CYAN );
	else if (ch->color == 7)
	    sprintf( clcode, R_WHITE );
	else if (ch->color == 8)
	    sprintf( clcode, R_D_GREY );
	else if (ch->color == 9)
	    sprintf( clcode, R_B_RED );
	else if (ch->color == 10)
	    sprintf( clcode, R_B_GREEN );
	else if (ch->color == 11)
	    sprintf( clcode, R_B_YELLOW );
	else if (ch->color == 12)
	    sprintf( clcode, R_B_BLUE );
	else if (ch->color == 13)
	    sprintf( clcode, R_B_MAGENTA );
	else if (ch->color == 14)
	    sprintf( clcode, R_B_CYAN );
	else if (ch->color == 15)
	    sprintf( clcode, R_B_WHITE );
	else if (ch->color == 16)
	    sprintf( clcode, R_BLACK );
	else
	    sprintf( clcode, CLEAR );
    }
    else
    {
	sprintf( clcode, CLEAR );
    }
    return clcode;
}

char *colour_channel( int colornum, CHAR_DATA *ch )
{

    if (colornum == 1)
	    sprintf( clcode, C_RED );
    else if (colornum == 2)
	    sprintf( clcode, C_GREEN );
    else if (colornum == 3)
	    sprintf( clcode, C_YELLOW );
    else if (colornum == 4)
	    sprintf( clcode, C_BLUE );
    else if (colornum == 5)
	    sprintf( clcode, C_MAGENTA );
    else if (colornum == 6)
	    sprintf( clcode, C_CYAN );
    else if (colornum == 7)
	    sprintf( clcode, C_WHITE );
    else if (colornum == 8)
	    sprintf( clcode, C_D_GREY );
    else if (colornum == 9)
	    sprintf( clcode, C_B_RED );
    else if (colornum == 10)
	    sprintf( clcode, C_B_GREEN );
    else if (colornum == 11)
	    sprintf( clcode, C_B_YELLOW );
    else if (colornum == 12)
	    sprintf( clcode, C_B_BLUE );
    else if (colornum == 13)
	    sprintf( clcode, C_B_MAGENTA );
    else if (colornum == 14)
	    sprintf( clcode, C_B_CYAN );
    else if (colornum == 15)
	    sprintf( clcode, C_B_WHITE );
    else if (colornum == 16)
	    sprintf( clcode, C_BLACK );
    else
	sprintf( clcode, colour_clear( ch ));

    return clcode;
}


/*
 * Macintosh support functions.
 */
#if defined(macintosh)
int gettimeofday( struct timeval *tp, void *tzp )
{
    tp->tv_sec  = time( NULL );
    tp->tv_usec = 0;
}
#endif

void init_descriptor2 (DESCRIPTOR_DATA *dnew, int desc)
{
 	static DESCRIPTOR_DATA d_zero;
 
 	*dnew = d_zero;
 	dnew->descriptor = desc;
 	dnew->character = NULL;
 	dnew->connected = CON_GET_NAME;
 	dnew->showstr_head = str_dup ("");
 	dnew->showstr_point = 0;
 	dnew->pEdit = NULL;			/* OLC */
 	dnew->pString = NULL;		/* OLC */
 	dnew->editor = 0;			/* OLC */
 	dnew->outsize = 2000;
 	dnew->outbuf = alloc_mem (dnew->outsize);
 	
     /* Initialize command history list. */
    /* {
         int  i;
         for ( i = 0; i < MAX_HISTORY; i++ )
             dnew->hist_cmd[i][0] = '\0';
     }*/
 
}

void sig_handler(int sig)
{
 switch(sig)
 {
 case SIGBUS:
  bug("Sig handler SIGBUS.",0);
/*  signal(SIGBUS,SIG_IGN);*/
  do_auto_shutdown();
  break;
 case SIGTERM:
  bug("Sig handler SIGTERM.",0);
/*  signal(SIGTERM,SIG_IGN);*/
  do_auto_shutdown();
  break;
 case SIGABRT:
  bug("Sig handler SIGABRT.",0);
/*  signal(SIGABRT,SIG_IGN);*/
  do_auto_shutdown();
  break;
 case SIGSEGV:
  bug("Sig handler SIGSEGV.",0);
/*  signal(SIGSEGV,SIG_IGN);*/
  do_auto_shutdown();
  break;
 }
}

void init_signals()
{
 signal(SIGBUS,sig_handler);
 signal(SIGTERM,sig_handler);
 signal(SIGABRT,sig_handler);
 signal(SIGSEGV,sig_handler);
}

void add_socket(char *socket,char *name)
{
 SOCKET_DATA *pSocket=NULL;
 SOCKET_CHAR_DATA *pChar=NULL;
 int sockets=0,in_socket=0;
 if((socket==NULL) || (name==NULL))
  return;
 if(socket_first==NULL)
 {
  socket_first = alloc_perm(sizeof(*socket_first));
  socket_first->socket_char_first = 
     alloc_perm(sizeof(*socket_first->socket_char_first));
  socket_first->next=NULL;
  socket_first->socket_char_first->next=NULL;
  socket_first->socket=str_dup(socket);
  socket_first->socket_char_first->character=str_dup(capitalize(name));
 }
 else
 {
  for(pSocket=socket_first;pSocket;pSocket=pSocket->next)
   if(!strcmp(pSocket->socket,socket))
    break;
   else
    sockets++;
  if(pSocket==NULL)
  {
   pSocket = alloc_perm(sizeof(*pSocket));
   pSocket->next = socket_first;
   pSocket->socket_char_first =
     alloc_perm(sizeof(*pSocket->socket_char_first));
   pSocket->socket_char_first->next=NULL;
   pSocket->socket=str_dup(socket);
   pSocket->socket_char_first->character=str_dup(capitalize(name));
   socket_first=pSocket;
   if(((sockets+1)>MAX_SOCKET)&&(((sockets+1)%20)==0))
    printf_log("add_socket: MAX_SOCKET reached");
  }
  else
  {
   for(pChar=pSocket->socket_char_first;pChar;pChar=pChar->next)
    if(!strcmp(pChar->character,capitalize(name)))
     break;
    else
     in_socket++;
   if(pChar==NULL)
   {
    pChar = alloc_perm(sizeof(*pChar));
    pChar->next=pSocket->socket_char_first;
    pChar->character=str_dup(capitalize(name));
    pSocket->socket_char_first=pChar;
    if(((in_socket+1)>MAX_IN_SOCKET)
       && (((in_socket+1)%20)==0))
     printf_log("add_socket: MAX_IN_SOCKET reached in socket %s",
        pSocket->socket);
   }
  }
 }
}

void save_sockets(void)
{
 FILE *fp;
 SOCKET_DATA *pSocket;
 SOCKET_CHAR_DATA *pChar;
 
 fclose(fpReserve);
 if((fp=fopen(SOCKET_FILE,"w"))==NULL)
 {
  bug("save_sockets: couldn't open SOCKET_FILE",0);
  fpReserve=fopen(NULL_FILE,"r");
  return;
 }
 for(pSocket=socket_first;pSocket;pSocket=pSocket->next)
 {
  fprintf(fp,"So %s~\n",pSocket->socket);
  for(pChar=pSocket->socket_char_first;pChar;pChar=pChar->next)
  {
   fprintf(fp,"%s ",pChar->character);
  }
  fprintf(fp,"\n");
 }
 fprintf(fp,"End\n\n");
 fprintf(fp,"%d\n",max_on_ever); 
 fclose(fp);
 fpReserve=fopen(NULL_FILE,"r");
 return;
}

void load_sockets(void)
{
 FILE *fp,*pfile;
 char *socket;
 char *word;
 char c,buf[MAX_STRING_LENGTH];

 fclose(fpReserve);
 if((fp=fopen(SOCKET_FILE,"r"))==NULL)
 {
  bug("load_sockets: couldn't open SOCKET_FILE",0);
  fpReserve=fopen(NULL_FILE,"r");
  return;
 }
 word=str_dup(fread_word(fp));
 while(strcmp(word,"End"))
 { 
  if(strcmp(word,"So"))
  {
   bug("load_sockets: corrupted SOCKET_FILE",0);
   fclose(fp);
   fpReserve=fopen(NULL_FILE,"r");
   return;
  }
  socket=fread_string(fp);
  word=str_dup(fread_word(fp));
  while(strcmp(word,"So") && strcmp(word,"End"))
  {
   sprintf(buf,"%s%s",PLAYER_DIR,capitalize(word));
   if((pfile=fopen(buf,"r"))!=NULL)
   {
    add_socket(socket,word);
    fclose(pfile);
   }
   word=str_dup(fread_word(fp));
  }
 }
 while(isspace(c=fgetc(fp)) && !feof(fp));
 if(!feof(fp))
 {
  ungetc(c,fp);
  max_on_ever=fread_number(fp);
 }
 fclose(fp);
 fpReserve=fopen(NULL_FILE,"r");
 return;
}

void add_allow(char *name,char *mask,int type)
{
 ALLOW_DATA *pAllow;
 if(allow_list==NULL)
 {
  allow_list = alloc_perm(sizeof(*allow_list));
  allow_list->next=NULL;
  allow_list->name=str_dup(capitalize(name));
  allow_list->mask=str_dup(mask);
  allow_list->type=type;
 }
 else
 {
  pAllow = alloc_perm(sizeof(*pAllow));
  pAllow->next=allow_list; 
  allow_list=pAllow; 
  pAllow->name=str_dup(capitalize(name));
  pAllow->mask=str_dup(mask);
  pAllow->type=type;
 }
 return;
}

bool is_allowed(char *name,char *socket)
{
 ALLOW_DATA *pAllow;
 
 for(pAllow=allow_list;pAllow;pAllow=pAllow->next)
  if(!strcmp(capitalize(name),pAllow->name))
  { 
   switch(pAllow->type)
   {
    default: return FALSE;
    case ALLOW_ALL: return TRUE;
    case ALLOW_EXACT: if(!strcmp(socket,pAllow->mask))
                       return TRUE;
                      break;
    case ALLOW_PREFIX: if(!str_prefix(pAllow->mask,socket))
                        return TRUE;
                      break;
    case ALLOW_SUFFIX: if(!str_suffix(pAllow->mask,socket))
                        return TRUE;
		      break;	
  }
 }
 return FALSE;
}

void save_allows(void)
{
 FILE *fp;
 ALLOW_DATA *pAllow;
 fclose(fpReserve);
 if((fp=fopen(ALLOW_FILE,"w"))==NULL)
 {
  bug("save_allows: couldn't open ALLOW_FILE",0);
  fpReserve=fopen(NULL_FILE,"r");
  return;
 }
 for(pAllow=allow_list;pAllow;pAllow=pAllow->next)
  fprintf(fp,"%s %s~ %d\n",pAllow->name,pAllow->mask,pAllow->type);
 fprintf(fp,"End\n\n");
 fclose(fp);
 fpReserve=fopen(NULL_FILE,"r");
 return;
}

void load_allows(void)
{
 FILE *fp;
 char *word,*str;
 int type;

 fclose(fpReserve);
 if((fp=fopen(ALLOW_FILE,"r"))==NULL)
 {
  bug("load_allows: couldn't open ALLOW_FILE",0);
  fpReserve=fopen(NULL_FILE,"r");
  return;
 }
 word=str_dup(fread_word(fp));
 while(strcmp(word,"End") && !feof(fp))
 {
  str=fread_string(fp); /*fprintf(stderr,"-->str: %s\n",str);*/
  type=fread_number(fp); /*fprintf(stderr,"-->type: %d\n",type);*/
  add_allow(word,str,type);
  word=str_dup(fread_word(fp));
  /*fprintf(stderr,"-->word: %s\n",word);*/
 }
 fclose(fp);
 fpReserve=fopen(NULL_FILE,"r");
 return;
}

void save_pkills(void)
{
 FILE *fp;
 PKILL_DATA *pKill;
 char buf[MAX_PKILL][256];
 int count,i
;
 fclose(fpReserve);
 if((fp=fopen(PKILL_FILE,"w"))==NULL)
 {
  bug("save_pkills: couldn't open PKILL_FILE",0);
  fpReserve=fopen(NULL_FILE,"r");
  return;
 }
 for(pKill=pkill_list,count=0;pKill;pKill=pKill->next,count++)
   sprintf(buf[count],"%s %s %d %d %d\n",pKill->killer,pKill->victim,pKill->gs,
          pKill->pkp,pKill->killers);
 for(i=count-1;i>=0;i--)
 {
   fprintf(fp,buf[i]);
 }
 fprintf(fp,"End\n\n");
 fclose(fp);
 fpReserve=fopen(NULL_FILE,"r");
 return;
}

void load_pkills(void)
{
 FILE *fp;
 char *killer,*victim;
 int gs,pkp,killers;

 fclose(fpReserve);
 if((fp=fopen(PKILL_FILE,"r"))==NULL)
 {
  bug("load_pkills: couldn't open PKILL_FILE",0);
  fpReserve=fopen(NULL_FILE,"r");
  return;
 }
 killer=str_dup(fread_word(fp));
 while(strcmp(killer,"End") && !feof(fp))
 {
  victim=str_dup(fread_word(fp));
  gs=fread_number(fp);
  pkp=fread_number(fp);
  killers=fread_number(fp);
  register_pk(killer,victim,gs,pkp,killers);
  killer=str_dup(fread_word(fp));
 }
 fclose(fp);
 fpReserve=fopen(NULL_FILE,"r");
 return;
}

void printf_to_char(CHAR_DATA *ch, char *fmt, ...)
{
	char buf[MAX_STRING_LENGTH];
	va_list args;
	va_start (args, fmt);
	vsnprintf(buf, MAX_STRING_LENGTH, fmt, args);
	va_end(args);
	send_to_char(buf,ch);
}

void save_ranks(void)
{
 FILE *fp;
 RANK_DATA *pRank;
 fclose(fpReserve);
 if((fp=fopen(RANK_FILE,"w"))==NULL)
 {
  bug("save_ranks: couldn't open RANK_FILE",0);
  fpReserve=fopen(NULL_FILE,"r");
  return;
 }
 for(pRank=rank_list;pRank;pRank=pRank->next)
  fprintf(fp,"%s %d %d %d %d\n",pRank->name,pRank->pkp,pRank->race,pRank->class,
 		pRank->clan);
 fprintf(fp,"End\n\n");
 fclose(fp);
 fpReserve=fopen(NULL_FILE,"r");
 return;
}

void load_ranks(void)
{
 FILE *fp,*pfile;
 char *word;
 int pkp,race,class,clan;
 char buf[MAX_STRING_LENGTH];

 fclose(fpReserve);
 if((fp=fopen(RANK_FILE,"r"))==NULL)
 {
  bug("load_ranks: couldn't open RANK_FILE",0);
  fpReserve=fopen(NULL_FILE,"r");
  return;
 }
 word=str_dup(fread_word(fp));
 while(strcmp(word,"End") && !feof(fp))
 {
  pkp=fread_number(fp);
  race=fread_number(fp);
  class=fread_number(fp);
  clan=fread_number(fp);
  sprintf(buf,"%s%s",PLAYER_DIR,capitalize(word));
  if((pfile=fopen(buf,"r"))!=NULL)
  {
   register_rank(word,pkp,race,class,clan);
   fclose(pfile);
  }
  word=str_dup(fread_word(fp));
 }
 fclose(fp);
 fpReserve=fopen(NULL_FILE,"r");
 return;
}
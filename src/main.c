#define _GNU_SOURCE


#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include "main.h"
#include "conf.h"
#include "log.h"
#include "kad.h"
#include "utils.h"
#include "unix.h"
#include "net.h"

#ifdef DNS
#include "ext-dns.h"
#endif
#ifdef WEB
#include "ext-web.h"
#endif
#ifdef NSS
#include "ext-nss.h"
#endif
#ifdef CMD
#include "ext-cmd.h"
#endif


int main( int argc, char **argv ) {

	conf_init();
	conf_load( argc, argv );
	conf_check();

    if( gstate->is_daemon == 1 ) {

		/* Close pipes */
		fclose( stderr );
		fclose( stdout );
		fclose( stdin );

		if( chdir( "/" ) != 0 ) {
			log_err( "UNX: Changing working directory to / failed: %s", strerror( errno ) );
		}

		/* Fork before any threads are started */
		unix_fork();
	}

	/* Catch SIG INT */
	unix_signal();

	/* Write a pid file */
	unix_write_pidfile( getpid() );

	/* Drop privileges */
	unix_dropuid0();

	/* Setup the Kademlia DHT */
	kad_setup();

	/* Setup interfaces */
#ifdef DNS
	dns_setup();
#endif
#ifdef WEB
	web_setup();
#endif
#ifdef NSS
	nss_setup();
#endif
#ifdef CMD
	cmd_setup();
#endif

	/* Loop over all sockets and FDs */
	net_loop();

	conf_free();

	return 0;
}

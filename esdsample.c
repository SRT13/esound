
#include "esd.h"

#include <stdlib.h>
#include <dirent.h>
#include <signal.h>
#include <sys/stat.h>

volatile int terminate = 0;

void clean_exit(int signum) {
    terminate = 1;
    return;
}

int main(int argc, char **argv)
{
    char buf[ESD_BUF_SIZE];
    int sock = -1, rate = 44100;
    int arg = 0, length = 0, total = 0;

    int bits = ESD_BITS16, channels = ESD_STEREO;
    int mode = ESD_STREAM, func = ESD_PLAY ;
    esd_format_t format = 0;

    int sample_id = 0, confirm_id = 0;
    FILE *source = NULL;
    struct stat source_stats;
    char *host = NULL;
    char *name = NULL;
    
    for ( arg = 1 ; arg < argc ; arg++)
    {
	if (!strcmp("-h",argv[arg]))
	{
	    printf("usage:\n\t%s [-s server] [-b] [-m] [-r freq] < file\n",
		   argv[0]);
	    exit(0);
	}
	else if ( !strcmp( "-s", argv[ arg ] ) )
	    host = argv[ ++arg ];
	else if ( !strcmp( "-b", argv[ arg ] ) )
	    bits = ESD_BITS8;
	else if ( !strcmp( "-m", argv[ arg ] ) )
	    channels = ESD_MONO;
	else if ( !strcmp( "-r", argv[ arg ] ) )
	{
	    arg++;
	    rate = atoi( argv[ arg ] );
	} else {
	    name = argv[ arg ];
	    source = fopen( name, "r" );
	}
    }

    if ( source == NULL ) {
	fprintf( stderr, "%s, sample file not specified\n", argv[ 0 ] );
	return -1;
    }
    
    format = bits | channels | mode | func;
    printf( "opening socket, format = 0x%08x at %d Hz\n", 
	    format, rate );
   
    sock = esd_open_sound( host );
    if ( sock <= 0 ) 
	return 1;

    stat( name, &source_stats );
    sample_id = esd_sample_cache( sock, format, rate, source_stats.st_size, name );
    printf( "sample id is <%d>\n", sample_id );

    /* if we see any of these, terminate */
    signal( SIGINT, clean_exit );
    signal( SIGKILL, clean_exit );
    signal( SIGPIPE, clean_exit );

    while ( ( length = fread( buf, 1, ESD_BUF_SIZE, source ) ) > 0 )
    {
	/* fprintf( stderr, "read %d\n", length ); */
	if ( ( length = write( sock, buf, length)  ) <= 0 )
	    return 1;
	else
	    total += length;
    }

    confirm_id = esd_confirm_sample_cache( sock );
    if ( sample_id != confirm_id ) {
	printf( "error while caching sample <%d>: confirm returned %d\n",
		sample_id, confirm_id );
	exit( 1 );
    }
    printf( "sample <%d> uploaded, %d bytes\n", sample_id, total );

    printf( "press \'q\' <enter> to quit, <enter> to trigger.\n" );
    while ( !terminate ) {
	if ( getchar() == 'q' ) break;
	printf( "<playing sample>\n" );
	esd_sample_play( sock, sample_id );
    }

    esd_sample_free( sock, sample_id );

    printf( "closing down\n" );
    close( sock );

    return 0;
}

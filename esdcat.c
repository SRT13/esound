
#include "esd.h"

#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

int main(int argc, char **argv)
{
    char buf[ESD_BUF_SIZE];
    int sock = -1, rate = 44100;
    int length = 0, arg = 0;

    int bits = ESD_BITS16, channels = ESD_STEREO;
    int mode = ESD_STREAM, func = ESD_PLAY ;
    esd_format_t format = 0;

    FILE *source = stdin;
    
    for ( arg = 1 ; arg < argc ; arg++)
    {
	if (!strcmp("-h",argv[arg]))
	{
	    printf("usage:\n\t%s [-b] [-m] [-r freq] < file\n",argv[0]);
	    exit(0);
	}
	else if ( !strcmp( "-b", argv[ arg ] ) )
	    bits = ESD_BITS8;
	else if ( !strcmp( "-m", argv[ arg ] ) )
	    channels = ESD_MONO;
	else if ( !strcmp( "-r", argv[ arg ] ) )
	{
	    arg++;
	    rate = atoi( argv[ arg ] );
	} else {
	    source = fopen( argv[arg], "r" );
	}
    }
    
    format = bits | channels | mode | func;
    printf( "opening socket, format = 0x%08x at %d Hz\n", 
	    format, rate );
   
    /* sock = esd_play_stream( format, rate ); */
    sock = esd_play_stream_fallback( format, rate );
    if ( sock <= 0 ) 
	return 1;
    
    while ( ( length = fread( buf, 1, ESD_BUF_SIZE, source ) ) > 0 )
    {
	/* fprintf( stderr, "read %d\n", length ); */
	if ( write( sock, buf, length ) <= 0 ) 
	    return 1;
    }
    close( sock );

    return 0;
}

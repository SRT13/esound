
#include "esd.h"

#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

int main(int argc, char **argv)
{
    char buf[ESD_BUF_SIZE];
    int sock = -1, rate = 44100;
    int arg = 0, length = 0, total = 0;

    int bits = ESD_BITS16, channels = ESD_STEREO;
    int mode = ESD_STREAM, func = ESD_PLAY ;
    esd_format_t format = 0;

    int sample_id = 0;
    FILE *source = NULL;
    char *source_name = NULL;
    struct stat source_stats;
    
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
	    source_name = argv[ arg ];
	}
    }

    if ( source == NULL ) {
	fprintf( stderr, "%s, sample file not specified\n", argv[ 0 ] );
	return -1;
    }
    
    format = bits | channels | mode | func;
    printf( "opening socket, format = 0x%08x at %d Hz\n", 
	    format, rate );
   
    sock = esd_open_sound( format, rate );
    if ( sock <= 0 ) 
	return 1;

    stat( source_name, &source_stats );
    sample_id = esd_sample_cache( sock, format, rate, source_stats.st_size );
    printf( "sample id is <%d>\n", sample_id );

    while ( ( length = fread( buf, 1, ESD_BUF_SIZE, source ) ) > 0 )
    {
	/* fprintf( stderr, "read %d\n", length ); */
	if ( ( length = write( sock, buf, length)  ) <= 0 )
	    return 1;
	else
	    total += length;
    }

    printf( "sample uploaded, %d bytes\n", total );
    esd_sample_play( sock, sample_id );
    esd_sample_free( sock, sample_id );

    printf( "closing down\n" );
    close( sock );

    return 0;
}

/* -*- C++ -*- */

#include "esd.h"

#include <limits.h>

/*******************************************************************/
signed int mixed_buffer[ ESD_BUF_SIZE ];

/*******************************************************************/
/* takes the 16 bit signed source waveform, and mixes to player */
int mix_from_stereo_16s( signed short *source_data_ss, 
		       esd_player_t *player, int length )
{
    int wr_dat = 0, rd_dat = 0;
    register unsigned char *target_data_uc = NULL;
    register signed short *target_data_ss = NULL;
    signed short lsample, rsample;

    /* if nothing to mix, just bail */
    if ( !length ) {
	return 0;
    }

    /* mix it down */
    switch ( player->format & ESD_MASK_BITS )
    {
    case ESD_BITS8:
	target_data_uc = (unsigned char *) player->data_buffer;

	if ( ( player->format & ESD_MASK_CHAN ) == ESD_MONO ) {

	    /* mix mono, 8 bit sound source to stereo, 16 bit */
	    while ( wr_dat < player->buffer_length )
	    {
		rd_dat = wr_dat * 44100 / player->rate;
		rd_dat *= 2;		/* adjust for mono */
		rd_dat += rd_dat % 2;	/* force to left sample */

		lsample = source_data_ss[ rd_dat++ ];
		rsample = source_data_ss[ rd_dat++ ];

		lsample /= 256; lsample += 127;
		rsample /= 256; rsample += 127;

		target_data_uc[ wr_dat++ ] = (lsample + rsample) / 2;
	    }

	} else {

	    /* mix stereo, 8 bit sound source to stereo, 16 bit */
	    while ( wr_dat < player->buffer_length )
	    {
		rd_dat = wr_dat * 44100 / player->rate;

		lsample = source_data_ss[ rd_dat++ ];
		lsample /= 256; lsample += 127;
		rsample = source_data_ss[ rd_dat++ ];
		rsample /= 256; rsample += 127;

		target_data_uc[ wr_dat++ ] = lsample;
		target_data_uc[ wr_dat++ ] = rsample;
	    }
	}

	break;

    case ESD_BITS16:
	target_data_ss = (signed short *) player->data_buffer;

	if ( ( player->format & ESD_MASK_CHAN ) == ESD_MONO ) {

	    /* mix mono, 16 bit sound source from stereo, 16 bit */
	    while ( wr_dat < player->buffer_length / sizeof(signed short) )
	    {
		rd_dat = wr_dat * 44100 / player->rate;
		rd_dat *= 2;		/* adjust for stereo */
		rd_dat += rd_dat % 2;	/* force to left sample */

		lsample = source_data_ss[ rd_dat++ ];
		rsample = source_data_ss[ rd_dat++ ];

		target_data_ss[ wr_dat++ ] = (lsample + rsample) / 2;
	    }

	} else {

	    /* mix stereo, 16 bit sound source from stereo, 16 bit */
	    while ( wr_dat < player->buffer_length / sizeof(signed short) )
	    {
		rd_dat = wr_dat * 44100 / player->rate;
		/* rd_dat += (rd_dat % 2); */ /* force to left sample */

		lsample = source_data_ss[ rd_dat++ ];
		rsample = source_data_ss[ rd_dat++ ];

		target_data_ss[ wr_dat++ ] = lsample;
		target_data_ss[ wr_dat++ ] = rsample;
	    }
	}

	break;

    default:
	printf( "mix from: format 0x%08x not supported (%d)\n", 
		player->format & ESD_MASK_BITS, player->source_id );
	break;
    }

    return wr_dat * sizeof(signed short);
}

/*******************************************************************/
/* takes the input player, and mixes to 16 bit signed waveform */
int mix_to_stereo_32s( esd_player_t *player, int length )
{
    int wr_dat = 0, rd_dat = 0;
    register unsigned char *source_data_uc = NULL;
    register signed short *source_data_ss = NULL;
    signed short sample;

    switch ( player->format & ESD_MASK_BITS )
    {
    case ESD_BITS8:
	source_data_uc = (unsigned char *) player->data_buffer;

	if ( ( player->format & ESD_MASK_CHAN ) == ESD_MONO ) {

	    /* mix mono, 8 bit sound source to stereo, 16 bit */
	    while ( wr_dat < length/sizeof(signed short) )
	    {
		rd_dat = wr_dat * player->rate / 44100;
		rd_dat /= 2;	/* adjust for mono */

		sample = source_data_uc[ rd_dat++ ];
		sample -= 127; sample *= 256;

		mixed_buffer[ wr_dat++ ] += sample;
		mixed_buffer[ wr_dat++ ] += sample;
	    }

	} else {

	    /* mix stereo, 8 bit sound source to stereo, 16 bit */
	    if ( player->rate == 44100 ) {
		while ( wr_dat < length/sizeof(signed short) )
		{
		    sample = ( source_data_uc[ wr_dat ] - 127 ) * 256;
		    mixed_buffer[ wr_dat ] += sample;
		    wr_dat++;
		}
	    } else {
		while ( wr_dat < length/sizeof(signed short) )
		{
		    rd_dat = wr_dat * player->rate / 44100;
		    
		    sample = source_data_uc[ rd_dat++ ];
		    sample -= 127; sample *= 256;
		    
		    mixed_buffer[ wr_dat++ ] += sample;
		}
	    }
	}

	break;
    case ESD_BITS16:
	source_data_ss = (signed short *) player->data_buffer;

	/* rough sketch, based on 8 bit stuff */
	if ( ( player->format & ESD_MASK_CHAN ) == ESD_MONO ) {

	    /* mix mono, 16 bit sound source to stereo, 16 bit */
	    while ( wr_dat < length/sizeof(signed short) )
	    {
		rd_dat = wr_dat * player->rate / 44100;
		rd_dat /= 2;	/* adjust for mono */

		sample = source_data_ss[ rd_dat++ ];

		mixed_buffer[ wr_dat++ ] += sample;
		mixed_buffer[ wr_dat++ ] += sample;
	    }

	} else {

	    /* mix stereo, 16 bit sound source to stereo, 16 bit */
	    if ( player->rate == 44100 ) {
		while ( wr_dat < length/sizeof(signed short) )
		{
		    mixed_buffer[ wr_dat ] += source_data_ss[ wr_dat ];
		    wr_dat++;
		}
	    } else {
		while ( wr_dat < length/sizeof(signed short) )
		{
		    rd_dat = wr_dat * player->rate / 44100;
		    sample = source_data_ss[ rd_dat++ ];
		    mixed_buffer[ wr_dat++ ] += sample;
		}
	    }
	}
	break;

    default:
	printf( "mix_to: format 0x%08x not supported (%d)\n", 
		player->format & ESD_MASK_BITS, player->source_id );
	break;
    }

    return wr_dat * sizeof(signed short);
}

/*******************************************************************/
/* takes mixed data, and clips data to the output buffer */
void clip_mix_to_output_16s( signed short *output, int length )
{
    signed int *mixed = mixed_buffer;
    signed int *end = mixed_buffer + length/sizeof(signed short);

    while ( mixed != end ) {
	if (*mixed < SHRT_MIN) {
	    *output++ = SHRT_MIN; mixed++;
	} else if (*mixed > SHRT_MAX) {
	    *output++ = SHRT_MAX; mixed++;
	} else {
	    *output++ = *mixed++;
	}
    }
}

/*******************************************************************/
/* takes all input players, and mixes them to the mixed_buffer */
int mix_players_16s( void *output, int length )
{
    int actual = 0, max = 0;
    esd_player_t *iterator = NULL;
    esd_player_t *erase = NULL;

    /* zero the sum buffer */
    memset( mixed_buffer, 0, length * sizeof(int) );
    
    /* as long as there's a player out there */
    iterator = esd_players_list;
    while( iterator != NULL )
    {
	/* read the client sound data */
	actual = read_player( iterator );

	/* read_player(): >0 = data, ==0 = no data, <0 = erase it */
	if ( actual > 0  ) {
	    /* printf( "received: %d bytes from %d\n", 
	            actual, iterator->source_id ); */
	    actual = mix_to_stereo_32s( iterator, length );
	    if ( actual > max ) max = actual;
	    
	} else if ( actual == 0 ) {
	    /* printf( "no data available from player (%d)\n", 
		     iterator->source_id, iterator ); */
	} else {
	    /* actual < 0 means erase the player */
	    erase = iterator;
	}

	/* check out the next item in the list */
	iterator = iterator->next;

	/* clean up any fished players */
	if ( erase != NULL ) {
	    erase_player( erase );
	    erase = NULL;
	}
    }

    clip_mix_to_output_16s( output, max );
    return max;
}

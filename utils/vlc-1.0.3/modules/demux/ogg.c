/*****************************************************************************
 * ogg.c : ogg stream demux module for vlc
 *****************************************************************************
 * Copyright (C) 2001-2007 the VideoLAN team
 * $Id: 2b62be732d43aaeb9a975e2576082de68156e5bc $
 *
 * Authors: Gildas Bazin <gbazin@netcourrier.com>
 *          Andre Pang <Andre.Pang@csiro.au> (Annodex support)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

/*****************************************************************************
 * Preamble
 *****************************************************************************/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_demux.h>
#include <vlc_meta.h>
#include <vlc_input.h>

#include <ogg/ogg.h>

#include <vlc_codecs.h>
#include <vlc_bits.h>
#include <vlc_charset.h>
#include "vorbis.h"
#include "kate_categories.h"

/*****************************************************************************
 * Module descriptor
 *****************************************************************************/
static int  Open ( vlc_object_t * );
static void Close( vlc_object_t * );

vlc_module_begin ()
    set_shortname ( "OGG" )
    set_description( N_("OGG demuxer" ) )
    set_category( CAT_INPUT )
    set_subcategory( SUBCAT_INPUT_DEMUX )
    set_capability( "demux", 50 )
    set_callbacks( Open, Close )
    add_shortcut( "ogg" )
vlc_module_end ()


/*****************************************************************************
 * Definitions of structures and functions used by this plugins
 *****************************************************************************/
typedef struct logical_stream_s
{
    ogg_stream_state os;                        /* logical stream of packets */

    es_format_t      fmt;
    es_format_t      fmt_old;                  /* format of old ES is reused */
    es_out_id_t      *p_es;
    double           f_rate;

    int              i_serial_no;

    /* the header of some logical streams (eg vorbis) contain essential
     * data for the decoder. We back them up here in case we need to re-feed
     * them to the decoder. */
    int              b_force_backup;
    int              i_packets_backup;
    uint8_t          *p_headers;
    int              i_headers;

    /* program clock reference (in units of 90kHz) derived from the previous
     * granulepos */
    mtime_t          i_pcr;
    mtime_t          i_interpolated_pcr;
    mtime_t          i_previous_pcr;

    /* Misc */
    bool b_reinit;
    int i_granule_shift;

    /* kate streams have the number of headers in the ID header */
    int i_kate_num_headers;

    /* for Annodex logical bitstreams */
    int i_secondary_header_packets;

} logical_stream_t;

struct demux_sys_t
{
    ogg_sync_state oy;        /* sync and verify incoming physical bitstream */

    int i_streams;                           /* number of logical bitstreams */
    logical_stream_t **pp_stream;  /* pointer to an array of logical streams */

    logical_stream_t *p_old_stream; /* pointer to a old logical stream to avoid recreating it */

    /* program clock reference (in units of 90kHz) derived from the pcr of
     * the sub-streams */
    mtime_t i_pcr;

    /* stream state */
    int     i_bos;
    int     i_eos;

    /* bitrate */
    int     i_bitrate;

    /* after reading all headers, the first data page is stuffed into the relevant stream, ready to use */
    bool    b_page_waiting;

    /* */
    vlc_meta_t *p_meta;
};

/* OggDS headers for the new header format (used in ogm files) */
typedef struct
{
    ogg_int32_t width;
    ogg_int32_t height;
} stream_header_video_t;

typedef struct
{
    ogg_int16_t channels;
    ogg_int16_t padding;
    ogg_int16_t blockalign;
    ogg_int32_t avgbytespersec;
} stream_header_audio_t;

typedef struct
{
    char        streamtype[8];
    char        subtype[4];

    ogg_int32_t size;                               /* size of the structure */

    ogg_int64_t time_unit;                              /* in reference time */
    ogg_int64_t samples_per_unit;
    ogg_int32_t default_len;                                /* in media time */

    ogg_int32_t buffersize;
    ogg_int16_t bits_per_sample;
    ogg_int16_t padding;

    union
    {
        /* Video specific */
        stream_header_video_t video;
        /* Audio specific */
        stream_header_audio_t audio;
    } sh;
} stream_header_t;

#define OGG_BLOCK_SIZE 4096

/* Some defines from OggDS */
#define PACKET_TYPE_HEADER   0x01
#define PACKET_TYPE_BITS     0x07
#define PACKET_LEN_BITS01    0xc0
#define PACKET_LEN_BITS2     0x02
#define PACKET_IS_SYNCPOINT  0x08

/*****************************************************************************
 * Local prototypes
 *****************************************************************************/
static int  Demux  ( demux_t * );
static int  Control( demux_t *, int, va_list );

/* Bitstream manipulation */
static int  Ogg_ReadPage     ( demux_t *, ogg_page * );
static void Ogg_UpdatePCR    ( logical_stream_t *, ogg_packet * );
static void Ogg_DecodePacket ( demux_t *, logical_stream_t *, ogg_packet * );

static int Ogg_BeginningOfStream( demux_t *p_demux );
static int Ogg_FindLogicalStreams( demux_t *p_demux );
static void Ogg_EndOfStream( demux_t *p_demux );

/* */
static void Ogg_LogicalStreamDelete( demux_t *p_demux, logical_stream_t *p_stream );
static bool Ogg_LogicalStreamResetEsFormat( demux_t *p_demux, logical_stream_t *p_stream );

/* */
static void Ogg_ExtractMeta( demux_t *p_demux, vlc_fourcc_t i_codec, const uint8_t *p_headers, int i_headers );

/* Logical bitstream headers */
static void Ogg_ReadTheoraHeader( logical_stream_t *, ogg_packet * );
static void Ogg_ReadVorbisHeader( logical_stream_t *, ogg_packet * );
static void Ogg_ReadSpeexHeader( logical_stream_t *, ogg_packet * );
static void Ogg_ReadKateHeader( logical_stream_t *, ogg_packet * );
static void Ogg_ReadFlacHeader( demux_t *, logical_stream_t *, ogg_packet * );
static void Ogg_ReadAnnodexHeader( vlc_object_t *, logical_stream_t *, ogg_packet * );
static bool Ogg_ReadDiracHeader( logical_stream_t *, ogg_packet * );

/*****************************************************************************
 * Open: initializes ogg demux structures
 *****************************************************************************/
static int Open( vlc_object_t * p_this )
{
    demux_t *p_demux = (demux_t *)p_this;
    demux_sys_t    *p_sys;
    const uint8_t  *p_peek;


    /* Check if we are dealing with an ogg stream */
    if( stream_Peek( p_demux->s, &p_peek, 4 ) < 4 ) return VLC_EGENERIC;
    if( !p_demux->b_force && memcmp( p_peek, "OggS", 4 ) )
    {
        return VLC_EGENERIC;
    }

    /* Set exported functions */
    p_demux->pf_demux = Demux;
    p_demux->pf_control = Control;
    p_demux->p_sys = p_sys = malloc( sizeof( demux_sys_t ) );
    if( !p_sys )
        return VLC_ENOMEM;

    memset( p_sys, 0, sizeof( demux_sys_t ) );
    p_sys->i_bitrate = 0;
    p_sys->pp_stream = NULL;
    p_sys->p_old_stream = NULL;

    /* Begnning of stream, tell the demux to look for elementary streams. */
    p_sys->i_bos = 0;
    p_sys->i_eos = 0;

    /* Initialize the Ogg physical bitstream parser */
    ogg_sync_init( &p_sys->oy );
    p_sys->b_page_waiting = false;

    /* */
    p_sys->p_meta = NULL;

    return VLC_SUCCESS;
}

/*****************************************************************************
 * Close: frees unused data
 *****************************************************************************/
static void Close( vlc_object_t *p_this )
{
    demux_t *p_demux = (demux_t *)p_this;
    demux_sys_t *p_sys = p_demux->p_sys  ;

    /* Cleanup the bitstream parser */
    ogg_sync_clear( &p_sys->oy );

    Ogg_EndOfStream( p_demux );

    if( p_sys->p_old_stream )
        Ogg_LogicalStreamDelete( p_demux, p_sys->p_old_stream );

    free( p_sys );
}

/*****************************************************************************
 * Demux: reads and demuxes data packets
 *****************************************************************************
 * Returns -1 in case of error, 0 in case of EOF, 1 otherwise
 *****************************************************************************/
static int Demux( demux_t * p_demux )
{
    demux_sys_t *p_sys = p_demux->p_sys;
    ogg_page    oggpage;
    ogg_packet  oggpacket;
    int         i_stream;


    if( p_sys->i_eos == p_sys->i_streams )
    {
        if( p_sys->i_eos )
        {
            msg_Dbg( p_demux, "end of a group of logical streams" );
            /* We keep the ES to try reusing it in Ogg_BeginningOfStream
             * only 1 ES is supported (common case for ogg web radio) */
            if( p_sys->i_streams == 1 )
            {
                p_sys->p_old_stream = p_sys->pp_stream[0];
                TAB_CLEAN( p_sys->i_streams, p_sys->pp_stream );
            }
            Ogg_EndOfStream( p_demux );
        }

        p_sys->i_eos = 0;
        if( Ogg_BeginningOfStream( p_demux ) != VLC_SUCCESS )
            return 0;

        msg_Dbg( p_demux, "beginning of a group of logical streams" );
        es_out_Control( p_demux->out, ES_OUT_RESET_PCR );
    }

    /*
     * The first data page of a physical stream is stored in the relevant logical stream
     * in Ogg_FindLogicalStreams. Therefore, we must not read a page and only update the
     * stream it belongs to if we haven't processed this first page yet. If we do, we
     * will only process that first page whenever we find the second page for this stream.
     * While this is fine for Vorbis and Theora, which are continuous codecs, which means
     * the second page will arrive real quick, this is not fine for Kate, whose second
     * data page will typically arrive much later.
     * This means it is now possible to seek right at the start of a stream where the last
     * logical stream is Kate, without having to wait for the second data page to unblock
     * the first one, which is the one that triggers the 'no more headers to backup' code.
     * And, as we all know, seeking without having backed up all headers is bad, since the
     * codec will fail to initialize if it's missing its headers.
     */
    if( !p_sys->b_page_waiting)
    {
        /*
         * Demux an ogg page from the stream
         */
        if( Ogg_ReadPage( p_demux, &oggpage ) != VLC_SUCCESS )
            return 0; /* EOF */

        /* Test for End of Stream */
        if( ogg_page_eos( &oggpage ) )
            p_sys->i_eos++;
    }


    for( i_stream = 0; i_stream < p_sys->i_streams; i_stream++ )
    {
        logical_stream_t *p_stream = p_sys->pp_stream[i_stream];

        /* if we've just pulled page, look for the right logical stream */
        if( !p_sys->b_page_waiting )
        {
            if( p_sys->i_streams == 1 &&
                ogg_page_serialno( &oggpage ) != p_stream->os.serialno )
            {
                msg_Err( p_demux, "Broken Ogg stream (serialno) mismatch" );
                ogg_stream_reset_serialno( &p_stream->os, ogg_page_serialno( &oggpage ) );

                p_stream->b_reinit = true;
                p_stream->i_pcr = -1;
                p_stream->i_interpolated_pcr = -1;
                es_out_Control( p_demux->out, ES_OUT_RESET_PCR );
            }

            if( ogg_stream_pagein( &p_stream->os, &oggpage ) != 0 )
                continue;
        }

        while( ogg_stream_packetout( &p_stream->os, &oggpacket ) > 0 )
        {
            /* Read info from any secondary header packets, if there are any */
            if( p_stream->i_secondary_header_packets > 0 )
            {
                if( p_stream->fmt.i_codec == VLC_FOURCC('t','h','e','o') &&
                        oggpacket.bytes >= 7 &&
                        ! memcmp( oggpacket.packet, "\x80theora", 7 ) )
                {
                    Ogg_ReadTheoraHeader( p_stream, &oggpacket );
                    p_stream->i_secondary_header_packets = 0;
                }
                else if( p_stream->fmt.i_codec == VLC_FOURCC('v','o','r','b') &&
                        oggpacket.bytes >= 7 &&
                        ! memcmp( oggpacket.packet, "\x01vorbis", 7 ) )
                {
                    Ogg_ReadVorbisHeader( p_stream, &oggpacket );
                    p_stream->i_secondary_header_packets = 0;
                }
                else if( p_stream->fmt.i_codec == VLC_FOURCC('c','m','m','l') )
                {
                    p_stream->i_secondary_header_packets = 0;
                }
            }

            if( p_stream->b_reinit )
            {
                /* If synchro is re-initialized we need to drop all the packets
                 * until we find a new dated one. */
                Ogg_UpdatePCR( p_stream, &oggpacket );

                if( p_stream->i_pcr >= 0 )
                {
                    p_stream->b_reinit = false;
                }
                else
                {
                    p_stream->i_interpolated_pcr = -1;
                    continue;
                }

                /* An Ogg/vorbis packet contains an end date granulepos */
                if( p_stream->fmt.i_codec == VLC_FOURCC( 'v','o','r','b' ) ||
                    p_stream->fmt.i_codec == VLC_FOURCC( 's','p','x',' ' ) ||
                    p_stream->fmt.i_codec == VLC_FOURCC( 'f','l','a','c' ) )
                {
                    if( ogg_stream_packetout( &p_stream->os, &oggpacket ) > 0 )
                    {
                        Ogg_DecodePacket( p_demux, p_stream, &oggpacket );
                    }
                    else
                    {
                        es_out_Control( p_demux->out, ES_OUT_SET_PCR,
                                        p_stream->i_pcr );
                    }
                    continue;
                }
            }

            Ogg_DecodePacket( p_demux, p_stream, &oggpacket );
        }

        if( !p_sys->b_page_waiting )
            break;
    }

    /* if a page was waiting, it's now processed */
    p_sys->b_page_waiting = false;

    p_sys->i_pcr = -1;
    for( i_stream = 0; i_stream < p_sys->i_streams; i_stream++ )
    {
        logical_stream_t *p_stream = p_sys->pp_stream[i_stream];

        if( p_stream->fmt.i_cat == SPU_ES )
            continue;
        if( p_stream->i_interpolated_pcr < 0 )
            continue;

        if( p_sys->i_pcr < 0 || p_stream->i_interpolated_pcr < p_sys->i_pcr )
            p_sys->i_pcr = p_stream->i_interpolated_pcr;
    }

    if( p_sys->i_pcr >= 0 )
        es_out_Control( p_demux->out, ES_OUT_SET_PCR, p_sys->i_pcr );

    return 1;
}

/*****************************************************************************
 * Control:
 *****************************************************************************/
static int Control( demux_t *p_demux, int i_query, va_list args )
{
    demux_sys_t *p_sys  = p_demux->p_sys;
    vlc_meta_t *p_meta;
    int64_t *pi64;
    bool *pb_bool;
    int i;

    switch( i_query )
    {
        case DEMUX_GET_META:
            p_meta = (vlc_meta_t *)va_arg( args, vlc_meta_t* );
            if( p_sys->p_meta )
                vlc_meta_Merge( p_meta, p_sys->p_meta );
            return VLC_SUCCESS;

        case DEMUX_HAS_UNSUPPORTED_META:
            pb_bool = (bool*)va_arg( args, bool* );
            *pb_bool = true;
            return VLC_SUCCESS;

        case DEMUX_GET_TIME:
            pi64 = (int64_t*)va_arg( args, int64_t * );
            *pi64 = p_sys->i_pcr;
            return VLC_SUCCESS;

        case DEMUX_SET_TIME:
            return VLC_EGENERIC;

        case DEMUX_SET_POSITION:
            /* forbid seeking if we haven't initialized all logical bitstreams yet;
               if we allowed, some headers would not get backed up and decoder init
               would fail, making that logical stream unusable */
            if( p_sys->i_bos > 0 )
            {
                return VLC_EGENERIC;
            }

            for( i = 0; i < p_sys->i_streams; i++ )
            {
                logical_stream_t *p_stream = p_sys->pp_stream[i];

                /* we'll trash all the data until we find the next pcr */
                p_stream->b_reinit = true;
                p_stream->i_pcr = -1;
                p_stream->i_interpolated_pcr = -1;
                ogg_stream_reset( &p_stream->os );
            }
            ogg_sync_reset( &p_sys->oy );
            /* XXX The break/return is missing on purpose as
             * demux_vaControlHelper will do the last part of the job */

        default:
            return demux_vaControlHelper( p_demux->s, 0, -1, p_sys->i_bitrate,
                                           1, i_query, args );
    }
}

/****************************************************************************
 * Ogg_ReadPage: Read a full Ogg page from the physical bitstream.
 ****************************************************************************
 * Returns VLC_SUCCESS if a page has been read. An error might happen if we
 * are at the end of stream.
 ****************************************************************************/
static int Ogg_ReadPage( demux_t *p_demux, ogg_page *p_oggpage )
{
    demux_sys_t *p_ogg = p_demux->p_sys  ;
    int i_read = 0;
    char *p_buffer;

    while( ogg_sync_pageout( &p_ogg->oy, p_oggpage ) != 1 )
    {
        p_buffer = ogg_sync_buffer( &p_ogg->oy, OGG_BLOCK_SIZE );

        i_read = stream_Read( p_demux->s, p_buffer, OGG_BLOCK_SIZE );
        if( i_read <= 0 )
            return VLC_EGENERIC;

        ogg_sync_wrote( &p_ogg->oy, i_read );
    }

    return VLC_SUCCESS;
}

/****************************************************************************
 * Ogg_UpdatePCR: update the PCR (90kHz program clock reference) for the
 *                current stream.
 ****************************************************************************/
static void Ogg_UpdatePCR( logical_stream_t *p_stream,
                           ogg_packet *p_oggpacket )
{
    /* Convert the granulepos into a pcr */
    if( p_oggpacket->granulepos >= 0 )
    {
        if( p_stream->fmt.i_codec == VLC_FOURCC( 't','h','e','o' ) ||
            p_stream->fmt.i_codec == VLC_FOURCC( 'k','a','t','e' ) )
        {
            ogg_int64_t iframe = p_oggpacket->granulepos >>
              p_stream->i_granule_shift;
            ogg_int64_t pframe = p_oggpacket->granulepos -
              ( iframe << p_stream->i_granule_shift );

            p_stream->i_pcr = ( iframe + pframe ) * INT64_C(1000000)
                              / p_stream->f_rate;
        }
        else if( p_stream->fmt.i_codec == VLC_FOURCC( 'd','r','a','c' ) )
        {
            ogg_int64_t i_dts = p_oggpacket->granulepos >> 31;
            /* NB, OggDirac granulepos values are in units of 2*picturerate */
            p_stream->i_pcr = (i_dts/2) * INT64_C(1000000) / p_stream->f_rate;
        }
        else
        {
            p_stream->i_pcr = p_oggpacket->granulepos * INT64_C(1000000)
                              / p_stream->f_rate;
        }

        p_stream->i_pcr += 1;
        p_stream->i_interpolated_pcr = p_stream->i_pcr;
    }
    else
    {
        p_stream->i_pcr = -1;

        /* no granulepos available, try to interpolate the pcr.
         * If we can't then don't touch the old value. */
        if( p_stream->fmt.i_cat == VIDEO_ES )
            /* 1 frame per packet */
            p_stream->i_interpolated_pcr += (INT64_C(1000000) / p_stream->f_rate);
        else if( p_stream->fmt.i_bitrate )
            p_stream->i_interpolated_pcr +=
                ( p_oggpacket->bytes * INT64_C(1000000) /
                  p_stream->fmt.i_bitrate / 8 );
    }
}

/****************************************************************************
 * Ogg_DecodePacket: Decode an Ogg packet.
 ****************************************************************************/
static void Ogg_DecodePacket( demux_t *p_demux,
                              logical_stream_t *p_stream,
                              ogg_packet *p_oggpacket )
{
    block_t *p_block;
    bool b_selected;
    int i_header_len = 0;
    mtime_t i_pts = -1, i_interpolated_pts;
    demux_sys_t *p_ogg = p_demux->p_sys;

    /* Sanity check */
    if( !p_oggpacket->bytes )
    {
        msg_Dbg( p_demux, "discarding 0 sized packet" );
        return;
    }

    if( p_oggpacket->bytes >= 7 &&
        ! memcmp ( p_oggpacket->packet, "Annodex", 7 ) )
    {
        /* it's an Annodex packet -- skip it (do nothing) */
        return;
    }
    else if( p_oggpacket->bytes >= 7 &&
        ! memcmp ( p_oggpacket->packet, "AnxData", 7 ) )
    {
        /* it's an AnxData packet -- skip it (do nothing) */
        return;
    }

    if( p_stream->fmt.i_codec == VLC_FOURCC( 's','u','b','t' ) &&
        p_oggpacket->packet[0] & PACKET_TYPE_BITS ) return;

    /* Check the ES is selected */
    es_out_Control( p_demux->out, ES_OUT_GET_ES_STATE,
                    p_stream->p_es, &b_selected );

    if( p_stream->b_force_backup )
    {
        uint8_t *p_sav;
        bool b_store_size = true;
        bool b_store_num_headers = false;

        p_stream->i_packets_backup++;
        switch( p_stream->fmt.i_codec )
        {
        case VLC_FOURCC( 'v','o','r','b' ):
        case VLC_FOURCC( 's','p','x',' ' ):
        case VLC_FOURCC( 't','h','e','o' ):
            if( p_stream->i_packets_backup == 3 ) p_stream->b_force_backup = 0;
            break;

        case VLC_FOURCC( 'f','l','a','c' ):
            if( !p_stream->fmt.audio.i_rate && p_stream->i_packets_backup == 2 )
            {
                Ogg_ReadFlacHeader( p_demux, p_stream, p_oggpacket );
                p_stream->b_force_backup = 0;
            }
            else if( p_stream->fmt.audio.i_rate )
            {
                p_stream->b_force_backup = 0;
                if( p_oggpacket->bytes >= 9 )
                {
                    p_oggpacket->packet += 9;
                    p_oggpacket->bytes -= 9;
                }
            }
            b_store_size = false;
            break;

        case VLC_FOURCC( 'k','a','t','e' ):
            if( p_stream->i_packets_backup == 1)
                b_store_num_headers = true;
            if( p_stream->i_packets_backup == p_stream->i_kate_num_headers ) p_stream->b_force_backup = 0;
            break;

        default:
            p_stream->b_force_backup = 0;
            break;
        }

        /* Backup the ogg packet (likely an header packet) */
        p_stream->p_headers =
            realloc( p_sav = p_stream->p_headers, p_stream->i_headers +
                     p_oggpacket->bytes + (b_store_size ? 2 : 0) + (b_store_num_headers ? 1 : 0) );
        if( p_stream->p_headers )
        {
            uint8_t *p_extra = p_stream->p_headers + p_stream->i_headers;

            if( b_store_num_headers )
            {
                /* Kate streams store the number of headers in the first header,
                   so we can't just test for 3 as Vorbis/Theora */
                *(p_extra++) = p_stream->i_kate_num_headers;
            }
            if( b_store_size )
            {
                *(p_extra++) = p_oggpacket->bytes >> 8;
                *(p_extra++) = p_oggpacket->bytes & 0xFF;
            }
            memcpy( p_extra, p_oggpacket->packet, p_oggpacket->bytes );
            p_stream->i_headers += p_oggpacket->bytes + (b_store_size ? 2 : 0) + (b_store_num_headers ? 1 : 0);

            if( !p_stream->b_force_backup )
            {
                /* Last header received, commit changes */
                free( p_stream->fmt.p_extra );

                p_stream->fmt.i_extra = p_stream->i_headers;
                p_stream->fmt.p_extra = malloc( p_stream->i_headers );
                if( p_stream->fmt.p_extra )
                    memcpy( p_stream->fmt.p_extra, p_stream->p_headers,
                            p_stream->i_headers );
                else
                    p_stream->fmt.i_extra = 0;

                if( Ogg_LogicalStreamResetEsFormat( p_demux, p_stream ) )
                    es_out_Control( p_demux->out, ES_OUT_SET_ES_FMT,
                                    p_stream->p_es, &p_stream->fmt );

                if( p_stream->i_headers > 0 )
                    Ogg_ExtractMeta( p_demux, p_stream->fmt.i_codec,
                                     p_stream->p_headers, p_stream->i_headers );

                /* we're not at BOS anymore for this logical stream */
                p_ogg->i_bos--;
            }
        }
        else
        {
                p_stream->p_headers = p_sav;
        }

        b_selected = false; /* Discard the header packet */
    }

    /* Convert the pcr into a pts */
    if( p_stream->fmt.i_codec == VLC_FOURCC( 'v','o','r','b' ) ||
        p_stream->fmt.i_codec == VLC_FOURCC( 's','p','x',' ' ) ||
        p_stream->fmt.i_codec == VLC_FOURCC( 'f','l','a','c' ) )
    {
        if( p_stream->i_pcr >= 0 )
        {
            /* This is for streams where the granulepos of the header packets
             * doesn't match these of the data packets (eg. ogg web radios). */
            if( p_stream->i_previous_pcr == 0 &&
                p_stream->i_pcr  > 3 * DEFAULT_PTS_DELAY )
            {
                es_out_Control( p_demux->out, ES_OUT_RESET_PCR );

                /* Call the pace control */
                es_out_Control( p_demux->out, ES_OUT_SET_PCR,
                                p_stream->i_pcr );
            }

            p_stream->i_previous_pcr = p_stream->i_pcr;

            /* The granulepos is the end date of the sample */
            i_pts =  p_stream->i_pcr;
        }
    }

    /* Convert the granulepos into the next pcr */
    i_interpolated_pts = p_stream->i_interpolated_pcr;
    Ogg_UpdatePCR( p_stream, p_oggpacket );

    /* SPU streams are typically discontinuous, do not mind large gaps */
    if( p_stream->fmt.i_cat != SPU_ES )
    {
        if( p_stream->i_pcr >= 0 )
        {
            /* This is for streams where the granulepos of the header packets
             * doesn't match these of the data packets (eg. ogg web radios). */
            if( p_stream->i_previous_pcr == 0 &&
                p_stream->i_pcr  > 3 * DEFAULT_PTS_DELAY )
            {
                es_out_Control( p_demux->out, ES_OUT_RESET_PCR );

                /* Call the pace control */
                es_out_Control( p_demux->out, ES_OUT_SET_PCR, p_stream->i_pcr );
            }
        }
    }

    if( p_stream->fmt.i_codec != VLC_FOURCC( 'v','o','r','b' ) &&
        p_stream->fmt.i_codec != VLC_FOURCC( 's','p','x',' ' ) &&
        p_stream->fmt.i_codec != VLC_FOURCC( 'f','l','a','c' ) &&
        p_stream->i_pcr >= 0 )
    {
        p_stream->i_previous_pcr = p_stream->i_pcr;

        /* The granulepos is the start date of the sample */
        i_pts = p_stream->i_pcr;
    }

    if( !b_selected )
    {
        /* This stream isn't currently selected so we don't need to decode it,
         * but we did need to store its pcr as it might be selected later on */
        return;
    }

    if( p_oggpacket->bytes <= 0 )
        return;

    if( !( p_block = block_New( p_demux, p_oggpacket->bytes ) ) ) return;

    /* Normalize PTS */
    if( i_pts == 0 ) i_pts = 1;
    else if( i_pts == -1 && i_interpolated_pts == 0 ) i_pts = 1;
    else if( i_pts == -1 ) i_pts = 0;

    if( p_stream->fmt.i_cat == AUDIO_ES )
        p_block->i_dts = p_block->i_pts = i_pts;
    else if( p_stream->fmt.i_cat == SPU_ES )
    {
        p_block->i_dts = p_block->i_pts = i_pts;
        p_block->i_length = 0;
    }
    else if( p_stream->fmt.i_codec == VLC_FOURCC( 't','h','e','o' ) )
    {
        p_block->i_dts = p_block->i_pts = i_pts;
        if( (p_oggpacket->granulepos & ((1<<p_stream->i_granule_shift)-1)) == 0 )
        {
            p_block->i_flags |= BLOCK_FLAG_TYPE_I;
        }
    }
    else if( p_stream->fmt.i_codec == VLC_FOURCC( 'd','r','a','c' ) )
    {
        ogg_int64_t dts = p_oggpacket->granulepos >> 31;
        ogg_int64_t delay = (p_oggpacket->granulepos >> 9) & 0x1fff;

        uint64_t u_pnum = dts + delay;

        p_block->i_dts = p_stream->i_pcr;
        p_block->i_pts = 0;
        /* NB, OggDirac granulepos values are in units of 2*picturerate */
        if( -1 != p_oggpacket->granulepos )
            p_block->i_pts = u_pnum * INT64_C(1000000) / p_stream->f_rate / 2;
    }
    else
    {
        p_block->i_dts = i_pts;
        p_block->i_pts = 0;
    }

    if( p_stream->fmt.i_codec != VLC_FOURCC( 'v','o','r','b' ) &&
        p_stream->fmt.i_codec != VLC_FOURCC( 's','p','x',' ' ) &&
        p_stream->fmt.i_codec != VLC_FOURCC( 'f','l','a','c' ) &&
        p_stream->fmt.i_codec != VLC_FOURCC( 't','a','r','k' ) &&
        p_stream->fmt.i_codec != VLC_FOURCC( 't','h','e','o' ) &&
        p_stream->fmt.i_codec != VLC_FOURCC( 'c','m','m','l' ) &&
        p_stream->fmt.i_codec != VLC_FOURCC( 'd','r','a','c' ) &&
        p_stream->fmt.i_codec != VLC_FOURCC( 'k','a','t','e' ) )
    {
        /* We remove the header from the packet */
        i_header_len = (*p_oggpacket->packet & PACKET_LEN_BITS01) >> 6;
        i_header_len |= (*p_oggpacket->packet & PACKET_LEN_BITS2) << 1;

        if( p_stream->fmt.i_codec == VLC_FOURCC( 's','u','b','t' ))
        {
            /* But with subtitles we need to retrieve the duration first */
            int i, lenbytes = 0;

            if( i_header_len > 0 && p_oggpacket->bytes >= i_header_len + 1 )
            {
                for( i = 0, lenbytes = 0; i < i_header_len; i++ )
                {
                    lenbytes = lenbytes << 8;
                    lenbytes += *(p_oggpacket->packet + i_header_len - i);
                }
            }
            if( p_oggpacket->bytes - 1 - i_header_len > 2 ||
                ( p_oggpacket->packet[i_header_len + 1] != ' ' &&
                  p_oggpacket->packet[i_header_len + 1] != 0 &&
                  p_oggpacket->packet[i_header_len + 1] != '\n' &&
                  p_oggpacket->packet[i_header_len + 1] != '\r' ) )
            {
                p_block->i_length = (mtime_t)lenbytes * 1000;
            }
        }

        i_header_len++;
        if( p_block->i_buffer >= (unsigned int)i_header_len )
            p_block->i_buffer -= i_header_len;
        else
            p_block->i_buffer = 0;
    }

    if( p_stream->fmt.i_codec == VLC_FOURCC( 't','a','r','k' ) )
    {
        /* FIXME: the biggest hack I've ever done */
        msg_Warn( p_demux, "tarkin pts: %"PRId64", granule: %"PRId64,
                  p_block->i_pts, p_block->i_dts );
        msleep(10000);
    }

    memcpy( p_block->p_buffer, p_oggpacket->packet + i_header_len,
            p_oggpacket->bytes - i_header_len );

    es_out_Send( p_demux->out, p_stream->p_es, p_block );
}

/****************************************************************************
 * Ogg_FindLogicalStreams: Find the logical streams embedded in the physical
 *                         stream and fill p_ogg.
 *****************************************************************************
 * The initial page of a logical stream is marked as a 'bos' page.
 * Furthermore, the Ogg specification mandates that grouped bitstreams begin
 * together and all of the initial pages must appear before any data pages.
 *
 * On success this function returns VLC_SUCCESS.
 ****************************************************************************/
static int Ogg_FindLogicalStreams( demux_t *p_demux )
{
    demux_sys_t *p_ogg = p_demux->p_sys  ;
    ogg_packet oggpacket;
    ogg_page oggpage;
    int i_stream;

    while( Ogg_ReadPage( p_demux, &oggpage ) == VLC_SUCCESS )
    {
        if( ogg_page_bos( &oggpage ) )
        {

            /* All is wonderful in our fine fine little world.
             * We found the beginning of our first logical stream. */
            while( ogg_page_bos( &oggpage ) )
            {
                logical_stream_t *p_stream;

                p_stream = malloc( sizeof(logical_stream_t) );
                if( !p_stream )
                    return VLC_ENOMEM;

                TAB_APPEND( p_ogg->i_streams, p_ogg->pp_stream, p_stream );

                memset( p_stream, 0, sizeof(logical_stream_t) );
                p_stream->p_headers = 0;
                p_stream->i_secondary_header_packets = 0;

                es_format_Init( &p_stream->fmt, 0, 0 );
                es_format_Init( &p_stream->fmt_old, 0, 0 );

                /* Setup the logical stream */
                p_stream->i_serial_no = ogg_page_serialno( &oggpage );
                ogg_stream_init( &p_stream->os, p_stream->i_serial_no );

                /* Extract the initial header from the first page and verify
                 * the codec type of this Ogg bitstream */
                if( ogg_stream_pagein( &p_stream->os, &oggpage ) < 0 )
                {
                    /* error. stream version mismatch perhaps */
                    msg_Err( p_demux, "error reading first page of "
                             "Ogg bitstream data" );
                    return VLC_EGENERIC;
                }

                /* FIXME: check return value */
                ogg_stream_packetpeek( &p_stream->os, &oggpacket );

                /* Check for Vorbis header */
                if( oggpacket.bytes >= 7 &&
                    ! memcmp( oggpacket.packet, "\x01vorbis", 7 ) )
                {
                    Ogg_ReadVorbisHeader( p_stream, &oggpacket );
                    msg_Dbg( p_demux, "found vorbis header" );
                }
                /* Check for Speex header */
                else if( oggpacket.bytes >= 5 &&
                    ! memcmp( oggpacket.packet, "Speex", 5 ) )
                {
                    Ogg_ReadSpeexHeader( p_stream, &oggpacket );
                    msg_Dbg( p_demux, "found speex header, channels: %i, "
                             "rate: %i,  bitrate: %i",
                             p_stream->fmt.audio.i_channels,
                             (int)p_stream->f_rate, p_stream->fmt.i_bitrate );
                }
                /* Check for Flac header (< version 1.1.1) */
                else if( oggpacket.bytes >= 4 &&
                    ! memcmp( oggpacket.packet, "fLaC", 4 ) )
                {
                    msg_Dbg( p_demux, "found FLAC header" );

                    /* Grrrr!!!! Did they really have to put all the
                     * important info in the second header packet!!!
                     * (STREAMINFO metadata is in the following packet) */
                    p_stream->b_force_backup = 1;

                    p_stream->fmt.i_cat = AUDIO_ES;
                    p_stream->fmt.i_codec = VLC_FOURCC( 'f','l','a','c' );
                }
                /* Check for Flac header (>= version 1.1.1) */
                else if( oggpacket.bytes >= 13 && oggpacket.packet[0] ==0x7F &&
                    ! memcmp( &oggpacket.packet[1], "FLAC", 4 ) &&
                    ! memcmp( &oggpacket.packet[9], "fLaC", 4 ) )
                {
                    int i_packets = ((int)oggpacket.packet[7]) << 8 |
                        oggpacket.packet[8];
                    msg_Dbg( p_demux, "found FLAC header version %i.%i "
                             "(%i header packets)",
                             oggpacket.packet[5], oggpacket.packet[6],
                             i_packets );

                    p_stream->b_force_backup = 1;

                    p_stream->fmt.i_cat = AUDIO_ES;
                    p_stream->fmt.i_codec = VLC_FOURCC( 'f','l','a','c' );
                    oggpacket.packet += 13; oggpacket.bytes -= 13;
                    Ogg_ReadFlacHeader( p_demux, p_stream, &oggpacket );
                }
                /* Check for Theora header */
                else if( oggpacket.bytes >= 7 &&
                         ! memcmp( oggpacket.packet, "\x80theora", 7 ) )
                {
                    Ogg_ReadTheoraHeader( p_stream, &oggpacket );

                    msg_Dbg( p_demux,
                             "found theora header, bitrate: %i, rate: %f",
                             p_stream->fmt.i_bitrate, p_stream->f_rate );
                }
                /* Check for Dirac header */
                else if( oggpacket.bytes >= 5 &&
                         ! memcmp( oggpacket.packet, "BBCD\x00", 5 ) )
                {
                    if( Ogg_ReadDiracHeader( p_stream, &oggpacket ) )
                        msg_Dbg( p_demux, "found dirac header" );
                    else
                    {
                        msg_Warn( p_demux, "found dirac header isn't decodable" );
                        free( p_stream );
                        p_ogg->i_streams--;
                    }
                }
                /* Check for Tarkin header */
                else if( oggpacket.bytes >= 7 &&
                         ! memcmp( &oggpacket.packet[1], "tarkin", 6 ) )
                {
                    oggpack_buffer opb;

                    msg_Dbg( p_demux, "found tarkin header" );
                    p_stream->fmt.i_cat = VIDEO_ES;
                    p_stream->fmt.i_codec = VLC_FOURCC( 't','a','r','k' );

                    /* Cheat and get additionnal info ;) */
                    oggpack_readinit( &opb, oggpacket.packet, oggpacket.bytes);
                    oggpack_adv( &opb, 88 );
                    oggpack_adv( &opb, 104 );
                    p_stream->fmt.i_bitrate = oggpack_read( &opb, 32 );
                    p_stream->f_rate = 2; /* FIXME */
                    msg_Dbg( p_demux,
                             "found tarkin header, bitrate: %i, rate: %f",
                             p_stream->fmt.i_bitrate, p_stream->f_rate );
                }
                /* Check for Annodex header */
                else if( oggpacket.bytes >= 7 &&
                         ! memcmp( oggpacket.packet, "Annodex", 7 ) )
                {
                    Ogg_ReadAnnodexHeader( VLC_OBJECT(p_demux), p_stream,
                                           &oggpacket );
                    /* kill annodex track */
                    free( p_stream );
                    p_ogg->i_streams--;
                }
                /* Check for Annodex header */
                else if( oggpacket.bytes >= 7 &&
                         ! memcmp( oggpacket.packet, "AnxData", 7 ) )
                {
                    Ogg_ReadAnnodexHeader( VLC_OBJECT(p_demux), p_stream,
                                           &oggpacket );
                }
                /* Check for Kate header */
                else if( oggpacket.bytes >= 8 &&
                    ! memcmp( &oggpacket.packet[1], "kate\0\0\0", 7 ) )
                {
                    Ogg_ReadKateHeader( p_stream, &oggpacket );
                    msg_Dbg( p_demux, "found kate header" );
                }
                else if( oggpacket.bytes >= 142 &&
                         !memcmp( &oggpacket.packet[1],
                                   "Direct Show Samples embedded in Ogg", 35 ))
                {
                    /* Old header type */

                    /* Check for video header (old format) */
                    if( GetDWLE((oggpacket.packet+96)) == 0x05589f80 &&
                        oggpacket.bytes >= 184 )
                    {
                        p_stream->fmt.i_cat = VIDEO_ES;
                        p_stream->fmt.i_codec =
                            VLC_FOURCC( oggpacket.packet[68],
                                        oggpacket.packet[69],
                                        oggpacket.packet[70],
                                        oggpacket.packet[71] );
                        msg_Dbg( p_demux, "found video header of type: %.4s",
                                 (char *)&p_stream->fmt.i_codec );

                        p_stream->fmt.video.i_frame_rate = 10000000;
                        p_stream->fmt.video.i_frame_rate_base =
                            GetQWLE((oggpacket.packet+164));
                        p_stream->f_rate = 10000000.0 /
                            GetQWLE((oggpacket.packet+164));
                        p_stream->fmt.video.i_bits_per_pixel =
                            GetWLE((oggpacket.packet+182));
                        if( !p_stream->fmt.video.i_bits_per_pixel )
                            /* hack, FIXME */
                            p_stream->fmt.video.i_bits_per_pixel = 24;
                        p_stream->fmt.video.i_width =
                            GetDWLE((oggpacket.packet+176));
                        p_stream->fmt.video.i_height =
                            GetDWLE((oggpacket.packet+180));

                        msg_Dbg( p_demux,
                                 "fps: %f, width:%i; height:%i, bitcount:%i",
                                 p_stream->f_rate,
                                 p_stream->fmt.video.i_width,
                                 p_stream->fmt.video.i_height,
                                 p_stream->fmt.video.i_bits_per_pixel);

                    }
                    /* Check for audio header (old format) */
                    else if( GetDWLE((oggpacket.packet+96)) == 0x05589F81 )
                    {
                        unsigned int i_extra_size;
                        unsigned int i_format_tag;

                        p_stream->fmt.i_cat = AUDIO_ES;

                        i_extra_size = GetWLE((oggpacket.packet+140));
                        if( i_extra_size > 0 && i_extra_size < oggpacket.bytes - 142 )
                        {
                            p_stream->fmt.i_extra = i_extra_size;
                            p_stream->fmt.p_extra = malloc( i_extra_size );
                            if( p_stream->fmt.p_extra )
                                memcpy( p_stream->fmt.p_extra,
                                        oggpacket.packet + 142, i_extra_size );
                            else
                                p_stream->fmt.i_extra = 0;
                        }

                        i_format_tag = GetWLE((oggpacket.packet+124));
                        p_stream->fmt.audio.i_channels =
                            GetWLE((oggpacket.packet+126));
                        p_stream->f_rate = p_stream->fmt.audio.i_rate =
                            GetDWLE((oggpacket.packet+128));
                        p_stream->fmt.i_bitrate =
                            GetDWLE((oggpacket.packet+132)) * 8;
                        p_stream->fmt.audio.i_blockalign =
                            GetWLE((oggpacket.packet+136));
                        p_stream->fmt.audio.i_bitspersample =
                            GetWLE((oggpacket.packet+138));

                        wf_tag_to_fourcc( i_format_tag,
                                          &p_stream->fmt.i_codec, 0 );

                        if( p_stream->fmt.i_codec ==
                            VLC_FOURCC('u','n','d','f') )
                        {
                            p_stream->fmt.i_codec = VLC_FOURCC( 'm', 's',
                                ( i_format_tag >> 8 ) & 0xff,
                                i_format_tag & 0xff );
                        }

                        msg_Dbg( p_demux, "found audio header of type: %.4s",
                                 (char *)&p_stream->fmt.i_codec );
                        msg_Dbg( p_demux, "audio:0x%4.4x channels:%d %dHz "
                                 "%dbits/sample %dkb/s",
                                 i_format_tag,
                                 p_stream->fmt.audio.i_channels,
                                 p_stream->fmt.audio.i_rate,
                                 p_stream->fmt.audio.i_bitspersample,
                                 p_stream->fmt.i_bitrate / 1024 );

                    }
                    else
                    {
                        msg_Dbg( p_demux, "stream %d has an old header "
                            "but is of an unknown type", p_ogg->i_streams-1 );
                        free( p_stream );
                        p_ogg->i_streams--;
                    }
                }
                else if( (*oggpacket.packet & PACKET_TYPE_BITS ) == PACKET_TYPE_HEADER &&
                         oggpacket.bytes >= 56+1 )
                {
                    stream_header_t tmp;
                    stream_header_t *st = &tmp;

                    memcpy( st->streamtype, &oggpacket.packet[1+0], 8 );
                    memcpy( st->subtype, &oggpacket.packet[1+8], 4 );
                    st->size = GetDWLE( &oggpacket.packet[1+12] );
                    st->time_unit = GetQWLE( &oggpacket.packet[1+16] );
                    st->samples_per_unit = GetQWLE( &oggpacket.packet[1+24] );
                    st->default_len = GetDWLE( &oggpacket.packet[1+32] );
                    st->buffersize = GetDWLE( &oggpacket.packet[1+36] );
                    st->bits_per_sample = GetWLE( &oggpacket.packet[1+40] ); // (padding 2)

                    /* Check for video header (new format) */
                    if( !strncmp( st->streamtype, "video", 5 ) )
                    {
                        st->sh.video.width = GetDWLE( &oggpacket.packet[1+44] );
                        st->sh.video.height = GetDWLE( &oggpacket.packet[1+48] );

                        p_stream->fmt.i_cat = VIDEO_ES;

                        /* We need to get rid of the header packet */
                        ogg_stream_packetout( &p_stream->os, &oggpacket );

                        p_stream->fmt.i_codec =
                            VLC_FOURCC( st->subtype[0], st->subtype[1],
                                        st->subtype[2], st->subtype[3] );
                        msg_Dbg( p_demux, "found video header of type: %.4s",
                                 (char *)&p_stream->fmt.i_codec );

                        p_stream->fmt.video.i_frame_rate = 10000000;
                        p_stream->fmt.video.i_frame_rate_base = st->time_unit;
                        if( st->time_unit <= 0 )
                            st->time_unit = 400000;
                        p_stream->f_rate = 10000000.0 / st->time_unit;
                        p_stream->fmt.video.i_bits_per_pixel = st->bits_per_sample;
                        p_stream->fmt.video.i_width = st->sh.video.width;
                        p_stream->fmt.video.i_height = st->sh.video.height;

                        msg_Dbg( p_demux,
                                 "fps: %f, width:%i; height:%i, bitcount:%i",
                                 p_stream->f_rate,
                                 p_stream->fmt.video.i_width,
                                 p_stream->fmt.video.i_height,
                                 p_stream->fmt.video.i_bits_per_pixel );
                    }
                    /* Check for audio header (new format) */
                    else if( !strncmp( st->streamtype, "audio", 5 ) )
                    {
                        char p_buffer[5];
                        unsigned int i_extra_size;
                        int i_format_tag;

                        st->sh.audio.channels = GetWLE( &oggpacket.packet[1+44] );
                        st->sh.audio.blockalign = GetWLE( &oggpacket.packet[1+48] );
                        st->sh.audio.avgbytespersec = GetDWLE( &oggpacket.packet[1+52] );

                        p_stream->fmt.i_cat = AUDIO_ES;

                        /* We need to get rid of the header packet */
                        ogg_stream_packetout( &p_stream->os, &oggpacket );

                        i_extra_size = st->size - 56;

                        if( i_extra_size > 0 &&
                            i_extra_size < oggpacket.bytes - 1 - 56 )
                        {
                            p_stream->fmt.i_extra = i_extra_size;
                            p_stream->fmt.p_extra = malloc( p_stream->fmt.i_extra );
                            if( p_stream->fmt.p_extra )
                                memcpy( p_stream->fmt.p_extra, st + 1,
                                        p_stream->fmt.i_extra );
                            else
                                p_stream->fmt.i_extra = 0;
                        }

                        memcpy( p_buffer, st->subtype, 4 );
                        p_buffer[4] = '\0';
                        i_format_tag = strtol(p_buffer,NULL,16);
                        p_stream->fmt.audio.i_channels = st->sh.audio.channels;
                        if( st->time_unit <= 0 )
                            st->time_unit = 10000000;
                        p_stream->f_rate = p_stream->fmt.audio.i_rate = st->samples_per_unit * 10000000 / st->time_unit;
                        p_stream->fmt.i_bitrate = st->sh.audio.avgbytespersec * 8;
                        p_stream->fmt.audio.i_blockalign = st->sh.audio.blockalign;
                        p_stream->fmt.audio.i_bitspersample = st->bits_per_sample;

                        wf_tag_to_fourcc( i_format_tag,
                                          &p_stream->fmt.i_codec, 0 );

                        if( p_stream->fmt.i_codec ==
                            VLC_FOURCC('u','n','d','f') )
                        {
                            p_stream->fmt.i_codec = VLC_FOURCC( 'm', 's',
                                ( i_format_tag >> 8 ) & 0xff,
                                i_format_tag & 0xff );
                        }

                        msg_Dbg( p_demux, "found audio header of type: %.4s",
                                 (char *)&p_stream->fmt.i_codec );
                        msg_Dbg( p_demux, "audio:0x%4.4x channels:%d %dHz "
                                 "%dbits/sample %dkb/s",
                                 i_format_tag,
                                 p_stream->fmt.audio.i_channels,
                                 p_stream->fmt.audio.i_rate,
                                 p_stream->fmt.audio.i_bitspersample,
                                 p_stream->fmt.i_bitrate / 1024 );
                    }
                    /* Check for text (subtitles) header */
                    else if( !strncmp(st->streamtype, "text", 4) )
                    {
                        /* We need to get rid of the header packet */
                        ogg_stream_packetout( &p_stream->os, &oggpacket );

                        msg_Dbg( p_demux, "found text subtitles header" );
                        p_stream->fmt.i_cat = SPU_ES;
                        p_stream->fmt.i_codec = VLC_FOURCC('s','u','b','t');
                        p_stream->f_rate = 1000; /* granulepos is in millisec */
                    }
                    else
                    {
                        msg_Dbg( p_demux, "stream %d has a header marker "
                            "but is of an unknown type", p_ogg->i_streams-1 );
                        free( p_stream );
                        p_ogg->i_streams--;
                    }
                }
                else if( oggpacket.bytes >= 7 &&
                             ! memcmp( oggpacket.packet, "fishead", 7 ) )

                {
                    /* Skeleton */
                    msg_Dbg( p_demux, "stream %d is a skeleton",
                                p_ogg->i_streams-1 );
                    /* FIXME: https://trac.videolan.org/vlc/ticket/1412 */
                }
                else
                {
                    msg_Dbg( p_demux, "stream %d is of unknown type",
                             p_ogg->i_streams-1 );
                    free( p_stream );
                    p_ogg->i_streams--;
                }

                if( Ogg_ReadPage( p_demux, &oggpage ) != VLC_SUCCESS )
                    return VLC_EGENERIC;
            }

            /* we'll need to get all headers for all of those streams
               that we have to backup headers for */
            p_ogg->i_bos = 0;
            for( i_stream = 0; i_stream < p_ogg->i_streams; i_stream++ )
            {
                if( p_ogg->pp_stream[i_stream]->b_force_backup )
                    p_ogg->i_bos++;
            }


            /* This is the first data page, which means we are now finished
             * with the initial pages. We just need to store it in the relevant
             * bitstream. */
            for( i_stream = 0; i_stream < p_ogg->i_streams; i_stream++ )
            {
                if( ogg_stream_pagein( &p_ogg->pp_stream[i_stream]->os,
                                       &oggpage ) == 0 )
                {
                    p_ogg->b_page_waiting = true;
                    break;
                }
            }

            return VLC_SUCCESS;
        }
    }

    return VLC_EGENERIC;
}

/****************************************************************************
 * Ogg_BeginningOfStream: Look for Beginning of Stream ogg pages and add
 *                        Elementary streams.
 ****************************************************************************/
static int Ogg_BeginningOfStream( demux_t *p_demux )
{
    demux_sys_t *p_ogg = p_demux->p_sys  ;
    logical_stream_t *p_old_stream = p_ogg->p_old_stream;
    int i_stream;

    /* Find the logical streams embedded in the physical stream and
     * initialize our p_ogg structure. */
    if( Ogg_FindLogicalStreams( p_demux ) != VLC_SUCCESS )
    {
        msg_Warn( p_demux, "couldn't find any ogg logical stream" );
        return VLC_EGENERIC;
    }

    p_ogg->i_bitrate = 0;

    for( i_stream = 0 ; i_stream < p_ogg->i_streams; i_stream++ )
    {
        logical_stream_t *p_stream = p_ogg->pp_stream[i_stream];

        p_stream->p_es = NULL;

        /* Try first to reuse an old ES */
        if( p_old_stream &&
            p_old_stream->fmt.i_cat == p_stream->fmt.i_cat &&
            p_old_stream->fmt.i_codec == p_stream->fmt.i_codec )
        {
            msg_Dbg( p_demux, "will reuse old stream to avoid glitch" );

            p_stream->p_es = p_old_stream->p_es;
            es_format_Copy( &p_stream->fmt_old, &p_old_stream->fmt );

            p_old_stream->p_es = NULL;
            p_old_stream = NULL;
        }

        if( !p_stream->p_es )
        {
            /* Better be safe than sorry when possible with ogm */
            if( p_stream->fmt.i_codec == VLC_FOURCC( 'm', 'p', 'g', 'a' ) ||
                p_stream->fmt.i_codec == VLC_FOURCC( 'a', '5', '2', ' ' ) )
                p_stream->fmt.b_packetized = false;

            p_stream->p_es = es_out_Add( p_demux->out, &p_stream->fmt );
        }

        // TODO: something to do here ?
        if( p_stream->fmt.i_codec == VLC_FOURCC('c','m','m','l') )
        {
            /* Set the CMML stream active */
            es_out_Control( p_demux->out, ES_OUT_SET_ES, p_stream->p_es );
        }

        p_ogg->i_bitrate += p_stream->fmt.i_bitrate;

        p_stream->i_pcr = p_stream->i_previous_pcr =
            p_stream->i_interpolated_pcr = -1;
        p_stream->b_reinit = false;
    }

    if( p_ogg->p_old_stream )
    {
        if( p_ogg->p_old_stream->p_es )
            msg_Dbg( p_demux, "old stream not reused" );
        Ogg_LogicalStreamDelete( p_demux, p_ogg->p_old_stream );
        p_ogg->p_old_stream = NULL;
    }
    return VLC_SUCCESS;
}

/****************************************************************************
 * Ogg_EndOfStream: clean up the ES when an End of Stream is detected.
 ****************************************************************************/
static void Ogg_EndOfStream( demux_t *p_demux )
{
    demux_sys_t *p_ogg = p_demux->p_sys  ;
    int i_stream;

    for( i_stream = 0 ; i_stream < p_ogg->i_streams; i_stream++ )
        Ogg_LogicalStreamDelete( p_demux, p_ogg->pp_stream[i_stream] );
    free( p_ogg->pp_stream );

    /* Reinit p_ogg */
    p_ogg->i_bitrate = 0;
    p_ogg->i_streams = 0;
    p_ogg->pp_stream = NULL;

    /* */
    if( p_ogg->p_meta )
        vlc_meta_Delete( p_ogg->p_meta );
    p_ogg->p_meta = NULL;
}

/**
 * This function delete and release all data associated to a logical_stream_t
 */
static void Ogg_LogicalStreamDelete( demux_t *p_demux, logical_stream_t *p_stream )
{
    if( p_stream->p_es )
        es_out_Del( p_demux->out, p_stream->p_es );

    ogg_stream_clear( &p_stream->os );
    free( p_stream->p_headers );

    es_format_Clean( &p_stream->fmt_old );
    es_format_Clean( &p_stream->fmt );

    free( p_stream );
}
/**
 * This function check if a we need to reset a decoder in case we are
 * reusing an old ES
 */
static bool Ogg_IsVorbisFormatCompatible( const es_format_t *p_new, const es_format_t *p_old )
{
    int i_new = 0;
    int i_old = 0;
    int i;

    for( i = 0; i < 3; i++ )
    {
        const uint8_t *p_new_extra = ( const uint8_t*)p_new->p_extra + i_new;
        const uint8_t *p_old_extra = ( const uint8_t*)p_old->p_extra + i_old;

        if( p_new->i_extra < i_new+2 || p_old->i_extra < i_old+2 )
            return false;

        const int i_new_size = GetWBE( &p_new_extra[0] );
        const int i_old_size = GetWBE( &p_old_extra[0] );

        if( i != 1 ) /* Ignore vorbis comment */
        {
            if( i_new_size != i_old_size )
                return false;
            if( memcmp( &p_new_extra[2], &p_old_extra[2], i_new_size ) )
                return false;
        }

        i_new += 2 + i_new_size;
        i_old += 2 + i_old_size;
    }
    return true;
}
static bool Ogg_LogicalStreamResetEsFormat( demux_t *p_demux, logical_stream_t *p_stream )
{
    bool b_compatible = false;
    if( !p_stream->fmt_old.i_cat || !p_stream->fmt_old.i_codec )
        return true;

    /* Only vorbis is supported */
    if( p_stream->fmt.i_codec == VLC_FOURCC( 'v','o','r','b' ) )
        b_compatible = Ogg_IsVorbisFormatCompatible( &p_stream->fmt, &p_stream->fmt_old );

    if( !b_compatible )
        msg_Warn( p_demux, "cannot reuse old stream, resetting the decoder" );

    return !b_compatible;
}
static void Ogg_ExtractXiphMeta( demux_t *p_demux, const uint8_t *p_headers, int i_headers, int i_skip, bool b_has_num_headers )
{
    demux_sys_t *p_ogg = p_demux->p_sys;

    if (b_has_num_headers)
    {
        if (i_headers <= 0)
            return;
        /* number of headers on a byte, we're interested in the second header, so should be at least 2 to go on */
        if (*p_headers++ < 2)
            return;
        --i_headers;
    }

    if( i_headers <= 2 )
        return;

    /* Skip first packet */
    const int i_tmp = GetWBE( &p_headers[0] );
    if( i_tmp > i_headers-2 )
        return;
    p_headers += 2 + i_tmp;
    i_headers -= 2 + i_tmp;

    if( i_headers <= 2 )
        return;

    /* */
    int i_comment = GetWBE( &p_headers[0] );
    const uint8_t *p_comment = &p_headers[2];
    if( i_comment > i_headers - 2 )
        return;

    if( i_comment <= i_skip )
        return;

    /* TODO how to handle multiple comments properly ? */
    vorbis_ParseComment( &p_ogg->p_meta, &p_comment[i_skip], i_comment - i_skip );
}
static void Ogg_ExtractMeta( demux_t *p_demux, vlc_fourcc_t i_codec, const uint8_t *p_headers, int i_headers )
{
    demux_sys_t *p_ogg = p_demux->p_sys;

    switch( i_codec )
    {
    /* 3 headers with the 2� one being the comments */
    case VLC_FOURCC( 'v','o','r','b' ):
        Ogg_ExtractXiphMeta( p_demux, p_headers, i_headers, 1+6, false );
        break;
    case VLC_FOURCC( 't','h','e','o' ):
        Ogg_ExtractXiphMeta( p_demux, p_headers, i_headers, 1+6, false );
        break;
    case VLC_FOURCC( 's','p','x',' ' ):
        Ogg_ExtractXiphMeta( p_demux, p_headers, i_headers, 0, false );
        break;

    /* N headers with the 2� one being the comments */
    case VLC_FOURCC( 'k','a','t','e' ):
        /* 1 byte for header type, 7 bit for magic, 1 reserved zero byte */
        Ogg_ExtractXiphMeta( p_demux, p_headers, i_headers, 1+7+1, true );
        break;

    /* TODO */
    case VLC_FOURCC( 'f','l','a','c' ):
        msg_Warn( p_demux, "Ogg_ExtractMeta does not support %4.4s", (const char*)&i_codec );
        break;

    /* No meta data */
    case VLC_FOURCC( 'c','m','m','l' ): /* CMML is XML text, doesn't have Vorbis comments */
    case VLC_FOURCC( 'd','r','a','c' ):
    default:
        break;
    }
    if( p_ogg->p_meta )
        p_demux->info.i_update |= INPUT_UPDATE_META;
}

static void Ogg_ReadTheoraHeader( logical_stream_t *p_stream,
                                  ogg_packet *p_oggpacket )
{
    bs_t bitstream;
    int i_fps_numerator;
    int i_fps_denominator;
    int i_keyframe_frequency_force;

    p_stream->fmt.i_cat = VIDEO_ES;
    p_stream->fmt.i_codec = VLC_FOURCC( 't','h','e','o' );

    /* Signal that we want to keep a backup of the theora
     * stream headers. They will be used when switching between
     * audio streams. */
    p_stream->b_force_backup = 1;

    /* Cheat and get additionnal info ;) */
    bs_init( &bitstream, p_oggpacket->packet, p_oggpacket->bytes );
    bs_skip( &bitstream, 56 );
    bs_read( &bitstream, 8 ); /* major version num */
    bs_read( &bitstream, 8 ); /* minor version num */
    bs_read( &bitstream, 8 ); /* subminor version num */
    bs_read( &bitstream, 16 ) /*<< 4*/; /* width */
    bs_read( &bitstream, 16 ) /*<< 4*/; /* height */
    bs_read( &bitstream, 24 ); /* frame width */
    bs_read( &bitstream, 24 ); /* frame height */
    bs_read( &bitstream, 8 ); /* x offset */
    bs_read( &bitstream, 8 ); /* y offset */

    i_fps_numerator = bs_read( &bitstream, 32 );
    i_fps_denominator = bs_read( &bitstream, 32 );
    bs_read( &bitstream, 24 ); /* aspect_numerator */
    bs_read( &bitstream, 24 ); /* aspect_denominator */

    p_stream->fmt.video.i_frame_rate = i_fps_numerator;
    p_stream->fmt.video.i_frame_rate_base = i_fps_denominator;

    bs_read( &bitstream, 8 ); /* colorspace */
    p_stream->fmt.i_bitrate = bs_read( &bitstream, 24 );
    bs_read( &bitstream, 6 ); /* quality */

    i_keyframe_frequency_force = 1 << bs_read( &bitstream, 5 );

    /* granule_shift = i_log( frequency_force -1 ) */
    p_stream->i_granule_shift = 0;
    i_keyframe_frequency_force--;
    while( i_keyframe_frequency_force )
    {
        p_stream->i_granule_shift++;
        i_keyframe_frequency_force >>= 1;
    }

    p_stream->f_rate = ((float)i_fps_numerator) / i_fps_denominator;
}

static void Ogg_ReadVorbisHeader( logical_stream_t *p_stream,
                                  ogg_packet *p_oggpacket )
{
    oggpack_buffer opb;

    p_stream->fmt.i_cat = AUDIO_ES;
    p_stream->fmt.i_codec = VLC_FOURCC( 'v','o','r','b' );

    /* Signal that we want to keep a backup of the vorbis
     * stream headers. They will be used when switching between
     * audio streams. */
    p_stream->b_force_backup = 1;

    /* Cheat and get additionnal info ;) */
    oggpack_readinit( &opb, p_oggpacket->packet, p_oggpacket->bytes);
    oggpack_adv( &opb, 88 );
    p_stream->fmt.audio.i_channels = oggpack_read( &opb, 8 );
    p_stream->f_rate = p_stream->fmt.audio.i_rate =
        oggpack_read( &opb, 32 );
    oggpack_adv( &opb, 32 );
    p_stream->fmt.i_bitrate = oggpack_read( &opb, 32 );
}

static void Ogg_ReadSpeexHeader( logical_stream_t *p_stream,
                                 ogg_packet *p_oggpacket )
{
    oggpack_buffer opb;

    p_stream->fmt.i_cat = AUDIO_ES;
    p_stream->fmt.i_codec = VLC_FOURCC( 's','p','x',' ' );

    /* Signal that we want to keep a backup of the speex
     * stream headers. They will be used when switching between
     * audio streams. */
    p_stream->b_force_backup = 1;

    /* Cheat and get additionnal info ;) */
    oggpack_readinit( &opb, p_oggpacket->packet, p_oggpacket->bytes);
    oggpack_adv( &opb, 224 );
    oggpack_adv( &opb, 32 ); /* speex_version_id */
    oggpack_adv( &opb, 32 ); /* header_size */
    p_stream->f_rate = p_stream->fmt.audio.i_rate = oggpack_read( &opb, 32 );
    oggpack_adv( &opb, 32 ); /* mode */
    oggpack_adv( &opb, 32 ); /* mode_bitstream_version */
    p_stream->fmt.audio.i_channels = oggpack_read( &opb, 32 );
    p_stream->fmt.i_bitrate = oggpack_read( &opb, 32 );
}

static void Ogg_ReadFlacHeader( demux_t *p_demux, logical_stream_t *p_stream,
                                ogg_packet *p_oggpacket )
{
    /* Parse the STREAMINFO metadata */
    bs_t s;

    bs_init( &s, p_oggpacket->packet, p_oggpacket->bytes );

    bs_read( &s, 1 );
    if( bs_read( &s, 7 ) == 0 )
    {
        if( bs_read( &s, 24 ) >= 34 /*size STREAMINFO*/ )
        {
            bs_skip( &s, 80 );
            p_stream->f_rate = p_stream->fmt.audio.i_rate = bs_read( &s, 20 );
            p_stream->fmt.audio.i_channels = bs_read( &s, 3 ) + 1;

            msg_Dbg( p_demux, "FLAC header, channels: %i, rate: %i",
                     p_stream->fmt.audio.i_channels, (int)p_stream->f_rate );
        }
        else
        {
            msg_Dbg( p_demux, "FLAC STREAMINFO metadata too short" );
        }

        /* Fake this as the last metadata block */
        *((uint8_t*)p_oggpacket->packet) |= 0x80;
    }
    else
    {
        /* This ain't a STREAMINFO metadata */
        msg_Dbg( p_demux, "Invalid FLAC STREAMINFO metadata" );
    }
}

static void Ogg_ReadKateHeader( logical_stream_t *p_stream,
                                ogg_packet *p_oggpacket )
{
    oggpack_buffer opb;
    int32_t gnum;
    int32_t gden;
    int n;
    char *psz_desc;

    p_stream->fmt.i_cat = SPU_ES;
    p_stream->fmt.i_codec = VLC_FOURCC( 'k','a','t','e' );

    /* Signal that we want to keep a backup of the kate
     * stream headers. They will be used when switching between
     * kate streams. */
    p_stream->b_force_backup = 1;

    /* Cheat and get additionnal info ;) */
    oggpack_readinit( &opb, p_oggpacket->packet, p_oggpacket->bytes);
    oggpack_adv( &opb, 11*8 ); /* packet type, kate magic, version */
    p_stream->i_kate_num_headers = oggpack_read( &opb, 8 );
    oggpack_adv( &opb, 3*8 );
    p_stream->i_granule_shift = oggpack_read( &opb, 8 );
    oggpack_adv( &opb, 8*8 ); /* reserved */
    gnum = oggpack_read( &opb, 32 );
    gden = oggpack_read( &opb, 32 );
    p_stream->f_rate = (double)gnum/gden;

    p_stream->fmt.psz_language = malloc(16);
    if( p_stream->fmt.psz_language )
    {
        for( n = 0; n < 16; n++ )
            p_stream->fmt.psz_language[n] = oggpack_read(&opb,8);
        p_stream->fmt.psz_language[15] = 0; /* just in case */
    }
    else
    {
        for( n = 0; n < 16; n++ )
            oggpack_read(&opb,8);
    }
    p_stream->fmt.psz_description = malloc(16);
    if( p_stream->fmt.psz_description )
    {
        for( n = 0; n < 16; n++ )
            p_stream->fmt.psz_description[n] = oggpack_read(&opb,8);
        p_stream->fmt.psz_description[15] = 0; /* just in case */

        /* Now find a localized user readable description for this category */
        psz_desc = strdup(FindKateCategoryName(p_stream->fmt.psz_description));
        if( psz_desc )
        {
            free( p_stream->fmt.psz_description );
            p_stream->fmt.psz_description = psz_desc;
        }
    }
    else
    {
        for( n = 0; n < 16; n++ )
            oggpack_read(&opb,8);
    }
}

static void Ogg_ReadAnnodexHeader( vlc_object_t *p_this,
                                   logical_stream_t *p_stream,
                                   ogg_packet *p_oggpacket )
{
    if( p_oggpacket->bytes >= 28 &&
        !memcmp( p_oggpacket->packet, "Annodex", 7 ) )
    {
        oggpack_buffer opb;

        uint16_t major_version;
        uint16_t minor_version;
        uint64_t timebase_numerator;
        uint64_t timebase_denominator;

        Ogg_ReadTheoraHeader( p_stream, p_oggpacket );

        oggpack_readinit( &opb, p_oggpacket->packet, p_oggpacket->bytes);
        oggpack_adv( &opb, 8*8 ); /* "Annodex\0" header */
        major_version = oggpack_read( &opb, 2*8 ); /* major version */
        minor_version = oggpack_read( &opb, 2*8 ); /* minor version */
        timebase_numerator = GetQWLE( &p_oggpacket->packet[16] );
        timebase_denominator = GetQWLE( &p_oggpacket->packet[24] );
    }
    else if( p_oggpacket->bytes >= 42 &&
             !memcmp( p_oggpacket->packet, "AnxData", 7 ) )
    {
        uint64_t granule_rate_numerator;
        uint64_t granule_rate_denominator;
        char content_type_string[1024];

        /* Read in Annodex header fields */

        granule_rate_numerator = GetQWLE( &p_oggpacket->packet[8] );
        granule_rate_denominator = GetQWLE( &p_oggpacket->packet[16] );
        p_stream->i_secondary_header_packets =
            GetDWLE( &p_oggpacket->packet[24] );

        /* we are guaranteed that the first header field will be
         * the content-type (by the Annodex standard) */
        content_type_string[0] = '\0';
        if( !strncasecmp( (char*)(&p_oggpacket->packet[28]), "Content-Type: ", 14 ) )
        {
            uint8_t *p = memchr( &p_oggpacket->packet[42], '\r',
                                 p_oggpacket->bytes - 1 );
            if( p && p[0] == '\r' && p[1] == '\n' )
                sscanf( (char*)(&p_oggpacket->packet[42]), "%1024s\r\n",
                        content_type_string );
        }

        msg_Dbg( p_this, "AnxData packet info: %"PRId64" / %"PRId64", %d, ``%s''",
                 granule_rate_numerator, granule_rate_denominator,
                 p_stream->i_secondary_header_packets, content_type_string );

        p_stream->f_rate = (float) granule_rate_numerator /
            (float) granule_rate_denominator;

        /* What type of file do we have?
         * strcmp is safe to use here because we've extracted
         * content_type_string from the stream manually */
        if( !strncmp(content_type_string, "audio/x-wav", 11) )
        {
            /* n.b. WAVs are unsupported right now */
            p_stream->fmt.i_cat = UNKNOWN_ES;
        }
        else if( !strncmp(content_type_string, "audio/x-vorbis", 14) )
        {
            p_stream->fmt.i_cat = AUDIO_ES;
            p_stream->fmt.i_codec = VLC_FOURCC( 'v','o','r','b' );

            p_stream->b_force_backup = 1;
        }
        else if( !strncmp(content_type_string, "audio/x-speex", 14) )
        {
            p_stream->fmt.i_cat = AUDIO_ES;
            p_stream->fmt.i_codec = VLC_FOURCC( 's','p','x',' ' );

            p_stream->b_force_backup = 1;
        }
        else if( !strncmp(content_type_string, "video/x-theora", 14) )
        {
            p_stream->fmt.i_cat = VIDEO_ES;
            p_stream->fmt.i_codec = VLC_FOURCC( 't','h','e','o' );

            p_stream->b_force_backup = 1;
        }
        else if( !strncmp(content_type_string, "video/x-xvid", 14) )
        {
            p_stream->fmt.i_cat = VIDEO_ES;
            p_stream->fmt.i_codec = VLC_FOURCC( 'x','v','i','d' );

            p_stream->b_force_backup = 1;
        }
        else if( !strncmp(content_type_string, "video/mpeg", 14) )
        {
            /* n.b. MPEG streams are unsupported right now */
            p_stream->fmt.i_cat = VIDEO_ES;
            p_stream->fmt.i_codec = VLC_FOURCC( 'm','p','g','v' );
        }
        else if( !strncmp(content_type_string, "text/x-cmml", 11) )
        {
            ogg_stream_packetout( &p_stream->os, p_oggpacket );
            p_stream->fmt.i_cat = SPU_ES;
            p_stream->fmt.i_codec = VLC_FOURCC( 'c','m','m','l' );
        }
    }
}

static uint32_t dirac_uint( bs_t *p_bs )
{
    uint32_t u_count = 0, u_value = 0;

    while( !bs_eof( p_bs ) && !bs_read( p_bs, 1 ) )
    {
        u_count++;
        u_value <<= 1;
        u_value |= bs_read( p_bs, 1 );
    }

    return (1<<u_count) - 1 + u_value;
}

static int dirac_bool( bs_t *p_bs )
{
    return bs_read( p_bs, 1 );
}

static bool Ogg_ReadDiracHeader( logical_stream_t *p_stream,
                                 ogg_packet *p_oggpacket )
{
    static const struct {
        uint32_t u_n /* numerator */, u_d /* denominator */;
    } p_dirac_frate_tbl[] = { /* table 10.3 */
        {1,1}, /* this first value is never used */
        {24000,1001}, {24,1}, {25,1}, {30000,1001}, {30,1},
        {50,1}, {60000,1001}, {60,1}, {15000,1001}, {25,2},
    };
    static const size_t u_dirac_frate_tbl = sizeof(p_dirac_frate_tbl)/sizeof(*p_dirac_frate_tbl);

    static const uint32_t pu_dirac_vidfmt_frate[] = { /* table C.1 */
        1, 9, 10, 9, 10, 9, 10, 4, 3, 7, 6, 4, 3, 7, 6, 2, 2, 7, 6, 7, 6,
    };
    static const size_t u_dirac_vidfmt_frate = sizeof(pu_dirac_vidfmt_frate)/sizeof(*pu_dirac_vidfmt_frate);

    bs_t bs;

    p_stream->i_granule_shift = 22; /* not 32 */

    /* Backing up stream headers is not required -- seqhdrs are repeated
     * thoughout the stream at suitable decoding start points */
    p_stream->b_force_backup = 0;

    /* read in useful bits from sequence header */
    bs_init( &bs, p_oggpacket->packet, p_oggpacket->bytes );
    bs_skip( &bs, 13*8); /* parse_info_header */
    dirac_uint( &bs ); /* major_version */
    dirac_uint( &bs ); /* minor_version */
    dirac_uint( &bs ); /* profile */
    dirac_uint( &bs ); /* level */

    uint32_t u_video_format = dirac_uint( &bs ); /* index */
    if( u_video_format >= u_dirac_vidfmt_frate )
    {
        /* don't know how to parse this ogg dirac stream */
        return false;
    }

    if( dirac_bool( &bs ) )
    {
        dirac_uint( &bs ); /* frame_width */
        dirac_uint( &bs ); /* frame_height */
    }

    if( dirac_bool( &bs ) )
    {
        dirac_uint( &bs ); /* chroma_format */
    }

    if( dirac_bool( &bs ) )
    {
        dirac_uint( &bs ); /* scan_format */
    }

    uint32_t u_n = p_dirac_frate_tbl[pu_dirac_vidfmt_frate[u_video_format]].u_n;
    uint32_t u_d = p_dirac_frate_tbl[pu_dirac_vidfmt_frate[u_video_format]].u_d;
    if( dirac_bool( &bs ) )
    {
        uint32_t u_frame_rate_index = dirac_uint( &bs );
        if( u_frame_rate_index >= u_dirac_frate_tbl )
        {
            /* something is wrong with this stream */
            return false;
        }
        u_n = p_dirac_frate_tbl[u_frame_rate_index].u_n;
        u_d = p_dirac_frate_tbl[u_frame_rate_index].u_d;
        if( u_frame_rate_index == 0 )
        {
            u_n = dirac_uint( &bs ); /* frame_rate_numerator */
            u_d = dirac_uint( &bs ); /* frame_rate_denominator */
        }
    }
    p_stream->f_rate = (float) u_n / u_d;

    /* probably is an ogg dirac es */
    p_stream->fmt.i_cat = VIDEO_ES;
    p_stream->fmt.i_codec = VLC_FOURCC( 'd','r','a','c' );

    return true;
}
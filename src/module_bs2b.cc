/*
    Copyright 2010 (C) Jan Bujak <j@exia.io>

    This file is part of DSPR.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "module_bs2b.h"

meta_module_bs2b_t g_meta_module_bs2b;

meta_module_t * module_bs2b_t :: get_meta_module()
{
    return &g_meta_module_bs2b;
}

const char ** meta_module_bs2b_t :: fields()
{
    static const char * array[] = { "fcut", "feed", "version", 0 };
    return array;
}

bool meta_module_bs2b_t :: is_readonly( const char * key )
{
    if( !strcmp( key, "version" ) ) return true;
    return false;
}

field_type meta_module_bs2b_t :: get_type( const char * key )
{
    if( !strcmp( key, "version" ) ) return field_string;
    return field_long;
}

module_bs2b_t :: module_bs2b_t()
{
    bs2b.set_level_fcut( 700 );
    bs2b.set_level_feed( 45 );
}

bool module_bs2b_t :: set_long( const char * key, long value )
{
    if( !strcmp( key, "fcut" ) )
    {
        bs2b.set_level_fcut( value );
        return true;
    }
    else if( !strcmp( key, "feed" ) )
    {
        bs2b.set_level_feed( value );
        return true;
    }
    return false;
}

bool module_bs2b_t :: get_long( const char * key, long& value )
{
    if( !strcmp( key, "fcut" ) )
    {
        value = bs2b.get_level_fcut();
        return true;
    }
    else if( !strcmp( key, "feed" ) )
    {
        value = bs2b.get_level_feed();
        return true;
    }
    return false;
}

bool module_bs2b_t :: get_string( const char * key, std :: string& value )
{
    if( !strcmp( key, "version" ) )
    {
        value = std :: string( bs2b.runtime_version() );
        return true;
    }
    return false;
}

void module_bs2b_t :: process( const char * input, char * output, unsigned length, unsigned samples, unsigned channels, unsigned sample_rate, format_t format )
{
    if( bs2b.get_srate() != sample_rate ) bs2b.set_srate( sample_rate );

    if( format.representation == format_short )
    {
        if( format.endianness == format_le )
            bs2b.cross_feed_le( ( short * )input, samples );
        else
            bs2b.cross_feed_be( ( short * )input, samples );
    }
    else
    {
        if( format.endianness == format_le )
            bs2b.cross_feed_le( ( float * )input, samples );
        else
            bs2b.cross_feed_be( ( float * )input, samples );
    }
}

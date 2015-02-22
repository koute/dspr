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

#include "dspr_client_c.h"
#include "dspr_client.h"

extern "C"
{

#pragma GCC visibility push( default )

dspr_client_t * dspr_client_new()
{
    return new dspr_client_t();
}

void dspr_client_delete( dspr_client_t * client )
{
    delete client;
}

void dspr_client_initialize( dspr_client_t * self )
{
    self->initialize();
}

void dspr_client_set_sample_rate( dspr_client_t * self, unsigned sample_rate )
{
    self->set_sample_rate( sample_rate );
}

unsigned dspr_client_sample_rate( dspr_client_t * self )
{
    return self->sample_rate();
}

void dspr_client_set_channels( dspr_client_t * self, unsigned channels )
{
    self->set_channels( channels );
}

unsigned dspr_client_channels( dspr_client_t * self )
{
    return self->channels();
}

void dspr_client_set_server_ip( dspr_client_t * self, const char * ip )
{
    self->set_server_ip( ip );
}

char * dspr_client_server_ip( dspr_client_t * self )
{
    return strdup( self->server_ip().c_str() );
}

void dspr_client_set_server_port( dspr_client_t * self, unsigned port )
{
    self->set_server_port( port );
}

unsigned dspr_client_server_port( dspr_client_t * self )
{
    return self->server_port();
}

dspr_bool dspr_client_activate( dspr_client_t * self )
{
    return self->activate();
}

void dspr_client_deactivate( dspr_client_t * self )
{
    self->deactivate();
}

dspr_bool dspr_client_prepare_noninterleaved_float( dspr_client_t * self, unsigned samples )
{
    return self->prepare_noninterleaved_float( samples );
}
dspr_bool dspr_client_prepare_noninterleaved_short( dspr_client_t * self, unsigned samples )
{
    return self->prepare_noninterleaved_short( samples );
}

dspr_bool dspr_client_write_channel_float( dspr_client_t * self, const float * buffer )
{
    return self->write_channel( buffer );
}

dspr_bool dspr_client_read_channel_float( dspr_client_t * self, float * buffer )
{
    return self->read_channel( buffer );
}

dspr_bool dspr_client_write_channel_short( dspr_client_t * self, const short * buffer )
{
    return self->write_channel( buffer );
}

dspr_bool dspr_client_read_channel_short( dspr_client_t * self, short * buffer )
{
    return self->read_channel( buffer );
}

dspr_bool dspr_client_interleaved_write_float( dspr_client_t * self, const float * buffer, unsigned samples )
{
    return self->interleaved_write( buffer, samples );
}

dspr_bool dspr_client_interleaved_read_float( dspr_client_t * self, float * buffer )
{
    return self->interleaved_read( buffer );
}

dspr_bool dspr_client_interleaved_write_short( dspr_client_t * self, const short * buffer, unsigned samples )
{
    return self->interleaved_write( buffer, samples );
}

dspr_bool dspr_client_interleaved_read_short( dspr_client_t * self, short * buffer )
{
    return self->interleaved_read( buffer );
}

dspr_bool dspr_client_try_activate( dspr_client_t * self )
{
    return self->activate();
}
dspr_bool dspr_client_is_active( dspr_client_t * self )
{
    return self->is_active();
}

#pragma GCC visibility pop

}

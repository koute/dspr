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

#ifndef DSPR_CLIENT_C_H
#define DSPR_CLIENT_C_H

#ifdef __cplusplus
extern "C" {
#endif

typedef int dspr_bool;

struct dspr_client_t;
typedef struct dspr_client_t dspr_client_t;

dspr_client_t * dspr_client_new();
void dspr_client_delete( dspr_client_t * client );

void dspr_client_initialize( dspr_client_t * self );

void dspr_client_set_sample_rate( dspr_client_t * self, unsigned sample_rate );
unsigned dspr_client_sample_rate( dspr_client_t * self );

void dspr_client_set_channels( dspr_client_t * self, unsigned channels );
unsigned dspr_client_channels( dspr_client_t * self );

void dspr_client_set_server_ip( dspr_client_t * self, const char * ip );
char * dspr_client_server_ip( dspr_client_t * self );

void dspr_client_set_server_port( dspr_client_t * self, unsigned port );
unsigned dspr_client_server_port( dspr_client_t * self );

dspr_bool dspr_client_activate( dspr_client_t * self );
void dspr_client_deactivate( dspr_client_t * self );

dspr_bool dspr_client_prepare_noninterleaved_float( dspr_client_t * self, unsigned samples );
dspr_bool dspr_client_prepare_noninterleaved_short( dspr_client_t * self, unsigned samples );

dspr_bool dspr_client_write_channel_float( dspr_client_t * self, const float * buffer );
dspr_bool dspr_client_read_channel_float( dspr_client_t * self, float * buffer );
dspr_bool dspr_client_write_channel_short( dspr_client_t * self, const short * buffer );
dspr_bool dspr_client_read_channel_short( dspr_client_t * self, short * buffer );

dspr_bool dspr_client_interleaved_write_float( dspr_client_t * self, const float * buffer, unsigned samples );
dspr_bool dspr_client_interleaved_read_float( dspr_client_t * self, float * buffer );
dspr_bool dspr_client_interleaved_write_short( dspr_client_t * self, const short * buffer, unsigned samples );
dspr_bool dspr_client_interleaved_read_short( dspr_client_t * self, short * buffer );

dspr_bool dspr_client_try_activate( dspr_client_t * self );
dspr_bool dspr_client_is_active( dspr_client_t * self );

#ifdef __cplusplus
}
#endif

#endif

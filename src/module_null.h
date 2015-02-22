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

#ifndef MODULE_NULL_H
#define MODULE_NULL_H

#include "processor.h"

class module_null_t : public module_t
{
    public:

#if 0
        virtual format_representation representation()
        {
            return format_float;
        }

        virtual format_interleaveness interleaveness()
        {
            return format_interleaved;
        }
#endif

        virtual bool inplace()
        {
            return true;
        }

        virtual meta_module_t * get_meta_module();
        virtual void process( const char * input, char * output, unsigned length, unsigned samples, unsigned channels, unsigned sample_rate, format_t format )
        {
#if 0
            float * p = ( float * )input;
            for( unsigned i = 0; i < samples * 2; i += 2 )
                p[ i ] = 0;
#endif
        }
};

class meta_module_null_t : public meta_module_t
{
    virtual const char * name()
    {
        return "null";
    }

    virtual const char * pretty_name()
    {
        return "Null Module";
    }

    virtual const char * version()
    {
        return "1.0";
    }

    virtual const char ** fields()
    {
        static const char * array[] = { 0 };
        return array;
    }

    virtual void construct_by_reference( module_t *& value )
    {
        value = new module_null_t();
    }
};

extern meta_module_null_t g_meta_module_null;

#endif

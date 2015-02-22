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

#include "sigpipe.h"

sigpipe_t :: sigpipe_t()
{
    #ifndef WINDOWS
        sigpipe_blocked = false;
    #endif
}

void sigpipe_t :: block()
{
    #ifndef WINDOWS
        if( sigpipe_blocked ) return;

        sigemptyset( &signal_set );
        sigaddset( &signal_set, SIGPIPE );
        pthread_sigmask( SIG_BLOCK, &signal_set, &signal_old );
        /* sigprocmask( SIG_BLOCK, &signal_set, &signal_old ); */

        sigpipe_blocked = true;
    #endif
}

void sigpipe_t :: unblock()
{
    #ifndef WINDOWS
        if( sigpipe_blocked == false ) return;

        pthread_sigmask( SIG_SETMASK, &signal_old, NULL );
        /* sigprocmask( SIG_UNBLOCK, &signal_set, &signal_old ); */

        sigpipe_blocked = false;
    #endif
}

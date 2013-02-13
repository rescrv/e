// Copyright (c) 2011, Robert Escriva
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright notice,
//       this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of this project nor the names of its contributors may
//       be used to endorse or promote products derived from this software
//       without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef e_timer_h_
#define e_timer_h_

// C
#include <stdint.h>

#if HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef _MSC_VER
// Windows
#define _WINSOCKAPI_
#include <windows.h>
#endif

//mach
#ifdef HAVE_MACH_ABSOLUTE_TIME
#include <mach/mach_time.h>
#endif

// POSIX
#include <errno.h>
#include <time.h>

// STL
#include <exception>

// po6
#include <po6/error.h>

// e
#include "e/time.h"

uint64_t
e :: time()
{
#ifdef _MSC_VER
    LARGE_INTEGER tickfreq, timestamp;
    tickfreq.QuadPart = 0;
    timestamp.QuadPart = 0;
    QueryPerformanceFrequency((LARGE_INTEGER*)&tickfreq);
    QueryPerformanceCounter((LARGE_INTEGER*)&timestamp);
    return timestamp.QuadPart / (tickfreq.QuadPart/1000000000.0);
#elif defined HAVE_MACH_ABSOLUTE_TIME
    mach_timebase_info_data_t info;
    mach_timebase_info(&info);
    return mach_absolute_time()*info.numer/info.denom;
#else
    timespec ts;

    if (clock_gettime(CLOCK_REALTIME, &ts) < 0)
    {
        throw po6::error(errno);
    }

    return ts.tv_sec * 1000000000 + ts.tv_nsec;
#endif
}

#endif // e_timer_h__

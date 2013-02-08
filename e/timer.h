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

#ifdef _MSC_VER
// Windows 
#define _WINSOCKAPI_
#include <windows.h>
#endif


// POSIX
#include <errno.h>
#include <time.h>

// STL
#include <exception>

// po6
#include <po6/error.h>

#include <stdint.h>

namespace e
{
inline uint64_t
time()
{
#ifdef _MSC_VER
    LARGE_INTEGER tickfreq, timestamp;
    tickfreq.QuadPart = 0;
    timestamp.QuadPart = 0;
    QueryPerformanceFrequency((LARGE_INTEGER*)&tickfreq);
    QueryPerformanceCounter((LARGE_INTEGER*)&timestamp);
    return timestamp.QuadPart / (tickfreq.QuadPart/1000000000.0);
#else
    timespec ts;

    if (clock_gettime(CLOCK_REALTIME, &ts) < 0)
    {
        throw po6::error(errno);
    }

    return ts.tv_sec * 1000000000 + ts.tv_nsec;
#endif
}

// These sleep functions do not return early when interrupted by signals.
// If you give a value more than 1 second for ms, us, or ns, the resulting time
// slept will not necessarily be what you think.
inline void
sleep_ns(time_t s, long ns)
{
#ifdef _MSC_VER
    HANDLE ts = NULL;
    LARGE_INTEGER duetime;

    duetime.QuadPart = -1*(ns/100 + s*10000000);

    // Create an unnamed waitable timer.
    ts = CreateWaitableTimer(NULL, true, NULL);
    if (ts == NULL)
    {
        throw po6::error(GetLastError());
    }

    if(!SetWaitableTimer(ts, &duetime, 0, NULL, NULL, 0))
    {
        throw po6::error(GetLastError());
    }

    while(WaitForSingleObject(ts, INFINITE) != WAIT_OBJECT_0)
    {
        switch(GetLastError())
        {
            case EFAULT:
            case EINVAL:
            case ENOSYS:
                throw po6::error(errno);
            case EINTR:
                break;
            default:
                throw std::logic_error("SetWaitableTimer returned unexpected errno.");
        }
    }
#else
    timespec ts;
    timespec rem;
    ts.tv_sec = s;
    ts.tv_nsec = ns;

    while (nanosleep(&ts, &rem) < 0)
    {
        switch (errno)
        {
            case EFAULT:
            case EINVAL:
            case ENOSYS:
                throw po6::error(errno);
            case EINTR:
                memmove(&ts, &rem, sizeof(ts));
                break;
            default:
                throw std::logic_error("Nanosleep returned unexpected errno.");
        }
    }
#endif
}

inline void
sleep_ns(long ns)
{
    sleep_ns(0, ns);
}

inline void
sleep_us(time_t s, long us)
{
    sleep_ns(s, us * 1000);
}

inline void
sleep_us(long us)
{
    sleep_us(0, us);
}

inline void
sleep_ms(time_t s, long ms)
{
    sleep_us(s, ms * 1000);
}

inline void
sleep_ms(long ms)
{
    sleep_ms(0, ms);
}

class stopwatch
{
    public:
        stopwatch() : m_start() {}
        ~stopwatch() throw () {}

    public:
        void start() { reset(); }
        void reset()
        {
            m_start = time();
        }

        uint64_t resolution()
        {
            return 100;
        }

        uint64_t peek()
        {
            return time() - m_start;

        }

        uint64_t peek_ms()
        {
            return peek() / 1000000.;
        }

    private:
        uint64_t m_start;
};
        
} // namespace e

#endif // e_timer_win_h_

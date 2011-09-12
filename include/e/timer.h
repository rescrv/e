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

// POSIX
#include <errno.h>
#include <time.h>

// STL
#include <exception>

// po6
#include <po6/error.h>

namespace e
{

// These sleep functions do not return early when interrupted by signals.
// If you give a value more than 1 second for ms, us, or ns, the resulting time
// slept will not necessarily be what you think.

inline void
sleep_ns(time_t s, long ns)
{
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
            if (clock_gettime(CLOCK_REALTIME, &m_start) < 0)
            {
                throw po6::error(errno);
            }
        }

        uint64_t resolution()
        {
            timespec res;

            if (clock_getres(CLOCK_REALTIME, &res) < 0)
            {
                throw po6::error(errno);
            }

            return res.tv_sec * 1000000000 + res.tv_nsec;
        }

        uint64_t peek()
        {
            timespec end;

            if (clock_gettime(CLOCK_REALTIME, &end) < 0)
            {
                throw po6::error(errno);
            }

            timespec diff;

            if ((end.tv_nsec < m_start.tv_nsec) < 0)
            {
                diff.tv_sec = end.tv_sec - m_start.tv_sec - 1;
                diff.tv_nsec = 1000000000 + end.tv_nsec - m_start.tv_nsec;
            }
            else
            {
                diff.tv_sec = end.tv_sec - m_start.tv_sec;
                diff.tv_nsec = end.tv_nsec - m_start.tv_nsec;
            }

            return diff.tv_sec * 1000000000 + diff.tv_nsec;
        }

        uint64_t peek_ms()
        {
            return peek() / 1000000.;
        }

    private:
        timespec m_start;
};

} // namespace e

#endif // e_buffer_h_

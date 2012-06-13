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

#ifndef e_profiler_h_
#define e_profiler_h_

// C
#include <stdint.h>

// STL
#include <vector>

// e
#include <e/timer.h>

// This is a tool for profiling code.  It is designed to be similar to a
// stopwatch in that a call to ``start`` starts the timer.  Calls to ``measure``
// increment a shared counter by the number of nanoseconds between the call to
// ``start`` and the call to ``measure``.  The class is capable of tracking many
// different measurments (so that it may be used to track cumulative time
// throughout a request).  The ``name`` argument to the constructor should last
// longer than the lifetime of this object.  There is an overhead of 1us per
// call.
//
// At destruction time, the profiler reports to std::cerr with the observed
// results.  The results are converted to microseconds.

namespace e
{

class profiler
{
    public:
        class pathtimer
        {
            public:
                pathtimer(const pathtimer& other);
                ~pathtimer() throw ();

            public:
                void measure(size_t i);

            public:
                pathtimer& operator = (const pathtimer& rhs);

            private:
                friend class profiler;

            private:
                pathtimer(profiler* prof);

            private:
                profiler* m_prof;
                e::stopwatch m_stopw;
        };

    public:
        profiler(const char* name, size_t measurements);
        profiler(const profiler& other);
        ~profiler() throw ();

    public:
        pathtimer start();

    public:
        profiler& operator = (const profiler& rhs);

    private:
        friend class pathtimer;

    private:
        const char* m_name;
        std::vector<uint64_t> m_nanos;
        std::vector<uint64_t> m_count;
};

inline
profiler :: profiler(const char* name, size_t measurements)
    : m_name(name)
    , m_nanos(measurements, 0)
    , m_count(measurements, 0)
{
}

inline
profiler :: profiler(const profiler& other)
    : m_name(other.m_name)
    , m_nanos(other.m_nanos)
    , m_count(other.m_count)
{
}

inline
profiler :: ~profiler() throw ()
{
    std::cerr << "Profile of \"" << m_name << "\"\n";
    std::cerr << "Timer\tOps\tMicros/op\n";

    for (size_t i = 0; i < m_nanos.size(); ++i)
    {
        double micros = m_nanos[i];
        micros /= 1000;

        std::cerr << i << "\t" << m_count[i] << "\t";

        if (m_count[i] == 0)
        {
            std::cerr << "N/A\n";
        }
        else
        {
            std::cerr << micros / m_count[i] << "\n";
        }
    }

    std::cerr << std::flush;
}

inline profiler::pathtimer
profiler :: start()
{
    return pathtimer(this);
}

inline profiler&
profiler :: operator = (const profiler& rhs)
{
    m_name = rhs.m_name;
    m_nanos = rhs.m_nanos;
    m_count = rhs.m_count;
    return *this;
}

inline
profiler :: pathtimer :: pathtimer(const pathtimer& other)
    : m_prof(other.m_prof)
    , m_stopw(other.m_stopw)
{
}

inline
profiler :: pathtimer :: ~pathtimer() throw ()
{
}

inline void
profiler :: pathtimer :: measure(size_t i)
{
    __sync_add_and_fetch(&m_prof->m_nanos[i], m_stopw.peek());
    __sync_add_and_fetch(&m_prof->m_count[i], 1);
}

inline profiler::pathtimer&
profiler :: pathtimer :: operator = (const pathtimer& rhs)
{
    m_prof = rhs.m_prof;
    m_stopw = rhs.m_stopw;
    return *this;
}

inline
profiler :: pathtimer :: pathtimer(profiler* prof)
    : m_prof(prof)
    , m_stopw()
{
    m_stopw.start();
}

} // namespace e

#endif // e_profiler_h_

// Copyright (c) 2012, Robert Escriva
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

#ifndef e_microbench_h_
#define e_microbench_h_

#include <cassert>
#include <cstring>
#include <stdint.h>

#include <algorithm>
#include <iostream>
#include <vector>

namespace e
{

#define RDTSC(a, d) \
    __asm__ volatile("rdtsc\n" : "=a"(a), "=d"(d) : : )
#define CPUID(a, b, c, d) \
    __asm__ volatile("cpuid\n" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : : )

class microbench
{
    public:
        microbench(size_t trials);
        ~microbench() throw ();

    public:
        uint64_t max() const;
        uint64_t mean() const;
        uint64_t median() const;
        uint64_t min() const;
        uint64_t percentile(double p) const;
        void print(std::ostream& out) const;

    public:
        volatile void start(size_t i) __attribute__ ((noinline));
        volatile void end(size_t i) __attribute__ ((noinline));

    private:
        uint64_t trial(size_t i) const;

    private:
        size_t m_trials;
        uint32_t* m_numbers;
};

inline
microbench :: microbench(size_t trials)
    : m_trials(trials)
    , m_numbers(new uint32_t[trials * 4])
{
    memset(m_numbers, 0, trials * 4 * sizeof(uint32_t));
}

inline
microbench :: ~microbench() throw ()
{
    if (m_numbers)
    {
        delete[] m_numbers;
    }
}

inline uint64_t
microbench :: max() const
{
    uint64_t res = 0;

    for (size_t i = 0; i < m_trials; ++i)
    {
        res = std::max(res, trial(i));
    }

    return res;
}

inline uint64_t
microbench :: mean() const
{
    uint64_t sum = 0;

    for (size_t i = 0; i < m_trials; ++i)
    {
        sum += trial(i);
    }

    return sum / m_trials;
}

inline uint64_t
microbench :: median() const
{
    std::vector<uint64_t> v;
    v.reserve(m_trials);

    for (size_t i = 0; i < m_trials; ++i)
    {
        v.push_back(trial(i));
    }

    std::sort(v.begin(), v.end());

    if (v.size() % 2 == 0)
    {
        uint64_t a = v[v.size() / 2];
        uint64_t b = v[v.size() / 2 + 1];
        return (a / 2) + (b / 2);
    }
    else
    {
        return v[v.size() / 2];
    }
}

inline uint64_t
microbench :: min() const
{
    uint64_t res = 0;

    for (size_t i = 0; i < m_trials; ++i)
    {
        res = std::max(res, trial(i));
    }

    return res;
}

inline uint64_t
microbench :: percentile(double p) const
{
    std::vector<uint64_t> v;
    v.reserve(m_trials);

    for (size_t i = 0; i < m_trials; ++i)
    {
        v.push_back(trial(i));
    }

    std::sort(v.begin(), v.end());
    return v[m_trials * p];
}

inline void
microbench :: print(std::ostream& out) const
{
    out << "Trials:  " << m_trials << "\n"
        << "Mean:    " << mean() << "\n"
        << " 0.1%:   " << percentile(0.001) << "\n"
        << " 1.0%:   " << percentile(0.01) << "\n"
        << " 5.0%:   " << percentile(0.05) << "\n"
        << "95.0%:   " << percentile(0.95) << "\n"
        << "99.0%:   " << percentile(0.99) << "\n"
        << "99.9%:   " << percentile(0.999) << "\n"
        << std::flush;
}

volatile void
microbench :: start(size_t i)
{
    // Grabbing the pointers before calling rdtsc results in 3 instructions
    // after the rdtsc instruction as opposed to the 6 that result from calling
    // RDTSC(m_numbers[4 * i], m_numbers[4 * i + 1]) directly.
    uint32_t* a = m_numbers + 4 * i;
    uint32_t* d = m_numbers + 4 * i + 1;
    RDTSC(*a, *d);
    uint64_t xa;
    uint64_t xb;
    uint64_t xc;
    uint64_t xd;
    CPUID(xa, xb, xc, xd);
}

volatile void
microbench :: end(size_t i)
{
    uint64_t xa;
    uint64_t xb;
    uint64_t xc;
    uint64_t xd;
    CPUID(xa, xb, xc, xd);
    uint32_t a;
    uint32_t d;
    RDTSC(a, d);
    m_numbers[4 * i + 2] = a;
    m_numbers[4 * i + 3] = d;
}

uint64_t
microbench :: trial(size_t i) const
{
    uint64_t end = m_numbers[4 * i + 3];
    end <<= 32;
    end += m_numbers[4 * i + 2];
    uint64_t start = m_numbers[4 * i + 1];
    start <<= 32;
    start += m_numbers[4 * i];
    return end - start;
}

#undef RDTSC
#undef CPUID

} // namespace e

#endif // e_microbench_h_

// Copyright (c) 2014, Robert Escriva
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

#define __STDC_LIMIT_MACROS

// e
#include "e/atomic.h"
#include "e/seqno_collector.h"

using namespace e::atomic;
using e::seqno_collector;

struct seqno_collector::run
{
    run() { for (size_t i = 0; i < 8; ++i) { nums[i] = 0; } }
    uint64_t nums[8];
} __attribute__ ((aligned (64)));

seqno_collector :: seqno_collector(garbage_collector* gc)
    : m_gc(gc)
    , m_runs(gc)
    , m_lb_hint(0)
{
    assert(sizeof(run) == 64);
}

seqno_collector :: ~seqno_collector() throw ()
{
    for (run_map_t::iterator it = m_runs.begin(); it != m_runs.end(); ++it)
    {
        if (m_runs.del(it->first))
        {
            m_gc->collect(it->second, garbage_collector::free_ptr<run>);
        }
    }
}

void
seqno_collector :: collect(uint64_t seqno)
{
    const uint64_t idx = seqno & ~511ULL;
    run* r = get_run(idx);
    collect(seqno, idx, r);
}

void
seqno_collector :: collect_up_to(uint64_t seqno)
{
    assert(seqno < UINT64_MAX);
    const uint64_t idx = seqno & ~511ULL;
    run* r = get_run(idx);
    set_hint(idx);

    for (uint64_t i = idx; i < seqno; ++i)
    {
        collect(i, idx, r);
    }

    for (run_map_t::iterator it = m_runs.begin(); it != m_runs.end(); ++it)
    {
        if (false && it->first < idx &&
            m_runs.del(it->first))
        {
            m_gc->collect(it->second, garbage_collector::free_ptr<run>);
        }
    }
}

void
seqno_collector :: lower_bound(uint64_t* seqno)
{
    while (true)
    {
        uint64_t lb = load_64_nobarrier(&m_lb_hint);
        run* r = NULL;

        if (!m_runs.get(lb, &r))
        {
            *seqno = lb;
            return;
        }

        assert(r);
        size_t i = 0;
        uint64_t witness = 0;

        for (; i < 8; ++i)
        {
            if ((witness = compare_and_swap_64_nobarrier(&r->nums[i], UINT64_MAX, UINT64_MAX)) != UINT64_MAX)
            {
                break;
            }
        }

        if (i >= 8)
        {
            continue;
        }

        *seqno = lb + i * 64;

        while ((witness & 1))
        {
            ++*seqno;
            witness >>= 1;
        }

        return;
    }
}

seqno_collector::run*
seqno_collector :: get_run(uint64_t idx)
{
    run* r = NULL;

    // fill in r with a run that's in the table for idx
    while (true)
    {
        if (!m_runs.get(idx, &r))
        {
            r = new run();

            if (m_runs.put_ine(idx, r))
            {
                break;
            }

            delete r;
            r = NULL;
            continue;
        }
        else
        {
            break;
        }
    }

    assert(r);
    return r;
}

void
seqno_collector :: collect(uint64_t seqno, uint64_t idx, run* r)
{
    const uint64_t diff = (seqno - idx);
    const uint64_t byte = diff >> 6;
    const uint64_t bit  = diff & 63ULL;

    uint64_t expect = load_64_nobarrier(&r->nums[byte]);
    uint64_t newval = expect | (1ULL << bit);
    uint64_t witness;

    while ((witness = compare_and_swap_64_nobarrier(&r->nums[byte], expect, newval)) != expect)
    {
        expect = witness;
        newval = expect | (1ULL << bit);
    }

    if (newval == UINT64_MAX)
    {
        compress(idx, r);
    }
}

void
seqno_collector :: compress(uint64_t idx, run* r)
{
    for (size_t i = 0; i < 8; ++i)
    {
        if (compare_and_swap_64_nobarrier(&r->nums[i], UINT64_MAX, UINT64_MAX) != UINT64_MAX)
        {
            return;
        }
    }

    if (load_64_nobarrier(&m_lb_hint) != idx)
    {
        return;
    }

    set_hint(idx + 512);

    if (m_runs.del(idx))
    {
        m_gc->collect(r, garbage_collector::free_ptr<run>);
        r = get_run(idx + 512);
        compress(idx + 512, r);
    }
}

void
seqno_collector :: set_hint(uint64_t idx)
{
    uint64_t expect = load_64_nobarrier(&m_lb_hint);
    uint64_t newval = idx;
    uint64_t witness;

    while (expect < newval &&
           (witness = compare_and_swap_64_nobarrier(&m_lb_hint, expect, newval)) != expect)
    {
        expect = witness;
    }
}

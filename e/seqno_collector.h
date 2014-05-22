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

#ifndef e_seqno_collector_h_
#define e_seqno_collector_h_

// C
#include <stdint.h>

// e
#include <e/garbage_collector.h>
#include <e/nwf_hash_map.h>

// collect sequential identifiers in non-sequential order efficiently.
// valid identifiers are any uint64_t < UINT64_MAX.
// start counting at 0, and work upwards.

namespace e
{

class seqno_collector
{
    public:
        seqno_collector(garbage_collector* gc);
        ~seqno_collector() throw ();

    public:
        void collect(uint64_t seqno);
        void collect_up_to(uint64_t seqno);
        void lower_bound(uint64_t* seqno);

    private:
        struct run;

    private:
        run* get_run(uint64_t idx);
        void collect(uint64_t seqno, uint64_t idx, run* r);
        void compress(uint64_t idx, run* r);
        void set_hint(uint64_t idx);

    private:
        static uint64_t id(const uint64_t& x) { return x; }
        typedef nwf_hash_map<uint64_t, run*, id> run_map_t;
        garbage_collector* m_gc;
        run_map_t m_runs;
        uint64_t m_lb_hint;

    private:
        seqno_collector(const seqno_collector&);
        seqno_collector& operator = (const seqno_collector&);
};

} // namespace e

#endif // e_seqno_collector_h_

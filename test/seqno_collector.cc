// Copyright (c) 2013, Cornell University
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

// e
#include "th.h"
#include "e/seqno_collector.h"

TEST(SeqnoCollector, Test)
{
    e::garbage_collector gc;
    e::garbage_collector::thread_state ts;
    gc.register_thread(&ts);
    e::seqno_collector sc(&gc);
    uint64_t id;
    // first try, nothing collected
    sc.lower_bound(&id);
    ASSERT_EQ(id, 0U);
    // collect zero
    sc.collect(0);
    sc.lower_bound(&id);
    ASSERT_EQ(id, 1U);
    // collect one
    sc.collect(1);
    sc.lower_bound(&id);
    ASSERT_EQ(id, 2U);
    // collect three
    sc.collect(3);
    sc.lower_bound(&id);
    ASSERT_EQ(id, 2U);
    // collect three again
    sc.collect(3);
    sc.lower_bound(&id);
    ASSERT_EQ(id, 2U);
    // collect two
    sc.collect(2);
    sc.lower_bound(&id);
    ASSERT_EQ(id, 4U);
    // collect_up_to!
    sc.collect_up_to(9);
    sc.lower_bound(&id);
    ASSERT_EQ(id, 9U);
    sc.lower_bound(&id);
    ASSERT_EQ(id, 9U);

    for (uint64_t i = 9; i < 65536; ++i)
    {
        sc.collect(i);
        sc.lower_bound(&id);
        ASSERT_EQ(id, i + 1);
    }

    for (uint64_t i = 65536 + 512; i < 65536 + 1024; ++i)
    {
        sc.collect(i);
        sc.lower_bound(&id);
        ASSERT_EQ(id, 65536);
    }

    for (uint64_t i = 65536; i < 65536 + 511; ++i)
    {
        sc.collect(i);
        sc.lower_bound(&id);
        ASSERT_EQ(id, i + 1);
    }

    sc.collect(65536 + 511);
    sc.lower_bound(&id);
    ASSERT_EQ(id, 65536 + 1024);

    gc.quiescent_state(&ts);
    gc.deregister_thread(&ts);
}

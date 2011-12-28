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

// C
#include <stdint.h>

// Google Test
#include <gtest/gtest.h>

// po6
#include <e/bitsteal.h>

#pragma GCC diagnostic ignored "-Wswitch-default"

namespace
{

TEST(BitStealingTest, Strip)
{
    uintptr_t v = 0xdeadbeefcafebabeul;
    uint64_t* p = reinterpret_cast<uint64_t*>(v);
    p = e::bitsteal::strip(p);
    v = reinterpret_cast<uintptr_t>(p);
    EXPECT_EQ(0x0000beefcafebabeul, v);
}

TEST(BitStealingTest, SetAndUnset)
{
    using e::bitsteal::set;
    using e::bitsteal::unset;
    uintptr_t v1 = 0x0000beefcafebabeULL;
    uint64_t* p1 = reinterpret_cast<uint64_t*>(v1);
    uintptr_t v2 = 0xffffbeefcafebabeULL;
    uint64_t* p2 = reinterpret_cast<uint64_t*>(v2);
    p1 = set(p1, 0); p2 = set(p2, 0);
    p1 = unset(p1, 1); p2 = unset(p2, 1);
    p1 = set(p1, 2); p2 = set(p2, 2);
    p1 = set(p1, 3); p2 = set(p2, 3);
    p1 = unset(p1, 4); p2 = unset(p2, 4);
    p1 = set(p1, 5); p2 = set(p2, 5);
    p1 = unset(p1, 6); p2 = unset(p2, 6);
    p1 = set(p1, 7); p2 = set(p2, 7);
    p1 = unset(p1, 8); p2 = unset(p2, 8);
    p1 = set(p1, 9); p2 = set(p2, 9);
    p1 = set(p1, 10); p2 = set(p2, 10);
    p1 = set(p1, 11); p2 = set(p2, 11);
    p1 = set(p1, 12); p2 = set(p2, 12);
    p1 = unset(p1, 13); p2 = unset(p2, 13);
    p1 = set(p1, 14); p2 = set(p2, 14);
    p1 = set(p1, 15); p2 = set(p2, 15);
    EXPECT_EQ(p1, p2);
    EXPECT_EQ(0xdeadbeefcafebabeul, reinterpret_cast<uintptr_t>(p1));
    EXPECT_EQ(0xdeadbeefcafebabeul, reinterpret_cast<uintptr_t>(p2));
}

} // namespace

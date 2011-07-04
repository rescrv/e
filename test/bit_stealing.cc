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
#include <e/bit_stealing.h>

#pragma GCC diagnostic ignored "-Wswitch-default"

namespace
{

TEST(BitStealingTest, Strip)
{
    uintptr_t v = 0xdeadbeefcafebabeul;
    uint64_t* p = reinterpret_cast<uint64_t*>(v);
    p = e::bit_stealing::strip(p);
    v = reinterpret_cast<uintptr_t>(p);
    EXPECT_EQ(0x0000beefcafebabeul, v);
}

TEST(BitStealingTest, Get)
{
    uintptr_t v = 0xdeadbeefcafebabeul;
    uint64_t* p = reinterpret_cast<uint64_t*>(v);
    EXPECT_EQ(0xdead, e::bit_stealing::get(p));
}

TEST(BitStealingTest, SetAndUnset)
{
    using e::bit_stealing::set;
    using e::bit_stealing::unset;
    uintptr_t v1 = 0x0000beefcafebabeul;
    uint64_t* p1 = reinterpret_cast<uint64_t*>(v1);
    uintptr_t v2 = 0xffffbeefcafebabeul;
    uint64_t* p2 = reinterpret_cast<uint64_t*>(v2);
    set(p1, 0); set(p2, 0);
    unset(p1, 1); unset(p2, 1);
    set(p1, 2); set(p2, 2);
    set(p1, 3); set(p2, 3);
    unset(p1, 4); unset(p2, 4);
    set(p1, 5); set(p2, 5);
    unset(p1, 6); unset(p2, 6);
    set(p1, 7); set(p2, 7);
    unset(p1, 8); unset(p2, 8);
    set(p1, 9); set(p2, 9);
    set(p1, 10); set(p2, 10);
    set(p1, 11); set(p2, 11);
    set(p1, 12); set(p2, 12);
    unset(p1, 13); unset(p2, 13);
    set(p1, 14); set(p2, 14);
    set(p1, 15); set(p2, 15);
    EXPECT_EQ(p1, p2);
    EXPECT_EQ(0xdeadbeefcafebabeul, reinterpret_cast<uintptr_t>(p1));
    EXPECT_EQ(0xdeadbeefcafebabeul, reinterpret_cast<uintptr_t>(p2));
}

} // namespace

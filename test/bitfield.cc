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

// Google Test
#include <gtest/gtest.h>

// e
#include <e/bitfield.h>

#pragma GCC diagnostic ignored "-Wswitch-default"

namespace
{

TEST(BitfieldTest, CtorAndDtor)
{
    e::bitfield b1(8);
    EXPECT_EQ(8, b1.bits());
    EXPECT_EQ(1, b1.bytes());
    e::bitfield b2(16);
    EXPECT_EQ(16, b2.bits());
    EXPECT_EQ(2, b2.bytes());
    e::bitfield b3(75);
    EXPECT_EQ(75, b3.bits());
    EXPECT_EQ(10, b3.bytes());
}

static void
all_but_one(const e::bitfield& b, size_t which, bool all_others)
{
    for (size_t i = 0; i < b.bits(); ++i)
    {
        if (i == which)
        {
            EXPECT_NE(all_others, b.get(i));
        }
        else
        {
            EXPECT_EQ(all_others, b.get(i));
        }
    }
}

static void
rolling_bitfield(size_t size)
{
    e::bitfield b(size);

    for (size_t i = 0; i < size; ++i)
    {
        b.set(i);
        all_but_one(b, i, false);
        b.unset(i);
    }

    for (size_t i = 0; i < size; ++i)
    {
        b.set(i);
    }

    for (size_t i = 0; i < size; ++i)
    {
        b.unset(i);
        all_but_one(b, i, true);
        b.set(i);
    }
}

TEST(BitfieldTest, RollingBitfield)
{
    rolling_bitfield(8);
    rolling_bitfield(16);
    rolling_bitfield(75);
}

}

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

// STL
#include <memory>

// Google Test
#include <gtest/gtest.h>

// e
#include "../include/e/bitfield.h"
#include "../include/e/buffer.h"

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

TEST(BitfieldTest, BufferPack)
{
    e::bitfield bf(32);
    // 0xde
    bf.set(1);
    bf.set(2);
    bf.set(3);
    bf.set(4);
    bf.set(6);
    bf.set(7);

    // 0xad
    bf.set(8);
    bf.set(10);
    bf.set(11);
    bf.set(13);
    bf.set(15);

    // 0xbe
    bf.set(17);
    bf.set(18);
    bf.set(19);
    bf.set(20);
    bf.set(21);
    bf.set(23);

    // 0xef
    bf.set(24);
    bf.set(25);
    bf.set(26);
    bf.set(27);
    bf.set(29);
    bf.set(30);
    bf.set(31);

    std::auto_ptr<e::buffer> buf(e::buffer::create(12));
    *buf << bf;
    EXPECT_TRUE(buf->cmp("\x00\x00\x00\x20"
                         "\x00\x00\x00\x04"
                         "\xde\xad\xbe\xef", 12));
}

TEST(BitfieldTest, BufferUnpack)
{
    e::bitfield bf(32);
    std::auto_ptr<e::buffer> buf(e::buffer::create("\x00\x00\x00\x20"
                                                   "\x00\x00\x00\x04"
                                                   "\xde\xad\xbe\xef", 12));
    *buf >> bf;

    // 0xde
    EXPECT_FALSE(bf.get(0));
    EXPECT_TRUE(bf.get(1));
    EXPECT_TRUE(bf.get(2));
    EXPECT_TRUE(bf.get(3));
    EXPECT_TRUE(bf.get(4));
    EXPECT_FALSE(bf.get(5));
    EXPECT_TRUE(bf.get(6));
    EXPECT_TRUE(bf.get(7));

    // 0xad
    EXPECT_TRUE(bf.get(8));
    EXPECT_FALSE(bf.get(9));
    EXPECT_TRUE(bf.get(10));
    EXPECT_TRUE(bf.get(11));
    EXPECT_FALSE(bf.get(12));
    EXPECT_TRUE(bf.get(13));
    EXPECT_FALSE(bf.get(14));
    EXPECT_TRUE(bf.get(15));

    // 0xbe
    EXPECT_FALSE(bf.get(16));
    EXPECT_TRUE(bf.get(17));
    EXPECT_TRUE(bf.get(18));
    EXPECT_TRUE(bf.get(19));
    EXPECT_TRUE(bf.get(20));
    EXPECT_TRUE(bf.get(21));
    EXPECT_FALSE(bf.get(22));
    EXPECT_TRUE(bf.get(23));

    // 0xef
    EXPECT_TRUE(bf.get(24));
    EXPECT_TRUE(bf.get(25));
    EXPECT_TRUE(bf.get(26));
    EXPECT_TRUE(bf.get(27));
    EXPECT_FALSE(bf.get(28));
    EXPECT_TRUE(bf.get(29));
    EXPECT_TRUE(bf.get(30));
    EXPECT_TRUE(bf.get(31));
}

}

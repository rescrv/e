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
#include <e/buffer.h>

#pragma GCC diagnostic ignored "-Wswitch-default"

namespace
{

TEST(BufferTest, CtorAndDtor)
{
    e::buffer a; // Create a buffer without any size.
    e::buffer b(3); // Create a buffer which can pack 3 bytes without resizing.
    e::buffer c("xyz", 3); // Create a buffer with the three bytes "XYZ".
    e::buffer d(reinterpret_cast<const void*>("xyz"), 3);
}

TEST(BufferTest, PackBuffer)
{
    uint64_t a = 0xdeadbeefcafebabe;
    uint32_t b = 0x8badf00d;
    uint16_t c = 0xface;
    uint8_t d = '!';
    e::buffer buf("the buffer", 10);
    e::buffer packed;

    packed.pack() << a << b << c << d << e::buffer::padding(5) << buf;
    EXPECT_EQ(0,
        memcmp(packed.get(), "\xde\xad\xbe\xef\xca\xfe\xba\xbe"
                             "\x8b\xad\xf0\x0d"
                             "\xfa\xce"
                             "!"
                             "\x00\x00\x00\x00\x00"
                             "the buffer", 30));
}

TEST(BufferTest, UnpackBuffer)
{
    uint64_t a;
    uint32_t b;
    uint16_t c;
    uint8_t d;
    e::buffer buf;
    e::buffer packed("\xde\xad\xbe\xef\xca\xfe\xba\xbe"
                     "\x8b\xad\xf0\x0d"
                     "\xfa\xce"
                     "!"
                     "\x00\x00\x00\x00\x00"
                     "the buffer", 30);
    packed.unpack() >> a >> b >> c >> d >> e::buffer::padding(5) >> buf;
    EXPECT_EQ(0xdeadbeefcafebabe, a);
    EXPECT_EQ(0x8badf00d, b);
    EXPECT_EQ(0xface, c);
    EXPECT_EQ('!', d);
    EXPECT_EQ(10, buf.size());
    EXPECT_TRUE(e::buffer("the buffer", 10) == buf);
}

TEST(BufferTest, UnpackErrors)
{
    bool caught = false;

    e::buffer buf("\x8b\xad\xf0\x0d" "\xfa\xce", 6);
    uint32_t a;
    e::unpacker u(buf);
    u >> a;
    EXPECT_EQ(0x8badf00d, a);

    try
    {
        u >> a;
    }
    catch (std::out_of_range& e)
    {
        caught = true;
    }

    EXPECT_TRUE(caught);
    // "a" should not change
    EXPECT_EQ(0x8badf00d, a);
    // Getting the next value should succeed
    uint16_t b;
    u >> b;
    EXPECT_EQ(0xface, b);
}

TEST(BufferTest, TrimPrefix)
{
    e::buffer buf("\xde\xad\xbe\xef", 4);
    EXPECT_EQ(4, buf.size());
    EXPECT_FALSE(buf.empty());
    buf.trim_prefix(2);
    EXPECT_TRUE(e::buffer("\xbe\xef", 2) == buf);
    EXPECT_EQ(2, buf.size());
    EXPECT_FALSE(buf.empty());
    buf.trim_prefix(4);
    EXPECT_TRUE(e::buffer() == buf);
    EXPECT_EQ(0, buf.size());
    EXPECT_TRUE(buf.empty());
}

TEST(BufferTest, Index)
{
    e::buffer buf("0123456789", 10);
    EXPECT_EQ(0, buf.index('0'));
    EXPECT_EQ(1, buf.index('1'));
    EXPECT_EQ(2, buf.index('2'));
    EXPECT_EQ(3, buf.index('3'));
    EXPECT_EQ(4, buf.index('4'));
    EXPECT_EQ(5, buf.index('5'));
    EXPECT_EQ(6, buf.index('6'));
    EXPECT_EQ(7, buf.index('7'));
    EXPECT_EQ(8, buf.index('8'));
    EXPECT_EQ(9, buf.index('9'));
    EXPECT_EQ(10, buf.index('A')); // It's not there.
    EXPECT_EQ(10, buf.index('B')); // It's not there.
}

TEST(BufferTest, Contains)
{
    e::buffer buf("0123456789", 10);
    EXPECT_TRUE(buf.contains('0'));
    EXPECT_TRUE(buf.contains('1'));
    EXPECT_TRUE(buf.contains('2'));
    EXPECT_TRUE(buf.contains('3'));
    EXPECT_TRUE(buf.contains('4'));
    EXPECT_TRUE(buf.contains('5'));
    EXPECT_TRUE(buf.contains('6'));
    EXPECT_TRUE(buf.contains('7'));
    EXPECT_TRUE(buf.contains('8'));
    EXPECT_TRUE(buf.contains('9'));
    EXPECT_FALSE(buf.contains('A')); // It's not there.
    EXPECT_FALSE(buf.contains('B')); // It's not there.
}

TEST(BufferTest, UnpackSize)
{
    e::buffer hello;
    e::buffer world;
    e::buffer packed("hello world", 11);
    packed.unpack() >> e::buffer::sized(static_cast<size_t>(5), &hello)
                    >> e::buffer::padding(1)
                    >> e::buffer::sized(static_cast<size_t>(5), &world);
    EXPECT_TRUE(e::buffer("hello", 5) == hello);
    EXPECT_TRUE(e::buffer("world", 5) == world);
}

} // namespace

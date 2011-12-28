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

// C++
#include <memory>

// Google Test
#include <gtest/gtest.h>

// e
#include "../include/e/buffer.h"

#define EXPECT_MEMCMP(X, Y, S) EXPECT_EQ(0, memcmp(X, Y, S))
#pragma GCC diagnostic ignored "-Wswitch-default"

namespace
{

TEST(BufferTest, CtorAndDtor)
{
    // Create a buffer without any size
    std::auto_ptr<e::buffer> a(e::buffer::create(0));
    EXPECT_EQ(0, a->size());
    EXPECT_EQ(0, a->capacity());
    // Create a buffer which can pack 2 bytes
    std::auto_ptr<e::buffer> b(e::buffer::create(2));
    EXPECT_EQ(0, b->size());
    EXPECT_EQ(2, b->capacity());
    // Create a buffer with the three bytes "XYZ"
    std::auto_ptr<e::buffer> c(e::buffer::create("xyz", 3));
    EXPECT_EQ(3, c->size());
    EXPECT_EQ(3, c->capacity());
}

TEST(BufferTest, PackBuffer)
{
    uint64_t a = 0xdeadbeefcafebabe;
    uint32_t b = 0x8badf00d;
    uint16_t c = 0xface;
    uint8_t d = '!';
    std::auto_ptr<e::buffer> buf(e::buffer::create("the buffer", 10));
    std::auto_ptr<e::buffer> packed(e::buffer::create(34));

    *packed << a << b << c << d << e::buffer::padding(5) << buf->as_slice();
    EXPECT_EQ(34, packed->size());
    EXPECT_MEMCMP(packed->data(),
                  "\xde\xad\xbe\xef\xca\xfe\xba\xbe"
                  "\x8b\xad\xf0\x0d"
                  "\xfa\xce"
                  "!"
                  "\x00\x00\x00\x00\x00"
                  "\x00\x00\x00\x0athe buffer",
                  34);

    packed->pack_at(12) << d << c << buf->as_slice() << e::buffer::padding(4);
    EXPECT_EQ(34, packed->size());
    EXPECT_MEMCMP(packed->data(),
                  "\xde\xad\xbe\xef\xca\xfe\xba\xbe"
                  "\x8b\xad\xf0\x0d"
                  "!"
                  "\xfa\xce"
                  "\x00\x00\x00\x0athe buffer"
                  "uffer",
                  34);
}

TEST(BufferTest, UnpackBuffer)
{
    uint64_t a;
    uint32_t b;
    uint16_t c;
    uint8_t d;
    e::slice sl;
    std::auto_ptr<e::buffer> packed(e::buffer::create(
                "\xde\xad\xbe\xef\xca\xfe\xba\xbe"
                "\x8b\xad\xf0\x0d"
                "\xfa\xce"
                "!"
                "\x00\x00\x00\x00\x00"
                "\x00\x00\x00\x0athe buffer", 34));

    *packed >> a >> b >> c >> d >> e::buffer::padding(5) >> sl;
    EXPECT_EQ(0xdeadbeefcafebabe, a);
    EXPECT_EQ(0x8badf00d, b);
    EXPECT_EQ(0xface, c);
    EXPECT_EQ('!', d);
    EXPECT_EQ(10, sl.size());
    EXPECT_MEMCMP("the buffer", sl.data(), 10);
}

TEST(BufferTest, UnpackErrors)
{
    std::auto_ptr<e::buffer> buf(e::buffer::create("\x8b\xad\xf0\x0d" "\xfa\xce", 6));
    uint32_t a;
    e::buffer::unpacker up = *buf >> a;
    EXPECT_EQ(0x8badf00d, a);
    EXPECT_EQ(2, up.remain());
    EXPECT_FALSE(up.error());

    // "a" should not change even if nup fails
    e::buffer::unpacker nup = up >> a;
    EXPECT_EQ(0x8badf00d, a);
    EXPECT_EQ(2, up.remain());
    EXPECT_FALSE(up.error());
    EXPECT_TRUE(nup.error());

    // Getting the next value should succeed
    uint16_t b;
    up = up >> b;
    EXPECT_EQ(0xface, b);
    EXPECT_EQ(0, up.remain());
    EXPECT_FALSE(up.error());
}

TEST(BufferTest, Shift)
{
    // Create a buffer of four characters
    std::auto_ptr<e::buffer> buf(e::buffer::create("\xde\xad\xbe\xef", 4));
    EXPECT_EQ(4, buf->size());
    EXPECT_EQ(4, buf->capacity());
    EXPECT_TRUE(buf->cmp("\xde\xad\xbe\xef", 4));

    // Shift once
    buf->shift(2);
    EXPECT_EQ(2, buf->size());
    EXPECT_EQ(4, buf->capacity());
    EXPECT_TRUE(buf->cmp("\xbe\xef", 2));

    // Shift again
    buf->shift(2);
    EXPECT_EQ(0, buf->size());
    EXPECT_EQ(4, buf->capacity());
    EXPECT_TRUE(buf->cmp("", 0));
}

TEST(BufferTest, ShiftExcess)
{
    // Create a buffer of four characters
    std::auto_ptr<e::buffer> buf(e::buffer::create("\xde\xad\xbe\xef", 4));
    EXPECT_EQ(4, buf->size());
    EXPECT_EQ(4, buf->capacity());
    EXPECT_TRUE(buf->cmp("\xde\xad\xbe\xef", 4));

    // Shift once
    buf->shift(6);
    EXPECT_EQ(0, buf->size());
    EXPECT_EQ(4, buf->capacity());
    EXPECT_TRUE(buf->cmp("", 0));
}

TEST(BufferTest, Index)
{
    std::auto_ptr<e::buffer> buf(e::buffer::create("0123456789", 10));
    EXPECT_EQ(0, buf->index('0'));
    EXPECT_EQ(1, buf->index('1'));
    EXPECT_EQ(2, buf->index('2'));
    EXPECT_EQ(3, buf->index('3'));
    EXPECT_EQ(4, buf->index('4'));
    EXPECT_EQ(5, buf->index('5'));
    EXPECT_EQ(6, buf->index('6'));
    EXPECT_EQ(7, buf->index('7'));
    EXPECT_EQ(8, buf->index('8'));
    EXPECT_EQ(9, buf->index('9'));
    EXPECT_EQ(buf->capacity(), buf->index('A')); // It's not there.
    EXPECT_EQ(buf->capacity(), buf->index('B')); // It's not there.
}

TEST(BufferTest, Hex)
{
    std::auto_ptr<e::buffer> buf1(e::buffer::create("\xde\xad\xbe\xef", 4));
    std::auto_ptr<e::buffer> buf2(e::buffer::create("\x00\xff\x0f\xf0", 4));

    EXPECT_EQ("deadbeef", buf1->hex());
    EXPECT_EQ("00ff0ff0", buf2->hex());
}

TEST(BufferTest, VectorPack)
{
    std::auto_ptr<e::buffer> buf(e::buffer::create(12));
    std::vector<uint16_t> vector;
    vector.push_back(0xdead);
    vector.push_back(0xbeef);
    vector.push_back(0xcafe);
    vector.push_back(0xbabe);
    e::buffer::packer p = *buf << vector;
    EXPECT_TRUE(buf->cmp("\x00\x00\x00\x04"
                         "\xde\xad\xbe\xef"
                         "\xca\xfe\xba\xbe", 12));
}

TEST(BufferTest, VectorUnpack)
{
    std::auto_ptr<e::buffer> buf(e::buffer::create("\x00\x00\x00\x04"
                                                   "\xde\xad\xbe\xef"
                                                   "\xca\xfe\xba\xbe", 12));
    std::vector<uint16_t> vector;
    *buf >> vector;
    EXPECT_EQ(4, vector.size());
    EXPECT_EQ(0xdead, vector[0]);
    EXPECT_EQ(0xbeef, vector[1]);
    EXPECT_EQ(0xcafe, vector[2]);
    EXPECT_EQ(0xbabe, vector[3]);
}

TEST(BufferTest, VectorUnpackFail)
{
    std::auto_ptr<e::buffer> buf(e::buffer::create("\x00\x00\x00\x04"
                                                   "\xde\xad\xbe\xef"
                                                   "\xca\xfe\xba\xbe", 12));
    std::vector<uint32_t> vector_bad;
    std::vector<uint16_t> vector_good;
    e::buffer::unpacker bad = *buf >> vector_bad;
    e::buffer::unpacker good = *buf >> vector_good;
    EXPECT_TRUE(bad.error());
    EXPECT_FALSE(good.error());
    EXPECT_EQ(4, vector_good.size());
    EXPECT_EQ(0xdead, vector_good[0]);
    EXPECT_EQ(0xbeef, vector_good[1]);
    EXPECT_EQ(0xcafe, vector_good[2]);
    EXPECT_EQ(0xbabe, vector_good[3]);
}

} // namespace

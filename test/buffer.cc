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

// e
#include "th.h"
#include "e/buffer.h"

#define ASSERT_MEMCMP(X, Y, S) ASSERT_EQ(0, memcmp(X, Y, S))

namespace
{

TEST(BufferTest, CtorAndDtor)
{
    // Create a buffer without any size
    std::auto_ptr<e::buffer> a(e::buffer::create(0));
    ASSERT_EQ(0U, a->size());
    ASSERT_EQ(0U, a->capacity());
    // Create a buffer which can pack 2 bytes
    std::auto_ptr<e::buffer> b(e::buffer::create(2));
    ASSERT_EQ(0U, b->size());
    ASSERT_EQ(2U, b->capacity());
    // Create a buffer with the three bytes "XYZ"
    std::auto_ptr<e::buffer> c(e::buffer::create("xyz", 3));
    ASSERT_EQ(3U, c->size());
    ASSERT_EQ(3U, c->capacity());
}

TEST(BufferTest, PackBuffer)
{
    uint64_t a = 0xdeadbeefcafebabe;
    uint32_t b = 0x8badf00d;
    uint16_t c = 0xface;
    uint8_t d = '!';
    std::auto_ptr<e::buffer> buf(e::buffer::create("the buffer", 10));
    std::auto_ptr<e::buffer> packed(e::buffer::create(34));

    packed->pack() << a << b << c << d << buf->as_slice();
    ASSERT_EQ(26U, packed->size());
    ASSERT_MEMCMP(packed->data(),
                  "\xde\xad\xbe\xef\xca\xfe\xba\xbe"
                  "\x8b\xad\xf0\x0d"
                  "\xfa\xce"
                  "!"
                  "\x0athe buffer",
                  26);

    packed->pack_at(12) << d << c << buf->as_slice();
    ASSERT_EQ(26U, packed->size());
    ASSERT_MEMCMP(packed->data(),
                  "\xde\xad\xbe\xef\xca\xfe\xba\xbe"
                  "\x8b\xad\xf0\x0d"
                  "!"
                  "\xfa\xce"
                  "\x0athe buffer",
                  26);
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
                "\x0athe buffer", 26));

    packed->unpack() >> a >> b >> c >> d >> sl;
    ASSERT_EQ(0xdeadbeefcafebabeULL, a);
    ASSERT_EQ(0x8badf00dUL, b);
    ASSERT_EQ(0xface, c);
    ASSERT_EQ('!', d);
    ASSERT_EQ(10U, sl.size());
    ASSERT_MEMCMP("the buffer", sl.data(), 10);
}

TEST(BufferTest, UnpackErrors)
{
    std::auto_ptr<e::buffer> buf(e::buffer::create("\x8b\xad\xf0\x0d" "\xfa\xce", 6));
    uint32_t a;
    e::unpacker up = buf->unpack() >> a;
    ASSERT_EQ(0x8badf00d, a);
    ASSERT_EQ(2U, up.remain());
    ASSERT_FALSE(up.error());

    // "a" should not change even if nup fails
    e::unpacker nup = up >> a;
    ASSERT_EQ(0x8badf00d, a);
    ASSERT_EQ(2U, up.remain());
    ASSERT_FALSE(up.error());
    ASSERT_TRUE(nup.error());

    // Getting the next value should succeed
    uint16_t b;
    up = up >> b;
    ASSERT_EQ(0xface, b);
    ASSERT_EQ(0U, up.remain());
    ASSERT_FALSE(up.error());
}

TEST(BufferTest, Hex)
{
    std::auto_ptr<e::buffer> buf1(e::buffer::create("\xde\xad\xbe\xef", 4));
    std::auto_ptr<e::buffer> buf2(e::buffer::create("\x00\xff\x0f\xf0", 4));

    ASSERT_EQ("deadbeef", buf1->hex());
    ASSERT_EQ("00ff0ff0", buf2->hex());
}

TEST(BufferTest, VectorPack)
{
    std::auto_ptr<e::buffer> buf(e::buffer::create(12));
    std::vector<uint16_t> vector;
    vector.push_back(0xdead);
    vector.push_back(0xbeef);
    vector.push_back(0xcafe);
    vector.push_back(0xbabe);
    e::packer p = buf->pack_at(0);
    p = p << vector;
    ASSERT_TRUE(buf->cmp("\x04"
                         "\xde\xad\xbe\xef"
                         "\xca\xfe\xba\xbe", 9));
}

TEST(BufferTest, VectorUnpack)
{
    std::auto_ptr<e::buffer> buf(e::buffer::create("\x04"
                                                   "\xde\xad\xbe\xef"
                                                   "\xca\xfe\xba\xbe", 9));
    std::vector<uint16_t> vector;
    buf->unpack() >> vector;
    ASSERT_EQ(4U, vector.size());
    ASSERT_EQ(0xdead, vector[0]);
    ASSERT_EQ(0xbeef, vector[1]);
    ASSERT_EQ(0xcafe, vector[2]);
    ASSERT_EQ(0xbabe, vector[3]);
}

TEST(BufferTest, VectorUnpackFail)
{
    std::auto_ptr<e::buffer> buf(e::buffer::create("\x04"
                                                   "\xde\xad\xbe\xef"
                                                   "\xca\xfe\xba\xbe", 9));
    std::vector<uint32_t> vector_bad;
    std::vector<uint16_t> vector_good;
    e::unpacker bad = buf->unpack() >> vector_bad;
    e::unpacker good = buf->unpack() >> vector_good;
    ASSERT_TRUE(bad.error());
    ASSERT_FALSE(good.error());
    ASSERT_EQ(4U, vector_good.size());
    ASSERT_EQ(0xdead, vector_good[0]);
    ASSERT_EQ(0xbeef, vector_good[1]);
    ASSERT_EQ(0xcafe, vector_good[2]);
    ASSERT_EQ(0xbabe, vector_good[3]);
}

} // namespace

// Copyright (c) 2012, Robert Escriva
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
#include "e/endian.h"

#define EXPECT_MEMCMP(X, Y, S) EXPECT_EQ(0, memcmp(X, Y, S))
#pragma GCC diagnostic ignored "-Wswitch-default"

namespace
{

TEST(EndianTest, Pack)
{
    uint8_t buffer[sizeof(int64_t)];

    e::pack8be(uint8_t(0xde), buffer);
    EXPECT_MEMCMP(buffer, "\xde", 1);
    e::pack8le(uint8_t(0xde), buffer);
    EXPECT_MEMCMP(buffer, "\xde", 1);
    e::pack8be(int8_t(0xde), buffer);
    EXPECT_MEMCMP(buffer, "\xde", 1);
    e::pack8le(int8_t(0xde), buffer);
    EXPECT_MEMCMP(buffer, "\xde", 1);

    e::pack16be(uint16_t(0xdead), buffer);
    EXPECT_MEMCMP(buffer, "\xde\xad", 2);
    e::pack16le(uint16_t(0xdead), buffer);
    EXPECT_MEMCMP(buffer, "\xad\xde", 2);
    e::pack16be(int16_t(0xdead), buffer);
    EXPECT_MEMCMP(buffer, "\xde\xad", 2);
    e::pack16le(int16_t(0xdead), buffer);
    EXPECT_MEMCMP(buffer, "\xad\xde", 2);

    e::pack32be(uint32_t(0xdeadbeefUL), buffer);
    EXPECT_MEMCMP(buffer, "\xde\xad\xbe\xef", 4);
    e::pack32le(uint32_t(0xdeadbeefUL), buffer);
    EXPECT_MEMCMP(buffer, "\xef\xbe\xad\xde", 4);
    e::pack32be(int32_t(0xdeadbeefUL), buffer);
    EXPECT_MEMCMP(buffer, "\xde\xad\xbe\xef", 4);
    e::pack32le(int32_t(0xdeadbeefUL), buffer);
    EXPECT_MEMCMP(buffer, "\xef\xbe\xad\xde", 4);

    e::pack64be(uint64_t(0xdeadbeefcafebabeULL), buffer);
    EXPECT_MEMCMP(buffer, "\xde\xad\xbe\xef\xca\xfe\xba\xbe", 8);
    e::pack64le(uint64_t(0xdeadbeefcafebabeULL), buffer);
    EXPECT_MEMCMP(buffer, "\xbe\xba\xfe\xca\xef\xbe\xad\xde", 8);
    e::pack64be(int64_t(0xdeadbeefcafebabeULL), buffer);
    EXPECT_MEMCMP(buffer, "\xde\xad\xbe\xef\xca\xfe\xba\xbe", 8);
    e::pack64le(int64_t(0xdeadbeefcafebabeULL), buffer);
    EXPECT_MEMCMP(buffer, "\xbe\xba\xfe\xca\xef\xbe\xad\xde", 8);

    float f = 16711938.0;
    e::packfloatbe(f, buffer);
    EXPECT_MEMCMP(buffer, "\x4b\x7f\x01\x02", 4);
    e::packfloatle(f, buffer);
    EXPECT_MEMCMP(buffer, "\x02\x01\x7f\x4b", 4);

    double d = 9006104071832581.0;
    e::packdoublebe(d, buffer);
    EXPECT_MEMCMP(buffer, "\x43\x3f\xff\x01\x02\x03\x04\x05", 8);
    e::packdoublele(d, buffer);
    EXPECT_MEMCMP(buffer, "\x05\x04\x03\x02\x01\xff\x3f\x43", 8);
}

TEST(EndianTest, Unpack)
{
    const uint8_t buffer8[] = "\xde";
    const uint8_t buffer16[] = "\xde\xad";
    const uint8_t buffer32[] = "\xde\xad\xbe\xef";
    const uint8_t buffer64[] = "\xde\xad\xbe\xef\xca\xfe\xba\xbe";
    const uint8_t bufferfloatbe[] = "\x4b\x7f\x01\x02";
    const uint8_t bufferfloatle[] = "\x02\x01\x7f\x4b";
    const uint8_t bufferdoublebe[] = "\x43\x3f\xff\x01\x02\x03\x04\x05";
    const uint8_t bufferdoublele[] = "\x05\x04\x03\x02\x01\xff\x3f\x43";

    uint8_t unsigned8;
    int8_t signed8;
    uint16_t unsigned16;
    int16_t signed16;
    uint32_t unsigned32;
    int32_t signed32;
    uint64_t unsigned64;
    int64_t signed64;
    float f;
    double d;

    e::unpack8be(buffer8, &unsigned8);
    EXPECT_EQ(0xde, unsigned8);
    e::unpack8le(buffer8, &unsigned8);
    EXPECT_EQ(0xde, unsigned8);
    e::unpack8be(buffer8, &signed8);
    EXPECT_EQ(int8_t(0xde), signed8);
    e::unpack8le(buffer8, &signed8);
    EXPECT_EQ(int8_t(0xde), signed8);

    e::unpack16be(buffer16, &unsigned16);
    EXPECT_EQ(0xdead, unsigned16);
    e::unpack16le(buffer16, &unsigned16);
    EXPECT_EQ(0xadde, unsigned16);
    e::unpack16be(buffer16, &signed16);
    EXPECT_EQ(int16_t(0xdead), signed16);
    e::unpack16le(buffer16, &signed16);
    EXPECT_EQ(int16_t(0xadde), signed16);

    e::unpack32be(buffer32, &unsigned32);
    EXPECT_EQ(0xdeadbeefUL, unsigned32);
    e::unpack32le(buffer32, &unsigned32);
    EXPECT_EQ(0xefbeaddeUL, unsigned32);
    e::unpack32be(buffer32, &signed32);
    EXPECT_EQ(int32_t(0xdeadbeefUL), signed32);
    e::unpack32le(buffer32, &signed32);
    EXPECT_EQ(int32_t(0xefbeaddeUL), signed32);

    e::unpack64be(buffer64, &unsigned64);
    EXPECT_EQ(0xdeadbeefcafebabeULL, unsigned64);
    e::unpack64le(buffer64, &unsigned64);
    EXPECT_EQ(0xbebafecaefbeaddeULL, unsigned64);
    e::unpack64be(buffer64, &signed64);
    EXPECT_EQ(int64_t(0xdeadbeefcafebabeULL), signed64);
    e::unpack64le(buffer64, &signed64);
    EXPECT_EQ(int64_t(0xbebafecaefbeaddeULL), signed64);

    e::unpackfloatbe(bufferfloatbe, &f);
    EXPECT_EQ(16711938.0, f);
    e::unpackfloatle(bufferfloatle, &f);
    EXPECT_EQ(16711938.0, f);

    e::unpackdoublebe(bufferdoublebe, &d);
    EXPECT_EQ(9006104071832581.0, d);
    e::unpackdoublele(bufferdoublele, &d);
    EXPECT_EQ(9006104071832581.0, d);
}

} // namespace

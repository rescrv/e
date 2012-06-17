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

    e::pack8be(0xde, buffer);
    EXPECT_MEMCMP(buffer, "\xde", 1);
    e::pack8le(0xde, buffer);
    EXPECT_MEMCMP(buffer, "\xde", 1);

    e::pack16be(0xdead, buffer);
    EXPECT_MEMCMP(buffer, "\xde\xad", 2);
    e::pack16le(0xdead, buffer);
    EXPECT_MEMCMP(buffer, "\xad\xde", 2);

    e::pack32be(0xdeadbeefUL, buffer);
    EXPECT_MEMCMP(buffer, "\xde\xad\xbe\xef", 4);
    e::pack32le(0xdeadbeefUL, buffer);
    EXPECT_MEMCMP(buffer, "\xef\xbe\xad\xde", 4);

    e::pack64be(0xdeadbeefcafebabeULL, buffer);
    EXPECT_MEMCMP(buffer, "\xde\xad\xbe\xef\xca\xfe\xba\xbe", 8);
    e::pack64le(0xdeadbeefcafebabeULL, buffer);
    EXPECT_MEMCMP(buffer, "\xbe\xba\xfe\xca\xef\xbe\xad\xde", 8);
}

TEST(EndianTest, Unpack)
{
    const uint8_t buffer8[] = "\xde";
    const uint8_t buffer16[] = "\xde\xad";
    const uint8_t buffer32[] = "\xde\xad\xbe\xef";
    const uint8_t buffer64[] = "\xde\xad\xbe\xef\xca\xfe\xba\xbe";

    uint8_t number8;
    uint16_t number16;
    uint32_t number32;
    uint64_t number64;

    e::unpack8be(buffer8, &number8);
    EXPECT_EQ(0xde, number8);
    e::unpack8le(buffer8, &number8);
    EXPECT_EQ(0xde, number8);

    e::unpack16be(buffer16, &number16);
    EXPECT_EQ(0xdead, number16);
    e::unpack16le(buffer16, &number16);
    EXPECT_EQ(0xadde, number16);

    e::unpack32be(buffer32, &number32);
    EXPECT_EQ(0xdeadbeefUL, number32);
    e::unpack32le(buffer32, &number32);
    EXPECT_EQ(0xefbeaddeUL, number32);

    e::unpack64be(buffer64, &number64);
    EXPECT_EQ(0xdeadbeefcafebabeULL, number64);
    e::unpack64le(buffer64, &number64);
    EXPECT_EQ(0xbebafecaefbeaddeULL, number64);
}

} // namespace

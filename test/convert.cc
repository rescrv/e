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

// e
#include "th.h"
#include "e/convert.h"

namespace
{

TEST(ConvertTest, Uint64NormalCases)
{
    ASSERT_GE(sizeof(unsigned long int), sizeof(uint32_t));

    try
    {
        ASSERT_EQ(0, e::convert::to_uint64_t("0"));
        ASSERT_EQ(0, e::convert::to_uint64_t("0x0"));
        ASSERT_EQ(0, e::convert::to_uint64_t("0x0", 16));
        ASSERT_EQ(0, e::convert::to_uint64_t("00"));
        ASSERT_EQ(0, e::convert::to_uint64_t("00", 8));

        ASSERT_EQ(18446744073709551615ULL, e::convert::to_uint64_t("18446744073709551615"));
        ASSERT_EQ(18446744073709551615ULL, e::convert::to_uint64_t("0xffffffffffffffff"));
        ASSERT_EQ(18446744073709551615ULL, e::convert::to_uint64_t("0xffffffffffffffff", 16));
        ASSERT_EQ(18446744073709551615ULL, e::convert::to_uint64_t("01777777777777777777777"));
        ASSERT_EQ(18446744073709551615ULL, e::convert::to_uint64_t("01777777777777777777777", 8));
    }
    catch (...)
    {
        FAIL();
    }
}

TEST(ConvertTest, Uint32NormalCases)
{
    ASSERT_GE(sizeof(unsigned long int), sizeof(uint32_t));

    try
    {
        ASSERT_EQ(0, e::convert::to_uint32_t("0"));
        ASSERT_EQ(0, e::convert::to_uint32_t("0x0"));
        ASSERT_EQ(0, e::convert::to_uint32_t("0x0", 16));
        ASSERT_EQ(0, e::convert::to_uint32_t("00"));
        ASSERT_EQ(0, e::convert::to_uint32_t("00", 8));

        ASSERT_EQ(4294967295UL, e::convert::to_uint32_t("4294967295"));
        ASSERT_EQ(4294967295UL, e::convert::to_uint32_t("0xffffffff"));
        ASSERT_EQ(4294967295UL, e::convert::to_uint32_t("0xffffffff", 16));
        ASSERT_EQ(4294967295UL, e::convert::to_uint32_t("037777777777"));
        ASSERT_EQ(4294967295UL, e::convert::to_uint32_t("037777777777", 8));
    }
    catch (...)
    {
        FAIL();
    }
}

TEST(ConvertTest, Uint16NormalCases)
{
    ASSERT_GE(sizeof(unsigned long int), sizeof(uint16_t));

    try
    {
        ASSERT_EQ(0, e::convert::to_uint16_t("0"));
        ASSERT_EQ(0, e::convert::to_uint16_t("0x0"));
        ASSERT_EQ(0, e::convert::to_uint16_t("0x0", 16));
        ASSERT_EQ(0, e::convert::to_uint16_t("00"));
        ASSERT_EQ(0, e::convert::to_uint16_t("00", 8));

        ASSERT_EQ(65535, e::convert::to_uint16_t("65535"));
        ASSERT_EQ(65535, e::convert::to_uint16_t("0xffff"));
        ASSERT_EQ(65535, e::convert::to_uint16_t("0xffff", 16));
        ASSERT_EQ(65535, e::convert::to_uint16_t("0177777"));
        ASSERT_EQ(65535, e::convert::to_uint16_t("0177777", 8));
    }
    catch (...)
    {
        FAIL();
    }
}

TEST(ConvertTest, Uint8NormalCases)
{
    ASSERT_GE(sizeof(unsigned long int), sizeof(uint8_t));

    try
    {
        ASSERT_EQ(0, e::convert::to_uint8_t("0"));
        ASSERT_EQ(0, e::convert::to_uint8_t("0x0"));
        ASSERT_EQ(0, e::convert::to_uint8_t("0x0", 16));
        ASSERT_EQ(0, e::convert::to_uint8_t("00"));
        ASSERT_EQ(0, e::convert::to_uint8_t("00", 8));

        ASSERT_EQ(255, e::convert::to_uint8_t("255"));
        ASSERT_EQ(255, e::convert::to_uint8_t("0xff"));
        ASSERT_EQ(255, e::convert::to_uint8_t("0xff", 16));
        ASSERT_EQ(255, e::convert::to_uint8_t("0377"));
        ASSERT_EQ(255, e::convert::to_uint8_t("0377", 8));
    }
    catch (...)
    {
        FAIL();
    }
}

} // namespace

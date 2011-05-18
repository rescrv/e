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
#include <e/convert.h>

#pragma GCC diagnostic ignored "-Wswitch-default"

namespace
{

TEST(ConvertTest, Uint32NormalCases)
{
    EXPECT_GE(sizeof(unsigned long int), sizeof(uint32_t));

    try
    {
        EXPECT_EQ(0, e::convert::to_uint32_t("0"));
        EXPECT_EQ(0, e::convert::to_uint32_t("0x0"));
        EXPECT_EQ(0, e::convert::to_uint32_t("0x0", 16));
        EXPECT_EQ(0, e::convert::to_uint32_t("00"));
        EXPECT_EQ(0, e::convert::to_uint32_t("00", 8));

        EXPECT_EQ(4294967295, e::convert::to_uint32_t("4294967295"));
        EXPECT_EQ(4294967295, e::convert::to_uint32_t("0xffffffff"));
        EXPECT_EQ(4294967295, e::convert::to_uint32_t("0xffffffff", 16));
        EXPECT_EQ(4294967295, e::convert::to_uint32_t("037777777777"));
        EXPECT_EQ(4294967295, e::convert::to_uint32_t("037777777777", 8));
    }
    catch (...)
    {
        FAIL();
    }
}

TEST(ConvertTest, Uint16NormalCases)
{
    EXPECT_GE(sizeof(unsigned long int), sizeof(uint16_t));

    try
    {
        EXPECT_EQ(0, e::convert::to_uint16_t("0"));
        EXPECT_EQ(0, e::convert::to_uint16_t("0x0"));
        EXPECT_EQ(0, e::convert::to_uint16_t("0x0", 16));
        EXPECT_EQ(0, e::convert::to_uint16_t("00"));
        EXPECT_EQ(0, e::convert::to_uint16_t("00", 8));

        EXPECT_EQ(65535, e::convert::to_uint16_t("65535"));
        EXPECT_EQ(65535, e::convert::to_uint16_t("0xffff"));
        EXPECT_EQ(65535, e::convert::to_uint16_t("0xffff", 16));
        EXPECT_EQ(65535, e::convert::to_uint16_t("0177777"));
        EXPECT_EQ(65535, e::convert::to_uint16_t("0177777", 8));
    }
    catch (...)
    {
        FAIL();
    }
}

} // namespace

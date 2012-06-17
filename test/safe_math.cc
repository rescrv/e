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

#define __STDC_LIMIT_MACROS

// Google Test
#include <gtest/gtest.h>

// e
#include "e/safe_math.h"

#pragma GCC diagnostic ignored "-Wswitch-default"

namespace
{

TEST(SafeMath, Add)
{
    int64_t result;

    EXPECT_TRUE(e::safe_add(0, 0, &result));
    EXPECT_EQ(0, result);

    EXPECT_TRUE(e::safe_add(0, INT64_MAX, &result));
    EXPECT_EQ(INT64_MAX, result);
    EXPECT_TRUE(e::safe_add(INT64_MAX, 0, &result));
    EXPECT_EQ(INT64_MAX, result);

    EXPECT_FALSE(e::safe_add(1, INT64_MAX, &result));
    EXPECT_FALSE(e::safe_add(INT64_MAX, 1, &result));

    EXPECT_TRUE(e::safe_add(0, INT64_MIN, &result));
    EXPECT_EQ(INT64_MIN, result);
    EXPECT_TRUE(e::safe_add(INT64_MIN, 0, &result));
    EXPECT_EQ(INT64_MIN, result);

    EXPECT_FALSE(e::safe_add(-1, INT64_MIN, &result));
    EXPECT_FALSE(e::safe_add(INT64_MIN, -1, &result));

    EXPECT_TRUE(e::safe_add(INT64_MIN, INT64_MAX, &result));
    EXPECT_EQ(-1, result);
    EXPECT_TRUE(e::safe_add(INT64_MAX, INT64_MIN, &result));
    EXPECT_EQ(-1, result);
}

TEST(SafeMath, Sub)
{
    int64_t result;

    EXPECT_TRUE(e::safe_sub(0, 0, &result));
    EXPECT_EQ(0, result);

    EXPECT_TRUE(e::safe_sub(0, -INT64_MAX, &result));
    EXPECT_EQ(INT64_MAX, result);
    EXPECT_TRUE(e::safe_sub(INT64_MAX, 0, &result));
    EXPECT_EQ(INT64_MAX, result);

    EXPECT_FALSE(e::safe_sub(INT64_MAX, -1, &result));
    EXPECT_FALSE(e::safe_sub(INT64_MIN, 1, &result));

    EXPECT_TRUE(e::safe_sub(INT64_MIN, -INT64_MAX, &result));
    EXPECT_EQ(-1, result);
    EXPECT_TRUE(e::safe_sub(-INT64_MAX, INT64_MIN, &result));
    EXPECT_EQ(1, result);
}

TEST(SafeMath, Mul)
{
    int64_t result;

    EXPECT_TRUE(e::safe_mul(0, 0, &result));
    EXPECT_EQ(0, result);

    EXPECT_TRUE(e::safe_mul(1, INT64_MAX, &result));
    EXPECT_EQ(INT64_MAX, result);
    EXPECT_TRUE(e::safe_mul(INT64_MAX, 1, &result));
    EXPECT_EQ(INT64_MAX, result);
    EXPECT_TRUE(e::safe_mul(1, INT64_MIN, &result));
    EXPECT_EQ(INT64_MIN, result);
    EXPECT_TRUE(e::safe_mul(INT64_MIN, 1, &result));
    EXPECT_EQ(INT64_MIN, result);

    EXPECT_FALSE(e::safe_mul(4611686018427387904LL, 2, &result));
    EXPECT_FALSE(e::safe_mul(2, 4611686018427387904LL, &result));

    EXPECT_TRUE(e::safe_mul(-4611686018427387904LL, 2, &result));
    EXPECT_EQ(INT64_MIN, result);
    EXPECT_TRUE(e::safe_mul(2, -4611686018427387904LL, &result));
    EXPECT_EQ(INT64_MIN, result);
    EXPECT_TRUE(e::safe_mul(4611686018427387904LL, -2, &result));
    EXPECT_EQ(INT64_MIN, result);
    EXPECT_TRUE(e::safe_mul(-2, 4611686018427387904LL, &result));
    EXPECT_EQ(INT64_MIN, result);

    EXPECT_FALSE(e::safe_mul(3074457345618258603, -3, &result));
    EXPECT_FALSE(e::safe_mul(-3, 3074457345618258603, &result));
    EXPECT_FALSE(e::safe_mul(-3074457345618258603, 3, &result));
    EXPECT_FALSE(e::safe_mul(3, -3074457345618258603, &result));
}

TEST(SafeMath, Div)
{
    int64_t result;

    EXPECT_FALSE(e::safe_div(INT64_MIN, -1, &result));
    EXPECT_FALSE(e::safe_div(INT64_MIN, 0, &result));

    EXPECT_TRUE(e::safe_div(INT64_MIN, 1, &result));
    EXPECT_EQ(INT64_MIN, result);
    EXPECT_TRUE(e::safe_div(INT64_MAX, 1, &result));
    EXPECT_EQ(INT64_MAX, result);
    EXPECT_TRUE(e::safe_div(INT64_MAX, -1, &result));
    EXPECT_EQ(INT64_MIN + 1, result);

    EXPECT_TRUE(e::safe_div(-5, 2, &result));
    EXPECT_EQ(-3, result);

    EXPECT_TRUE(e::safe_div(-5, 3, &result));
    EXPECT_EQ(-2, result);

    EXPECT_TRUE(e::safe_div(5, -2, &result));
    EXPECT_EQ(-3, result);

    EXPECT_TRUE(e::safe_div(5, -3, &result));
    EXPECT_EQ(-2, result);
}

TEST(SafeMath, Mod)
{
    int64_t result;

    EXPECT_FALSE(e::safe_mod(INT64_MAX, 0, &result));

    EXPECT_TRUE(e::safe_mod(INT64_MAX, INT64_MAX, &result));
    EXPECT_EQ(0, result);
    EXPECT_TRUE(e::safe_mod(INT64_MAX, INT64_MIN, &result));
    EXPECT_EQ(-1, result);
    EXPECT_TRUE(e::safe_mod(INT64_MIN, INT64_MAX, &result));
    EXPECT_EQ(INT64_MAX - 1, result);
    EXPECT_TRUE(e::safe_mod(INT64_MIN, INT64_MIN, &result));
    EXPECT_EQ(0, result);

    EXPECT_TRUE(e::safe_mod(-5, 2, &result));
    EXPECT_EQ(1, result);

    EXPECT_TRUE(e::safe_mod(-5, 3, &result));
    EXPECT_EQ(1, result);

    EXPECT_TRUE(e::safe_mod(5, -2, &result));
    EXPECT_EQ(-1, result);       
                                
    EXPECT_TRUE(e::safe_mod(5, -3, &result));
    EXPECT_EQ(-1, result);
}

} // namespace

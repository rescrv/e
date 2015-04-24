// Copyright (c) 2014, Robert Escriva
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
#include <cstring>

// e
#include "th.h"
#include "e/pow2.h"

namespace
{

TEST(Pow2Test, Many)
{
    ASSERT_EQ(uint64_t(0), e::next_pow2(0));
    ASSERT_EQ(uint64_t(1), e::next_pow2(1));
    ASSERT_EQ(uint64_t(2), e::next_pow2(2));
    ASSERT_EQ(uint64_t(4), e::next_pow2(3));
    ASSERT_EQ(uint64_t(4), e::next_pow2(4));
    for (uint64_t i = 5; i <= 8; ++i) { ASSERT_EQ(8, e::next_pow2(i)); }
    for (uint64_t i = 9; i <= 16; ++i) { ASSERT_EQ(16, e::next_pow2(i)); }
    for (uint64_t i = 17; i <= 32; ++i) { ASSERT_EQ(32, e::next_pow2(i)); }
    ASSERT_EQ(562949953421312ULL, e::next_pow2(281474976710657));
}

TEST(Pow2Test, IsPow2)
{
    ASSERT_FALSE(e::is_pow2(0));
    ASSERT_TRUE(e::is_pow2(1));
    ASSERT_TRUE(e::is_pow2(2));
    ASSERT_FALSE(e::is_pow2(3));
    ASSERT_TRUE(e::is_pow2(4));
    ASSERT_FALSE(e::is_pow2(5));
    ASSERT_FALSE(e::is_pow2(6));
    ASSERT_FALSE(e::is_pow2(7));
    ASSERT_TRUE(e::is_pow2(8));
}

} // namespace

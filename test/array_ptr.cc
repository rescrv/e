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

// e
#include "th.h"
#include "e/array_ptr.h"

namespace
{

TEST(ArrayPtr, CtorAndDtor)
{
    e::array_ptr<int> a;
    e::array_ptr<int> b(new int[1]);
    e::array_ptr<int> c(b);
}

TEST(ArrayPtr, BoolOperator)
{
    e::array_ptr<int> x;
    ASSERT_FALSE(x);
    x = new int[5];
    ASSERT_TRUE(x);
}

TEST(ArrayPtr, BracketOperator)
{
    e::array_ptr<int> x(new int[5]);
    x[0] = 0;
    x[1] = 1;
    x[2] = 2;
    x[3] = 3;
    x[4] = 4;
    ASSERT_EQ(0, x[0]);
    ASSERT_EQ(1, x[1]);
    ASSERT_EQ(2, x[2]);
    ASSERT_EQ(3, x[3]);
    ASSERT_EQ(4, x[4]);

    const e::array_ptr<int> y(x);
    ASSERT_EQ(0, y[0]);
    ASSERT_EQ(1, y[1]);
    ASSERT_EQ(2, y[2]);
    ASSERT_EQ(3, y[3]);
    ASSERT_EQ(4, y[4]);
}

}

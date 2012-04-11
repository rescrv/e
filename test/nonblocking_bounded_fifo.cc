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

// HyperDisk
#include "e/nonblocking_bounded_fifo.h"

#pragma GCC diagnostic ignored "-Wswitch-default"

namespace
{

TEST(NonblockingBoundedFifo, CtorAndDtor)
{
    e::nonblocking_bounded_fifo<int> nbf(8);
}

TEST(NonblockingBoundedFifo, RoundRobin)
{
    e::nonblocking_bounded_fifo<size_t> nbf(8);

    for (size_t i = 0; i < 1000; ++i)
    {
        size_t base = i * 8;

        for (size_t j = 0; j < 8; ++j)
        {
            ASSERT_TRUE(nbf.push(base + j));
        }

        ASSERT_FALSE(nbf.push(0));

        for (size_t j = 0; j < 8; ++j)
        {
            size_t popped;
            nbf.pop(&popped);
            ASSERT_EQ(base + j, popped);
        }

        ASSERT_FALSE(nbf.pop(NULL));
    }
}

} // namespace

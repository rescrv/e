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
#include "e/locking_iterable_fifo.h"

#pragma GCC diagnostic ignored "-Wswitch-default"

namespace
{

TEST(LockingIterableFifoTest, CtorAndDtor)
{
    e::locking_iterable_fifo<int> l;
}

TEST(LockingIterableFifoTest, SimpleIteration)
{
    e::locking_iterable_fifo<int> l;

    // Add one thousand put/del pairs to the queue.
    for (int i = 0; i < 1000; ++i)
    {
        l.append(i);
    }

    // Verify that we get them back.
    e::locking_iterable_fifo<int>::iterator it = l.iterate();

    for (int i = 0; i < 1000; ++i)
    {
        ASSERT_TRUE(it.valid());
        EXPECT_EQ(i, *it);
        it.next();
    }

    EXPECT_FALSE(it.valid());
}

TEST(LockingIterableFifoTest, IterateAddIterate)
{
    e::locking_iterable_fifo<int> l;
    int count = 1;
    e::locking_iterable_fifo<int>::iterator it = l.iterate();

    // Add 10 items to the queue.
    for (; count <= 10; ++count)
    {
        l.append(count);
    }

    // Iterate to the end.
    for (int i = 1; i <= 10; ++i)
    {
        EXPECT_TRUE(it.valid());
        EXPECT_EQ(i, *it);
        it.next();
    }

    EXPECT_FALSE(it.valid());
    EXPECT_FALSE(it.valid());

    // Add 10 more items to the queue.
    for (; count <= 20; ++count)
    {
        l.append(count);
    }

    // Iterate to the end.
    for (int i = 11; i <= 20; ++i)
    {
        EXPECT_TRUE(it.valid());
        EXPECT_EQ(i, *it);
        it.next();
    }

    EXPECT_FALSE(it.valid());
}

TEST(LockingIterableFifoTest, IterateFlushIterate)
{
    e::locking_iterable_fifo<int> l;

    // Grab an iterator
    e::locking_iterable_fifo<int>::iterator it = l.iterate();

    // Add 20 items to the queue.
    for (int count = 1; count <= 20; ++count)
    {
        l.append(count);
    }

    // Iterate to the halfway point.
    for (int i = 1; i <= 10; ++i)
    {
        EXPECT_TRUE(it.valid());
        EXPECT_EQ(i, *it);
        it.next();
    }

    // Flush the log
    for (int i = 1; i <= 10; ++i)
    {
        EXPECT_FALSE(l.empty());
        EXPECT_EQ(i, l.oldest());
        l.remove_oldest();
    }

    // Iterate the rest of the way.
    for (size_t i = 11; i <= 20; ++i)
    {
        EXPECT_TRUE(it.valid());
        EXPECT_EQ(i, *it);
        it.next();
    }

    EXPECT_FALSE(it.valid());
}

TEST(LockingIterableFifoTest, CopyIterator)
{
    e::locking_iterable_fifo<int> l;

    // Add one thousand put/del pairs to the queue.
    for (int i = 0; i < 1000; ++i)
    {
        l.append(i);
    }

    // Verify that we get them back.
    e::locking_iterable_fifo<int>::iterator it = l.iterate();
    e::locking_iterable_fifo<int>::iterator copy1(it);
    e::locking_iterable_fifo<int>::iterator copy2 = copy1;

    for (int i = 0; i < 1000; ++i)
    {
        ASSERT_TRUE(it.valid());
        ASSERT_TRUE(copy1.valid());
        ASSERT_TRUE(copy2.valid());
        EXPECT_EQ(i, *it);
        EXPECT_EQ(i, *copy1);
        EXPECT_EQ(i, *copy2);
        // Advance for next iteration
        it.next();
        copy1.next();
        copy2.next();
    }

    EXPECT_FALSE(it.valid());
    EXPECT_FALSE(copy1.valid());
    EXPECT_FALSE(copy2.valid());
}

TEST(LockingIterableFifoTest, BatchAppend)
{
    e::locking_iterable_fifo<int> l;

    // Add batches of various sizes, iterating to verify presence.
    e::locking_iterable_fifo<int>::iterator it = l.iterate();

    for (size_t i = 0; i < 1000; ++i)
    {
        std::vector<int> values(i);

        for (size_t j = 0; j < i; ++j)
        {
            values[j] = j;
        }

        l.batch_append(values);

        for (size_t j = 0; j < i; ++j)
        {
            ASSERT_TRUE(it.valid());
            ASSERT_EQ(j, *it);
            it.next();
        }
    }
}

} // namespace

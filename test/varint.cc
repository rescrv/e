// This code is derived from code distributed as part of Google LevelDB.
// The original is available in leveldb as util/coding_test.cc.
// This file was retrieved Jul 15, 2013 by Robert Escriva.

// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

// STL
#include <vector>

// e
#include "th.h"
#include "e/varint.h"

TEST(Varint, Varint32)
{
    std::string s;

    for (uint32_t i = 0; i < (32 * 32); i++)
    {
        uint32_t v = (i / 32) << (i % 32);
        char buf[10];
        char* ptr = e::varint32_encode(buf, v);
        s += std::string(buf, ptr - buf);
    }

    const char* p = s.data();
    const char* limit = p + s.size();

    for (uint32_t i = 0; i < (32 * 32); i++)
    {
        uint32_t expected = (i / 32) << (i % 32);
        uint32_t actual;
        const char* start = p;
        p = e::varint32_decode(p, limit, &actual);
        ASSERT_TRUE(p != NULL);
        ASSERT_EQ(expected, actual);
        ASSERT_EQ(e::varint_length(actual), p - start);
    }

    ASSERT_EQ(p, s.data() + s.size());
}

TEST(Coding, Varint64)
{
    // Construct the list of values to check
    std::vector<uint64_t> values;
    // Some special values
    values.push_back(0);
    values.push_back(100);
    values.push_back(~static_cast<uint64_t>(0));
    values.push_back(~static_cast<uint64_t>(0) - 1);

    for (uint32_t k = 0; k < 64; k++)
    {
        // Test values near powers of two
        const uint64_t power = 1ull << k;
        values.push_back(power);
        values.push_back(power-1);
        values.push_back(power+1);
    }

    std::string s;

    for (size_t i = 0; i < values.size(); i++)
    {
        char buf[10];
        char* ptr = e::varint64_encode(buf, values[i]);
        s += std::string(buf, ptr - buf);
    }

    const char* p = s.data();
    const char* limit = p + s.size();

    for (size_t i = 0; i < values.size(); i++)
    {
        ASSERT_TRUE(p < limit);
        uint64_t actual;
        const char* start = p;
        p = e::varint64_decode(p, limit, &actual);
        ASSERT_TRUE(p != NULL);
        ASSERT_EQ(values[i], actual);
        ASSERT_EQ(e::varint_length(actual), p - start);
    }

    ASSERT_EQ(p, limit);
}

TEST(Coding, Varint32Overflow)
{
    uint32_t result;
    std::string input("\x81\x82\x83\x84\x85\x11");
    ASSERT_TRUE(e::varint32_decode(input.data(), input.data() + input.size(), &result)
                == NULL);
}

TEST(Coding, Varint32Truncation)
{
    uint32_t large_value = (1u << 31) + 100;
    std::string s;
    char buf[10];
    char* ptr = e::varint32_encode(buf, large_value);
    s += std::string(buf, ptr - buf);
    uint32_t result;

    for (size_t len = 0; len < s.size() - 1; len++)
    {
        ASSERT_TRUE(e::varint32_decode(s.data(), s.data() + len, &result) == NULL);
    }

    ASSERT_TRUE(e::varint32_decode(s.data(), s.data() + s.size(), &result) != NULL);
    ASSERT_EQ(large_value, result);
}

TEST(Coding, Varint64Overflow)
{
    uint64_t result;
    std::string input("\x81\x82\x83\x84\x85\x81\x82\x83\x84\x85\x11");
    ASSERT_TRUE(e::varint64_decode(input.data(), input.data() + input.size(), &result)
                == NULL);
}

TEST(Coding, Varint64Truncation)
{
    uint64_t large_value = (1ull << 63) + 100ull;
    std::string s;
    char buf[10];
    char* ptr = e::varint64_encode(buf, large_value);
    s += std::string(buf, ptr - buf);
    uint64_t result;

    for (size_t len = 0; len < s.size() - 1; len++)
    {
        ASSERT_TRUE(e::varint64_decode(s.data(), s.data() + len, &result) == NULL);
    }

    ASSERT_TRUE(e::varint64_decode(s.data(), s.data() + s.size(), &result) != NULL);
    ASSERT_EQ(large_value, result);
}

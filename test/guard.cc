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
#include "e/guard.h"

namespace
{

int check;

void
func0()
{
    check = 0;
}

void
func1(int a)
{
    check = a;
}

void
func2(int a, int b)
{
    check = a + b;
}

void
func3(int a, int b, int c)
{
    check = a + b + c;
}

TEST(GuardTest, Function0)
{
    check = -1;

    {
        e::guard g = e::makeguard(&func0);
        g.use_variable();
        ASSERT_EQ(-1, check);
    }

    ASSERT_EQ(0, check);
}

TEST(GuardTest, Function1)
{
    check = -1;

    {
        e::guard g = e::makeguard(func1, 1);
        g.use_variable();
        ASSERT_EQ(-1, check);
    }

    ASSERT_EQ(1, check);
}

TEST(GuardTest, Function2)
{
    check = -1;

    {
        e::guard g = e::makeguard(func2, 1, 2);
        g.use_variable();
        ASSERT_EQ(-1, check);
    }

    ASSERT_EQ(3, check);
}

TEST(GuardTest, Function3)
{
    check = -1;

    {
        e::guard g = e::makeguard(func3, 1, 2, 3);
        g.use_variable();
        ASSERT_EQ(-1, check);
    }

    ASSERT_EQ(6, check);
}

TEST(GuardTest, Function0Dismiss)
{
    check = -1;

    {
        e::guard g = e::makeguard(func0);
        ASSERT_EQ(-1, check);
        g.dismiss();
    }

    ASSERT_EQ(-1, check);
}

TEST(GuardTest, Function1Dismiss)
{
    check = -1;

    {
        e::guard g = e::makeguard(func1, 1);
        ASSERT_EQ(-1, check);
        g.dismiss();
    }

    ASSERT_EQ(-1, check);
}

TEST(GuardTest, Function2Dismiss)
{
    check = -1;

    {
        e::guard g = e::makeguard(func2, 1, 2);
        ASSERT_EQ(-1, check);
        g.dismiss();
    }

    ASSERT_EQ(-1, check);
}

TEST(GuardTest, Function3Dismiss)
{
    check = -1;

    {
        e::guard g = e::makeguard(func3, 1, 2, 3);
        ASSERT_EQ(-1, check);
        g.dismiss();
    }

    ASSERT_EQ(-1, check);
}

class object
{
    public:
        object() : m_count(-1) {}

    public:
        void func0() { m_count = 0; }
        void func1(int a) { m_count = a; }
        void func2(int a, int b) { m_count = a + b; }
        void func3(int a, int b, int c) { m_count = a + b + c; }
        int count() const { return m_count; }

    private:
        int m_count;
};

TEST(GuardTest, Object0)
{
    object obj;

    {
        e::guard g = e::makeobjguard(obj, &object::func0);
        g.use_variable();
        ASSERT_EQ(-1, obj.count());
    }

    ASSERT_EQ(0, obj.count());
}

TEST(GuardTest, Object1)
{
    object obj;

    {
        e::guard g = e::makeobjguard(obj, &object::func1, 1);
        g.use_variable();
        ASSERT_EQ(-1, obj.count());
    }

    ASSERT_EQ(1, obj.count());
}

TEST(GuardTest, Object2)
{
    object obj;

    {
        e::guard g = e::makeobjguard(obj, &object::func2, 1, 2);
        g.use_variable();
        ASSERT_EQ(-1, obj.count());
    }

    ASSERT_EQ(3, obj.count());
}

TEST(GuardTest, Object3)
{
    object obj;

    {
        e::guard g = e::makeobjguard(obj, &object::func3, 1, 2, 3);
        g.use_variable();
        ASSERT_EQ(-1, obj.count());
    }

    ASSERT_EQ(6, obj.count());
}

TEST(GuardTest, Object0Dismiss)
{
    object obj;

    {
        e::guard g = e::makeobjguard(obj, &object::func0);
        ASSERT_EQ(-1, obj.count());
        g.dismiss();
    }

    ASSERT_EQ(-1, obj.count());
}

TEST(GuardTest, Object1Dismiss)
{
    object obj;

    {
        e::guard g = e::makeobjguard(obj, &object::func1, 1);
        ASSERT_EQ(-1, obj.count());
        g.dismiss();
    }

    ASSERT_EQ(-1, obj.count());
}

TEST(GuardTest, Object2Dismiss)
{
    object obj;

    {
        e::guard g = e::makeobjguard(obj, &object::func2, 1, 2);
        ASSERT_EQ(-1, obj.count());
        g.dismiss();
    }

    ASSERT_EQ(-1, obj.count());
}

TEST(GuardTest, Object3Dismiss)
{
    object obj;

    {
        e::guard g = e::makeobjguard(obj, &object::func3, 1, 2, 3);
        ASSERT_EQ(-1, obj.count());
        g.dismiss();
    }

    ASSERT_EQ(-1, obj.count());
}

} // namespace

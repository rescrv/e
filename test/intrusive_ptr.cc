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
#include <e/intrusive_ptr.h>

#pragma GCC diagnostic ignored "-Wswitch-default"

namespace
{

class foo
{
    public:
        foo()
            : m_ref(0)
        {
        }

    private:
        friend class e::intrusive_ptr<foo>;

    private:
        size_t m_ref;
};

TEST(IntrusivePtr, CtorAndDtor)
{
    e::intrusive_ptr<foo> a;
    e::intrusive_ptr<foo> b(new foo());
    e::intrusive_ptr<foo> c(b);
}

class ctordtor
{
    public:
        ctordtor(bool& ctor, bool& dtor)
            : m_ref(0)
            , m_ctor(ctor)
            , m_dtor(dtor)
        {
            m_ctor = true;
        }

        ~ctordtor() throw ()
        {
            m_dtor = true;
        }

    public:
        size_t m_ref;
        bool& m_ctor;
        bool& m_dtor;
};

TEST(IntrusivePtr, Nesting)
{
    bool ctor = false;
    bool dtor = false;
    EXPECT_FALSE(ctor);
    EXPECT_FALSE(dtor);

    {
        e::intrusive_ptr<ctordtor> a(new ctordtor(ctor, dtor));
        EXPECT_TRUE(ctor);
        EXPECT_FALSE(dtor);

        {
            e::intrusive_ptr<ctordtor> b(a);
            EXPECT_TRUE(ctor);
            EXPECT_FALSE(dtor);

            {
                e::intrusive_ptr<ctordtor> c(b);
                EXPECT_TRUE(ctor);
                EXPECT_FALSE(dtor);

                {
                    e::intrusive_ptr<ctordtor> d(c);
                    EXPECT_TRUE(ctor);
                    EXPECT_FALSE(dtor);
                }

                EXPECT_TRUE(ctor);
                EXPECT_FALSE(dtor);
            }

            EXPECT_TRUE(ctor);
            EXPECT_FALSE(dtor);
        }

        EXPECT_TRUE(ctor);
        EXPECT_FALSE(dtor);
    }

    EXPECT_TRUE(ctor);
    EXPECT_TRUE(dtor);
}

} // namespace

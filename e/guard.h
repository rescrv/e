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

#ifndef e_guard_h_
#define e_guard_h_

namespace e
{

// A generalization of RAII using the scoped guard idiom from Andrei
// Alexandrescu and Petru Marginean's article[1].  This code very similar, but
// in my style as its what I like to use most.
//
// 1.  http://drdobbs.com/cpp/184403758

class guard_base
{
    public:
        void dismiss() const throw ()
        {
            m_dismissed = true;
        }

    public:
        void use_variable() const throw () {}

    protected:
        guard_base()
            : m_dismissed(false)
        {
        }

        guard_base(const guard_base& other)
            : m_dismissed(other.m_dismissed)
        {
            other.dismiss();
        }

        ~guard_base() {}

    protected:
        mutable bool m_dismissed;

    private:
        guard_base& operator = (const guard_base&);
};

template <typename F>
class guard_func0 : public guard_base
{
    public:
        guard_func0(const F& func)
            : m_func(func)
        {
        }

        ~guard_func0()
        {
            if (!m_dismissed)
            {
                m_func();
            }
        }

    private:
        F m_func;
};

template <typename F>
guard_func0<F>
makeguard(F func)
{
    return guard_func0<F>(func);
}

template <typename F, typename P1>
class guard_func1 : public guard_base
{
    public:
        guard_func1(const F& func, const P1& p1)
            : m_func(func)
            , m_p1(p1)
        {
        }

        ~guard_func1()
        {
            if (!m_dismissed)
            {
                m_func(m_p1);
            }
        }

    private:
        F m_func;
        const P1 m_p1;
};

template <typename F, typename P1>
guard_func1<F, P1>
makeguard(F func, P1 p1)
{
    return guard_func1<F, P1>(func, p1);
}

template <typename F, typename P1, typename P2>
class guard_func2 : public guard_base
{
    public:
        guard_func2(const F& func, const P1& p1, const P2& p2)
            : m_func(func)
            , m_p1(p1)
            , m_p2(p2)
        {
        }

        ~guard_func2()
        {
            if (!m_dismissed)
            {
                m_func(m_p1, m_p2);
            }
        }

    private:
        F m_func;
        const P1 m_p1;
        const P2 m_p2;
};

template <typename F, typename P1, typename P2>
guard_func2<F, P1, P2>
makeguard(F func, P1 p1, P2 p2)
{
    return guard_func2<F, P1, P2>(func, p1, p2);
}

template <typename F, typename P1, typename P2, typename P3>
class guard_func3 : public guard_base
{
    public:
        guard_func3(const F& func, const P1& p1, const P2& p2, const P3& p3)
            : m_func(func)
            , m_p1(p1)
            , m_p2(p2)
            , m_p3(p3)
        {
        }

        ~guard_func3()
        {
            if (!m_dismissed)
            {
                m_func(m_p1, m_p2, m_p3);
            }
        }

    private:
        F m_func;
        const P1 m_p1;
        const P2 m_p2;
        const P3 m_p3;
};

template <typename F, typename P1, typename P2, typename P3>
guard_func3<F, P1, P2, P3>
makeguard(F func, P1 p1, P2 p2, P3 p3)
{
    return guard_func3<F, P1, P2, P3>(func, p1, p2, p3);
}

template <typename O, typename F>
class guard_obj0 : public guard_base
{
    public:
        guard_obj0(O& obj, F func)
            : m_obj(obj)
            , m_func(func)
        {
        }

        ~guard_obj0()
        {
            if (!m_dismissed)
            {
                (m_obj.*m_func)();
            }
        }

    private:
        O& m_obj;
        F m_func;
};

template <typename O, typename F>
guard_obj0<O, F>
makeobjguard(O& obj, F func)
{
    return guard_obj0<O, F>(obj, func);
}

template <typename O, typename F, typename P1>
class guard_obj1 : public guard_base
{
    public:
        guard_obj1(O& obj, F func, P1 p1)
            : m_obj(obj)
            , m_func(func)
            , m_p1(p1)
        {
        }

        ~guard_obj1()
        {
            if (!m_dismissed)
            {
                (m_obj.*m_func)(m_p1);
            }
        }

    private:
        O& m_obj;
        F m_func;
        const P1 m_p1;
};

template <typename O, typename F, typename P1>
guard_obj1<O, F, P1>
makeobjguard(O& obj, F func, P1 p1)
{
    return guard_obj1<O, F, P1>(obj, func, p1);
}

template <typename O, typename F, typename P1, typename P2>
class guard_obj2 : public guard_base
{
    public:
        guard_obj2(O& obj, F func, P1 p1, P2 p2)
            : m_obj(obj)
            , m_func(func)
            , m_p1(p1)
            , m_p2(p2)
        {
        }

        ~guard_obj2()
        {
            if (!m_dismissed)
            {
                (m_obj.*m_func)(m_p1, m_p2);
            }
        }

    private:
        O& m_obj;
        F m_func;
        const P1 m_p1;
        const P2 m_p2;
};

template <typename O, typename F, typename P1, typename P2>
guard_obj2<O, F, P1, P2>
makeobjguard(O& obj, F func, P1 p1, P2 p2)
{
    return guard_obj2<O, F, P1, P2>(obj, func, p1, p2);
}

template <typename O, typename F, typename P1, typename P2, typename P3>
class guard_obj3 : public guard_base
{
    public:
        guard_obj3(O& obj, F func, P1 p1, P2 p2, P3 p3)
            : m_obj(obj)
            , m_func(func)
            , m_p1(p1)
            , m_p2(p2)
            , m_p3(p3)
        {
        }

        ~guard_obj3()
        {
            if (!m_dismissed)
            {
                (m_obj.*m_func)(m_p1, m_p2, m_p3);
            }
        }

    private:
        O& m_obj;
        F m_func;
        const P1 m_p1;
        const P2 m_p2;
        const P3 m_p3;
};

template <typename O, typename F, typename P1, typename P2, typename P3>
guard_obj3<O, F, P1, P2, P3>
makeobjguard(O& obj, F func, P1 p1, P2 p2, P3 p3)
{
    return guard_obj3<O, F, P1, P2, P3>(obj, func, p1, p2, p3);
}

template <typename O, typename F, typename P1, typename P2, typename P3, typename P4>
class guard_obj4 : public guard_base
{
    public:
        guard_obj4(O& obj, F func, P1 p1, P2 p2, P3 p3, P4 p4)
            : m_obj(obj)
            , m_func(func)
            , m_p1(p1)
            , m_p2(p2)
            , m_p3(p3)
            , m_p4(p4)
        {
        }

        ~guard_obj4()
        {
            if (!m_dismissed)
            {
                (m_obj.*m_func)(m_p1, m_p2, m_p3, m_p4);
            }
        }

    private:
        O& m_obj;
        F m_func;
        const P1 m_p1;
        const P2 m_p2;
        const P3 m_p3;
        const P4 m_p4;
};

template <typename O, typename F, typename P1, typename P2, typename P3, typename P4>
guard_obj4<O, F, P1, P2, P3, P4>
makeobjguard(O& obj, F func, P1 p1, P2 p2, P3 p3, P4 p4)
{
    return guard_obj4<O, F, P1, P2, P3, P4>(obj, func, p1, p2, p3, p4);
}

template <typename O, typename F, typename P1, typename P2, typename P3, typename P4, typename P5>
class guard_obj5 : public guard_base
{
    public:
        guard_obj5(O& obj, F func, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
            : m_obj(obj)
            , m_func(func)
            , m_p1(p1)
            , m_p2(p2)
            , m_p3(p3)
            , m_p4(p4)
            , m_p5(p5)
        {
        }

        ~guard_obj5()
        {
            if (!m_dismissed)
            {
                (m_obj.*m_func)(m_p1, m_p2, m_p3, m_p4, m_p5);
            }
        }

    private:
        O& m_obj;
        F m_func;
        const P1 m_p1;
        const P2 m_p2;
        const P3 m_p3;
        const P4 m_p4;
        const P5 m_p5;
};

template <typename O, typename F, typename P1, typename P2, typename P3, typename P4, typename P5>
guard_obj5<O, F, P1, P2, P3, P4, P5>
makeobjguard(O& obj, F func, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
{
    return guard_obj5<O, F, P1, P2, P3, P4, P5>(obj, func, p1, p2, p3, p4, p5);
}

typedef const guard_base& guard;

} // namespace e

#endif // e_buffer_h_

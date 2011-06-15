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

#ifndef e_hazard_ptrs_h_
#define e_hazard_ptrs_h_

// C
#include <cassert>

// STL
#include <set>

// e
#include <e/guard.h>

namespace e
{

template <typename T, size_t P>
class hazard_ptrs
{
    public:
        class hazard_ptr;

    public:
        hazard_ptrs();
        ~hazard_ptrs() throw ();

    public:
        std::auto_ptr<hazard_ptr> get();

    private:
        class hazard_rec;

    private:
        hazard_ptrs(const hazard_ptrs&);

    private:
        hazard_ptrs& operator = (const hazard_ptrs&);

    private:
        hazard_rec* m_recs;
        size_t m_num_recs;
};

template <typename T, size_t P>
class hazard_ptrs<T, P> :: hazard_ptr
{
    public:
        ~hazard_ptr() throw ();

    public:
        void set(size_t ptr_num, T* ptr);
        void retire(T* ptr);

    private:
        friend class hazard_ptrs<T, P>;

    private:
        hazard_ptr(hazard_rec* rec);
        hazard_ptr(const hazard_ptr& rec);

    private:
        hazard_ptr& operator = (const hazard_ptr& rhs);

    private:
        hazard_rec* m_rec;
};

template <typename T, size_t P>
class hazard_ptrs<T, P> :: hazard_rec
{
    public:
        ~hazard_rec() throw ();

    public:
        void scan();

    public:
        int taslock;
        hazard_rec* next;
        const T* ptrs[P];
        size_t rcount;
        std::vector<const T*> rlist;

    private:
        friend class hazard_ptrs<T, P>;

    private:
        hazard_rec(hazard_ptrs& parent);
        hazard_rec(const hazard_rec&);

    private:
        hazard_rec& operator = (const hazard_rec&);

    private:
        hazard_ptrs& m_parent;
};

template <typename T, size_t P>
inline
hazard_ptrs<T, P> :: hazard_ptrs()
    : m_recs(NULL)
    , m_num_recs(0)
{
}

template <typename T, size_t P>
inline
hazard_ptrs<T, P> :: ~hazard_ptrs() throw ()
{
    m_num_recs = 0;

    while (m_recs)
    {
        while (__sync_lock_test_and_set(&m_recs->taslock, 1) != 0)
            ;

        m_recs->scan();
        hazard_rec* tmp = m_recs;
        m_recs = m_recs->next;
        delete tmp;
    }
}

template <typename T, size_t P>
inline std::auto_ptr<typename e::hazard_ptrs<T, P>::hazard_ptr>
hazard_ptrs<T, P> :: get()
{
    // XXX if we fail to allocate the new hazard_ptr, we will have a permanently
    // locked hazard_rec.  __sync_lock_release doesn't play well with e::guard
    // because it is a builtin.
    hazard_rec* rec = m_recs;

    while (rec)
    {
        if (__sync_lock_test_and_set(&rec->taslock, 1) == 0)
        {
            std::auto_ptr<hazard_ptr> ret(new hazard_ptr(rec));
            return ret;
        }

        rec = rec->next;
    }

    std::auto_ptr<hazard_rec> newrec(new hazard_rec(*this));
    bool locked = __sync_lock_test_and_set(&newrec->taslock, 1) == 0;
    assert(locked);
    hazard_rec* oldhead;

    do
    {
        oldhead = m_recs;
        newrec->next = oldhead;
    }
    while (!__sync_bool_compare_and_swap(&m_recs, oldhead, newrec.get()));

    std::auto_ptr<hazard_ptr> ret(new hazard_ptr(newrec.get()));
    newrec.release();
    return ret;
}

template <typename T, size_t P>
inline
hazard_ptrs<T, P> :: hazard_ptr :: ~hazard_ptr() throw ()
{
    for (size_t i = 0; i < P; ++i)
    {
        set(i, NULL);
    }

    __sync_lock_release(&m_rec->taslock);
}

template <typename T, size_t P>
inline void
hazard_ptrs<T, P> :: hazard_ptr :: set(size_t ptr_num, T* ptr)
{
    m_rec->ptrs[ptr_num] = ptr;
    __sync_synchronize();
}

template <typename T, size_t P>
inline void
hazard_ptrs<T, P> :: hazard_ptr :: retire(T* ptr)
{
    size_t i;

    for (i = 0; i < m_rec->rlist.size(); ++i)
    {
        if (m_rec->rlist[i] == NULL)
        {
            m_rec->rlist[i] = ptr;
            break;
        }
    }

    if (i == m_rec->rlist.size())
    {
        m_rec->rlist.push_back(ptr);
    }

    ++m_rec->rcount;

    if (m_rec->rcount >= m_rec->m_parent.m_num_recs * P * 1.2)
    {
        m_rec->scan();
    }
}

template <typename T, size_t P>
hazard_ptrs<T, P> :: hazard_ptr :: hazard_ptr(hazard_rec* rec)
    : m_rec(rec)
{
}

template <typename T, size_t P>
hazard_ptrs<T, P> :: hazard_rec :: hazard_rec(hazard_ptrs& parent)
    : taslock(0)
    , next(NULL)
    , ptrs()
    , rcount(0)
    , rlist()
    , m_parent(parent)
{
    for (size_t i = 0; i < P; ++i)
    {
        ptrs[i] = NULL;
    }
}

template <typename T, size_t P>
hazard_ptrs<T, P> :: hazard_rec :: ~hazard_rec() throw ()
{
}

template <typename T, size_t P>
void
hazard_ptrs<T, P> :: hazard_rec :: scan()
{
    hazard_rec* rec = m_parent.m_recs;
    std::set<const T*> hazardous;

    while (rec != NULL)
    {
        for (size_t i = 0; i < P; ++i)
        {
            const T* ref = rec->ptrs[i];

            if (ref)
            {
                hazardous.insert(ref);
            }
        }

        rec = rec->next;
    }

    std::vector<const T*> tmp_rlist;
    tmp_rlist.swap(rlist);
    rcount = 0;

    for (size_t i = 0; i < tmp_rlist.size(); ++i)
    {
        if (hazardous.find(tmp_rlist[i]) != hazardous.end())
        {
            rlist.push_back(tmp_rlist[i]);
            ++rcount;
        }
        else
        {
            delete tmp_rlist[i];
        }
    }
}

} // namespace e

#endif // e_hazard_ptrs_h_

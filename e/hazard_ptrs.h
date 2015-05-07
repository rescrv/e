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
#include <assert.h>

// STL
#include <memory>
#include <set>
#include <vector>

// e
#include <e/atomic.h>
#include <e/guard.h>

// This mostly follows the safe memory reclamation method described in
//
//     Maged M. Michael: Hazard Pointers: Safe Memory Reclamation for Lock-Free
//     Objects. IEEE Trans. Parallel Distrib. Syst. 15(6): 491-504 (2004)
//
// There is one specific thing to note:  An object is only deleted once "retire"
// is called on it, *and* there are no remaining hazard pointers pointing to it.
// "retire" does not alter the current hazard record's pointers, so a call to
// "set(ptr)", "retire(ptr)" allows the object to be used until the pointer is
// unset, or the hazard record is released (which implicitly unsets pointers).

namespace e
{

template <typename T, size_t P, typename S = char>
class hazard_ptrs
{
    public:
        class hazard_ptr;

    public:
        hazard_ptrs();
        ~hazard_ptrs() throw ();

    public:
        void force_scan();
        std::auto_ptr<hazard_ptr> get();

    private:
        class hazard_rec;

    private:
        hazard_ptrs(const hazard_ptrs&);

    private:
        hazard_ptrs& operator = (const hazard_ptrs&);

    private:
        hazard_rec* m_recs;
        uint64_t m_num_recs;
};

template <typename T, size_t P, typename S>
class hazard_ptrs<T, P, S> :: hazard_ptr
{
    public:
        ~hazard_ptr() throw ();

    public:
        void set(size_t ptr_num, T* ptr);
        void retire(T* ptr);
        S& state() { return m_rec->state; }

    private:
        friend class hazard_ptrs<T, P, S>;

    private:
        hazard_ptr(hazard_rec* rec);
        hazard_ptr(const hazard_ptr& rec);

    private:
        hazard_ptr& operator = (const hazard_ptr& rhs);

    private:
        hazard_rec* m_rec;
};

template <typename T, size_t P, typename S>
class hazard_ptrs<T, P, S> :: hazard_rec
{
    public:
        ~hazard_rec() throw ();

    public:
        void scan();

    public:
        uint32_t taslock;
        hazard_rec* next;
        T* ptrs[P];
        uint64_t rcount;
        std::vector<const T*> rlist;
        S state;

    private:
        friend class hazard_ptrs<T, P, S>;

    private:
        hazard_rec(hazard_ptrs& parent);
        hazard_rec(const hazard_rec&);

    private:
        hazard_rec& operator = (const hazard_rec&);

    private:
        hazard_ptrs& m_parent;
};

template <typename T, size_t P, typename S>
inline
hazard_ptrs<T, P, S> :: hazard_ptrs()
    : m_recs()
    , m_num_recs()
{
    using namespace e::atomic;
    store_ptr_nobarrier(&m_recs, static_cast<hazard_rec*>(NULL));
    store_64_nobarrier(&m_num_recs, 0);
}

template <typename T, size_t P, typename S>
inline
hazard_ptrs<T, P, S> :: ~hazard_ptrs() throw ()
{
    using namespace e::atomic;
    store_64_nobarrier(&m_num_recs, 0);
    hazard_rec* rec;

    while ((rec = load_ptr_acquire(&m_recs)))
    {
        while (exchange_32_nobarrier(&m_recs->taslock, 1) != 0)
            ;

        rec->scan();
        store_ptr_release(&m_recs, rec->next);
        delete rec;
    }
}

template <typename T, size_t P, typename S>
inline void
hazard_ptrs<T, P, S> :: force_scan()
{
    using namespace e::atomic;
    hazard_rec* rec = load_ptr_acquire(&m_recs);

    while (rec)
    {
        while (exchange_32_nobarrier(&rec->taslock, 1) != 0)
            ;

        for (size_t i = 0; i < P; ++i)
        {
            rec->set(i, NULL);
        }

        store_32_nobarrier(&rec->taslock, 0);
        rec = load_ptr_acquire(&rec->next);
    }

    while (rec)
    {
        while (exchange_32_nobarrier(&rec->taslock, 1) != 0)
            ;

        rec->scan();
        store_32_nobarrier(&rec->taslock, 0);
        rec = load_ptr_acquire(&rec->next);
    }
}

template <typename T, size_t P, typename S>
inline std::auto_ptr<typename e::hazard_ptrs<T, P, S>::hazard_ptr>
hazard_ptrs<T, P, S> :: get()
{
    using namespace e::atomic;
    hazard_rec* rec = load_ptr_acquire(&m_recs);

    while (rec)
    {
        if (exchange_32_nobarrier(&rec->taslock, 1) == 0)
        {
            e::guard g = e::makeguard(store_32_nobarrier, &rec->taslock, 0);
            std::auto_ptr<hazard_ptr> ret(new hazard_ptr(rec));
            g.dismiss();
            return ret;
        }

        rec = load_ptr_acquire(&rec->next);
    }

    std::auto_ptr<hazard_rec> newrec(new hazard_rec(*this));
    store_32_nobarrier(&newrec->taslock, 1);
    e::guard g = e::makeguard(store_32_nobarrier, &rec->taslock, 0);
    hazard_rec* oldhead;

    do
    {
        oldhead = load_ptr_acquire(&m_recs);
        store_ptr_nobarrier(&newrec->next, oldhead);
    }
    while (compare_and_swap_ptr_release(&m_recs, oldhead, newrec.get()) != oldhead);

    std::auto_ptr<hazard_ptr> ret(new hazard_ptr(newrec.get()));
    newrec.release();
    g.dismiss();
    return ret;
}

template <typename T, size_t P, typename S>
inline
hazard_ptrs<T, P, S> :: hazard_ptr :: ~hazard_ptr() throw ()
{
    using namespace e::atomic;

    for (size_t i = 0; i < P; ++i)
    {
        set(i, NULL);
    }

    store_32_release(&m_rec->taslock, 0);
}

template <typename T, size_t P, typename S>
inline void
hazard_ptrs<T, P, S> :: hazard_ptr :: set(size_t ptr_num, T* ptr)
{
    using namespace e::atomic;
    store_ptr_fullbarrier(&(m_rec->ptrs[ptr_num]), ptr);
}

template <typename T, size_t P, typename S>
inline void
hazard_ptrs<T, P, S> :: hazard_ptr :: retire(T* ptr)
{
    // Operations on rcount/rlist don't happen with protection because the
    // taslock belonging to m_rec is acquired/released to provide the necessary
    // synchronization.
    using namespace e::atomic;
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

    if (m_rec->rcount >= load_64_nobarrier(&m_rec->m_parent.m_num_recs) * P * 1.2)
    {
        m_rec->scan();
    }
}

template <typename T, size_t P, typename S>
hazard_ptrs<T, P, S> :: hazard_ptr :: hazard_ptr(hazard_rec* rec)
    : m_rec(rec)
{
}

template <typename T, size_t P, typename S>
hazard_ptrs<T, P, S> :: hazard_rec :: hazard_rec(hazard_ptrs& parent)
    : taslock(0)
    , next(NULL)
    , ptrs()
    , rcount(0)
    , rlist()
    , state()
    , m_parent(parent)
{
    using namespace e::atomic;
    store_32_nobarrier(&taslock, 0);
    store_ptr_nobarrier(&next, static_cast<hazard_rec*>(NULL));
    store_64_nobarrier(&rcount, 0);

    for (size_t i = 0; i < P; ++i)
    {
        store_ptr_nobarrier(&ptrs[i], static_cast<T*>(NULL));
    }
}

template <typename T, size_t P, typename S>
hazard_ptrs<T, P, S> :: hazard_rec :: ~hazard_rec() throw ()
{
}

template <typename T, size_t P, typename S>
void
hazard_ptrs<T, P, S> :: hazard_rec :: scan()
{
    using namespace e::atomic;
    hazard_rec* rec = load_ptr_nobarrier(&m_parent.m_recs);
    std::set<const T*> hazardous;

    while (rec != NULL)
    {
        for (size_t i = 0; i < P; ++i)
        {
            const T* ref = load_ptr_nobarrier(&rec->ptrs[i]);

            if (ref)
            {
                hazardous.insert(ref);
            }
        }

        rec = load_ptr_nobarrier(&rec->next);
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

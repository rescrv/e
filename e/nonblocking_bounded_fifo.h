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

#ifndef e_nonblocking_bounded_fifo_h_
#define e_nonblocking_bounded_fifo_h_

// C
#include <assert.h>

// e
#include <e/atomic.h>

namespace e
{

// A non-blocking bounded buffer allowing an arbitrary number of producers and
// consumers.
//
// This is not linearizable because a "push" may fail because the queue is full,
// yet the last item is not poppable yet.  It does, however, avoid spurious
// failures due to memory visibility issues.  If the object has been pushed, it
// can be popped.

template <typename T>
class nonblocking_bounded_fifo
{
    public:
        nonblocking_bounded_fifo(size_t sz);
        ~nonblocking_bounded_fifo() throw ();

    public:
        bool push(T data);
        bool pop(T* data);

    private:
        class element;

    private:
        nonblocking_bounded_fifo(const nonblocking_bounded_fifo&);

    private:
        nonblocking_bounded_fifo& operator = (const nonblocking_bounded_fifo&);

    private:
#ifdef _MSC_VER
		__declspec(align(64)) uint64_t m_push;
		__declspec(align(64)) uint64_t m_pop;
#else
        uint64_t m_push __attribute__ ((aligned (64)));
        uint64_t m_pop __attribute__ ((aligned (64)));
#endif
        const size_t m_sz;
        element* m_elems;
};

template <typename T>
nonblocking_bounded_fifo<T> :: nonblocking_bounded_fifo(size_t sz)
    : m_push(0)
    , m_pop(0)
    , m_sz(sz)
    , m_elems(NULL)
{
    using namespace e::atomic;
    assert(m_sz >= 2);
    assert((m_sz & (m_sz - 1)) == 0);
    element* elems = new element[m_sz];

    for (size_t i = 0; i < m_sz; ++i)
    {
        store_64_nobarrier(&elems[i].count, i);
    }

    store_64_nobarrier(&m_push, 0);
    store_64_nobarrier(&m_pop, 0);
    store_ptr_release(&m_elems, elems);
}

template <typename T>
nonblocking_bounded_fifo<T> :: ~nonblocking_bounded_fifo() throw ()
{
    element* elems = e::atomic::load_ptr_acquire(&m_elems);
    delete[] elems;
}

template <typename T>
bool
nonblocking_bounded_fifo<T> :: push(T t)
{
    using namespace e::atomic;
    element* elems = load_ptr_acquire(&m_elems);
    element* elem;
    uint64_t po;
    uint64_t pu;
    uint64_t count;

    while (true)
    {
        pu = load_64_nobarrier(&m_push);
        elem = elems + (pu & (m_sz - 1));
        count = load_64_acquire(&elem->count);
        po = load_64_nobarrier(&m_pop);

        // If the next in the sequence agrees with the elements count, then the
        // element must be empty or being updated.
        if (count == pu)
        {
            if (compare_and_swap_64_nobarrier(&m_push, pu, pu + 1) == pu)
            {
                break;
            }
        }
        // Otherwise we know that the m_push sequence number has wrapped
        // around.  If that is the case, the only stable explanation is that the
        // item is unpopped.  Because we read "po" after the acquire barrier on
        // &elem->count, we know that it is a value that is recent enough to
        // have existed concurrently with this operation.  Therefore, we take
        // that as the linearization point that things fail.
        else if (count == pu - m_sz + 1 && pu == po + m_sz)
        {
            return false;
        }
    }

    elem->t = t;
    store_64_release(&elem->count, pu + 1);
    return true;
}

template <typename T>
bool
nonblocking_bounded_fifo<T> :: pop(T* t)
{
    using namespace e::atomic;
    element* elems = load_ptr_acquire(&m_elems);
    element* elem;
    uint64_t po;
    uint64_t pu;
    uint64_t count;

    while (true)
    {
        po = load_64_nobarrier(&m_pop);
        elem = elems + (po & (m_sz - 1));
        count = load_64_acquire(&elem->count);
        pu = load_64_nobarrier(&m_push);

        // If the next in the sequence is off by one from the element's count, then the
        // element must be valid or in the process of removal
        if (count == po + 1)
        {
            if (compare_and_swap_64_nobarrier(&m_pop, po, po + 1) == po)
            {
                break;
            }
        }
        // Otherwise we know that there is nothing to pop.  Because we read "pu"
        // after the acquire barrier on &elem->count, we know that it is a value
        // that is recent enough to have existed concurrently with this
        // operation.  Therefore, we take that as the linearization point that
        // things fail because the queue is empty.
        else if (count == po && pu == po)
        {
            return false;
        }
    }

    *t = elem->t;
    T fresh = T();
    elem->t = fresh;
    store_64_release(&elem->count, po + m_sz);
    return true;
}

template <typename T>
struct nonblocking_bounded_fifo<T>::element
{
    uint64_t count;
    T t;
};

} // namespace e

#endif // e_nonblocking_bounded_fifo_h_

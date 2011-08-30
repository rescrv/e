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

#ifndef e_locking_iterable_fifo_h_
#define e_locking_iterable_fifo_h_

// C
#include <cassert>
#include <cstdlib>

// STL
#include <memory>

// po6
#include <po6/threads/spinlock.h>

namespace e
{

// This class is a concurrent fifo.  Many threads may push onto the fifo at a
// time, and many may iterate concurrently.  Additionally, threads may remove
// items one at a time using external synchronization.  Here's the intended
// behavior:
//
// - Items may be appended at any time.
// - Items are inserted in a total order.
// - Threads iterating the fifo will see every item inserted after creating the
//   iterator.
// - A removed item will be visible to all threads which constructed an
//   iterator to the log prior to the item's removal.
// - It is up to the caller to provide synchronization across different calls
//   dealing with the oldest item.

template <typename N>
class locking_iterable_fifo
{
    public:
        class iterator;

    public:
        locking_iterable_fifo();
        ~locking_iterable_fifo() throw ();

    public:
        bool empty();
        N& oldest();
        iterator iterate();
        void append(const N&);
        void remove_oldest();

    private:
        class node;

    private:
        locking_iterable_fifo(const locking_iterable_fifo&);

    private:
        locking_iterable_fifo& operator = (const locking_iterable_fifo&);

    private:
        void step_list(node** pos);
        void release(node* pos);

    private:
        // Lock acquire order is order of declaration.
        po6::threads::spinlock m_head_lock;
        po6::threads::spinlock m_tail_lock;
        node* m_head;
        node* m_tail;
};

template <typename N>
class locking_iterable_fifo<N> :: iterator
{
    public:
        iterator(const iterator&);
        ~iterator() throw ();

    public:
        bool valid();
        void next();

    public:
        N& operator * () const throw();
        N* operator -> () const throw();
        iterator& operator = (const iterator&);

    private:
        friend class locking_iterable_fifo<N>;

    private:
        iterator(locking_iterable_fifo<N>* lif, locking_iterable_fifo<N>::node* node);

    private:
        locking_iterable_fifo* m_l;
        locking_iterable_fifo::node* m_n;
        bool m_valid;
};

template <typename N>
class locking_iterable_fifo<N> :: node
{
    public:
        node();
        node(const N& val);
        ~node() throw ();

    public:
        int inc() { return __sync_add_and_fetch(&m_ref, 1); }
        int dec() { return __sync_sub_and_fetch(&m_ref, 1); }

    private:
        friend class locking_iterable_fifo<N>;

    private:
        node(const node&);

    private:
        node& operator = (const node&);

    private:
        int m_ref;
        node* m_next;
        bool m_real;
        bool m_gone;
        N m_val;
};

template <typename N>
locking_iterable_fifo<N> :: locking_iterable_fifo()
    : m_head_lock()
    , m_tail_lock()
    , m_head(NULL)
    , m_tail(NULL)
{
    m_head = m_tail = new node();
    int ref = m_head->inc();
    assert(ref == 1);
}

template <typename N>
locking_iterable_fifo<N> :: ~locking_iterable_fifo() throw()
{
    release(m_head);
}

template <typename N>
typename locking_iterable_fifo<N>::iterator
locking_iterable_fifo<N> :: iterate()
{
    po6::threads::spinlock::hold hold_hd(&m_head_lock);
    m_head->inc();
    iterator i(this, m_head);
    return i;
}

template <typename N>
bool
locking_iterable_fifo<N> :: empty()
{
    po6::threads::spinlock::hold hold(&m_head_lock);
    return !m_head->m_gone && m_head->m_real;
}

template <typename N>
N&
locking_iterable_fifo<N> :: oldest()
{
    po6::threads::spinlock::hold hold(&m_head_lock);

    if (!m_head->m_real && m_head->m_next)
    {
        return m_head->m_next->m_val;
    }
    else
    {
        return m_head->m_val;
    }
}

template <typename N>
void
locking_iterable_fifo<N> :: append(const N& n)
{
    std::auto_ptr<node> newnode(new node(n));
    int ref = newnode->inc();
    assert(ref == 1);
    po6::threads::spinlock::hold hold(&m_tail_lock);
    m_tail->m_next = newnode.get();
    m_tail = m_tail->m_next;
    newnode.release();
}

template <typename N>
void
locking_iterable_fifo<N> :: remove_oldest()
{
    po6::threads::spinlock::hold hold_hd(&m_head_lock);

    while (m_head->m_gone || !m_head->m_real)
    {
        // We grab this lock as an acquire barrier.  We need to know that our
        // value of m_next is current.  The easiest way to do so is to prevent
        // it from changing 0->PTR after our load.
        po6::threads::spinlock::hold hold_hd(&m_tail_lock);

        if (m_head->m_next)
        {
            step_list(&m_head);
        }
        else
        {
            // We know for sure there is nothing to remove.
            return;
        }
    }

    if (m_head->m_next)
    {
        step_list(&m_head);
    }
    else
    {
        m_head->m_gone = true;
    }
}

#if 0
template <typename N>
void
locking_iterable_fifo<N> :: remove_oldest()
{

    if (m_head->m_next)
    {
        step_list(&m_head);
    }


    assert(m_head->m_removed < NS);
    ++m_head->m_removed;
    assert(m_head->m_removed <= NS);

    if (m_head->m_removed == NS)
    {
        // We have a dead segment to cleanup.
        po6::threads::spinlock::hold hold_tl(&m_tail_lock);

        if (!m_head->m_next)
        {
            assert(m_head == m_tail);
            add_new_node();
        }

        step_node(&m_head);
    }
}
#endif

template <typename N>
void
locking_iterable_fifo<N> :: step_list(node** pos)
{
    node* cur = *pos;
    node* next = cur->m_next;
    bool incr = false;
    int ref;

    if (next)
    {
        ref = next->inc();
        assert(ref >= 2);
        incr = true;
    }

    *pos = next;

    ref = cur->dec();
    assert(ref >= 0);

    if (ref == 0)
    {
        __sync_synchronize();

        if (cur->m_next && incr)
        {
            ref = cur->m_next->dec();
            assert(ref >= 1);
        }

        cur->m_next = NULL;
        delete cur;
    }
}

template <typename N>
void
locking_iterable_fifo<N> :: release(node* pos)
{
// XXX This way is super slow compared to what we could do.  On the other hand,
// it's much easier to maintain.  This implementation requires iterating
// iterating the whole log, which we shouldn't have to do; on the other hand,
// the typical usage pattern of an iterator is to iterate until the end anyway,
// so this is not a big concern).
    while (pos && pos->m_next)
    {
        step_list(&pos);
    }

    po6::threads::spinlock::hold hold(&m_tail_lock);

    while (pos)
    {
        step_list(&pos);
    }
}

template <typename N>
locking_iterable_fifo<N> :: iterator :: iterator(const iterator& other)
    : m_l(other.m_l)
    , m_n(other.m_n)
    , m_valid(other.m_valid)
{
    if (m_n)
    {
        int ref = m_n->inc();
        assert(ref >= 2);
    }
}

template <typename N>
locking_iterable_fifo<N> :: iterator :: ~iterator() throw ()
{
    m_l->release(m_n);
}

template <typename N>
bool
locking_iterable_fifo<N> :: iterator :: valid()
{
    while (m_n->m_next && (!m_valid || !m_n->m_real))
    {
        m_valid = true;
        m_l->step_list(&m_n);
    }

    return m_valid && m_n->m_real;
}

template <typename N>
void
locking_iterable_fifo<N> :: iterator :: next()
{
    m_valid = false;
}

template <typename N>
N&
locking_iterable_fifo<N> :: iterator :: operator * () const throw()
{
    return m_n->m_val;
}

template <typename N>
N*
locking_iterable_fifo<N> :: iterator :: operator -> () const throw()
{
    return &m_n->m_val;
}

template <typename N>
typename locking_iterable_fifo<N>::iterator&
locking_iterable_fifo<N> :: iterator :: operator = (const iterator& other)
{
    if (this == &other)
    {
        return *this;
    }

    m_l->release(m_n);
    m_l = other.m_m_l;
    m_n = other.m_m_n;
    m_valid = other.m_m_valid;

    if (m_n)
    {
        int ref = m_n->inc();
        assert(ref >= 2);
    }

    return *this;
}

template <typename N>
locking_iterable_fifo<N> :: iterator :: iterator(locking_iterable_fifo<N>* lif,
                                                 locking_iterable_fifo<N>::node* node)
    : m_l(lif)
    , m_n(node)
    , m_valid(true)
{
}

template <typename N>
locking_iterable_fifo<N> :: node :: node()
    : m_ref(0)
    , m_next(NULL)
    , m_real(false)
    , m_gone(false)
    , m_val()
{
}

template <typename N>
locking_iterable_fifo<N> :: node :: node(const N& n)
    : m_ref(0)
    , m_next(NULL)
    , m_real(true)
    , m_gone(false)
    , m_val(n)
{
}

template <typename N>
locking_iterable_fifo<N> :: node :: ~node() throw ()
{
}

} // namespace e

#endif // e_locking_iterable_fifo_h_

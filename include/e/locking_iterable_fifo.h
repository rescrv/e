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
        void advance_to(iterator newhead);

    private:
        class node;

    private:
        locking_iterable_fifo(const locking_iterable_fifo&);

    private:
        locking_iterable_fifo& operator = (const locking_iterable_fifo&);

    private:
        // This will remove all nodes from m_head that are dead and point to
        // another node.  It will leave at least one node.  If this node is a
        // dummy node, or is gone and does not point to a next node the fifo is
        // empty.  It is the caller's responsibility to hold m_head_lock.  When
        // checking after returning from this function, the caller should check
        // m_head->m_next in such a way that guarantees that it is current.
        void remove_dead_nodes();
        // This will step the list by one unit.  It is the caller's
        // responsibility to ensure that there does exist a "next" pointer.
        void step_list(node* volatile * pos);
        // Release a reference counted pointer.
        void release(node* pos);

    private:
        // Lock acquire order is order of declaration.
        po6::threads::spinlock m_head_lock;
        po6::threads::spinlock m_tail_lock;
        node* volatile m_head;
        node* volatile m_tail;
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
        locking_iterable_fifo<N>* m_l;
        typename locking_iterable_fifo<N>::node* m_n;
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
        int inc() volatile throw () { return __sync_add_and_fetch(&m_ref, 1); }
        int dec() volatile throw () { return __sync_sub_and_fetch(&m_ref, 1); }

    private:
        friend class locking_iterable_fifo<N>;

    private:
        node(const node&);

    private:
        node& operator = (const node&);

    private:
        volatile int m_ref;
        node* volatile m_next;
        volatile bool m_dummy;
        volatile bool m_gone;
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
bool
locking_iterable_fifo<N> :: empty()
{
    po6::threads::spinlock::hold hold_hd(&m_head_lock);
    remove_dead_nodes();

    if (m_head->m_dummy || m_head->m_gone)
    {
        po6::threads::spinlock::hold hold_tl(&m_tail_lock);
        return !m_head->m_next;
    }

    return false;
}

template <typename N>
N&
locking_iterable_fifo<N> :: oldest()
{
    po6::threads::spinlock::hold hold(&m_head_lock);
    remove_dead_nodes();
    // After removing dead nodes, we should have a valid node at the head of the
    // list.  If this is not true, then the list is empty.
    assert(!m_head->m_dummy && !m_head->m_gone);
    return m_head->m_val;
}

template <typename N>
typename locking_iterable_fifo<N>::iterator
locking_iterable_fifo<N> :: iterate()
{
    po6::threads::spinlock::hold hold_hd(&m_head_lock);
    iterator i(this, m_head);
    return i;
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
    m_tail = newnode.get();
    assert(m_tail == newnode.get());
    newnode.release();
}

template <typename N>
void
locking_iterable_fifo<N> :: remove_oldest()
{
    po6::threads::spinlock::hold hold(&m_head_lock);
    remove_dead_nodes();
    // After removing dead nodes, we should have a valid node at the head of the
    // list.  If this is not true, then the list is empty.
    m_head->m_gone = true;
    remove_dead_nodes();
}

template <typename N>
void
locking_iterable_fifo<N> :: advance_to(iterator newhead)
{
    node* oldhead = m_head;
    po6::threads::spinlock::hold hold(&m_head_lock);
    m_head = newhead.m_n;
    int ref = m_head->inc();
    assert(ref >= 3);
    m_head->m_gone = !newhead.m_valid;
    release(oldhead);
    remove_dead_nodes();
}

template <typename N>
void
locking_iterable_fifo<N> :: remove_dead_nodes()
{
    while (m_head->m_dummy || m_head->m_gone)
    {
        if (!m_head->m_next)
        {
            po6::threads::spinlock::hold hold(&m_tail_lock);

            if (!m_head->m_next)
            {
                assert(m_head == m_tail);
                return;
            }
        }

        step_list(&m_head);
    }
}

template <typename N>
void
locking_iterable_fifo<N> :: step_list(node* volatile * pos)
{
    assert((*pos)->m_next);
    int ref = (*pos)->m_next->inc();
    assert(ref >= 2);
    node* oldhead = *pos;
    *pos = (*pos)->m_next;
    assert(oldhead->m_next == *pos);
    ref = oldhead->dec();
    assert(ref >= 0);

    if (ref == 0)
    {
        ref = (*pos)->dec();
        assert(ref >= 1);
        delete oldhead;
    }
}

template <typename N>
void
locking_iterable_fifo<N> :: release(node* pos)
{
    while (pos)
    {
        int ref = pos->dec();
        assert(ref >= 0);

        if (ref == 0)
        {
            node* old = pos;

            if (!pos->m_next)
            {
                po6::threads::spinlock::hold hold(&m_tail_lock);
                pos = pos->m_next;
            }
            else
            {
                pos = pos->m_next;
            }

            delete old;
        }
        else
        {
            break;
        }
    }
}

template <typename N>
locking_iterable_fifo<N> :: iterator :: iterator(const iterator& other)
    : m_l(other.m_l)
    , m_n(other.m_n)
    , m_valid(other.m_valid)
{
    int ref = m_n->inc();
    assert(ref >= 2);
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
    while (m_n->m_next && (!m_valid || m_n->m_dummy))
    {
        m_valid = true;
        m_l->step_list(&m_n);
    }

    return m_valid && !m_n->m_dummy;
}

template <typename N>
void
locking_iterable_fifo<N> :: iterator :: next()
{
    m_valid = false;
    valid();
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
    m_l = other.m_l;
    m_n = other.m_n;
    m_valid = other.m_valid;

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
    , m_valid(!m_n->m_gone)
{
    int ref = m_n->inc();
    assert(ref >= 2);
}

template <typename N>
locking_iterable_fifo<N> :: node :: node()
    : m_ref(0)
    , m_next(NULL)
    , m_dummy(true)
    , m_gone(false)
    , m_val()
{
}

template <typename N>
locking_iterable_fifo<N> :: node :: node(const N& n)
    : m_ref(0)
    , m_next(NULL)
    , m_dummy(false)
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

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

template <typename N, size_t NS = 64>
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
        void add_new_node();
        void step_node(node** pos);
        void release(node* pos);

    private:
        // Lock acquire order is order of declaration.
        po6::threads::spinlock m_head_lock;
        po6::threads::spinlock m_tail_lock;
        node* m_head;
        node* m_tail;
};

template <typename N, size_t NS>
class locking_iterable_fifo<N, NS> :: iterator
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
        friend class locking_iterable_fifo<N, NS>;

    private:
        iterator(size_t pos, locking_iterable_fifo<N, NS>* lif, locking_iterable_fifo<N, NS>::node* node);

    private:
        bool m_valid;
        size_t m_pos;
        locking_iterable_fifo<N, NS>* m_lif;
        locking_iterable_fifo<N, NS>::node* m_node;
};

template <typename N, size_t NS>
class locking_iterable_fifo<N, NS> :: node
{
    public:
        node();
        ~node() throw ();

    public:
        int inc() { return __sync_add_and_fetch(&m_ref, 1); }
        int dec() { return __sync_sub_and_fetch(&m_ref, 1); }

    private:
        friend class locking_iterable_fifo<N, NS>;

    private:
        node(const node&);

    private:
        node& operator = (const node&);

    private:
        size_t m_ref; // Atomically modified
        node* m_next; // Protected by m_tail_lock
        N m_vals[NS]; // Protected by m_tail_lock
        size_t m_stored; // Protected by m_tail_lock
        size_t m_removed; // Protected by m_head_lock
};

template <typename N, size_t NS>
locking_iterable_fifo<N, NS> :: locking_iterable_fifo()
    : m_head_lock()
    , m_tail_lock()
    , m_head(NULL)
    , m_tail(NULL)
{
    std::auto_ptr<node> newnode(new node());
    int ref = newnode->inc();
    assert(ref == 1);
    m_tail = m_head = newnode.get();
    newnode.release();
}

template <typename N, size_t NS>
locking_iterable_fifo<N, NS> :: ~locking_iterable_fifo() throw()
{
    release(m_head);
}

template <typename N, size_t NS>
typename locking_iterable_fifo<N, NS>::iterator
locking_iterable_fifo<N, NS> :: iterate()
{
    po6::threads::spinlock::hold hold_hd(&m_head_lock);
    iterator i(m_head->m_removed, this, m_head);
    return i;
}

template <typename N, size_t NS>
bool
locking_iterable_fifo<N, NS> :: empty()
{
    po6::threads::spinlock::hold hold_hd(&m_head_lock);
    po6::threads::spinlock::hold hold_tl(&m_tail_lock);
    return m_head->m_removed == m_head->m_stored;
}

template <typename N, size_t NS>
N&
locking_iterable_fifo<N, NS> :: oldest()
{
    po6::threads::spinlock::hold hold(&m_head_lock);
    return m_head->m_vals[m_head->m_removed];
}

template <typename N, size_t NS>
void
locking_iterable_fifo<N, NS> :: append(const N& n)
{
    po6::threads::spinlock::hold hold(&m_tail_lock);

    if (m_tail->m_stored >= NS)
    {
        add_new_node();
    }

    m_tail->m_vals[m_tail->m_stored] = n;
    // We need the stored value to be visible before m_stored is.
    __sync_synchronize;
    ++m_tail->m_stored;
}

template <typename N, size_t NS>
void
locking_iterable_fifo<N, NS> :: remove_oldest()
{
    po6::threads::spinlock::hold hold_hd(&m_head_lock);

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

template <typename N, size_t NS>
void
locking_iterable_fifo<N, NS> :: add_new_node()
{
    std::auto_ptr<node> newnode(new node());
    int ref = newnode->inc();
    assert(ref == 1);
    m_tail->m_next = newnode.get();
    m_tail = newnode.get();
    newnode.release();
}

template <typename N, size_t NS>
void
locking_iterable_fifo<N, NS> :: step_node(node** pos)
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

template <typename N, size_t NS>
void
locking_iterable_fifo<N, NS> :: release(node* pos)
{
    // XXX This way is super slow compared to what we could do.  On the other
    // hand, it's much easier to maintain.  This implementation requires
    // iterating iterating the whole log, which we shouldn't have to do; on the
    // other hand, the typical usage pattern of an iterator is to iterate until
    // the end anyway, so this is not a big concern).
    while (pos && pos->m_next)
    {
        step_node(&pos);
    }

    po6::threads::spinlock::hold hold(&m_tail_lock);

    while (pos)
    {
        step_node(&pos);
    }
}

template <typename N, size_t NS>
locking_iterable_fifo<N, NS> :: iterator :: iterator(const iterator& other)
    : m_valid(other.m_valid)
    , m_pos(other.m_pos)
    , m_lif(other.m_lif)
    , m_node(other.m_node)
{
    if (m_node)
    {
        int ref = m_node->inc();
        assert(ref >= 2);
    }
}

template <typename N, size_t NS>
locking_iterable_fifo<N, NS> :: iterator :: ~iterator() throw ()
{
    m_lif->release(m_node);
}

template <typename N, size_t NS>
bool
locking_iterable_fifo<N, NS> :: iterator :: valid()
{
    while (!m_valid)
    {
        __sync_synchronize();

        if (m_pos == NS)
        {
            if (m_node->m_next)
            {
                m_lif->step_node(&m_node);
                m_pos = 0;
            }
            else
            {
                return false;
            }
        }
        else
        {
            if (m_pos == m_node->m_stored)
            {
                return false;
            }
            else
            {
                m_valid = true;
            }
        }
    }

    return true;
}

template <typename N, size_t NS>
void
locking_iterable_fifo<N, NS> :: iterator :: next()
{
    ++m_pos;
    m_valid = false;
}

template <typename N, size_t NS>
N&
locking_iterable_fifo<N, NS> :: iterator :: operator * () const throw()
{
    return m_node->m_vals[m_pos];
}

template <typename N, size_t NS>
N*
locking_iterable_fifo<N, NS> :: iterator :: operator -> () const throw()
{
    return m_node->m_vals[m_pos];
}

template <typename N, size_t NS>
typename locking_iterable_fifo<N, NS>::iterator&
locking_iterable_fifo<N, NS> :: iterator :: operator = (const iterator& other)
{
    if (this == &other)
    {
        return;
    }

    m_lif->release(m_node);
    m_valid = other.m_valid;
    m_pos = other.m_pos;
    m_lif = other.m_lif;
    m_node = other.m_node;

    if (m_node)
    {
        int ref = m_node->inc();
        assert(ref >= 2);
    }

    return *this;
}

template <typename N, size_t NS>
locking_iterable_fifo<N, NS> :: iterator :: iterator(size_t pos,
                                                     locking_iterable_fifo<N, NS>* lif,
                                                     locking_iterable_fifo<N, NS>::node* node)
    : m_valid(false)
    , m_pos(pos)
    , m_lif(lif)
    , m_node(node)
{
    if (m_node)
    {
        int ref = m_node->inc();
        assert(ref >= 2);
    }
}

template <typename N, size_t NS>
locking_iterable_fifo<N, NS> :: node :: node()
    : m_ref(0)
    , m_next(NULL)
    , m_vals()
    , m_stored(0)
    , m_removed(0)
{
}

template <typename N, size_t NS>
locking_iterable_fifo<N, NS> :: node :: ~node() throw ()
{
}

} // namespace e

#endif // e_locking_iterable_fifo_h_

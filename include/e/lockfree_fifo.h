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

#ifndef e_lockfree_fifo_h_
#define e_lockfree_fifo_h_

// C
#include <cstdlib>

// STL
#include <memory>

// e
#include <e/hazard_ptrs.h>

namespace e
{

template <typename T>
class lockfree_fifo
{
    public:
        lockfree_fifo();
        ~lockfree_fifo() throw ();

    public:
        // Return true if the queue is temporarily empty.  If there are no other
        // operations in progress, then this will intuitively correspond to the
        // notion of empty() in the STL queue.
        bool optimistically_empty();
        void push(const T& val);
        bool pop(T* val);

    private:
        class node;

    private:
        lockfree_fifo(const lockfree_fifo&);

    private:
        lockfree_fifo& operator = (const lockfree_fifo&);

    private:
        hazard_ptrs<node, 2> m_hazards;
        node* m_head;
        node* m_tail;
        size_t m_estimate;
};

template <typename T>
lockfree_fifo<T> :: lockfree_fifo()
    : m_hazards()
    , m_head(new node())
    , m_tail(m_head)
    , m_estimate(0)
{
}

template <typename T>
lockfree_fifo<T> :: ~lockfree_fifo() throw ()
{
    std::auto_ptr<typename hazard_ptrs<node, 2>::hazard_ptr> hptr = m_hazards.get();

    while (m_head)
    {
        hptr->set(0, m_head);
        hptr->retire(m_head);
        m_head = m_head->next;
    }
}

template <typename T>
bool
lockfree_fifo<T> :: optimistically_empty()
{
    return __sync_add_and_fetch(&m_estimate, 0) == 0;
}

template <typename T>
void
lockfree_fifo<T> :: push(const T& val)
{
    __sync_add_and_fetch(&m_estimate, 1);
    std::auto_ptr<typename hazard_ptrs<node, 2>::hazard_ptr> hptr = m_hazards.get();
    node* tail;
    node* next;
    std::auto_ptr<node> n(new node(NULL, val));

    while (true)
    {
        tail = m_tail;
        hptr->set(0, tail);

        // Check to make sure the tail reference wasn't changed.
        if (tail != m_tail)
        {
            continue;
        }

        next = tail->next;

        // Make sure that it still hasn't changed.
        if (tail != m_tail)
        {
            continue;
        }

        // If we need to swing the tail pointer.
        if (next)
        {
            __sync_bool_compare_and_swap(&m_tail, tail, next);
            continue;
        }

        // Set our node as the next one.  We win!
        if (__sync_bool_compare_and_swap(&tail->next, NULL, n.get()))
        {
            break;
        }
    }

    // Swing the tail pointer.
    __sync_bool_compare_and_swap(&m_tail, tail, n.get());
    n.release();
}

template <typename T>
bool
lockfree_fifo<T> :: pop(T* val)
{
    std::auto_ptr<typename hazard_ptrs<node, 2>::hazard_ptr> hptr = m_hazards.get();
    node* head;
    node* tail;
    node* next;

    while (true)
    {
        head = m_head;
        hptr->set(0, head);

        // Check to make sure the head reference wasn't changed.
        if (head != m_head)
        {
            continue;
        }

        tail = m_tail;
        next = head->next;
        hptr->set(1, next);

        // Check that the head is still valid.
        if (head != m_head)
        {
            continue;
        }

        // Check if there is an element to return.
        if (next == NULL)
        {
            return false;
        }

        // Swing the tail.
        if (head == tail)
        {
            __sync_bool_compare_and_swap(&m_tail, tail, next);
            continue;
        }

        if (__sync_bool_compare_and_swap(&m_head, head, next))
        {
            break;
        }
    }

    // XXX We should put a scope guard in place to make sure we retire head
    // even if the assignment fails.
    *val = next->data;
    hptr->retire(head);
    __sync_sub_and_fetch(&m_estimate, 1);
    return true;
}

template <typename T>
class lockfree_fifo<T> :: node
{
    public:
        node() : next(NULL), data() {}
        node(node* n, const T& d) : next(n), data(d) {}

    public:
        node* next;
        T data;
};

} // namespace e

#endif // e_lockfree_fifo_h_

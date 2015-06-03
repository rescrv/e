// Copyright (c) 2015, Robert Escriva
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

#ifndef e_lockfree_mpsc_fifo_h_
#define e_lockfree_mpsc_fifo_h_

// e
#include <e/atomic.h>
#include <e/garbage_collector.h>

namespace e
{

template <typename T>
class lockfree_mpsc_fifo
{
    public:
        lockfree_mpsc_fifo();
        ~lockfree_mpsc_fifo() throw ();

    public:
        void push(T* val);
        bool pop(e::garbage_collector* gc, T** val);

    private:
        class node;

    private:
        node* m_head;
        node* m_tail;

    private:
        lockfree_mpsc_fifo(const lockfree_mpsc_fifo&);
        lockfree_mpsc_fifo& operator = (const lockfree_mpsc_fifo&);
};

template <typename T>
class lockfree_mpsc_fifo<T> :: node
{
    public:
        node() : next(NULL), data(NULL) {}
        node(T* d) : next(NULL), data(d) {}
        ~node() throw () { if (data) { delete data; } }

    public:
        node* next;
        T* data;

    private:
        node(const node&);
        node& operator = (const node&);
};

template <typename T>
lockfree_mpsc_fifo<T> :: lockfree_mpsc_fifo()
    : m_head(new node())
    , m_tail(m_head)
{
    atomic::memory_barrier();
}

template <typename T>
lockfree_mpsc_fifo<T> :: ~lockfree_mpsc_fifo() throw ()
{
    while (atomic::load_ptr_acquire(&m_head))
    {
        node* tmp = atomic::load_ptr_acquire(&m_head);
        node* next = atomic::load_ptr_acquire(&tmp->next);
        atomic::store_ptr_nobarrier(&m_head, next);
        delete tmp;
    }
}

template <typename T>
void
lockfree_mpsc_fifo<T> :: push(T* val)
{
    node* nn = new node(val);
    node* tail;

    while (true)
    {
        tail = atomic::load_ptr_acquire(&m_tail);
        node* next = atomic::compare_and_swap_ptr_fullbarrier(&tail->next, (node*)NULL, nn);

        if (next == NULL)
        {
            break;
        }
        else
        {
            atomic::compare_and_swap_ptr_fullbarrier(&m_tail, tail, next);
        }
    }

    atomic::compare_and_swap_ptr_fullbarrier(&m_tail, tail, nn);
}

template <typename T>
bool
lockfree_mpsc_fifo<T> :: pop(e::garbage_collector* gc, T** val)
{
    atomic::memory_barrier();
    node* head = atomic::load_ptr_acquire(&m_head);
    node* tail = atomic::load_ptr_acquire(&m_tail);
    node* next = atomic::load_ptr_acquire(&head->next);

    if (next == NULL)
    {
        return false;
    }

    if (head == tail)
    {
        atomic::compare_and_swap_ptr_fullbarrier(&m_tail, tail, next);
    }

    atomic::store_ptr_release(&m_head, next);
    *val = atomic::load_ptr_acquire(&next->data);
    atomic::store_ptr_release(&next->data, (T*)NULL);
    gc->collect(head, garbage_collector::free_ptr<node>);
    return true;
}

} // namespace e

#endif // e_lockfree_mpsc_fifo_h_

// Copyright (c) 2014, Robert Escriva
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

#define __STDC_LIMIT_MACROS

// STL
#include <algorithm>
#include <vector>

// e
#include "e/garbage_collector.h"

using e::garbage_collector;

class garbage_collector::thread_state_node
{
    public:
        static bool heap_cmp(const garbage& lhs, const garbage& rhs);

    public:
        thread_state_node()
            : next(NULL), quiescent_timestamp(1), offline_timestamp(0), heap() {}
        ~thread_state_node() throw () {}

    public:
        void purge(uint64_t min_timestamp);

    public:
        thread_state_node* next;
        uint64_t quiescent_timestamp;
        uint64_t offline_timestamp;
        po6::threads::mutex heap_mtx;
        std::vector<garbage> heap;

    private:
        thread_state_node(const thread_state_node&);
        thread_state_node& operator = (const thread_state_node&);
};

class garbage_collector::garbage
{
    public:
        garbage()
            : next(NULL), timestamp(0), ptr(NULL), func(NULL) {}
        garbage(uint64_t t, void* p, void (*f)(void*))
            : next(NULL), timestamp(t), ptr(p), func(f) {}
        garbage(const garbage& other)
            : next(other.next)
            , timestamp(other.timestamp)
            , ptr(other.ptr)
            , func(other.func)
        {
        }

        garbage& operator = (const garbage& rhs)
        {
            // self-assign-safe
            next = rhs.next;
            timestamp = rhs.timestamp;
            ptr = rhs.ptr;
            func = rhs.func;
            return *this;
        }

    public:
        garbage* next;
        uint64_t timestamp;
        void* ptr;
        void (*func)(void* ptr);
};

bool
garbage_collector :: thread_state_node :: heap_cmp(const garbage& lhs,
                                                   const garbage& rhs)
{
    // not an accident; STL heap maximizes, so we need our "less than"
    // operator to be inverted.
    return lhs.timestamp > rhs.timestamp;
}

void
garbage_collector :: thread_state_node :: purge(uint64_t min_timestamp)
{
    po6::threads::mutex::hold hold(&heap_mtx);

    // purge from the heap all items less than min_timestamp
    while (!heap.empty() && heap[0].timestamp < min_timestamp)
    {
        const garbage& g(heap[0]);
        g.func(g.ptr);
        std::pop_heap(heap.begin(), heap.end(), thread_state_node::heap_cmp);
        heap.pop_back();
    }
}

garbage_collector :: garbage_collector()
    : m_timestamp(0)
    , m_offline_transitions(0)
    , m_minimum(0)
    , m_registered(NULL)
    , m_garbage()
    , m_protect_registration()
{
    po6::threads::mutex::hold hold(&m_protect_registration);
    e::atomic::store_64_nobarrier(&m_timestamp, 2);
    e::atomic::store_64_nobarrier(&m_offline_transitions, 0);
    e::atomic::store_ptr_nobarrier(&m_garbage, static_cast<garbage*>(NULL));
}

garbage_collector :: ~garbage_collector() throw ()
{
    po6::threads::mutex::hold hold(&m_protect_registration);

    while (m_registered)
    {
        thread_state_node* tmp = m_registered;
        m_registered = tmp->next;
        delete tmp;
    }

    garbage* gc = e::atomic::load_ptr_acquire(&m_garbage);

    while (gc)
    {
        garbage* tmp = gc;
        gc = e::atomic::load_ptr_acquire(&tmp->next);
        tmp->func(tmp->ptr);
        delete tmp;
    }
}

void
garbage_collector :: register_thread(thread_state* ts)
{
    assert(ts->m_tsn == NULL);
    // create the new thread state node and save a ref to it
    thread_state_node* tsn = new thread_state_node();
    ts->m_tsn = tsn;
    po6::threads::mutex::hold hold(&m_protect_registration);
    e::atomic::store_ptr_nobarrier(&tsn->next, m_registered);
    e::atomic::store_ptr_release(&m_registered, tsn);
    uint64_t timestamp = read_timestamp();
    e::atomic::store_64_release(&tsn->quiescent_timestamp, timestamp);
}

void
garbage_collector :: deregister_thread(thread_state* ts)
{
    assert(ts->m_tsn != NULL);
    po6::threads::mutex::hold hold(&m_protect_registration);
    thread_state_node** ptr = &m_registered;
    thread_state_node* node = e::atomic::load_ptr_acquire(ptr);

    while (node && node != ts->m_tsn)
    {
        ptr  = &node->next;
        node = e::atomic::load_ptr_acquire(ptr);
    }

    // if these asserts fail, it means that ts is corrupt
    assert(node == ts->m_tsn);
    assert(*ptr == node);

    // unlink the node from the chain
    e::atomic::store_ptr_release(ptr, node->next);
    po6::threads::mutex::hold holdheap(&node->heap_mtx);

    for (size_t i = 0; i < node->heap.size(); ++i)
    {
        garbage* g = new garbage(node->heap[i]);
        enqueue(&m_garbage, g);
    }

    // now destroy the unlinked tsn
    collect(node, garbage_collector::free_ptr<thread_state_node>);
}

void
garbage_collector :: quiescent_state(thread_state* ts)
{
    using namespace e::atomic;
    thread_state_node* const tsn = ts->m_tsn;
    uint64_t timestamp = 0;
    uint64_t min_timestamp = 0;
    const uint64_t prev_min_timestamp = e::atomic::load_64_nobarrier(&m_minimum);

    while (true)
    {
        // This loop finds the largest timestamp that is less than each thread
        // state's quiescent_timestamp.  For every other thread, use the value
        // that they advertise in their thread_state_node.  For this thread, use
        // the value we read from the counter.

        // read the timestamp here
        timestamp = read_timestamp();
        min_timestamp = timestamp;
        assert(tsn->quiescent_timestamp < timestamp);

        // Figure out the total number of offline->online transitions that have
        // happened to date.
        uint64_t transitions = load_64_acquire(&m_offline_transitions);

        // Look through the list and find the timestamps
        thread_state_node* node = load_ptr_acquire(&m_registered);

        while (node)
        {
            if (node != tsn)
            {
                // quiescent before offline, so the acquire has effect
                uint64_t qst = load_64_acquire(&node->quiescent_timestamp);
                uint64_t oft = load_64_acquire(&node->offline_timestamp);

                // This node is online, so use its timestamp as the lower bound
                if (qst > oft)
                {
                    min_timestamp = std::min(qst, min_timestamp);
                }
                else
                {
                    node->purge(prev_min_timestamp);
                }
            }

            node = load_ptr_acquire(&node->next);
        }

        // This acts as a barrier between the read of transitions above, and the
        // read below, so that anyone who read the counter to update their
        // timestamps will show up.
        read_timestamp();

        if (transitions == load_64_acquire(&m_offline_transitions))
        {
            break;
        }
    }

    // At this point, min_timestamp is the smallest quiescent_timestamp across
    // all threads.  We know this because every thread falls into one of two
    // cases.  The first case is pretty simple:  we visited it in the above
    // loop, and observed its timestamp directly.
    //
    // The second case is considerably more complex and involves the state of a
    // thread that registered but did not get seen by this loop.  Because
    // read_timestamp is a full barrier operation, and draws from a sequential
    // counter, we know that one of two things happened.  Either the "register"
    // timestamp comes before the timestamp found in this function, or it comes
    // after.  If it comes before, we know that it would be seen by the loop
    // because the release-store on the timestamp in register, and acquire-load
    // on the timestamp here would ensure that the value stored to m_registered
    // became visible to us.  This would mean that it falls into the first
    // paragraph's case.
    //
    // In the other subcase, the "register" timestamp comes after timestamp
    // found in this function.  In this case, the minimum timestamp is the one
    // we selected above and we're all set.
    //
    // In all cases, the minimum timestamp is the smallest of every thread's
    // quiescent timestamp.

    while (compare_and_swap_64_nobarrier(&m_minimum, e::atomic::load_64_nobarrier(&m_minimum), min_timestamp) < min_timestamp)
        ;

    garbage* gc = load_ptr_nobarrier(&m_garbage);

    if (compare_and_swap_ptr_fullbarrier(&m_garbage, gc, static_cast<garbage*>(NULL)) != gc)
    {
        gc = NULL;
    }

    // expose our timestamp to the world
    // no need to force tsn->quiescent_timestamp to be visible with a call to
    // read_timestamp() because it's strictly increasing, and seeing a lower
    // value only delays garbage collection, but cannot hurt safety.
    store_64_release(&tsn->quiescent_timestamp, timestamp);

    tsn->purge(min_timestamp);

    while (gc)
    {
        garbage* next = load_ptr_acquire(&gc->next);

        if (gc->timestamp < min_timestamp)
        {
            gc->func(gc->ptr);
            delete gc;
        }
        else
        {
            po6::threads::mutex::hold holdheap(&tsn->heap_mtx);
            tsn->heap.push_back(garbage(gc->timestamp, gc->ptr, gc->func));
            std::push_heap(tsn->heap.begin(), tsn->heap.end(), thread_state_node::heap_cmp);
            tsn->heap.push_back(garbage(gc->timestamp, gc, free_ptr<garbage>));
            std::push_heap(tsn->heap.begin(), tsn->heap.end(), thread_state_node::heap_cmp);
        }

        gc = next;
    }
}

void
garbage_collector :: offline(thread_state* ts)
{
    using namespace e::atomic;
    thread_state_node* const tsn = ts->m_tsn;
    uint64_t timestamp = read_timestamp();
    assert(tsn->quiescent_timestamp < timestamp);
    assert(tsn->offline_timestamp < timestamp);
    // offline before quiescent, so the release mates with the acquire above
    store_64_release(&tsn->offline_timestamp, timestamp);
    store_64_release(&tsn->quiescent_timestamp, timestamp);
    read_timestamp();
}

void
garbage_collector :: online(thread_state* ts)
{
    using namespace e::atomic;
    thread_state_node* const tsn = ts->m_tsn;
    uint64_t timestamp = read_timestamp();
    assert(tsn->quiescent_timestamp < timestamp);
    assert(tsn->offline_timestamp < timestamp);
    store_64_release(&tsn->quiescent_timestamp, timestamp);

    while (compare_and_swap_64_nobarrier(&m_offline_transitions, e::atomic::load_64_nobarrier(&m_offline_transitions), timestamp) < timestamp)
        ;

    read_timestamp();
}

void
garbage_collector :: collect(void* ptr, void(*func)(void* ptr))
{
    garbage* g(new garbage(UINT64_MAX, ptr, func));
    uint64_t timestamp = read_timestamp();
    e::atomic::store_64_release(&g->timestamp, timestamp);
    enqueue(&m_garbage, g);
}

uint64_t
garbage_collector :: read_timestamp()
{
    // The rest of the code relies upon read_timestamp to be a fullbarrier, so
    // keep it so.
    return e::atomic::increment_64_fullbarrier(&m_timestamp, 1);
}

void
garbage_collector :: enqueue(garbage* volatile* list, garbage* g)
{
    garbage* expect = e::atomic::load_ptr_acquire(list);
    garbage* witness = expect;
    e::atomic::store_ptr_release(&g->next, expect);

    while ((witness = e::atomic::compare_and_swap_ptr_fullbarrier(list, expect, g)) != expect)
    {
        expect = witness;
        e::atomic::store_ptr_release(&g->next, expect);
    }
}

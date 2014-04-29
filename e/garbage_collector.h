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

#ifndef e_garbage_collector_h_
#define e_garbage_collector_h_

// C
#include <cassert>
#include <stdint.h>

// STL
#include <list>

// po6
#include <po6/threads/mutex.h>

// e
#include <e/atomic.h>

namespace e
{

class garbage_collector
{
    public:
        class thread_state;
        template<typename T> static void free_ptr(void* ptr) { delete static_cast<T*>(ptr); }

    public:
        garbage_collector();
        ~garbage_collector() throw ();

    public:
        void register_thread(thread_state* ts);
        void deregister_thread(thread_state* ts);
        void quiescent_state(thread_state* ts);
        void offline(thread_state* ts);
        void online(thread_state* ts);
        void collect(void* ptr, void(*func)(void* ptr));

    private:
        class thread_state_node;
        class garbage;
        // read_timestamp is a full-barrier operation
        uint64_t read_timestamp();

    private:
        uint64_t m_timestamp;
        uint64_t m_smallest_garbage;
        po6::threads::mutex m_protect_registration;
        thread_state_node* m_registered;
        po6::threads::mutex m_protect_garbage;
        std::list<garbage> m_garbage;

    private:
        garbage_collector(const garbage_collector&);
        garbage_collector& operator = (const garbage_collector&);
};

class garbage_collector::thread_state
{
    public:
        thread_state() : m_tsn(NULL) {}
        ~thread_state() throw () {}

    private:
        friend class garbage_collector;
        thread_state_node* m_tsn;

    private:
        thread_state(const thread_state&);
        thread_state& operator = (const thread_state&);
};

class garbage_collector::thread_state_node
{
    public:
        thread_state_node() : next(NULL), quiescent_timestamp(1), offline_timestamp(0) {}
        ~thread_state_node() throw () {}

    public:
        thread_state_node* next;
        uint64_t quiescent_timestamp;
        uint64_t offline_timestamp;

    private:
        thread_state_node(const thread_state_node&);
        thread_state_node& operator = (const thread_state_node&);
};

class garbage_collector::garbage
{
    public:
        garbage()
            : timestamp(0), ptr(NULL), func(NULL) {}
        garbage(uint64_t t, void* p, void (*f)(void*))
            : timestamp(t), ptr(p), func(f) {}
        garbage(const garbage& other)
            : timestamp(other.timestamp), ptr(other.ptr), func(other.func) {}
        ~garbage() throw () {}

    public:
        garbage& operator = (const garbage& rhs)
        {
            if (this != &rhs)
            {
                timestamp = rhs.timestamp;
                ptr = rhs.ptr;
                func = rhs.func;
            }

            return *this;
        }

    public:
        uint64_t timestamp;
        void* ptr;
        void (*func)(void* ptr);
};

inline
garbage_collector :: garbage_collector()
    : m_timestamp(2)
    , m_smallest_garbage(UINT64_MAX)
    , m_protect_registration()
    , m_registered(NULL)
    , m_protect_garbage()
    , m_garbage()
{
}

inline
garbage_collector :: ~garbage_collector() throw ()
{
    po6::threads::mutex::hold hold1(&m_protect_registration);
    po6::threads::mutex::hold hold2(&m_protect_garbage);
    std::list<garbage>::iterator it = m_garbage.begin();

    while (it != m_garbage.end())
    {
        it->func(it->ptr);
        it = m_garbage.erase(it);
    }

    while (m_registered)
    {
        thread_state_node* tmp = m_registered;
        m_registered = tmp->next;
        delete tmp;
    }
}

inline void
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

inline void
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

    // now destroy the unlinked tsn
    collect(node, garbage_collector::free_ptr<thread_state_node>);
}

inline void
garbage_collector :: quiescent_state(thread_state* ts)
{
    using namespace e::atomic;
    thread_state_node* const tsn = ts->m_tsn;
    // read the timestamp here
    uint64_t timestamp = read_timestamp();
    assert(tsn->quiescent_timestamp < timestamp);

    // Find the largest timestamp that is less than each thread state's
    // quiescent_timestamp.  For every other thread, use the value that they
    // advertise in their thread_state_node.  For this thread, use the value we
    // just read above in the counter.
    uint64_t min_timestamp = timestamp;
    thread_state_node* node = load_ptr_acquire(&m_registered);

    while (node &&
           load_64_acquire(&m_smallest_garbage) < min_timestamp)
    {
        if (node != tsn)
        {
            uint64_t qst = load_64_acquire(&node->quiescent_timestamp);
            uint64_t oft = load_64_acquire(&node->offline_timestamp);

            if (qst > oft)
            {
                min_timestamp = std::min(qst, min_timestamp);
            }
        }

        node = load_ptr_acquire(&node->next);
        e::atomic::memory_barrier();
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
    std::list<garbage> to_collect;

    if (load_64_acquire(&m_smallest_garbage) < min_timestamp)
    {
        uint64_t smallest_garbage = UINT64_MAX;
        po6::threads::mutex::hold hold(&m_protect_garbage);

        for (std::list<garbage>::iterator it = m_garbage.begin();
                it != m_garbage.end(); )
        {
            if (it->timestamp < min_timestamp)
            {
                std::list<garbage>::iterator old = it;
                ++it;
                to_collect.splice(to_collect.begin(), m_garbage, old);
            }
            else
            {
                smallest_garbage = std::min(it->timestamp, smallest_garbage);
                ++it;
            }
        }

        store_64_release(&m_smallest_garbage, smallest_garbage);
    }

    // expose our timestamp to the world
    store_64_release(&tsn->quiescent_timestamp, timestamp);

    // cleanup the garbage we're to collect
    for (std::list<garbage>::iterator it = to_collect.begin();
            it != to_collect.end(); ++it)
    {
        it->func(it->ptr);
    }
}

inline void
garbage_collector :: offline(thread_state* ts)
{
    using namespace e::atomic;
    thread_state_node* const tsn = ts->m_tsn;
    uint64_t timestamp = read_timestamp();
    assert(tsn->quiescent_timestamp < timestamp);
    assert(tsn->offline_timestamp < timestamp);
    store_64_release(&tsn->offline_timestamp, timestamp);
    store_64_release(&tsn->quiescent_timestamp, timestamp);
}

inline void
garbage_collector :: online(thread_state* ts)
{
    using namespace e::atomic;
    thread_state_node* const tsn = ts->m_tsn;
    uint64_t timestamp = read_timestamp();
    assert(tsn->quiescent_timestamp < timestamp);
    assert(tsn->offline_timestamp < timestamp);
    store_64_release(&tsn->quiescent_timestamp, timestamp);
    e::atomic::memory_barrier();
}

inline void
garbage_collector :: collect(void* ptr, void(*func)(void* ptr))
{
    // create a list and create a new node on the list
    uint64_t timestamp = read_timestamp();
    std::list<garbage> tmp;
    tmp.push_back(garbage(timestamp, ptr, func));

    // copy the temporary list's contents into the real garbage list
    po6::threads::mutex::hold hold(&m_protect_garbage);
    m_garbage.splice(m_garbage.end(), tmp);
    // update the smallest garbage so that threads can detect it in quiescent
    // state without contending on m_protect_garbage
    e::atomic::store_64_release(&m_smallest_garbage,
            std::min(e::atomic::load_64_nobarrier(&m_smallest_garbage), timestamp));
}

inline uint64_t
garbage_collector :: read_timestamp()
{
    return e::atomic::increment_64_fullbarrier(&m_timestamp, 1);
}

} // namespace e

#endif // e_garbage_collector_h_

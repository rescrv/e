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

#ifndef e_garbage_collector_h_
#define e_garbage_collector_h_

// C
#include <assert.h>
#include <stdint.h>

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
        void enqueue(garbage** list, garbage* g);

    private:
        uint64_t m_timestamp;
        uint64_t m_offline_transitions;
        uint64_t m_minimum;
        thread_state_node* m_registered;
        garbage* m_garbage;
        po6::threads::mutex m_protect_registration;

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

} // namespace e

#endif // e_garbage_collector_h_

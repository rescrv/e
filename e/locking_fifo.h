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

#ifndef e_locking_fifo_h_
#define e_locking_fifo_h_

// STL includes
#include <queue>

// po6
#include <po6/threads/cond.h>
#include <po6/threads/mutex.h>

namespace e
{

template<typename T>
class locking_fifo
{
    public:
        locking_fifo();
        ~locking_fifo() throw ();

    public:
        void pause();
        void unpause();
        size_t num_paused();
        void shutdown();
        bool is_shutdown();
        size_t size();
        bool push(const T& t);
        bool pop(T* t);

    private:
        std::queue<T> m_queue;
        po6::threads::mutex m_lock;
        po6::threads::cond m_may_pop;
        bool m_paused;
        size_t m_num_paused;
        bool m_shutdown;
};

template<typename T>
locking_fifo<T> :: locking_fifo()
    : m_queue()
    , m_lock()
    , m_may_pop(&m_lock)
    , m_paused(false)
    , m_num_paused(0)
    , m_shutdown(false)
{
}

template<typename T>
locking_fifo<T> :: ~locking_fifo()
                   throw ()
{
}

template<typename T>
void
locking_fifo<T> :: pause()
{
    po6::threads::mutex::hold hold(&m_lock);
    m_paused = true;
}

template<typename T>
void
locking_fifo<T> :: unpause()
{
    po6::threads::mutex::hold hold(&m_lock);
    m_paused = false;
    m_may_pop.broadcast();
}

template<typename T>
size_t
locking_fifo<T> :: num_paused()
{
    po6::threads::mutex::hold hold(&m_lock);
    return m_num_paused;
}

template<typename T>
void
locking_fifo<T> :: shutdown()
{
    po6::threads::mutex::hold hold(&m_lock);
    m_shutdown = true;
    m_may_pop.broadcast();
}

template<typename T>
bool
locking_fifo<T> :: is_shutdown()
{
    po6::threads::mutex::hold hold(&m_lock);
    return m_shutdown;
}

template<typename T>
size_t
locking_fifo<T> :: size()
{
    po6::threads::mutex::hold hold(&m_lock);
    return m_queue.size();
}

template<typename T>
bool
locking_fifo<T> :: push(const T& t)
{
    po6::threads::mutex::hold hold(&m_lock);

    if (m_shutdown)
    {
        return false;
    }

    m_queue.push(t);
    m_may_pop.signal();
    return true;
}

template<typename T>
bool
locking_fifo<T> :: pop(T* t)
{
    po6::threads::mutex::hold hold(&m_lock);

    while (m_paused || (m_queue.empty() && !m_shutdown))
    {
        ++m_num_paused;
        m_may_pop.wait();
        --m_num_paused;
    }

    if (m_queue.empty() && m_shutdown)
    {
        return false;
    }

    *t = m_queue.front();
    m_queue.pop();
    return true;
}

} // namespace e

#endif // e_locking_fifo_h_

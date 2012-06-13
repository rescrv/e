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

#ifndef e_worker_barrier_h_
#define e_worker_barrier_h_

// po6
#include <po6/threads/cond.h>
#include <po6/threads/mutex.h>

namespace e
{

// A worker barrier allows one thread to block a number of worker threads from
// making progress.  The control thread uses `pause' to stop all other threads.
// When the thread has returned from `pause', all other threads will be blocked.
// Threads periodically call `pausepoint' to allow themselves to be blocked if
// the control thread has initiated a pause.  The thread which initiated the
// pause is allowed to continue.  At a later point in time, the thread may
// `unpause' all other threads, at which point all threads continue.
//
// Like a pthread barrier, this only works on a fixed number of worker threads.
// The number of threads specifies the *number of worker* threads, and does not
// include the single control thread.
//
// Only the control thread should call shutdown.

class worker_barrier
{
    public:
        worker_barrier(size_t count);
        ~worker_barrier() throw ();

    public:
        void pausepoint();
        void pause();
        void shutdown();
        void unpause();

    private:
        size_t m_count;
        bool m_paused;
        bool m_shutdown;
        size_t m_num_paused;
        po6::threads::mutex m_lock;
        po6::threads::cond m_all_paused;
        po6::threads::cond m_may_unpause;
};

inline
worker_barrier :: worker_barrier(size_t count)
    : m_count(count)
    , m_paused(false)
    , m_shutdown(false)
    , m_num_paused(0)
    , m_lock()
    , m_all_paused(&m_lock)
    , m_may_unpause(&m_lock)
{
}

inline
worker_barrier :: ~worker_barrier() throw ()
{
}

inline void
worker_barrier :: pausepoint()
{
    if (m_paused)
    {
        po6::threads::mutex::hold hold(&m_lock);

        while (m_paused && !m_shutdown)
        {
            ++ m_num_paused;
            assert(m_num_paused <= m_count);

            if (m_num_paused == m_count)
            {
                m_all_paused.signal();
            }

            m_may_unpause.wait();
            -- m_num_paused;
        }
    }
}

inline void
worker_barrier :: pause()
{
    po6::threads::mutex::hold hold(&m_lock);
    m_paused = true;

    while (m_num_paused < m_count)
    {
        m_all_paused.wait();
    }
}

inline void
worker_barrier :: shutdown()
{
    po6::threads::mutex::hold hold(&m_lock);
    m_shutdown = true;
}

inline void
worker_barrier :: unpause()
{
    po6::threads::mutex::hold hold(&m_lock);
    m_paused = false;
    m_may_unpause.broadcast();
}

} // namespace e

#endif // e_worker_barrier_h_

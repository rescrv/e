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

#ifndef e_striped_lock_h_
#define e_striped_lock_h_

namespace e
{

template <typename T>
class striped_lock
{
    public:
        class hold;

    public:
        striped_lock(size_t striping);
        ~striped_lock() throw ();

    private:
        size_t m_striping;
        T* m_locks;
};

template <typename T>
striped_lock<T> :: striped_lock(size_t striping)
    : m_striping(striping)
    , m_locks(new T[striping])
{
}

template <typename T>
striped_lock<T> :: ~striped_lock() throw ()
{
    try
    {
        delete[] m_locks;
    }
    catch (...)
    {
        try
        {
            PO6_DTOR_ERROR("Unable to destruct striped lock.");
        }
        catch (...)
        {
        }
    }
}

template <typename T>
class striped_lock<T> :: hold
{
    public:
        hold(striped_lock<T>* l, size_t stripe_num);
        ~hold() throw ();

    private:
        striped_lock<T>* m_l;
        size_t m_stripe;
};

template <typename T>
striped_lock<T> :: hold :: hold(striped_lock<T>* l, size_t stripe_num)
    : m_l(l)
    , m_stripe(stripe_num % m_l->m_striping)
{
    m_l->m_locks[m_stripe].lock();
}

template <typename T>
striped_lock<T> :: hold :: ~hold() throw ()
{
    try
    {
        m_l->m_locks[m_stripe].unlock();
    }
    catch (...)
    {
        try
        {
            PO6_DTOR_ERROR("Unable to release striped_lock with RAII.");
        }
        catch (...)
        {
        }
    }
}

} // namespace e

#endif // e_striped_lock_h_

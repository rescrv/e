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

#ifndef e_intrusive_ptr_h_
#define e_intrusive_ptr_h_

// C
#include <cstddef>

namespace e
{

// A reference-counted pointer which stores the reference count inside the
// pointer.  This requires you to have a variable "size_t m_ref" which is a
// member of type T.  This variable should be initialized to 0
//
// It is the explicit goal of this implementation to enable copies of the
// intrusive pointer to be created and destroyed without requiring any
// additional form of synchronization.  Specifically, as long as the
// intrusive_ptr<T> that is the source of a copy operation remains valid
// throughout the entire copy operation, there is no need for additional
// synchronization.
//
// Usage tips for maximum safety:
//  - m_ref must be initialized to 0.
//  - intrusive_ptr<T> must be a friend of T if m_ref is not public.
//  - Only use the intrusive_ptr<T>(T*) constructor for the initial
//    intrusive_ptr.  All copies thereafter should come from the copy
//    constructor and assignment operators.
//  - Make sure that the intrusive_ptr<T> you are copying from remains valid
//    throughout the copy.  This means don't do anything like copying from an
//    intrusive_ptr<T> you are going to delete from another thread.
//  - Destructors should never throw exceptions.  The throw specifiers in here
//    assume that this convention is adhered to.

template <typename T>
class intrusive_ptr
{
    public:
        intrusive_ptr() throw ()
            : m_ptr(0)
        {
        }

        intrusive_ptr(T* ptr) throw ()
            : m_ptr(ptr)
        {
            inc();
        }

        intrusive_ptr(const intrusive_ptr<T>& rhs) throw ()
            : m_ptr(rhs.m_ptr)
        {
            inc();
        }

        ~intrusive_ptr() throw ()
        {
            dec();
        }

    public:
        intrusive_ptr<T>&
        operator = (const intrusive_ptr<T>& rhs) throw ()
        {
            intrusive_ptr<T>& lhs(*this);

            if (this->m_ptr != rhs.m_ptr)
            {
                dec();
                lhs.m_ptr = rhs.m_ptr;
                inc();
            }

            return *this;
        }

        T&
        operator * () throw()
        {
            return *m_ptr;
        }

        T*
        operator -> () throw()
        {
            return m_ptr;
        }

        operator bool () const
        {
            return m_ptr;
        }

    private:
        void inc() throw ()
        {
            if (m_ptr)
            {
                __sync_add_and_fetch(&(m_ptr->m_ref), 1);
            }
        }

        void dec() throw ()
        {
            if (m_ptr)
            {
                if (__sync_add_and_fetch(&(m_ptr->m_ref), -1) == 0)
                {
                    delete m_ptr;
                }
            }
        }

    private:
        T* m_ptr;
};

} // namespace e

#endif // e_buffer_h_

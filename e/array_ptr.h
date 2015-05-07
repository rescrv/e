// Copyright (c) 2012, Robert Escriva
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

#ifndef e_array_ptr_h_
#define e_array_ptr_h_

// C
#include <stdlib.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunsafe-loop-optimizations"

namespace e
{

template <typename T>
class array_ptr
{
    public:
        array_ptr() throw ()
            : m_ptr(0)
        {
        }

        explicit
        array_ptr(T* ptr) throw ()
            : m_ptr(ptr)
        {
        }

        array_ptr(array_ptr<T>& other) throw ()
            : m_ptr(other.m_ptr)
        {
            other.m_ptr = NULL;
        }

        ~array_ptr() throw ()
        {
            if (m_ptr)
            {
                delete[] m_ptr;
            }
        }

    public:
        T*
        get() const throw ()
        {
            return m_ptr;
        }

    public:
        array_ptr<T>&
        operator = (array_ptr<T>& rhs) throw ()
        {
            if (m_ptr)
            {
                delete[] m_ptr;
                m_ptr = NULL;
            }

            m_ptr = rhs.m_ptr;

            if (this != &rhs)
            {
                rhs.m_ptr = NULL;
            }

            return *this;
        }

        array_ptr<T>&
        operator = (T* rhs)
        {
            if (m_ptr)
            {
                delete[] m_ptr;
                m_ptr = NULL;
            }

            m_ptr = rhs;
            return *this;
        }

        T&
        operator [] (size_t idx)
        {
            return m_ptr[idx];
        }

        const T&
        operator [] (size_t idx) const
        {
            return m_ptr[idx];
        }

    // Trick from the tr1 shared_ptr impl.
    private:
        typedef T* array_ptr<T>::*bool_type;

    public:
        operator bool_type () const
        {
            return m_ptr == 0 ? 0 : &array_ptr<T>::m_ptr;
        }

        bool
        operator < (const array_ptr<T>& rhs) const
        {
            return m_ptr < rhs.m_ptr;
        }

        bool
        operator <= (const array_ptr<T>& rhs) const
        {
            return m_ptr <= rhs.m_ptr;
        }

        bool
        operator == (const array_ptr<T>& rhs) const
        {
            return m_ptr == rhs.m_ptr;
        }

        bool
        operator != (const array_ptr<T>& rhs) const
        {
            return m_ptr != rhs.m_ptr;
        }

        bool
        operator >= (const array_ptr<T>& rhs) const
        {
            return m_ptr >= rhs.m_ptr;
        }

        bool
        operator > (const array_ptr<T>& rhs) const
        {
            return m_ptr > rhs.m_ptr;
        }

    private:
        T* m_ptr;
};

} // namespace e

#pragma GCC diagnostic pop

#endif // e_buffer_h_

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

#ifndef e_lockfree_hash_map_h_
#define e_lockfree_hash_map_h_

// STL
#include <vector>

// e
#include <e/bitsteal.h>
#include <e/hazard_ptrs.h>

namespace e
{

template <typename K, typename V, uint64_t (*H)(const K&)>
class lockfree_hash_map
{
    public:
        class iterator;

    public:
        lockfree_hash_map(uint16_t magnitude = 5);
        ~lockfree_hash_map() throw ();

    public:
        bool contains(const K& k);
        bool lookup(const K& k, V* v);
        bool insert(const K& k, const V& v);
        bool remove(const K& k);

    // Sloppy iteration
    public:
        iterator begin();
        iterator end();

    private:
        enum bits
        {
            VALID   = 0,
            DELETED = 8
        };

        class node;
        typedef std::auto_ptr<typename hazard_ptrs<node, 3>::hazard_ptr> hazard_ptr;

    private:
        lockfree_hash_map(const lockfree_hash_map&);

    private:
        bool is_clean(node* ptr) const
        {
            return e::bitsteal::get(ptr, VALID) &&
                   !e::bitsteal::get(ptr, DELETED);
        }

    private:
        bool cas(node** loc, node* A, node* B)
        {
            return __sync_bool_compare_and_swap(loc, A, B);
        }

        bool find(const hazard_ptr& hptr, uint64_t hash, const K& key,
                  node*** prev, node** cur);

    private:
        lockfree_hash_map& operator = (const lockfree_hash_map&);

    private:
        hazard_ptrs<node, 3> m_hazards;
        std::vector<node*> m_table;
};

template <typename K, typename V, uint64_t (*H)(const K&)>
class lockfree_hash_map<K, V, H> :: iterator
{
    public:
        iterator(const iterator& other);

    public:
        const K& key() const;
        const V& value() const;
        void next();

    public:
        bool operator == (const iterator& rhs) const;
        bool operator != (const iterator& rhs) const;
        iterator& operator = (const iterator& rhs);

    private:
        friend class lockfree_hash_map<K, V, H>;

    private:
        iterator(lockfree_hash_map<K, V, H>* c, uint64_t o, node* e);

    private:
        void prime();

    private:
        lockfree_hash_map<K, V, H>* m_container;
        hazard_ptr m_hptr;
        uint64_t m_offset;
        node* m_elem;
};

template <typename K, typename V, uint64_t (*H)(const K&)>
lockfree_hash_map<K, V, H> :: lockfree_hash_map(uint16_t magnitude)
    : m_hazards()
    , m_table()
{
    node* valid_empty = NULL;
    valid_empty = e::bitsteal::set(valid_empty, VALID);
    m_table.resize((1 << magnitude), valid_empty);
}

template <typename K, typename V, uint64_t (*H)(const K&)>
lockfree_hash_map<K, V, H> :: ~lockfree_hash_map() throw ()
{
    for (size_t i = 0; i < m_table.size(); ++i)
    {
        node* n = e::bitsteal::strip(m_table[i]);

        while (n)
        {
            node* tmp = n;
            n = e::bitsteal::strip(n->next);
            delete tmp;
        }
    }
}

template <typename K, typename V, uint64_t (*H)(const K&)>
inline bool
lockfree_hash_map<K, V, H> :: contains(const K& k)
{
    return lookup(k, NULL);
}

template <typename K, typename V, uint64_t (*H)(const K&)>
bool
lockfree_hash_map<K, V, H> :: lookup(const K& k, V* v)
{
    hazard_ptr hptr = m_hazards.get();
    const uint64_t hash = H(k);

    while (true)
    {
        node** prev;
        node* cur;

        if (find(hptr, hash, k, &prev, &cur))
        {
            assert(is_clean(cur));

            if (v)
            {
                *v = e::bitsteal::strip(cur)->value;
            }

            return true;
        }
        else
        {
            return false;
        }
    }
}

template <typename K, typename V, uint64_t (*H)(const K&)>
bool
lockfree_hash_map<K, V, H> :: insert(const K& k, const V& v)
{
    hazard_ptr hptr = m_hazards.get();
    const uint64_t hash = H(k);

    while (true)
    {
        node** prev;
        node* cur;

        if (find(hptr, hash, k, &prev, &cur))
        {
            return false;
        }

        assert(is_clean(cur));
        std::auto_ptr<node> nn(new node(hash, k, v, cur));
        node* inserted = e::bitsteal::set(nn.get(), VALID);

        if (cas(prev, cur, inserted))
        {
            nn.release();
            return true;
        }
    }
}

template <typename K, typename V, uint64_t (*H)(const K&)>
bool
lockfree_hash_map<K, V, H> :: remove(const K& k)
{
    hazard_ptr hptr = m_hazards.get();
    const uint64_t hash = H(k);

    while (true)
    {
        node** prev;
        node* cur;

        if (!find(hptr, hash, k, &prev, &cur))
        {
            return false;
        }

        assert(is_clean(cur));
        node* next_old = e::bitsteal::strip(cur)->next;
        node* next_new = next_old;
        assert(e::bitsteal::get(next_old, VALID));

        if (e::bitsteal::get(next_old, DELETED))
        {
            continue;
        }

        // Mark it as deleted.
        next_new = e::bitsteal::set(next_new, DELETED);
        next_new = e::bitsteal::set(next_new, VALID);

        if (!cas(&e::bitsteal::strip(cur)->next, next_old, next_new))
        {
            continue;
        }

        next_new = e::bitsteal::unset(next_new, DELETED);
        cur = e::bitsteal::unset(cur, DELETED);
        assert(is_clean(cur));

        if (cas(prev, cur, next_new))
        {
            hptr->retire(e::bitsteal::strip(cur));
        }
        else
        {
            find(hptr, hash, k, &prev, &cur);
        }

        return true;
    }
}

template <typename K, typename V, uint64_t (*H)(const K&)>
typename lockfree_hash_map<K, V, H>::iterator
lockfree_hash_map<K, V, H> :: begin()
{
    return iterator(this, 0, NULL);
}

template <typename K, typename V, uint64_t (*H)(const K&)>
typename lockfree_hash_map<K, V, H>::iterator
lockfree_hash_map<K, V, H> :: end()
{
    return iterator(this, m_table.size(), NULL);
}

template <typename K, typename V, uint64_t (*H)(const K&)>
class lockfree_hash_map<K, V, H> :: node
{
    public:
        node(uint64_t h, const K& k, const V& v, node* n)
            : hash(h)
            , next(n)
            , key(k)
            , value(v)
        {
        }

    public:
        uint64_t hash;
        node* next;
        K key;
        V value;
};

template <typename K, typename V, uint64_t (*H)(const K&)>
bool
lockfree_hash_map<K, V, H> :: find(const hazard_ptr& hptr, uint64_t hash,
                                   const K& key, node*** prev, node** cur)
{
    const uint64_t mask = m_table.size() - 1;
    const uint64_t offset = hash & mask;

    while (true)
    {
        *prev = &m_table[offset];
        *cur = **prev;
        assert(e::bitsteal::get(*cur, VALID));
        hptr->set(1, e::bitsteal::strip(*cur));

        if (**prev != *cur || e::bitsteal::get(*cur, DELETED))
        {
            continue;
        }

        while (true)
        {
            assert(e::bitsteal::get(*cur, VALID));

            if (e::bitsteal::get(*cur, DELETED))
            {
                break;
            }

            node* cur_stripped = e::bitsteal::strip(*cur);

            if (cur_stripped == NULL)
            {
                return false;
            }

            node* next = cur_stripped->next;
            bool cmark = e::bitsteal::get(next, DELETED);
            hptr->set(0, e::bitsteal::strip(next));

            if (cur_stripped->next != next)
            {
                break;
            }

            uint64_t chash = cur_stripped->hash;
            K& ckey = cur_stripped->key;

            if (**prev != *cur || e::bitsteal::get(*cur, DELETED))
            {
                break;
            }

            if (!cmark)
            {
                if ((hash == chash && !(key < ckey)) || hash > chash)
                {
                    return ckey == key;
                }

                *prev = &cur_stripped->next;
                hptr->set(2, cur_stripped);
            }
            else
            {
                node* A = *cur;
                node* B = next;
                A = e::bitsteal::unset(A, DELETED);
                B = e::bitsteal::unset(B, DELETED);

                if (cas(*prev, A, B))
                {
                    hptr->retire(cur_stripped);
                }
                else
                {
                    break;
                }
            }

            *cur = next;
            hptr->set(1, e::bitsteal::strip(*cur));
        }
    }
}

template <typename K, typename V, uint64_t (*H)(const K&)>
lockfree_hash_map<K, V, H> :: iterator :: iterator(const iterator& other)
    : m_container(other.m_container)
    , m_hptr(m_container->m_hazards.get())
    , m_offset(other.m_offset)
    , m_elem(other.m_elem)
{
}

template <typename K, typename V, uint64_t (*H)(const K&)>
const K&
lockfree_hash_map<K, V, H> :: iterator :: key() const
{
    assert(m_elem);
    return m_elem->key;
}

template <typename K, typename V, uint64_t (*H)(const K&)>
const V&
lockfree_hash_map<K, V, H> :: iterator :: value() const
{
    assert(m_elem);
    return m_elem->value;
}

template <typename K, typename V, uint64_t (*H)(const K&)>
void
lockfree_hash_map<K, V, H> :: iterator :: next()
{
    node* tmp;

    // Grab a safe reference to m_elem->next and put it in tmp.
    while (true)
    {
        tmp = m_elem->next;
        assert(e::bitsteal::get(tmp, VALID));
        m_hptr->set(1, e::bitsteal::strip(tmp));

        if (m_elem->next != tmp)
        {
            continue;
        }

        m_hptr->set(0, e::bitsteal::strip(tmp));
        break;
    }

    // The item can be deleted, valid, or NULL.
    if (e::bitsteal::get(tmp, DELETED))
    {
        // Go back to the beginning of this bucket and reprime.
        m_elem = NULL;
        prime();
    }
    else if (e::bitsteal::strip(tmp))
    {
        // We've just stepped forward in the linked list.
        m_elem = e::bitsteal::strip(tmp);
    }
    else
    {
        // We move on to the next bucket and reprime.
        ++m_offset;
        m_elem = NULL;
        prime();
    }
}

template <typename K, typename V, uint64_t (*H)(const K&)>
bool
lockfree_hash_map<K, V, H> :: iterator :: operator == (const iterator& rhs) const
{
    return m_container == rhs.m_container &&
           m_offset == rhs.m_offset &&
           m_elem == rhs.m_elem;
}

template <typename K, typename V, uint64_t (*H)(const K&)>
bool
lockfree_hash_map<K, V, H> :: iterator :: operator != (const iterator& rhs) const
{
    return !(*this == rhs);
}

template <typename K, typename V, uint64_t (*H)(const K&)>
typename lockfree_hash_map<K, V, H>::iterator&
lockfree_hash_map<K, V, H> :: iterator :: operator = (const iterator& rhs)
{
    // No need to check self-assignment
    m_container = rhs.m_container;
    m_hptr = m_container->m_hazards.get();
    m_offset = rhs.m_offset;
    m_elem = rhs.m_elem;
    return *this;
}

template <typename K, typename V, uint64_t (*H)(const K&)>
lockfree_hash_map<K, V, H> :: iterator :: iterator(lockfree_hash_map<K, V, H>* c,
                                                   uint64_t o,
                                                   node* e)
    : m_container(c)
    , m_hptr(m_container->m_hazards.get())
    , m_offset(o)
    , m_elem(e)
{
    prime();
}

// Advance to the first non-empty bucket.  It will first check the bucket
// pointed to by m_offset, and then move on from there.
// POST CONDITION:  m_offset == m_container->m_table.size() || m_elem
//      Hazard pointers will be set appropriately to protect m_elem and
//      safeguard all future accesses via m_elem.
template <typename K, typename V, uint64_t (*H)(const K&)>
void
lockfree_hash_map<K, V, H> :: iterator :: prime()
{
    while (m_offset < m_container->m_table.size() && !m_elem)
    {
        __sync_synchronize();
        node* tmp = m_container->m_table[m_offset];
        assert(e::bitsteal::get(tmp, VALID));
        m_hptr->set(1, e::bitsteal::strip(tmp));

        if (m_container->m_table[m_offset] != tmp || e::bitsteal::get(tmp, DELETED))
        {
            continue;
        }

        // We've grabbed a valid pointer.  Let's set m_elem, and our second
        // hazard pointer.
        m_elem = e::bitsteal::strip(tmp);
        m_hptr->set(0, m_elem);

        // If the pointer we've managed to grab is null, we're moving on to the
        // next bucket.  We know we can do this because we started with !m_elem
        // and only ever look at the head of the list in the bucket.  If we end
        // up with !m_elem, it means the list must be empty.
        if (!m_elem)
        {
            ++m_offset;
        }
    }
}

template <typename K>
uint64_t
hash_map_id(const K& k)
{
    return k;
}

} // namespace e

#endif // e_lockfree_hash_map_h_

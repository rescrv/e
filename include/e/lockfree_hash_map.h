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
#include <e/bit_stealing.h>
#include <e/hazard_ptrs.h>

namespace e
{

template <typename K, typename V, uint64_t (*H)(const K&)>
class lockfree_hash_map
{
    public:
        lockfree_hash_map(uint16_t magnitude = 5);
        ~lockfree_hash_map() throw ();

    public:
        bool contains(const K& k);
        bool lookup(const K& k, V* v);
        bool insert(const K& k, const V& v);
        bool remove(const K& k);

    private:
        enum bits
        {
            DELETED = 8,
            WEDGED = 9,
            VALID = 10
        };

        enum found
        {
            EXIST,
            NOTEXIST,
            RESIZE
        };

        class node;
        typedef std::auto_ptr<typename hazard_ptrs<std::vector<node*>, 2>::hazard_ptr> table_hazard_ptr;
        typedef std::auto_ptr<typename hazard_ptrs<node, 3>::hazard_ptr> node_hazard_ptr;

    private:
        lockfree_hash_map(const lockfree_hash_map&);

    private:
        bool is_clean(node* ptr) const
        {
            return e::bit_stealing::get(ptr, VALID) &&
                   !e::bit_stealing::get(ptr, DELETED) &&
                   !e::bit_stealing::get(ptr, WEDGED);
        }

    private:
        bool cas(node** loc, node* A, node* B)
        {
            return __sync_bool_compare_and_swap(loc, A, B);
        }

        found find(const node_hazard_ptr& nhptr, std::vector<node*>* table,
                   uint64_t hash, const K& key, node*** prev, node** cur);
        void resize_start(const table_hazard_ptr& thptr,
                          const node_hazard_ptr& nhptr,
                          size_t target);
        void resize_work(const table_hazard_ptr& thptr,
                         const node_hazard_ptr& nhptr,
                         std::vector<node*>* const table);
        void resize_work(const table_hazard_ptr& thptr,
                         const node_hazard_ptr& nhptr,
                         std::vector<node*>* cur_table,
                         std::vector<node*>* new_table);

    private:
        lockfree_hash_map& operator = (const lockfree_hash_map&);

    private:
        hazard_ptrs<std::vector<node*>, 2> m_table_hazards;
        hazard_ptrs<node, 3> m_node_hazards;
        std::vector<node*>* m_cur_table;
        std::vector<node*>* m_new_table;
        uint64_t m_size;
};

template <typename K, typename V, uint64_t (*H)(const K&)>
lockfree_hash_map<K, V, H> :: lockfree_hash_map(uint16_t magnitude)
    : m_table_hazards()
    , m_node_hazards()
    , m_cur_table()
    , m_new_table()
    , m_size(0)
{
    node* valid_empty = NULL;
    e::bit_stealing::set(valid_empty, VALID);
    e::bit_stealing::get(valid_empty) |= magnitude;
    m_cur_table = new std::vector<node*>((1 << magnitude), valid_empty);
    e::bit_stealing::get(m_cur_table) |= magnitude;
    e::bit_stealing::get(m_new_table) |= magnitude + 1;
}

template <typename K, typename V, uint64_t (*H)(const K&)>
lockfree_hash_map<K, V, H> :: ~lockfree_hash_map() throw ()
{
    std::set<node*> to_delete;
    m_cur_table = e::bit_stealing::strip(m_cur_table);
    m_new_table = e::bit_stealing::strip(m_new_table);

    for (size_t i = 0; i < m_cur_table->size(); ++i)
    {
        node* n = e::bit_stealing::strip((*m_cur_table)[i]);

        while (n)
        {
            to_delete.insert(n);
            n = e::bit_stealing::strip(n->next);
        }
    }

    delete m_cur_table;

    if (m_new_table)
    {
        for (size_t i = 0; i < m_new_table->size(); ++i)
        {
            node* n = e::bit_stealing::strip((*m_new_table)[i]);

            while (n)
            {
                to_delete.insert(n);
                n = e::bit_stealing::strip(n->next);
            }
        }

        delete m_new_table;
    }

    for (typename std::set<node*>::iterator i = to_delete.begin(); i != to_delete.end(); ++i)
    {
        if (*i)
        {
            delete *i;
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
    table_hazard_ptr thptr = m_table_hazards.get();
    node_hazard_ptr nhptr = m_node_hazards.get();
    const uint64_t hash = H(k);

    while (true)
    {
        std::vector<node*>* table = m_cur_table;
        thptr->set(0, e::bit_stealing::strip(table));

        // Check to make sure the table reference wasn't changed.
        if (table != m_cur_table)
        {
            continue;
        }

        node** prev;
        node* cur;

        switch (find(nhptr, table, hash, k, &prev, &cur))
        {
            case EXIST:
                assert(is_clean(cur));

                if (v)
                {
                    *v = e::bit_stealing::strip(cur)->value;
                }

                return true;
            case NOTEXIST:
                return false;
            case RESIZE:
                // If there is a resize in progress which affects our bucket,
                // then we contribute to resizing the table.
                assert(e::bit_stealing::get(cur, WEDGED));
                resize_work(thptr, nhptr, table);
                continue;
            default:
                assert(!"find returned non-enum type");
        }
    }
}

template <typename K, typename V, uint64_t (*H)(const K&)>
bool
lockfree_hash_map<K, V, H> :: insert(const K& k, const V& v)
{
    table_hazard_ptr thptr = m_table_hazards.get();
    node_hazard_ptr nhptr = m_node_hazards.get();
    const uint64_t hash = H(k);

    __sync_add_and_fetch(&m_size, 1);

    while (true)
    {
        std::vector<node*>* table = m_cur_table;
        thptr->set(0, e::bit_stealing::strip(table));

        // Check to make sure the table reference wasn't changed.
        if (table != m_cur_table)
        {
            continue;
        }

#if 0
        if (e::bit_stealing::strip(table)->size() <= size)
        {
            resize_start(thptr, nhptr, size);
            continue;
        }
#endif

        node** prev;
        node* cur;

        switch (find(nhptr, table, hash, k, &prev, &cur))
        {
            case EXIST:
                return false;
            case NOTEXIST:
                assert(is_clean(cur));
                break;
            case RESIZE:
                // If there is a resize in progress which affects our bucket,
                // then we contribute to resizing the table.
                assert(e::bit_stealing::get(cur, WEDGED));
                resize_work(thptr, nhptr, table);
                continue;
            default:
                assert(!"find returned non-enum type");
        }

        std::auto_ptr<node> nn(new node(hash, k, v, cur));
        node* inserted = nn.get();
        e::bit_stealing::set(inserted, VALID);

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
    __sync_fetch_and_sub(&m_size, 1);
    table_hazard_ptr thptr = m_table_hazards.get();
    node_hazard_ptr nhptr = m_node_hazards.get();
    const uint64_t hash = H(k);

    while (true)
    {
        std::vector<node*>* table = m_cur_table;
        thptr->set(0, e::bit_stealing::strip(table));

        // Check to make sure the table reference wasn't changed.
        if (table != m_cur_table)
        {
            continue;
        }

        node** prev;
        node* cur;

        switch (find(nhptr, table, hash, k, &prev, &cur))
        {
            case EXIST:
                assert(is_clean(cur));
                break;
            case NOTEXIST:
                return false;
            case RESIZE:
                // If there is a resize in progress which affects our bucket,
                // then we contribute to resizing the table.
                assert(e::bit_stealing::get(cur, WEDGED));
                resize_work(thptr, nhptr, table);
                continue;
            default:
                assert(!"find returned non-enum type");
        }

        node* next_old = e::bit_stealing::strip(cur)->next;
        node* next_new = next_old;
        assert(e::bit_stealing::get(next_old, VALID));

        if (e::bit_stealing::get(next_old, WEDGED))
        {
            continue;
        }

        // Mark it as deleted.
        e::bit_stealing::set(next_new, DELETED);

        if (!cas(&e::bit_stealing::strip(cur)->next, next_old, next_new))
        {
            continue;
        }

        e::bit_stealing::unset(next_new, DELETED);

        if (cas(prev, cur, next_new))
        {
            nhptr->retire(e::bit_stealing::strip(cur));
        }
        else
        {
            find(nhptr, table, hash, k, &prev, &cur);
        }

        return true;
    }
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

// Preconditions:
//  - table is protected by thptr.
//  - nhptr{0,1} may be overwritten.
template <typename K, typename V, uint64_t (*H)(const K&)>
typename lockfree_hash_map<K, V, H>::found
lockfree_hash_map<K, V, H> :: find(const node_hazard_ptr& nhptr,
                                   std::vector<node*>* table, uint64_t hash,
                                   const K& key, node*** prev, node** cur)
{
    const uint16_t tag = e::bit_stealing::get(table) & 0xff;
    table = e::bit_stealing::strip(table);
    const uint64_t mask = table->size() - 1;
    const uint64_t offset = hash & mask;
    assert(table->size() == static_cast<size_t>(1 << tag));

    while (true)
    {
        *prev = &(*table)[offset];
        *cur = **prev;
        assert(e::bit_stealing::get(*cur, VALID));
        nhptr->set(1, e::bit_stealing::strip(*cur));

        if (**prev != *cur || e::bit_stealing::get(*cur, DELETED))
        {
            continue;
        }

        if (e::bit_stealing::get(*cur, WEDGED))
        {
            return RESIZE;
        }

        // XXX If we assume threads can fail mid-delete, then we should really
        // try to advance deletes if cur->deleted() rather than just continue.

        while (true)
        {
            assert(e::bit_stealing::get(*cur, VALID));
            node* cur_stripped = e::bit_stealing::strip(*cur);

            if (e::bit_stealing::get(*cur, DELETED))
            {
                break;
            }

            if (cur_stripped == NULL)
            {
                uint16_t cur_tag = e::bit_stealing::get(*cur) & 0xff;

                if (cur_tag >= tag)
                {
                    return NOTEXIST;
                }
                else
                {
                    return RESIZE;
                }
            }

            assert((e::bit_stealing::get(*cur) & 0xff) == 0);
            node* next = cur_stripped->next;
            bool cmark = e::bit_stealing::get(next, DELETED);
            nhptr->set(0, e::bit_stealing::strip(next));

            if (cur_stripped->next != next)
            {
                break;
            }

            if (e::bit_stealing::get(next, WEDGED))
            {
                *prev = &cur_stripped->next;
                nhptr->set(2, cur_stripped);
                *cur = next;
                nhptr->set(1, e::bit_stealing::strip(*cur));
                return RESIZE;
            }

            uint64_t chash = cur_stripped->hash;
            K& ckey = cur_stripped->key;

            if (**prev != *cur || e::bit_stealing::get(*cur, DELETED))
            {
                break;
            }

            if (!cmark)
            {
                if ((hash == chash && key >= ckey) || hash > chash)
                {
                    return ckey == key ? EXIST : NOTEXIST;
                }

                *prev = &cur_stripped->next;
                nhptr->set(2, cur_stripped);
            }
            else
            {
                node* A = *cur;
                node* B = next;
                e::bit_stealing::unset(A, DELETED);
                e::bit_stealing::unset(B, DELETED);

                if (cas(*prev, A, B))
                {
                    nhptr->retire(cur_stripped);
                }
                else
                {
                    break;
                }
            }

            *cur = next;
            nhptr->set(1, e::bit_stealing::strip(*cur));
        }
    }
}

template <typename K, typename V, uint64_t (*H)(const K&)>
void
lockfree_hash_map<K, V, H> :: resize_start(const table_hazard_ptr& /*thptr*/,
                                           const node_hazard_ptr& /*nhptr*/,
                                           size_t /*target*/)
{
    assert(false);
#if 0
    std::vector<node*>* cur_table;
    std::vector<node*>* new_table;

    while (true)
    {
        cur_table = m_cur_table;
        new_table = m_new_table;

        thptr->set(0, e::bit_stealing::strip(cur_table));
        thptr->set(1, e::bit_stealing::strip(new_table));

        if (new_table != m_new_table)
        {
            continue;
        }

        if (cur_table != m_cur_table)
        {
            continue;
        }

        if ((1 << e::bit_stealing::get(cur_table) & 0xff) >= target)
        {
            return;
        }

        if (!e::bit_stealing::strip(new_table))
        {
            const uint16_t tag = e::bit_stealing::get(new_table) & 0xff;
            std::auto_ptr<std::vector<node*> > created_table(new std::vector<node*>(1 << tag, NULL));
            std::vector<node*>* to_cas = created_table.get();
            thptr->set(1, to_cas);
            e::bit_stealing::get(to_cas) |= tag;

            if (!__sync_bool_compare_and_swap(&m_new_table, new_table, to_cas))
            {
                continue;
            }

            new_table = to_cas;
            created_table.release();
        }

        resize_work(thptr, nhptr, cur_table, new_table);
        return;
    }
#endif
}

template <typename K, typename V, uint64_t (*H)(const K&)>
void
lockfree_hash_map<K, V, H> :: resize_work(const table_hazard_ptr& /*thptr*/,
                                          const node_hazard_ptr& /*nhptr*/,
                                          std::vector<node*>* const /*table*/)
{
    assert(false);
#if 0
    std::vector<node*>* cur_table;
    std::vector<node*>* new_table;

    while (true)
    {
        cur_table = m_cur_table;
        new_table = m_new_table;

        if (cur_table != table)
        {
            return;
        }

        thptr->set(0, e::bit_stealing::strip(cur_table));
        thptr->set(1, e::bit_stealing::strip(new_table));

        if (new_table != m_new_table)
        {
            continue;
        }

        if (cur_table != m_cur_table)
        {
            continue;
        }

        resize_work(thptr, nhptr, cur_table, new_table);
        return;
    }
#endif
}

template <typename K, typename V, uint64_t (*H)(const K&)>
void
lockfree_hash_map<K, V, H> :: resize_work(const table_hazard_ptr& /*thptr*/,
                                          const node_hazard_ptr& /*nhptr*/,
                                          std::vector<node*>* /*cur_table*/,
                                          std::vector<node*>* /*new_table*/)
{
    assert(false);
#if 0
    K k;
    cur_table = e::bit_stealing::strip(cur_table);
    new_table = e::bit_stealing::strip(new_table);

    for (size_t i = 0; i < cur_table->size(); ++i)
    {
        while (true)
        {
            uint16_t tag = e::bit_stealing::get((*cur_table)[i]) & 0xff;

            // If this bucket has been moved.
            if (static_cast<size_t>(1 << tag) > cur_table->size())
            {
                break;
            }

#if 0
            uint64_t hash = 1 << (tag + 1);
            node** prev;
            node* cur;
            find(nhptr, cur_table, hash, k, &prev, &cur);
            node* wedged = cur;
            e::bit_stealing::set(wedged, WEDGED);

            if (!cas(prev, cur, wedged))
            {
                continue;
            }


#endif
        }
    }
#endif
}

} // namespace e

#endif // e_lockfree_hash_map_h_

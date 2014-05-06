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

#ifndef e_nwf_hash_map_h_
#define e_nwf_hash_map_h_

// STL
#include <algorithm>

// e
#include <e/garbage_collector.h>
#include <e/lookup3.h>
#include <e/time.h>

// This is a nearly-wait-free hash map.  Strictly-speaking, it's lock-free
// because of resize operations, but operations that happen outside the resize
// (and don't hit the absolute worst cases of the resize) will see behavior that
// is wait-free.
//
// The design is borrowed from Cliff Click's lockfree hash table implementation
// in Java at org.cliffc.high_scale_lib.NonBlockingHashMap.  His code and
// presentation were used to construct this implementation.  For the lawyers out
// there, he's put his code in the public domain, "... as explained at
// http://creativecommons.org/licenses/publicdomain"

namespace e
{

template <typename K, typename V, uint64_t (*H)(const K&)>
class nwf_hash_map
{
    public:
        class iterator;

    public:
        nwf_hash_map(garbage_collector* gc);
        ~nwf_hash_map() throw ();

    public:
        size_t size();
        bool empty();
        bool put(const K& k, const V& v);
        bool put_ine(const K& k, const V& v);
        bool cas(const K& k, const V& o, const V& n);
        bool del(const K& k);
        bool del_if(const K& k, const V& v);
        bool has(const K& k);
        bool get(const K& k, V* v);
        iterator begin();
        iterator end();

    private:
        template <typename T>
        struct wrapper
        {
            typedef const T* type;
            static inline type NULLVALUE()    { return 0; }
            // no match old means don't perform a check against the old value
            static inline type NO_MATCH_OLD() { return reinterpret_cast<T*>(2); }
            // match any existing value, but only if there's actually a value
            static inline type MATCH_ANY()    { return reinterpret_cast<T*>(4); }
            // this is a dead value
            static inline type TOMBSTONE()    { return reinterpret_cast<T*>(8); }
            // this is a dead value we copied
            static inline type TOMBPRIME()    { return reinterpret_cast<T*>(9); }
            // has the prime bit been set?
            static inline bool is_primed(type t) { return reinterpret_cast<uintptr_t>(t) & 1; }
            static inline bool is_null(type t) { return t == NULLVALUE(); }
            static inline bool is_no_match_old(type t) { return t == NO_MATCH_OLD(); }
            static inline bool is_match_any(type t) { return t == MATCH_ANY(); }
            static inline bool is_tombstone(type t) { return t == TOMBSTONE() ||
                                                             t == TOMBPRIME(); }
            static inline bool is_tombprime(type t) { return t == TOMBPRIME(); }
            static inline bool is_empty(type t) { return is_tombstone(t) || is_null(t); }
            static inline bool is_special(type t) { return reinterpret_cast<uintptr_t>(t) <= 9; }
            // compare values
            static inline bool equal(T t1, type t2)
            { return equal(reference(t1), t2); }
            static inline bool equal(type t1, type t2)
            { return t1 == t2 ||
                     (!is_special(t1) && !is_special(t2) && unwrap(t1) == unwrap(t2) ); }
            // construct a reference, assuming K sticks around
            static inline type reference(const T& t) { return &t; }
            // get the T out of the type
            static inline const T& unwrap(const type& t)
            { return *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(t) & ~1ULL); }
            // get the boxed form
            static inline type prime(type t)
            { return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(t) | 1); }
            static inline type deprime(type t)
            { return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(t) & ~1ULL); }
            // garbage collect things
            static inline void collect_func(void* v)
            { delete reinterpret_cast<T*>(v); }
            static inline void collect(garbage_collector* gc, type t)
            { if (!is_special(t)) { gc->collect(const_cast<void*>(static_cast<const void*>(deprime(t))), collect_func); } }
            static inline void collect_immediate(type t)
            { if (!is_special(t)) { delete deprime(t); } }
            // do an acquire-load on the wrapped value
            static inline type load(type* t) { return e::atomic::load_ptr_acquire(t); }
            // do a compare-and-swap full-barrier on the wrapped value
            static inline type cas(type* t, type old_val, type _new_val)
            { type new_val = _new_val;
              bool alloc = false;
              if (reinterpret_cast<uintptr_t>(new_val) > 9 &&
                  deprime(old_val) != deprime(new_val)) { alloc = true; new_val = new T(unwrap(new_val)); }
              type witness = e::atomic::compare_and_swap_ptr_fullbarrier(t, old_val, new_val);
              if (witness != old_val && alloc) { delete new_val; }
              return witness; }
        };
        struct node
        {
            node() : key(), val() {}
            node(const node& other) : key(other.key), val(other.val) {}
            bool operator == (const node& rhs);
            node& operator = (const node& rhs)
            { key = rhs.key; val = rhs.val; return *this; }
            typename wrapper<K>::type key;
            typename wrapper<V>::type val;
        };
        struct table
        {
            static void collect(void*);
            static table* create(size_t cap, size_t depth) { return new (cap) table(cap, depth); }
            ~table() throw ();

            void inc_slots() { e::atomic::increment_64_nobarrier(&slots,  1); }
            size_t size() { e::atomic::memory_barrier();
                            return e::atomic::load_64_nobarrier(&elems); }
            void inc_size() { e::atomic::increment_64_nobarrier(&elems,  1); }
            void dec_size() { e::atomic::increment_64_nobarrier(&elems, -1); }
            bool table_is_full(size_t reprobes);

            table* resize(nwf_hash_map* top_map);
            void help_copy(nwf_hash_map* top_map, bool copy_all);
            table* copy_slot_and_check(nwf_hash_map* top_map, int idx, bool should_help);
            void copy_check_and_promote(nwf_hash_map* top_map, size_t work_done);
            bool copy_slot(nwf_hash_map* top_map, size_t idx, table* new_table);

            void operator delete (void* mem);

            const size_t capacity;
            size_t depth;
            uint64_t slots;
            uint64_t elems;
            uint64_t copy_idx;
            uint64_t copy_done;
            table* next;
            node nodes[1];

            private:
                table(size_t cap, size_t depth);
                table(const table&);
                table& operator = (const table&);
                void* operator new (size_t sz, size_t cap);
        };

    private:
        const static size_t MIN_SIZE_LOG = 3;
        const static size_t MIN_SIZE = (1ULL << MIN_SIZE_LOG);
        const static size_t REPROBE_LIMIT = 10;
        uint64_t hash_key(typename wrapper<K>::type k);
        size_t reprobe_limit(size_t capacity);
        bool key_compare(K k1, typename wrapper<K>::type k2);
        bool key_compare(typename wrapper<K>::type k1, typename wrapper<K>::type k2);
        bool get(table* t, typename wrapper<K>::type key, const uint64_t hash, V* val);
        uint64_t millis_now() { return e::time() / 1000000ULL; }
        typename wrapper<V>::type put_if_match(typename wrapper<K>::type key,
                                               typename wrapper<V>::type exp_val,
                                               typename wrapper<V>::type put_val);
        typename wrapper<V>::type put_if_match(table* const t,
                                               const typename wrapper<K>::type key,
                                               const typename wrapper<V>::type exp_val,
                                               const typename wrapper<V>::type put_val);
        table* help_copy(table* t);

    private:
        garbage_collector* m_gc;
        table* m_table;
        uint64_t m_last_resize_millis;

    private:
        nwf_hash_map(const nwf_hash_map&);
        nwf_hash_map& operator = (const nwf_hash_map&);
};

template <typename K, typename V, uint64_t (*H)(const K&)>
class nwf_hash_map<K, V, H>::iterator
{
    public:
        iterator();
        iterator(const iterator&);

    public:
        iterator& operator ++ ();
        const std::pair<K, V>& operator * () { return m_cached; }
        const std::pair<K, V>* operator -> () { return &m_cached; }
        bool operator == (const iterator& rhs);
        bool operator != (const iterator& rhs)
        { return !(*this == rhs); }
        iterator& operator = (const iterator& rhs);

    private:
        friend class nwf_hash_map;
        iterator(table* t);
        void prime();
        void advance();

    private:
        table* m_table;
        size_t m_index;
        bool m_primed;
        std::pair<K, V> m_cached;
};

template <typename K, typename V, uint64_t (*H)(const K&)>
nwf_hash_map<K, V, H> :: nwf_hash_map(garbage_collector* gc)
    : m_gc(gc)
    , m_table(NULL)
    , m_last_resize_millis(millis_now())

{
    e::atomic::store_ptr_fullbarrier(&m_table, table::create(MIN_SIZE, 0));
}

template <typename K, typename V, uint64_t (*H)(const K&)>
nwf_hash_map<K, V, H> :: ~nwf_hash_map() throw ()
{
    table* t = e::atomic::load_ptr_acquire(&m_table);
    m_gc->collect(t, table::collect);
}

template <typename K, typename V, uint64_t (*H)(const K&)>
size_t
nwf_hash_map<K, V, H> :: size()
{
    table* t = e::atomic::load_ptr_acquire(&m_table);
    return t->size();
}

template <typename K, typename V, uint64_t (*H)(const K&)>
bool
nwf_hash_map<K, V, H> :: empty()
{
    return size() == 0;
}

template <typename K, typename V, uint64_t (*H)(const K&)>
bool
nwf_hash_map<K, V, H> :: put(const K& k, const V& v)
{
    typename wrapper<V>::type c;
    c = put_if_match(wrapper<K>::reference(k),
                     wrapper<V>::NO_MATCH_OLD(),
                     wrapper<V>::reference(v));
    return true;
}

template <typename K, typename V, uint64_t (*H)(const K&)>
bool
nwf_hash_map<K, V, H> :: put_ine(const K& k, const V& v)
{
    typename wrapper<V>::type c;
    c = put_if_match(wrapper<K>::reference(k),
                     wrapper<V>::TOMBSTONE(),
                     wrapper<V>::reference(v));
    return true;
}

template <typename K, typename V, uint64_t (*H)(const K&)>
bool
nwf_hash_map<K, V, H> :: cas(const K& k, const V& o, const V& n)
{
    typename wrapper<V>::type c;
    c =  put_if_match(wrapper<K>::reference(k),
                      wrapper<V>::reference(o),
                      wrapper<V>::reference(n));
    return wrapper<V>::equal(o, c);
}

template <typename K, typename V, uint64_t (*H)(const K&)>
bool
nwf_hash_map<K, V, H> :: del(const K& k)
{
    typename wrapper<V>::type c;
    c = put_if_match(wrapper<K>::reference(k),
                     wrapper<V>::NO_MATCH_OLD(),
                     wrapper<V>::TOMBSTONE());
    return !wrapper<V>::is_empty(c);
}

template <typename K, typename V, uint64_t (*H)(const K&)>
bool
nwf_hash_map<K, V, H> :: del_if(const K& k, const V& v)
{
    typename wrapper<V>::type c;
    c = put_if_match(wrapper<K>::reference(k),
                     wrapper<V>::reference(v),
                     wrapper<V>::TOMBSTONE());
    return !wrapper<V>::is_empty(c);
}

template <typename K, typename V, uint64_t (*H)(const K&)>
bool
nwf_hash_map<K, V, H> :: has(const K& k)
{
    V v;
    return get(k, &v);
}

template <typename K, typename V, uint64_t (*H)(const K&)>
bool
nwf_hash_map<K, V, H> :: get(const K& _k, V* v)
{
    typename wrapper<K>::type k(wrapper<K>::reference(_k));
    const uint64_t hash = hash_key(k);
    e::atomic::memory_barrier();
    table* t = e::atomic::load_ptr_acquire(&m_table);
    return get(t, k, hash, v);
}

template <typename K, typename V, uint64_t (*H)(const K&)>
inline typename nwf_hash_map<K, V, H>::iterator
nwf_hash_map<K, V, H> :: begin()
{
    table* t = e::atomic::load_ptr_acquire(&m_table);
    return iterator(t);
}

template <typename K, typename V, uint64_t (*H)(const K&)>
inline typename nwf_hash_map<K, V, H>::iterator
nwf_hash_map<K, V, H> :: end()
{
    return iterator();
}

#if 0
template <typename K, typename V, uint64_t (*H)(const K&)>
void
nwf_hash_map<K, V, H> :: dump()
{
    table* t = e::atomic::load_ptr_acquire(&m_table);
    std::cout << "[";
    bool prev_elem = false;

    for (size_t i = 0; i < t->capacity; ++i)
    {
        if (prev_elem)
        {
            std::cout << ", ";
        }

        prev_elem = true;
        typename wrapper<K>::type k = t->nodes[i].key;
        typename wrapper<V>::type v = t->nodes[i].val;

        if (wrapper<K>::is_null(k) && wrapper<K>::is_null(v))
        {
            prev_elem = false;
        }
        else if (wrapper<K>::is_special(k) && wrapper<K>::is_special(v))
        {
            std::cout << "(" << k << ", " << v << ")";
        }
        else if (wrapper<K>::is_special(k))
        {
            std::cout << "(" << k
                      << ", " << v << "->" << wrapper<V>::unwrap(v) << ")";
        }
        else if (wrapper<V>::is_special(v))
        {
            std::cout << "(" << k << "->" << wrapper<K>::unwrap(k)
                      << ", " << v << ")";
        }
        else
        {
            std::cout << "(" << k << "->" << wrapper<K>::unwrap(k)
                      << ", " << v << "->" << wrapper<V>::unwrap(v) << ")";
        }
    }

    std::cout << "]" << std::endl;
}
#endif

template <typename K, typename V, uint64_t (*H)(const K&)>
uint64_t
nwf_hash_map<K, V, H> :: hash_key(typename wrapper<K>::type k)
{
    return e::lookup3_64(H(wrapper<K>::unwrap(k)));
}

template <typename K, typename V, uint64_t (*H)(const K&)>
size_t
nwf_hash_map<K, V, H> :: reprobe_limit(size_t capacity)
{
    return REPROBE_LIMIT + (capacity >> 2);
}

template <typename K, typename V, uint64_t (*H)(const K&)>
bool
nwf_hash_map<K, V, H> :: key_compare(K k1, typename wrapper<K>::type k2)
{
    return key_compare(wrapper<K>::reference(k1), k2);
}

template <typename K, typename V, uint64_t (*H)(const K&)>
bool
nwf_hash_map<K, V, H> :: key_compare(typename wrapper<K>::type k1,
                                     typename wrapper<K>::type k2)
{
    return wrapper<K>::equal(k1, k2);
}

template <typename K, typename V, uint64_t (*H)(const K&)>
bool
nwf_hash_map<K, V, H> :: get(table* t, typename wrapper<K>::type key, const uint64_t hash, V* val)
{
    const size_t mask = t->capacity - 1;
    size_t idx = hash & mask;
    size_t reprobes = 0;

    while (true)
    {
        typename wrapper<K>::type k = wrapper<K>::load(&t->nodes[idx].key);
        typename wrapper<V>::type v = wrapper<V>::load(&t->nodes[idx].val);

        if (wrapper<K>::is_null(k))
        {
            return false;
        }

        table* nested = e::atomic::load_ptr_acquire(&t->next);

        if (key_compare(key, k))
        {
            if (!wrapper<V>::is_primed(v))
            {
                if (wrapper<V>::is_tombstone(v) ||
                    wrapper<V>::is_null(v))
                {
                    return false;
                }

                *val = wrapper<V>::unwrap(v);
                return true;
            }

            nested = t->copy_slot_and_check(this, idx, true);
            return get(nested, key, hash, val);
        }

        ++reprobes;

        if (reprobes >= reprobe_limit(t->capacity) ||
            wrapper<K>::is_tombstone(k))
        {
            if (nested)
            {
                nested = help_copy(nested);
                return get(nested, key, hash, val);
            }

            return false;
        }

        idx = (idx + 1) & mask;
    }
}

template <typename K, typename V, uint64_t (*H)(const K&)>
typename nwf_hash_map<K, V, H>::template wrapper<V>::type
nwf_hash_map<K, V, H> :: put_if_match(typename wrapper<K>::type key,
                                      typename wrapper<V>::type exp_val,
                                      typename wrapper<V>::type put_val)
{
    assert(!wrapper<K>::is_null(key));
    assert(!wrapper<V>::is_null(exp_val));
    assert(!wrapper<V>::is_null(put_val));
    table* t = e::atomic::load_ptr_acquire(&m_table);
    typename wrapper<V>::type ret = put_if_match(t, key, exp_val, put_val);
    e::atomic::memory_barrier();
    return ret;
}

template <typename K, typename V, uint64_t (*H)(const K&)>
typename nwf_hash_map<K, V, H>::template wrapper<V>::type
nwf_hash_map<K, V, H> :: put_if_match(table* t,
                                      const typename wrapper<K>::type key,
                                      const typename wrapper<V>::type exp_val,
                                      const typename wrapper<V>::type put_val)
{
    assert(!wrapper<V>::is_null(put_val));
    assert(!wrapper<V>::is_primed(exp_val));
    assert(!wrapper<V>::is_primed(put_val));
    const uint64_t hash = hash_key(key);
    const size_t mask = t->capacity - 1;
    size_t idx = hash & mask;
    size_t reprobes = 0;

    // protect against infinite recursion
    if (e::atomic::load_ptr_acquire(&m_table)->depth > t->depth)
    {
        return put_if_match(e::atomic::load_ptr_acquire(&m_table), key, exp_val, put_val);
    }

    typename wrapper<K>::type k = wrapper<K>::NULLVALUE();
    typename wrapper<V>::type v = wrapper<V>::NULLVALUE();
    table* nested = NULL;

    while (true)
    {
        k = wrapper<K>::load(&t->nodes[idx].key);
        v = wrapper<V>::load(&t->nodes[idx].val);

        if (wrapper<K>::is_null(k))
        {
            if (wrapper<V>::is_tombstone(put_val))
            {
                return put_val;
            }

            typename wrapper<K>::type witness;
            witness = wrapper<K>::cas(&t->nodes[idx].key,
                                      wrapper<K>::NULLVALUE(), key);

            if (wrapper<K>::is_null(witness))
            {
                t->inc_slots();
                break;
            }

            k = witness;
        }

        nested = e::atomic::load_ptr_acquire(&t->next);

        if (key_compare(key, k))
        {
            break;
        }

        ++reprobes;

        if (reprobes >= reprobe_limit(t->capacity) ||
            wrapper<K>::is_tombstone(k))
        {
            nested = t->resize(this);

            if (!wrapper<V>::is_null(exp_val))
            {
                help_copy(nested);
            }

            return put_if_match(nested, key, exp_val, put_val);
        }

        idx = (idx + 1) & mask;
    }

    if (wrapper<V>::equal(put_val, v))
    {
        return v;
    }

    if (!nested &&
        ((wrapper<V>::is_null(v) && t->table_is_full(reprobes)) ||
         wrapper<V>::is_primed(v)))
    {
        nested = t->resize(this);
    }

    if (nested)
    {
        nested = t->copy_slot_and_check(this, idx, !wrapper<V>::is_null(exp_val));
        return put_if_match(nested, key, exp_val, put_val);
    }

    while (true)
    {
        assert(!wrapper<V>::is_primed(v));

        if (!wrapper<V>::is_no_match_old(exp_val) &&
            v != exp_val &&
            (!wrapper<V>::is_match_any(exp_val) || wrapper<V>::is_tombstone(v) || wrapper<V>::is_null(v)) &&
            !(wrapper<V>::is_null(v) && wrapper<V>::is_tombstone(exp_val)) &&
            (wrapper<V>::is_null(exp_val) || !wrapper<V>::equal(exp_val, v)))
        {
            return v;
        }

        typename wrapper<V>::type witness;
        witness = wrapper<V>::cas(&t->nodes[idx].val, v, put_val);

        if (v == witness)
        {
            if (!wrapper<V>::is_null(exp_val))
            {
                if ((wrapper<V>::is_null(v) ||
                     wrapper<V>::is_tombstone(v)) &&
                    !wrapper<V>::is_tombstone(put_val))
                {
                    t->inc_size();
                }
                if (!(wrapper<V>::is_null(v) ||
                      wrapper<V>::is_tombstone(v)) &&
                    wrapper<V>::is_tombstone(put_val))
                {
                    t->dec_size();
                }

                if (wrapper<V>::is_null(v))
                {
                    return wrapper<V>::TOMBSTONE();
                }
            }

            wrapper<V>::collect(m_gc, v);
            return v;
        }

        if (wrapper<V>::is_primed(witness))
        {
            nested = t->copy_slot_and_check(this, idx, !wrapper<V>::is_null(exp_val));
            return put_if_match(nested, key, exp_val, put_val);
        }

        v = witness;
    }
}

template <typename K, typename V, uint64_t (*H)(const K&)>
typename nwf_hash_map<K, V, H>::table*
nwf_hash_map<K, V, H> :: help_copy(table* t)
{
    table* top = e::atomic::load_ptr_acquire(&m_table);

    if (!e::atomic::load_ptr_acquire(&top->next))
    {
        return t;
    }

    top->help_copy(this, false);
    return t;
}

template <typename K, typename V, uint64_t (*H)(const K&)>
void
nwf_hash_map<K, V, H> :: table :: collect(void* _t)
{
    table* t = reinterpret_cast<table*>(_t);

    for (uint64_t idx = 0; idx < t->capacity; ++idx)
    {
        if (!wrapper<K>::is_special(t->nodes[idx].key))
        {
            wrapper<K>::collect_immediate(t->nodes[idx].key);
        }
        if (!wrapper<V>::is_special(t->nodes[idx].val))
        {
            wrapper<V>::collect_immediate(t->nodes[idx].val);
        }
    }

    delete t;
}

template <typename K, typename V, uint64_t (*H)(const K&)>
nwf_hash_map<K, V, H> :: table :: ~table() throw ()
{
}

template <typename K, typename V, uint64_t (*H)(const K&)>
nwf_hash_map<K, V, H> :: table :: table(size_t cap, size_t dep)
    : capacity(cap)
    , depth(dep)
    , slots(0)
    , elems(0)
    , copy_idx(0)
    , copy_done(0)
    , next(NULL)
{
    assert(capacity > 0 && (capacity & (capacity - 1)) == 0);

    for (size_t i = 0; i < capacity; ++i)
    {
        nodes[i].key = wrapper<K>::NULLVALUE();
        nodes[i].val = wrapper<V>::NULLVALUE();
    }
}

template <typename K, typename V, uint64_t (*H)(const K&)>
void*
nwf_hash_map<K, V, H> :: table :: operator new (size_t, size_t num)
{
    return new char[sizeof(table) + sizeof(node) * num];
}

template <typename K, typename V, uint64_t (*H)(const K&)>
void
nwf_hash_map<K, V, H> :: table :: operator delete (void* mem)
{
    delete[] static_cast<char*>(mem);
}

template <typename K, typename V, uint64_t (*H)(const K&)>
bool
nwf_hash_map<K, V, H> :: table :: table_is_full(size_t reprobes)
{
    e::atomic::memory_barrier();
    return reprobes >= REPROBE_LIMIT &&
           e::atomic::load_64_nobarrier(&slots) >= (capacity >> 2);
}

template <typename K, typename V, uint64_t (*H)(const K&)>
typename nwf_hash_map<K, V, H>::table*
nwf_hash_map<K, V, H> :: table :: resize(nwf_hash_map* top_map)
{
    using namespace e::atomic;
    table* nested = load_ptr_acquire(&next);

    if (nested)
    {
        return nested;
    }

    size_t old_sz = size();
    size_t new_sz = old_sz;

    if (old_sz >= (capacity >> 2))
    {
        new_sz = capacity << 1;

        if (old_sz >= (capacity >> 1))
        {
            new_sz = capacity << 2;
        }
    }

    uint64_t tm = top_map->millis_now();

    if (new_sz < capacity &&
        tm <= top_map->m_last_resize_millis + 1000 && 
        load_64_nobarrier(&slots) >= (old_sz << 1))
    {
        new_sz = capacity << 1;
    }

    if (new_sz < capacity)
    {
        new_sz = capacity;
    }

    unsigned log2;

    for (log2 = MIN_SIZE_LOG; (1ULL << log2) < new_sz; log2++)
        ;

    // XXX translate this heuristic from Java to C++.
#if 0
    // Now limit the number of threads actually allocating memory to a
    // handful - lest we have 750 threads all trying to allocate a giant
    // resized array.
    long r = _resizers;

    while(!_resizerUpdater.compareAndSet(this, r, r + 1))
        r = _resizers;

    // Size calculation: 2 words (K+V) per table entry, plus a handful.  We
    // guess at 32-bit pointers; 64-bit pointers screws up the size calc by
    // 2x but does not screw up the heuristic very much.
    int megs = ((((1 << log2) << 1) + 4) << 3/*word to bytes*/) >> 20/*megs*/;

    if(r >= 2 && megs > 0) {   // Already 2 guys trying; wait and see
        newkvs = _newkvs;        // Between dorking around, another thread did it

        if(newkvs != null)       // See if resize is already in progress
            return newkvs;         // Use the new table already

        // TODO - use a wait with timeout, so we'll wakeup as soon as the new table
        // is ready, or after the timeout in any case.
        //synchronized( this ) { wait(8*megs); }         // Timeout - we always wakeup
        // For now, sleep a tad and see if the 2 guys already trying to make
        // the table actually get around to making it happen.
        try {
            Thread.sleep(8 * megs);
        } catch(Exception e) { }
    }
#endif

    nested = load_ptr_acquire(&next);
    assert(new_sz >= capacity);
    assert((1ULL << log2) >= capacity);

    if (nested)
    {
        return nested;
    }

    table* new_table = table::create(1ULL << log2, depth + 1);
    nested = load_ptr_acquire(&next);

    if (nested)
    {
        return nested;
    }

    table* witness = compare_and_swap_ptr_fullbarrier(&next, static_cast<table*>(NULL), new_table);

    if (witness == NULL)
    {
        nested = new_table;
    }
    else
    {
        delete new_table;
        nested = witness;
    }

    assert(nested == load_ptr_acquire(&next));
    return nested;
}

template <typename K, typename V, uint64_t (*H)(const K&)>
void
nwf_hash_map<K, V, H> :: table :: help_copy(nwf_hash_map* top_map, bool copy_all)
{
    using namespace e::atomic;
    table* nested = load_ptr_acquire(&next);
    assert(nested);
    const size_t MIN_COPY_WORK = std::min(capacity, 1024UL);
    bool panic = false;
    size_t idx = 0;

    while (load_64_acquire(&copy_done) < capacity)
    {
        if (!panic)
        {
            idx = load_64_acquire(&copy_idx);

            while (idx < (capacity << 1) &&
                   compare_and_swap_64_nobarrier(&copy_idx, idx, idx + MIN_COPY_WORK) != idx)
            {
                idx = load_64_acquire(&copy_idx);
            }

            if (!(idx < (capacity << 1)))
            {
                panic = true;
            }
        }

        size_t work_done = 0;

        for (size_t i = 0; i < MIN_COPY_WORK; ++i)
        {
            if (copy_slot(top_map, (idx + i) & (capacity - 1), nested))
            {
                ++work_done;
            }
        }

        if (work_done > 0)
        {
            copy_check_and_promote(top_map, work_done);
        }

        idx += MIN_COPY_WORK;

        if (!copy_all && !panic)
        {
            return;
        }
    }

    copy_check_and_promote(top_map, 0);
}

template <typename K, typename V, uint64_t (*H)(const K&)>
typename nwf_hash_map<K, V, H>::table*
nwf_hash_map<K, V, H> :: table :: copy_slot_and_check(nwf_hash_map* top_map, int idx, bool should_help)
{
    table* nested = e::atomic::load_ptr_acquire(&next);
    assert(nested);

    if (copy_slot(top_map, idx, nested))
    {
        copy_check_and_promote(top_map, 1);
    }

    return should_help ? top_map->help_copy(nested) : nested;
}

template <typename K, typename V, uint64_t (*H)(const K&)>
void
nwf_hash_map<K, V, H> :: table :: copy_check_and_promote(nwf_hash_map* top_map, size_t work_done)
{
    using namespace e::atomic;
    size_t done = load_64_acquire(&copy_done);
    assert(done + work_done <= capacity);

    if (work_done > 0)
    {
        while (compare_and_swap_64_release(&copy_done, done, done + work_done) != done)
        {
            done = load_64_acquire(&copy_done);
            assert(done + work_done <= capacity);
        }
    }

    table* nested = e::atomic::load_ptr_acquire(&next);

    if (done + work_done == capacity &&
        top_map->m_table == this &&
        compare_and_swap_ptr_fullbarrier(&top_map->m_table, this, nested) == this)
    {
        e::atomic::store_64_release(&top_map->m_last_resize_millis, top_map->millis_now());
        top_map->m_gc->collect(this, table::collect);
    }
}

template <typename K, typename V, uint64_t (*H)(const K&)>
bool
nwf_hash_map<K, V, H> :: table :: copy_slot(nwf_hash_map* top_map, size_t idx, table* new_table)
{
    typename wrapper<K>::type kwitness(wrapper<K>::load(&nodes[idx].key));

    while (wrapper<K>::is_null(kwitness))
    {
        kwitness = wrapper<K>::cas(&nodes[idx].key,
                                   wrapper<K>::NULLVALUE(),
                                   wrapper<K>::TOMBSTONE());

        if (wrapper<K>::is_null(kwitness))
        {
            typename wrapper<V>::type tmp(wrapper<V>::load(&nodes[idx].val));

            while (wrapper<V>::cas(&nodes[idx].val, tmp, wrapper<V>::TOMBPRIME()) != tmp)
            {
                tmp = wrapper<V>::load(&nodes[idx].val);
            }

            return true;
        }
    }

    if (wrapper<K>::is_tombstone(kwitness))
    {
        return false;
    }

    typename wrapper<V>::type old_val(wrapper<V>::load(&nodes[idx].val));

    while (!wrapper<V>::is_primed(old_val))
    {
        typename wrapper<V>::type box(wrapper<V>::TOMBPRIME());

        if (!wrapper<V>::is_null(old_val) && !wrapper<V>::is_tombstone(old_val))
        {
            box = wrapper<V>::prime(old_val);
        }

        typename wrapper<V>::type witness;
        witness = wrapper<V>::cas(&nodes[idx].val, old_val, box);

        if (witness == old_val)
        {
            if (box == wrapper<V>::TOMBPRIME())
            {
                return true;
            }

            old_val = box;
            break;
        }

        old_val = wrapper<V>::load(&nodes[idx].val);
    }

    if (wrapper<V>::is_tombprime(old_val))
    {
        return false;
    }

    typename wrapper<K>::type key = wrapper<K>::load(&nodes[idx].key);
    typename wrapper<V>::type old_unboxed = wrapper<V>::deprime(old_val);
    assert(old_unboxed != wrapper<V>::TOMBSTONE());
    new_table->inc_size();
    top_map->put_if_match(new_table, key,
                          wrapper<V>::NULLVALUE(),
                          old_unboxed);
    typename wrapper<V>::type witness;

    while ((witness = wrapper<V>::cas(&nodes[idx].val, old_val, wrapper<V>::TOMBPRIME())) != old_val)
    {
        old_val = witness;
    }

    // I would love for this to be "if (copied) new_table->inc_size()"
    //
    // A rare race condition prevents us from simply copying the old elems
    // counter (think about a concurrent insert that hits the old table)
    //
    // An even rarer race would allow the inc_size/dec_size routine to push
    // elems negative.  We inc_size above and dec_size in the (less likely) case
    // that the copy failed.

    // There's something wrong with the Java implementation's accounting for
    // size because it measures null->not null in the new table.  Unfortunately,
    // put_if_match doesn't track null->not null in the new table, allowing
    // copy_done to fall short of the work done.  Count transitions to TOMBPRIME
    // instead, as this is the only spot we stomp down a TOMBPRIME

    if (wrapper<V>::is_tombprime(old_val))
    {
        new_table->dec_size();
        return false;
    }

    wrapper<V>::collect(top_map->m_gc, old_val);
    return true;
}

template <typename K, typename V, uint64_t (*H)(const K&)>
nwf_hash_map<K, V, H> :: iterator :: iterator()
    : m_table(NULL)
    , m_index(0)
    , m_primed(false)
    , m_cached()
{
    prime();
}

template <typename K, typename V, uint64_t (*H)(const K&)>
nwf_hash_map<K, V, H> :: iterator :: iterator(table* t)
    : m_table(t)
    , m_index(0)
    , m_primed(false)
    , m_cached()
{
    prime();
}

template <typename K, typename V, uint64_t (*H)(const K&)>
nwf_hash_map<K, V, H> :: iterator :: iterator(const iterator& other)
    : m_table(other.m_table)
    , m_index(other.m_index)
    , m_primed(other.m_primed)
    , m_cached(other.m_cached)
{
}

template <typename K, typename V, uint64_t (*H)(const K&)>
inline typename nwf_hash_map<K, V, H>::iterator&
nwf_hash_map<K, V, H> :: iterator :: operator ++ ()
{
    advance();
    return *this;
}

template <typename K, typename V, uint64_t (*H)(const K&)>
inline bool
nwf_hash_map<K, V, H> :: iterator :: operator == (const iterator& rhs)
{
    return this->m_table == rhs.m_table &&
           (this->m_table == NULL || this->m_index == rhs.m_index);
}

template <typename K, typename V, uint64_t (*H)(const K&)>
inline typename nwf_hash_map<K, V, H>::iterator&
nwf_hash_map<K, V, H> :: iterator :: operator = (const iterator& rhs)
{
    if (this != &rhs)
    {
        m_table = rhs.m_table;
        m_index = rhs.m_index;
        m_cached = rhs.m_cached;
    }

    return *this;
}

template <typename K, typename V, uint64_t (*H)(const K&)>
inline void
nwf_hash_map<K, V, H> :: iterator :: prime()
{
    while (true)
    {
        if (!m_table || m_primed)
        {
            return;
        }

        if (m_index >= m_table->capacity)
        {
            m_table = e::atomic::load_ptr_acquire(&m_table->next);
            m_index = 0;
            m_cached = std::pair<K, V>();
            continue;
        }

        typename wrapper<K>::type k = wrapper<K>::load(&m_table->nodes[m_index].key);
        typename wrapper<V>::type v = wrapper<V>::load(&m_table->nodes[m_index].val);

        if (wrapper<K>::is_special(k) || wrapper<V>::is_special(v) ||
            wrapper<K>::is_primed(k) || wrapper<V>::is_primed(v))
        {
            ++m_index;
            continue;
        }

        m_primed = true;
        m_cached = std::make_pair(wrapper<K>::unwrap(k),
                                  wrapper<V>::unwrap(v));
        return;
    }
}

template <typename K, typename V, uint64_t (*H)(const K&)>
inline void
nwf_hash_map<K, V, H> :: iterator :: advance()
{
    m_primed = false;
    ++m_index;
    prime();
}

} // namespace e

#endif // e_nwf_hash_map_h_

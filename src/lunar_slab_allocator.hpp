#ifndef LUNAR_SLAB_ALLOCATOR
#define LUNAR_SLAB_ALLOCATOR

/*
 * CAUTION! THIS ALLOCATOR IS MT-UNSAFE!
 */

#include <new>
#include <limits>

#include <stdlib.h>

#include "slab.hpp"

namespace lunar {

template <typename T>
class slab_allocator {
public:
    typedef T         value_type;
    typedef size_t    size_type;
    typedef ptrdiff_t difference_type;
    typedef T*        pointer;
    typedef const T*  const_pointer;
    typedef T&        reference;
    typedef const T&  const_reference;

    template <typename U> struct rebind { typedef slab_allocator<U> other; };
    slab_allocator() throw()
    {
        if (slab_allocator<T>::m_refcnt == 0)
            slab_init(&m_slab, sizeof(T));

        slab_allocator<T>::m_refcnt++;
    }
    slab_allocator(const slab_allocator&) throw()
    {
        if (slab_allocator<T>::m_refcnt == 0)
            slab_init(&m_slab, sizeof(T));

        slab_allocator<T>::m_refcnt++;
    }

    template <typename U> slab_allocator(const slab_allocator<U>&) throw()
    {
        if (slab_allocator<U>::m_refcnt == 0)
            slab_init(&slab_allocator<U>::m_slab, sizeof(U));

        slab_allocator<U>::m_refcnt++;
    }

    ~slab_allocator() throw() {
        m_refcnt--;

        if (m_refcnt == 0)
            slab_destroy(&m_slab);
    }

    pointer address(reference x) const { return &x; }
    const_pointer address(const_reference x) const { return &x; }

    pointer allocate(size_type s, void const * = 0) {
        if (s == 1) {
            return (pointer)slab_alloc(&m_slab);
        } else if (s >= 1) {
            pointer temp = (pointer)malloc(sizeof(void*) + s * sizeof(T));
            if (temp == nullptr)
                return nullptr;

            void **vp = (void**)temp;
            *vp = (void*)~(uint64_t)0;

            return (pointer)((char*)temp + sizeof(void*));
        } else {
            return nullptr;
        }
    }

    void deallocate(pointer p, size_type) {
        void **vp = (void**)((char*)p - sizeof(void*));

        if (*vp == (void*)~(uint64_t)0)
            free(vp);
        else
            slab_free(&m_slab, p);
    }

    size_type max_size() const throw() {
        return std::numeric_limits<size_t>::max() / sizeof(T);
    }

    void construct(pointer p, const T& val) {
        new((void *)p) T(val);
    }

    void destroy(pointer p) {
        p->~T();
    }

    static __thread uint64_t   m_refcnt;
    static __thread slab_chain m_slab;
};

template <typename T> __thread uint64_t   slab_allocator<T>::m_refcnt = 0;
template <typename T> __thread slab_chain slab_allocator<T>::m_slab;

}

#endif // LUNAR_SLAB_ALLOCATOR
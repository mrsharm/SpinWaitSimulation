//
// VolatileStore stores a T into the target of a pointer to T.  It is guaranteed that this store will
// not be optimized away by the compiler, and that any operation that occurs before this store, in program
// order, will not be moved after this store.  In general, it is not guaranteed that the store will be
// atomic, though this is the case for most aligned scalar data types.  If you need atomic loads or stores,
// you need to consult the compiler and CPU manuals to find which circumstances allow atomicity.
//
template<typename T>
inline
void VolatileStore(T* pt, T val)
{
#ifndef DACCESS_COMPILE
#if defined(HOST_ARM64) && defined(__GNUC__)
    static const unsigned lockFreeAtomicSizeMask = (1 << 1) | (1 << 2) | (1 << 4) | (1 << 8);
    if ((1 << sizeof(T)) & lockFreeAtomicSizeMask)
    {
        __atomic_store((T volatile*)pt, &val, __ATOMIC_RELEASE);
    }
    else
    {
        VOLATILE_MEMORY_BARRIER();
        *(T volatile*)pt = val;
    }
#elif defined(HOST_ARM64) && defined(_MSC_VER)
    // silence warnings on casts in branches that are not taken.
#pragma warning(push)
#pragma warning(disable : 4311)
#pragma warning(disable : 4312)
    T* pv = &val;
    switch (sizeof(T))
    {
    case 1:
        __stlr8((unsigned __int8  volatile*)pt, *(unsigned __int8*)pv);
        break;
    case 2:
        __stlr16((unsigned __int16 volatile*)pt, *(unsigned __int16*)pv);
        break;
    case 4:
        __stlr32((unsigned __int32 volatile*)pt, *(unsigned __int32*)pv);
        break;
    case 8:
        __stlr64((unsigned __int64 volatile*)pt, *(unsigned __int64*)pv);
        break;
    default:
        __dmb(_ARM64_BARRIER_ISH);
        *(T volatile*)pt = val;
    }
#pragma warning(pop)
#else
    MemoryBarrier();
    *(T volatile*)pt = val;
#endif
#else
    * pt = val;
#endif
}

template<typename T>
inline
T VolatileLoad(T const* pt)
{
#ifndef DACCESS_COMPILE
#if defined(HOST_ARM64) && defined(__GNUC__)
    T val;
    static const unsigned lockFreeAtomicSizeMask = (1 << 1) | (1 << 2) | (1 << 4) | (1 << 8);
    if ((1 << sizeof(T)) & lockFreeAtomicSizeMask)
    {
        __atomic_load((T const*)pt, const_cast<typename RemoveVolatile<T>::type*>(&val), __ATOMIC_ACQUIRE);
    }
    else
    {
        val = *(T volatile const*)pt;
        asm volatile ("dmb ishld" : : : "memory");
    }
#elif defined(HOST_ARM64) && defined(_MSC_VER)
    // silence warnings on casts in branches that are not taken.
#pragma warning(push)
#pragma warning(disable : 4311)
#pragma warning(disable : 4312)
    T val;
    T* pv = &val;
    switch (sizeof(T))
    {
    case 1:
        *(unsigned __int8*)pv = __ldar8((unsigned __int8   volatile*)pt);
        break;
    case 2:
        *(unsigned __int16*)pv = __ldar16((unsigned __int16  volatile*)pt);
        break;
    case 4:
        *(unsigned __int32*)pv = __ldar32((unsigned __int32  volatile*)pt);
        break;
    case 8:
        *(unsigned __int64*)pv = __ldar64((unsigned __int64  volatile*)pt);
        break;
    default:
        val = *(T volatile const*)pt;
        __dmb(_ARM64_BARRIER_ISHLD);
    }
#pragma warning(pop)
#else
    T val = *(T volatile const*)pt;
    MemoryBarrier();
#endif
#else
    T val = *pt;
#endif
    return val;
}


template <typename T>
class Volatile
{
private:
    //
    // The data which we are treating as volatile
    //
    T m_val;

public:
    //
    // Default constructor.
    //
    inline Volatile() = default;

    //
    // Allow initialization of Volatile<T> from a T
    //
    inline Volatile(const T& val)
    {
        ((volatile T&)m_val) = val;
    }

    //
    // Copy constructor
    //
    inline Volatile(const Volatile<T>& other)
    {
        ((volatile T&)m_val) = other.Load();
    }

    //
    // Loads the value of the volatile variable.  See code:VolatileLoad for the semantics of this operation.
    //
    inline T Load() const
    {
        return VolatileLoad(&m_val);
    }

    //
    // Loads the value of the volatile variable atomically without erecting the memory barrier.
    //
    inline T LoadWithoutBarrier() const
    {
        return ((volatile T &)m_val);
    }

    //
    // Stores a new value to the volatile variable.  See code:VolatileStore for the semantics of this
    // operation.
    //
    inline void Store(const T& val)
    {
        VolatileStore(&m_val, val);
    }

    //
    // Gets a pointer to the volatile variable.  This is dangerous, as it permits the variable to be
    // accessed without using Load and Store, but it is necessary for passing Volatile<T> to APIs like
    // InterlockedIncrement.
    //
    inline volatile T* GetPointer() { return (volatile T*)&m_val; }

    //
    // Assignment from T
    //
    inline Volatile<T>& operator=(T val) { Store(val); return *this; }

    //
    // Get the address of the volatile variable.  This is dangerous, as it allows the value of the
    // volatile variable to be accessed directly, without going through Load and Store, but it is
    // necessary for passing Volatile<T> to APIs like InterlockedIncrement.  Note that we are returning
    // a pointer to a volatile T here, so we cannot accidentally pass this pointer to an API that
    // expects a normal pointer.
    //
    inline T volatile* operator&() { return this->GetPointer(); }
    inline T volatile const* operator&() const { return this->GetPointer(); }

    //
    // Allow casts from Volatile<T> to T.  Note that this allows implicit casts, so you can
    // pass a Volatile<T> directly to a method that expects a T.
    //
    inline operator T() const
    {
        return this->Load();
    }

};
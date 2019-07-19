// This class defines an element that is stored
// in the garbage collection information list.
//
template <class T>
class PtrDetails
{
  public:
    // current reference count <-- This is the main purpose of this class.
    unsigned refcount;

    // pointer to allocated memory 
    T *memPtr;

    /* isArray is true if memPtr points
    to an allocated array. It is false
    otherwise. */
    bool isArray;

    /* If memPtr is pointing to an allocated
    array, then arraySize contains its size */
    unsigned arraySize;

    PtrDetails(void)
    {
        refcount = 0;
        memPtr = nullptr;
        isArray = false;
        arraySize = 0;
    }

    PtrDetails(T* ptr, unsigned size = 0)
    {
        refcount = 1;
        memPtr = ptr;
        isArray = (size > 0);
        arraySize = size;
    }

    void decrementRefCount()
    {
        if (refcount != 0)
        {
            refcount--;
        }
    }
};

// Overloading operator== allows two class objects to be compared.
// This is needed because the refContainer in the Pointer class is an STL list of PtrDetails
// and the type contained within a STL list class must have a == operator defined. 
template <class T>
bool operator==(const PtrDetails<T> &ob1,
                const PtrDetails<T> &ob2)
{
    // Only compares memPtr because it is not possible to have two different Iters 
    // with the same memPtr value but differing values for other attributes.
    // This is true even for arrays as the Pointer class does not allow pointing to 
    // a specific place within an array created via Pointer.
    return (ob1.memPtr == ob2.memPtr);
}
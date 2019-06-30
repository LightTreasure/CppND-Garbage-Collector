// This class defines an element that is stored
// in the garbage collection information list.
//
template <class T>
class PtrDetails
{
  public:
    // current reference count
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
};

// Overloading operator== allows two class objects to be compared.
// This is needed by the STL list class.
template <class T>
bool operator==(const PtrDetails<T> &ob1,
                const PtrDetails<T> &ob2)
{
    // Currently this ignores all the other members of PtrDetails.
    // TODO: find if it's necessary to check the other members of PtrDetails
    return (ob1.memPtr == ob2.memPtr);
}
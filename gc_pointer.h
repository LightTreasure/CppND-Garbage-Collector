#include <iostream>
#include <list>
#include <typeinfo>
#include <cstdlib>
#include "gc_details.h"
#include "gc_iterator.h"

/*
    Pointer implements a pointer type that uses
    garbage collection to release unused memory.
    A Pointer must only be used to point to memory
    that was dynamically allocated using new.
    When used to refer to an allocated array,
    specify the array size.
*/
template <class T, int size = 0>
class Pointer{
private:
    // refContainer maintains the garbage collection list.
    static std::list<PtrDetails<T> > refContainer;

    // addr points to the allocated memory to which
    // this Pointer currently points.
    T *addr;

    /*  isArray is true if this Pointer points
        to an allocated array. It is false
        otherwise. 
    */
    bool isArray; 

    // If this Pointer is pointing to an allocated
    // array, then arraySize contains its size.
    unsigned arraySize; // size of the array

    static bool first; // true when first Pointer is created

    // Return an iterator to pointer details in refContainer.
    typename std::list<PtrDetails<T> >::iterator findPtrInfo(T *ptr);
public:
    // Define an iterator type for Pointer<T>.
    typedef Iter<T> GCiterator;

    // Empty constructor
    // NOTE: templates aren't able to have prototypes with default arguments
    // this is why constructor is designed like this:
    Pointer(){
        Pointer(NULL);
    }

    Pointer(T*);

    // Copy constructor.
    Pointer(const Pointer &);

    // Destructor for Pointer.
    ~Pointer();

    // Collect garbage. Returns true if at least
    // one object was freed.
    static bool collect();

    // Overload assignment of pointer to Pointer.
    T *operator=(T *t);

    // Overload assignment of Pointer to Pointer.
    Pointer &operator=(Pointer &rv);

    // Return a reference to the object pointed
    // to by this Pointer.
    T &operator*(){
        return *addr;
    }

    // Return the address being pointed to.
    T *operator->() { return addr; }

    // Return a reference to the object at the
    // index specified by i.
    T &operator[](int i){ return addr[i];}

    // Conversion function to T *.
    operator T *() { return addr; }

    // Return an Iter to the start of the allocated memory.
    Iter<T> begin(){
        int _size;
        if (isArray)
            _size = arraySize;
        else
            _size = 1;
        return Iter<T>(addr, addr, addr + _size);
    }

    // Return an Iter to one past the end of an allocated array.
    Iter<T> end(){
        int _size;
        if (isArray)
            _size = arraySize;
        else
            _size = 1;
        return Iter<T>(addr + _size, addr, addr + _size);
    }

    // Return the size of refContainer for this type of Pointer.
    static int refContainerSize() { return refContainer.size(); }

    // A utility function that displays refContainer.
    static void showlist();

    // Clear refContainer when program exits.
    static void shutdown();
};

// STATIC INITIALIZATION
// Creates storage for the static variables
template <class T, int size>
std::list<PtrDetails<T> > Pointer<T, size>::refContainer;
template <class T, int size>
bool Pointer<T, size>::first = true;

// Constructor for both initialized and uninitialized objects. -> see class interface
template<class T,int size>
Pointer<T,size>::Pointer(T *t){
    // Register shutdown() as an exit function.
    if (first)
        atexit(shutdown);
    first = false;

    // First, construct the Pointer object by copying in the relevant attributes
    addr = t;
    arraySize = size;
    isArray = (size > 0);

    // Second, we need to ensure that our garbage collection takes care of this memory
    // If we're already doing that, great! Just increment the refcount. That is, if we already
    // have this memory in our refContainer, all we need to do is to increment the refcount. So let's check:
    typename std::list<PtrDetails<T> >::iterator p;
    p = findPtrInfo(t); // returns refContainer.end() if it cant' find that pointer

    // If we aren't tracking that memory in refContainer, add to it
    if (p == refContainer.end())
    {
        // Create a PtrDetails object of the same type and size from the raw pointer
        // This also initializes the refCount, so we don't have to worry about it here.
        PtrDetails<T> pDet(t, size);

        // Push it into the refContainer list
        refContainer.push_back(pDet);
    }
    else
    {
        // This memory is already being tracked by refContainer... just refcount it.
        p->refCount++;
    }
}

// Copy constructor.
template< class T, int size>
Pointer<T,size>::Pointer(const Pointer &ob){
    // Since we're creating a copy of an existing Pointer, we might already
    // have a PtrDetails in refContainer associated with it. All that we
    // need to do now is to increment the reference count for that memory address 
    typename std::list<PtrDetails<T> >::iterator p;
    p = findPtrInfo(ob.addr);
    if (p != refContainer.end())
    {
        p->refcount++;
    }

    // Now just copy over the rest of the members of the Pointer.
    addr = ob.addr;
    isArray = ob.isArray;
    arraySize = ob.arraySize;
}

// Destructor for Pointer.
template <class T, int size>
Pointer<T, size>::~Pointer(){
    // This pointer is going out of scope. Which means the location
    // it points to has reference count lowered by 1.
    typename std::list<PtrDetails<T> >::iterator p;
    p = findPtrInfo(addr);
    if (p != refContainer.end())
    {
        p->refcount--;
    }

    // This is where we call collect() for this Garbage Collector. 
    // Not exactly the most efficient location, but for this project that's fine.
    collect();
}

// Collect garbage. Returns true if at least
// one object was freed.
template <class T, int size>
bool Pointer<T, size>::collect(){
    bool memfreed = false;
    typename std::list<PtrDetails<T> >::iterator p;
    // Go through all the elements of refContainer and see 
    // if any of them has refcount = 0. If it does, delete it.
    // Note: The double loop is needed because refContainer is an STL list
    // and we alter it while iterating through it. So we need to restart
    // the search every time.
    do {
        // Scan refContainer looking for unreferenced pointers.
        for (p = refContainer.begin(); p != refContainer.end(); p++){
            // Skip locations that are being used
            if (p->refcount > 0)
            {
                continue;
            }
            
            // If we're here, that means the location has 0 refcount. Delete it!
            // First, remove unused entry from refContainer.
            refContainer.erase(p)

            // Now, delete the memory unless the Pointer is null.
            if (p->memPtr != nullptr)
            {
                if (p->isArray)
                {
                    delete[] p->memPtr;
                }
                else
                {
                    delete p;
                }
                memfreed = true;
            }
            
            // Restart the search.
            break;
        }
    } while (p != refContainer.end());

    return memfreed;
}

// Overload assignment of pointer to Pointer.
template <class T, int size>
T *Pointer<T, size>::operator=(T *t){
    // We want this Pointer to point to the same location as t.
    // Because we will be doing that, the refcount of the location 
    // that this pointer currently points to (if it exists) will go down by 1.
    typename std::list<PtrDetails<T> >::iterator oldloc;
    oldloc = findPtrInfo(addr);
    if (oldloc != refContainer.end())
    {
        oldloc->refcount--;
    }

    // Now, construct the Pointer object by copying in the relevant attributes
    addr = t;
    arraySize = size;
    isArray = (size > 0);

    // Finally, we need to ensure that our garbage collection takes care of this memory
    // If we're already doing that, great! Just increment the refcount. That is, if we already
    // have this memory in our refContainer, all we need to do is to increment the refcount. So let's check:
    typename std::list<PtrDetails<T> >::iterator p;
    p = findPtrInfo(t); // returns refContainer.end() if it cant' find that pointer

    // If we aren't tracking that memory in refContainer, add to it
    if (p == refContainer.end())
    {
        // Create a PtrDetails object of the same type and size from the raw pointer
        // This also initializes the refCount, so we don't have to worry about it here.
        PtrDetails<T> pDet(t, size);

        // Push it into the refContainer list
        refContainer.push_back(pDet);
    }
    else
    {
        // This memory is already being tracked by refContainer... just refcount it.
        p->refCount++;
    }

    return t;
}

// Overload assignment of Pointer to Pointer.
template <class T, int size>
Pointer<T, size> &Pointer<T, size>::operator=(Pointer &rv){
    // We want this Pointer to point to the same location as rv.
    // Because we will be doing that, the refcount of the currently
    // pointed location (if it exists) will go down by 1.
    typename std::list<PtrDetails<T> >::iterator oldloc;
    oldloc = findPtrInfo(addr);
    if (oldloc != refContainer.end())
    {
        oldloc->refcount--;
    }

    // Now that we have dealt with the old location, we want this Pointer to
    // reference the same location as rv. Which means that location's (if it exists) refcount goes up by 1.
    typename std::list<PtrDetails<T> >::iterator newloc;
    newloc = findPtrInfo(rv.addr);
    if (newloc != refContainer.end())
    {
        newloc->refcount++;
    }

    // Now just copy over the rest of the members of the Pointer.
    addr = rv.addr;
    isArray = rv.isArray;
    arraySize = rv.arraySize;

    rerurn *this;
}

// A utility function that displays refContainer.
template <class T, int size>
void Pointer<T, size>::showlist(){
    typename std::list<PtrDetails<T> >::iterator p;
    std::cout << "refContainer<" << typeid(T).name() << ", " << size << ">:\n";
    std::cout << "memPtr refcount value\n ";
    if (refContainer.begin() == refContainer.end())
    {
        std::cout << " Container is empty!\n\n ";
    }
    for (p = refContainer.begin(); p != refContainer.end(); p++)
    {
        std::cout << "[" << (void *)p->memPtr << "]"
             << " " << p->refcount << " ";
        if (p->memPtr)
            std::cout << " " << *p->memPtr;
        else
            std::cout << "---";
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

// Find a pointer in refContainer.
template <class T, int size>
typename std::list<PtrDetails<T> >::iterator
Pointer<T, size>::findPtrInfo(T *ptr){
    typename std::list<PtrDetails<T> >::iterator p;
    // Find ptr in refContainer.
    for (p = refContainer.begin(); p != refContainer.end(); p++)
        if (p->memPtr == ptr)
            return p;
    return p;
}

// Clear refContainer when program exits.
template <class T, int size>
void Pointer<T, size>::shutdown(){
    if (refContainerSize() == 0)
        return; // list is empty
    typename std::list<PtrDetails<T> >::iterator p;
    for (p = refContainer.begin(); p != refContainer.end(); p++)
    {
        // Set all reference counts to zero
        p->refcount = 0;
    }
    collect();
}
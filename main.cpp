#include "gc_pointer.h"
#include "LeakTester.h"

int main()
{
    Pointer<int> p = new int(19);
    p = new int(21);
    p = new int(28);

    Pointer <int> x;
    x = new int(2);
    
    *x = 88;
    
    int k = *x;
 
    Pointer <double, 5> d;
    d = new double[5];
    d[0] = 0.0;
    d[1] = 1.1;
    d[2] = 2.2;
    d[3] = 3.3;
    d[4] = 4.4;

    for (Iter<double> it = d.begin(); it < d.end(); it++)
    {
        std::cout << *it << " ";
    }
    return 0;
}
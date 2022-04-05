#include "src/memory/alloc.h"

#include <iostream>

using namespace std;
int main()
{
    int nCount = 4;
    corekv::SimpleFreeListAlloc simple_freelist_alloc;
    int *a = (int *)simple_freelist_alloc.Allocate(sizeof(int)*nCount);
    for (int i=0; i<nCount; ++i)
    {
        a[i] = i;
    }

    std::cout << "a[](" << a << "): ";
    for (int i=0; i<nCount; ++i)
    {
        std::cout << a[i] << " ";
    }

    simple_freelist_alloc.Deallocate(a, sizeof(int)*nCount);

    std::cout << std::endl;
    std::cout << "a[](" << a << "): ";
    for (int i=0; i<nCount; ++i)
    {
        std::cout << a[i] << " ";
    }

    return 0;
}
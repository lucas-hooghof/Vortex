#include "generic/constructers.h"

extern "C" int kernel_main()
{
    call_constructers();
    return 123;
}
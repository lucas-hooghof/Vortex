#include "constructers.h"

typedef void (*constructor_t)(void);

extern constructor_t __init_array_start[];
extern constructor_t __init_array_end[];

void call_constructers()
{
    for (constructor_t* ctor = __init_array_start;
         ctor != __init_array_end;
         ctor++)
    {
        (*ctor)();
    }
}
#pragma once

#ifdef _WIN32
    #include <windows.h>
#else
    #include <stdlib.h>
#endif


void* alloc(size_t);
void freeHeap(void* ptr);

void trusted_machine(void);
void XOR_maker(uint64_t* key);

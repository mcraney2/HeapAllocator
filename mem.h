#ifndef __mem_h__
#define __mem_h__

int Init_Mem(int sizeOfRegion);
void* Alloc_Mem(int size);
int Free_Mem(void *ptr);
void Dump_Mem();

void* malloc(size_t size) {
    return NULL;
}

#endif // __mem_h__


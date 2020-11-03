#include <unistd.h>
#include "prim.h"

Value *Prelude_IO_prim__getChar(Value *world)
{
    char c = getchar();
    return (Value *)makeChar(c);
}

// This is NOT THREAD SAFE in the current implementation

IORef_Storage *newIORef_Storage(int capacity)
{
    IORef_Storage *retVal = (IORef_Storage *)malloc(sizeof(IORef_Storage));
    retVal->filled = 0;
    retVal->total = capacity;
    retVal->refs = (Value **)malloc(sizeof(Value *) * retVal->total);
    return retVal;
}

void doubleIORef_Storage(IORef_Storage *ior)
{
    Value **values = (Value **)malloc(sizeof(Value *) * ior->total * 2);
    ior->total *= 2;
    for (int i = 0; i < ior->filled; i++)
    {
        values[i] = ior->refs[i];
    }
    free(ior->refs);
    ior->refs = values;
}

Value *newIORef(Value *erased, Value *input_value, Value *_world)
{
    // if no ioRef Storag exist, start with one
    Value_World *world = (Value_World *)_world;
    if (!world->listIORefs)
    {
        world->listIORefs = newIORef_Storage(128);
    }
    // expand size of needed
    if (world->listIORefs->filled >= world->listIORefs->total)
    {
        doubleIORef_Storage(world->listIORefs);
    }

    // store value
    Value_IORef *ioRef = (Value_IORef *)newValue();
    ioRef->header.tag = IOREF_TAG;
    ioRef->index = world->listIORefs->filled;
    world->listIORefs->refs[world->listIORefs->filled] = newReference(input_value);
    world->listIORefs->filled++;

    return (Value *)ioRef;
}

Value *readIORef(Value *erased, Value *_index, Value *_world)
{
    Value_World *world = (Value_World *)_world;
    Value_IORef *index = (Value_IORef *)_index;
    return newReference(world->listIORefs->refs[index->index]);
}

Value *writeIORef(Value *erased, Value *_index, Value *new_value, Value *_world)
{
    Value_World *world = (Value_World *)_world;
    Value_IORef *index = (Value_IORef *)_index;
    removeReference(world->listIORefs->refs[index->index]);
    world->listIORefs->refs[index->index] = newReference(new_value);
    return newReference(_index);
}

// -----------------------------------
//       System operations
// -----------------------------------

Value *sysOS(void)
{
#ifdef _WIN32
    return (Value *)makeString("windows");
#elif _WIN64
    return (Value *)makeString("windows");
#elif __APPLE__ || __MACH__
    return (Value *)makeString("Mac OSX");
#elif __linux__
    return (Value *)makeString("Linux");
#elif __FreeBSD__
    return (Value *)makeString("FreeBSD");
#elif __unix || __unix__
    return (Value *)makeString("Unix");
#else
    return (Value *)makeString("Other");
#endif
}

Value* idris2_crash(Value* msg) {
    Value_String* str = (Value_String*)msg;
    printf("ERROR: %s\n", str->str);
    exit(-1);
}


//
//
//
// // -----------------------------------
// //         Array operations
// // -----------------------------------

Value *newArray(Value *erased, Value *_length, Value *v, Value *_word)
{
    int length = extractInt(_length);
    Value_Array *a = makeArray(length);

    for (int i = 0; i < length; i++)
    {
        a->arr[i] = newReference(v);
    }

    return (Value *)a;
}

Value *arrayGet(Value *erased, Value *_array, Value *_index, Value *_word)
{
    Value_Array *a = (Value_Array *)_array;
    return newReference(a->arr[((Value_Int32 *)_index)->i32]);
}

Value *arraySet(Value *erased, Value *_array, Value *_index, Value *v, Value *_word)
{
    Value_Array *a = (Value_Array *)_array;
    removeReference(a->arr[((Value_Int32 *)_index)->i32]);
    a->arr[((Value_Int32 *)_index)->i32] = newReference(v);
    return NULL;
}

//
// -----------------------------------
//      Pointer operations
// -----------------------------------

Value *PrimIO_prim__nullAnyPtr(Value *ptr)
{
    void *p;
    switch (ptr->header.tag)
    {
    case STRING_TAG:
        p = ((Value_String *)ptr)->str;
        break;
    case POINTER_TAG:
        p = ((Value_Pointer *)ptr)->p;
        break;
    default:
        p = NULL;
    }
    if (p)
    {
        return (Value *)makeInt32(0);
    }
    else
    {
        return (Value *)makeInt32(1);
    }
}

Value *onCollect(Value *_erased, Value *_anyPtr, Value *_freeingFunction, Value *_world)
{
    printf("onCollect called\n");
    Value_GCPointer *retVal = (Value_GCPointer *)newValue();
    retVal->header.tag = GC_POINTER_TAG;
    retVal->p = (Value_Pointer *)newReference(_anyPtr);
    retVal->onCollectFct = (Value_Closure *)newReference(_freeingFunction);
    return (Value *)retVal;
}

Value *onCollectAny(Value *_erased, Value *_anyPtr, Value *_freeingFunction, Value *_world)
{
    printf("onCollectAny called\n");
    Value_GCPointer *retVal = (Value_GCPointer *)newValue();
    retVal->header.tag = GC_POINTER_TAG;
    retVal->p = (Value_Pointer *)_anyPtr;
    retVal->onCollectFct = (Value_Closure *)_freeingFunction;
    return (Value *)retVal;
}

Value *voidElim(Value *erased1, Value *erased2)
{
    return NULL;
}

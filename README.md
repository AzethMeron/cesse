# C-Essentials: fundamental algorithms and datastructures

Implements basic containers *(dynamic array, stack, hashmap)* and algorithms *(xoshiro256\*\* PRNG algorithm, stable merge-sort)*. All generic and thus reusable with any type, albeit with performance penalty. 
**All objects are provided by users, borrowed and stored in buckets**, which does hurt cache-locality.

# Memory ownership

C-Esse objects (with sole exception of ErrorCode) are created with \*\_new functions of adequate types:

```[c]
ErrorCode  error = CESSE_OK;
Stack* stack = stack_new(&error);
Array* array = array_new(100, &error);
Map* map = map_new(&error);
```

These create the objects and return memory allocated on heap, though you should NEVER try to free them on your own. 
Use adequate  \*\_delete functions *(third argument NULL for now, will explain bit later)*

```[c]
stack_delete(&stack, &error, NULL);
array_delete(&array, &error, NULL);
map_delete(&map, &error, NULL);
```

All pointers involved in the implementations are owned by the objects, but given C has no destructors you must invoke delete functions yourself. Whenever possible, library returns by value rather than by pointer but usually, it isn't possible.

**The contents are different story though**. All containers hold pointers, not objects - you're meant to put data on heap, pass pointers to container, then at your own volition remove them from container and finally, free the object on your own. This is obvious approach as only you know how to correctly free your own objects. **It does mean that internally, all that container sees is ```void*```**.

Delete functions do their best to help you with cleaning objects and this is why each of them has third argument: function_delete freer. If provided, it will be used to free the contents of container. Error management in this path is a bit finnicky though and it's recommanded to first drain the contents using remove/pop functions, and only once it's empty - remove.

Delete always removes the object. Abstaining from passing freer, or if freer errors-out will not stop it from removing container - leading to memory leak.

# Building

Header files are in ```headers/cesse``` directory. It's recommanded to add path to ```headers``` in compiler flags, so including the files remains as:
```[c]
#include "cesse/array.h"
#include "cesse/stack.h"
#include "cesse/map.h"
#include "cesse/sort.h"
#include "cesse/rng.h"
```

Source files are stored in ```src```.

To build, you may use cmake with some of provided presets. ```Tools``` directory stores bash scripts with commands to build/test/etc.

**This library has also documentation written for Doxygen.** You may build it by invoking ```tools/build_docs.sh```. 

Target directory *(for compilation and building, as well as for docs)* is ```build``` directory.

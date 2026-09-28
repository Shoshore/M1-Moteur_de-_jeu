#ifndef SYSTEMIZED
#define SYSTEMIZED

#include "src/ecs/utils.h"

struct Systemized {
    SystemId id;

    Systemized() {}
    Systemized(SystemId id) : id(id) {}
};

#endif // SYSTEMIZED
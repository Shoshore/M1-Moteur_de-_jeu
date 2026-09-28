#ifndef NODED
#define NODED

#include "src/GlobalScene.h"

struct Noded {
    NodeId id;

    Noded() {}
    Noded(NodeId id) : id(id) {}
};

#endif // NODED
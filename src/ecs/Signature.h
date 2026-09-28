#ifndef SIGNATURE
#define SIGNATURE

#include "src/ecs/Component.h"
#include <bitset>

namespace ECS {
const unsigned long MAX_ENTITIES = 8192;
using Signature = std::bitset<std::tuple_size_v<ComponentTypes>>;
} // namespace ECS

#endif // SIGNATURE
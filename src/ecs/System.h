#ifndef SYSTEM
#define SYSTEM
#include "utils.h"
#include <set>

class System {
    std::set<EntityId> entities;

  public:
    void addEntity(EntityId id);
    void removeEntity(EntityId id);
    void setEntities(const std::set<EntityId> &ids);
    const std::set<EntityId> &getEntities() const;

    virtual void registerComponents(SystemId id) = 0;
};

#endif // SYSTEM
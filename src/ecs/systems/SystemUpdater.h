#ifndef SYSTEM_UPDATER
#define SYSTEM_UPDATER

#include "src/ecs/ECSManager.h"
#include "src/ecs/System.h"
#include "src/ecs/components/Systemized.h"
#include "src/ecs/utils.h"
#include <memory>
#include <unordered_map>

class UpdatableSystem : public System {
  public:
    virtual void update(float deltaTime) = 0;
};

class SystemUpdater : public UpdatableSystem {
    std::unordered_map<SystemId, std::shared_ptr<UpdatableSystem>> systems;
    std::unordered_map<SystemId, bool> enabled;
    std::unordered_map<SystemId, EntityId> entityIds;

  public:
    SystemUpdater() {}

    void update(float deltaTime) override {
        for (EntityId entity : this->getEntities()) {
            SystemId id = ECSManager::getComponentOfEntity<Systemized>(entity)
                              .value()
                              .get()
                              .id;
            if (this->systems.count(id) != 0) {
                if (this->enabled[id]) {
                    this->systems[id]->update(deltaTime);
                }
            }
        }
    }

    void registerComponents(SystemId id) override {
        ECSManager::registerComponentToSystem<Systemized>(id);
    }

    SystemId addSystem(std::shared_ptr<UpdatableSystem> system) {
        SystemId systemId = ECSManager::registerSystem(system);

        system->registerComponents(systemId);
        EntityId entityId = ECSManager::generateEntityId();
        ECSManager::setComponentToEntity(Systemized(systemId), entityId);

        this->systems[systemId] = system;
        this->enabled[systemId] = true;
        this->entityIds[systemId] = entityId;

        return systemId;
    }

    void removeSystem(SystemId system) {
        if (this->systems.count(system) != 0) {
            ECSManager::removeSystem(system);
            ECSManager::removeEntity(this->entityIds[system]);

            this->systems.erase(system);
            this->enabled.erase(system);
            this->entityIds.erase(system);
        }
    }

    void enable(SystemId system) {
        if (this->systems.count(system) != 0) {
            this->enabled[system] = true;
        }
    }

    void disable(SystemId system) {
        if (this->systems.count(system) != 0) {
            this->enabled[system] = false;
        }
    }

    void toggle(SystemId system) {
        if (this->systems.count(system) != 0) {
            this->enabled[system] = !this->enabled[system];
        }
    }
};

#endif // SYSTEM_UPDATER
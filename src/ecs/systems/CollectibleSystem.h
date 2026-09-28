#ifndef COLLECTIBLE_SYSTEM
#define COLLECTIBLE_SYSTEM

#include <glm/ext.hpp>
#include <vector>

#include "src/GlobalScene.h"
#include "src/ecs/ECSManager.h"
#include "src/ecs/components/Collectible.h"
#include "src/ecs/components/Inventory.h"
#include "src/ecs/components/Noded.h"
#include "src/ecs/components/Positionable.h"
#include "src/ecs/components/VerletBody.h"
#include "src/ecs/systems/SystemUpdater.h"
#include "src/ecs/utils.h"

class CollectibleSystem : public UpdatableSystem {
    GlobalScene &scene;
    EntityId playerEntityId;

  public:
    CollectibleSystem(GlobalScene &scene, EntityId playerEntityId)
        : scene(scene), playerEntityId(playerEntityId) {}

    void registerComponents(SystemId id) override {
        ECSManager::registerComponentToSystem<Collectible>(id);
        ECSManager::registerComponentToSystem<Positionable>(id);
        ECSManager::registerComponentToSystem<VerletBody>(id);
        ECSManager::registerComponentToSystem<Noded>(id);
    }

    void update(float deltaTime) override {
        auto playerPositionOpt =
            ECSManager::getComponentOfEntity<Positionable>(playerEntityId);
        auto playerBodyOpt =
            ECSManager::getComponentOfEntity<VerletBody>(playerEntityId);
        auto playerInventoryOpt =
            ECSManager::getComponentOfEntity<Inventory>(playerEntityId);

        if (!playerPositionOpt.has_value() || !playerBodyOpt.has_value() ||
            !playerInventoryOpt.has_value()) {
            return;
        }

        const glm::vec3 playerPosition = playerPositionOpt.value().get().pos;
        const float playerRadius = playerBodyOpt.value().get().size;
        Inventory &playerInventory = playerInventoryOpt.value().get();

        const std::set<EntityId> &CollectibleSet = this->getEntities();
        std::vector<EntityId> CollectibleEntities(CollectibleSet.begin(),
                                                  CollectibleSet.end());

        std::vector<EntityId> currentlyCollected;

        for (EntityId collectibleEntityId : CollectibleEntities) {
            Collectible &collectible =
                ECSManager::getComponentOfEntity<Collectible>(
                    collectibleEntityId)
                    .value()
                    .get();

            if (collectible.interacted) {
                continue;
            }

            Positionable &collectiblePositionable =
                ECSManager::getComponentOfEntity<Positionable>(
                    collectibleEntityId)
                    .value()
                    .get();

            VerletBody &collectibleBody =
                ECSManager::getComponentOfEntity<VerletBody>(
                    collectibleEntityId)
                    .value()
                    .get();

            const glm::vec3 delta =
                collectiblePositionable.pos - playerPosition;
            const float distanceSquared = glm::dot(delta, delta);

            const float triggerRadius = playerRadius + collectibleBody.size;
            const float triggerRadiusSquared = triggerRadius * triggerRadius;

            if (distanceSquared <= triggerRadiusSquared) {
                collectible.interacted = true;
                playerInventory.collected += collectible.value;
                currentlyCollected.push_back(collectibleEntityId);
            }
        }

        for (EntityId collectibleEntityId : currentlyCollected) {
            NodeId nodeId =
                ECSManager::getComponentOfEntity<Noded>(collectibleEntityId)
                    .value()
                    .get()
                    .id;

            scene.removeNode(nodeId);

            ECSManager::removeComponentFromEntity<Collectible>(
                collectibleEntityId);
            ECSManager::removeComponentFromEntity<VerletBody>(
                collectibleEntityId);
            ECSManager::removeComponentFromEntity<Positionable>(
                collectibleEntityId);
            ECSManager::removeComponentFromEntity<Noded>(collectibleEntityId);
        }
    }
};

#endif
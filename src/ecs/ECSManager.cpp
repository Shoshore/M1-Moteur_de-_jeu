#include "ECSManager.h"
#include <cassert>

ECSManager::ECSManager() {
    registerAll<ComponentTypes>(
        std::make_index_sequence<std::tuple_size_v<ComponentTypes>>{});
}

EntityId ECSManager::generateEntityId() {
    ECSManager &ecs = ECSManager::getManager();
    EntityId entity = ecs.entityManager.generateId();
    assert(entity.value < ECS::MAX_ENTITIES &&
           "Too many entities in existence.");
    return entity;
}

void ECSManager::removeEntity(EntityId entity) {
    ECSManager &ecs = ECSManager::getManager();
    for (auto &[_, list] : ecs.componentMap) {
        list->remove(entity);
    }
    ecs.entityManager.remove(entity);
}

void ECSManager::entityComponentsChanged(EntityId entity) {
    ECSManager &ecs = ECSManager::getManager();
    ECS::Signature eSignature = ecs.entityManager.getSignature(entity);
    for (auto &[id, system] : ecs.systems) {
        ECS::Signature sSignature = ecs.systemManager.getSignature(id);
        if ((eSignature & sSignature) == sSignature) {
            system->addEntity(entity);
        } else {
            system->removeEntity(entity);
        }
    }
}

SystemId ECSManager::registerSystem(std::shared_ptr<System> system) {
    ECSManager &ecs = ECSManager::getManager();
    SystemId id = ecs.systemManager.generateId();
    ecs.systems[id] = system;
    return id;
}

void ECSManager::removeSystem(SystemId system) {
    ECSManager &ecs = ECSManager::getManager();
    ecs.systemManager.remove(system);
    ecs.systems.erase(system);
}
#include "System.h"

void System::addEntity(EntityId id) { this->entities.insert(id); }

void System::removeEntity(EntityId id) { this->entities.erase(id); }

void System::setEntities(const std::set<EntityId> &ids) {
    this->entities = ids;
}

const std::set<EntityId> &System::getEntities() const { return this->entities; }
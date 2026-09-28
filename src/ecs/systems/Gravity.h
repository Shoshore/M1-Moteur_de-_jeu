#ifndef GRAVITY
#define GRAVITY

#include <glm/ext.hpp>

#include "src/ecs/ECSManager.h"
#include "src/ecs/components/Attracted.h"
#include "src/ecs/components/Positionable.h"
#include "src/ecs/systems/SystemUpdater.h"

class Gravity : public UpdatableSystem {
  public:
    void update(float deltaTime) override {
        static const float GRAVITATIONAL_CONSTANT = 1;
        const std::set<EntityId> entities = this->getEntities();

        for (EntityId entity : entities) {
            glm::vec3 pos =
                ECSManager::getComponentOfEntity<Positionable>(entity)
                    .value()
                    .get()
                    .pos;
            VerletBody &verletBody =
                ECSManager::getComponentOfEntity<VerletBody>(entity).value();
            Attracted &attracted =
                ECSManager::getComponentOfEntity<Attracted>(entity).value();

            for (Attraction const &attraction : attracted.getAttractions()) {
                glm::vec3 acceleration;
                switch (attraction.mode) {
                case INWARD: {
                    glm::vec3 bodyPos =
                        ECSManager::getComponentOfEntity<Positionable>(
                            attraction.body.value())
                            .value()
                            .get()
                            .pos;

                    float distance = glm::distance(pos, bodyPos);
                    glm::vec3 direction = glm::normalize(bodyPos - pos);
                    acceleration = direction * GRAVITATIONAL_CONSTANT *
                                   (attraction.force / (distance * distance));
                    break;
                }
                case OUTWARD: {
                    glm::vec3 bodyPos =
                        ECSManager::getComponentOfEntity<Positionable>(
                            attraction.body.value())
                            .value()
                            .get()
                            .pos;

                    float distance = glm::distance(pos, bodyPos);
                    glm::vec3 direction = glm::normalize(bodyPos - pos);
                    acceleration = direction * GRAVITATIONAL_CONSTANT *
                                   (attraction.force / (distance * distance));
                    break;
                }
                case VECTOR:
                    glm::vec3 direction = attraction.direction.value();
                    acceleration =
                        direction * GRAVITATIONAL_CONSTANT * attraction.force;
                    break;
                }

                verletBody.acceleration += acceleration;
            }
        }
    }

    void registerComponents(SystemId id) override {
        ECSManager::registerComponentToSystem<Positionable>(id);
        ECSManager::registerComponentToSystem<VerletBody>(id);
        ECSManager::registerComponentToSystem<Attracted>(id);
    }
};

#endif // GRAVITY
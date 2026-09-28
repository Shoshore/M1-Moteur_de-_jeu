
#ifndef VERLET
#define VERLET

#include <cstdlib>
#include <optional>
#include <vector>

#include "glm/detail/func_geometric.hpp"
#include <glm/ext.hpp>

#include "src/GlobalScene.h"
#include "src/ecs/ECSManager.h"
#include "src/ecs/components/Positionable.h"
#include "src/ecs/components/VerletBody.h"
#include "src/ecs/systems/SystemUpdater.h"
#include "src/ecs/utils.h"
#include "src/prototype/Cuboid.h"

class Verlet : public UpdatableSystem {

    std::vector<Cuboid> cuboids;

  public:
    void update(float deltaTime) override {
        this->solveCollisions();
        this->updateBallPositions(deltaTime);
    }

    void registerComponents(SystemId id) override {
        ECSManager::registerComponentToSystem<Positionable>(id);
        ECSManager::registerComponentToSystem<VerletBody>(id);
    }

    void solveCollisions() {
        const std::set<EntityId> entities = this->getEntities();

        for (EntityId entity : entities) {
            Positionable &pos =
                ECSManager::getComponentOfEntity<Positionable>(entity).value();
            VerletBody &body =
                ECSManager::getComponentOfEntity<VerletBody>(entity).value();

            if (body.unmovable || body.isTrigger) {
                continue;
            }

            for (const Cuboid &cuboid : this->cuboids) {
                std::optional<glm::vec3> intersection =
                    cuboid.intersectsSphere(pos.pos, body.size);
                if (intersection.has_value()) {
                    pos.pos += intersection.value();
                }
            }
        }

        for (auto it1 = entities.begin(); it1 != entities.end(); it1++) {
            EntityId id1 = *it1;
            auto posOpt1 = ECSManager::getComponentOfEntity<Positionable>(id1);
            auto verletBodyOpt1 =
                ECSManager::getComponentOfEntity<VerletBody>(id1);

            if (posOpt1.has_value() && verletBodyOpt1.has_value()) {
                Positionable &pos1 = posOpt1.value();
                VerletBody &verletBody1 = verletBodyOpt1.value();

                auto it2 = it1;
                it2++;
                for (; it2 != entities.end(); it2++) {
                    EntityId id2 = *it2;
                    auto posOpt2 =
                        ECSManager::getComponentOfEntity<Positionable>(id2);
                    auto verletBodyOpt2 =
                        ECSManager::getComponentOfEntity<VerletBody>(id2);

                    if (posOpt2.has_value() && verletBodyOpt2.has_value()) {
                        Positionable &pos2 = posOpt2.value();
                        VerletBody &verletBody2 = verletBodyOpt2.value();

                        if (verletBody1.isTrigger || verletBody2.isTrigger) {
                            continue;
                        }

                        glm::vec3 collisionAxis = pos1.pos - pos2.pos;
                        float dist = glm::length(collisionAxis);

                        if (dist > 0.0001 &&
                            dist < verletBody1.size + verletBody2.size &&
                            dist > abs(verletBody2.size - verletBody1.size)) {

                            bool firstBiggest = true;
                            if (verletBody1.size < verletBody2.size) {
                                firstBiggest = false;
                            }

                            glm::vec3 n = collisionAxis / dist;
                            float diff =
                                verletBody1.size + verletBody2.size - dist;

                            if ((dist < verletBody1.size && firstBiggest) ||
                                (dist < verletBody2.size && !firstBiggest)) {
                                diff -= 2 * (firstBiggest ? verletBody2.size
                                                          : verletBody1.size);
                            }

                            if (verletBody1.unmovable) {
                                if (!verletBody2.unmovable) {
                                    pos2.pos -= diff * n;
                                }
                            } else {
                                if (verletBody2.unmovable) {
                                    pos1.pos += diff * n;
                                } else {
                                    pos1.pos += (verletBody1.size /
                                                 (verletBody1.size +
                                                  verletBody2.size)) *
                                                diff * n;
                                    pos2.pos -= (verletBody2.size /
                                                 (verletBody1.size +
                                                  verletBody2.size)) *
                                                diff * n;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    void updateBallPositions(float deltaTime) {
        static const float AIR_FRICTION = 0.3f;

        for (EntityId entity : this->getEntities()) {
            auto posOpt =
                ECSManager::getComponentOfEntity<Positionable>(entity);
            auto verletBodyOpt =
                ECSManager::getComponentOfEntity<VerletBody>(entity);

            if (posOpt.has_value() && verletBodyOpt.has_value() &&
                !verletBodyOpt.value().get().unmovable) {
                Positionable &pos = posOpt.value();
                VerletBody &verletBody = verletBodyOpt.value();
                glm::vec3 velocity = pos.pos - verletBody.last_position;

                if (glm::length(velocity) >= 0.2 * deltaTime) {
                    velocity *= (1. - (AIR_FRICTION * deltaTime));
                } else {
                    velocity *= (AIR_FRICTION * deltaTime);
                }

                verletBody.last_position = pos.pos;

                pos.pos +=
                    velocity + verletBody.acceleration * deltaTime * deltaTime;

                verletBody.acceleration = {};
            }
        }
    }

    void addCuboid(Cuboid cuboid) { this->cuboids.push_back(cuboid); }

    void toggleCuboids(GlobalScene &scene) {
        for (Cuboid cuboid : this->cuboids) {
            cuboid.toggle(scene);
        }
    }
};

#endif // VERLET
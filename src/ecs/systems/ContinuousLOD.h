#ifndef CONTINUOUSLOD
#define CONTINUOUSLOD

#include "glm/detail/func_geometric.hpp"

#include "src/Camera.h"
#include "src/GlobalScene.h"
#include "src/ecs/ECSManager.h"
#include "src/ecs/components/LOD.h"
#include "src/ecs/components/Positionable.h"
#include "src/ecs/components/VerletBody.h"
#include "src/ecs/systems/SystemUpdater.h"

using uint = unsigned int;

class ContinuousLOD : public UpdatableSystem {
    GlobalScene &scene;
    Camera &camera;

  public:
    ContinuousLOD(GlobalScene &scene, Camera &camera)
        : scene(scene), camera(camera) {}

    void registerComponents(SystemId id) override {
        ECSManager::registerComponentToSystem<Positionable>(id);
        ECSManager::registerComponentToSystem<LOD>(id);
        ECSManager::registerComponentToSystem<VerletBody>(id);
    }

    void update(float deltaTime) override {

        for (EntityId entity : this->getEntities()) {
            auto lodOpt = ECSManager::getComponentOfEntity<LOD>(entity);
            auto posOpt =
                ECSManager::getComponentOfEntity<Positionable>(entity);

            auto bodyOpt = ECSManager::getComponentOfEntity<VerletBody>(entity);

            if (lodOpt.has_value() && posOpt.has_value() &&
                bodyOpt.has_value()) {
                LOD &lod = lodOpt.value();
                Positionable &pos = posOpt.value();
                VerletBody &body = bodyOpt.value().get();

                const glm::vec3 cameraFront =
                    glm::normalize(this->camera.getFront());
                const glm::vec3 cameraPosition =
                    this->camera.getTarget() -
                    cameraFront * this->camera.getTargetDistance();

                const glm::vec3 cameraToObject = pos.pos - cameraPosition;
                float depth = glm::dot(cameraToObject, cameraFront);

                if (depth <= 0.001f) {
                    this->simplifyMesh(lod, 0.0f,
                                       (int)this->camera.getScreenHeight());
                    continue;
                }

                float fovRad = glm::radians(this->camera.getFov());
                float screenHeight = (float)this->camera.getScreenHeight();
                float focalPx = (screenHeight * 0.5f) / std::tan(fovRad * 0.5f);

                float radiusWorld = body.size;

                float radiusPx = focalPx * (radiusWorld / depth);
                float diameterPx = 2.f * radiusPx;

                this->simplifyMesh(lod, diameterPx,
                                   (int)this->camera.getScreenHeight());
            }
        }
    }

  private:
    void simplifyMesh(LOD &lod, float diameterPx, int screenHeightPx) {
        LODLevel newLevel =
            LOD::levelFromScreenDiameterPx(diameterPx, (float)screenHeightPx);

        if (newLevel == lod.current_level)
            return;

        lod.current_level = newLevel;

        float resolution;
        switch (newLevel) {
        case LODLevel::HIGH:
            resolution = 0.1f;
            break;
        case LODLevel::MEDIUM:
            resolution = 0.5f;
            break;
        case LODLevel::LOW:
            resolution = 2.0f;
            break;
        default:
            resolution = 1.0f;
            break;
        }

        lod.applyLOD(resolution);
        this->scene.getMesh(lod.id).value()->updateMeshData(
            lod.current_vertices, lod.current_indices, lod.current_normals,
            lod.current_uvs);
    }
};

#endif
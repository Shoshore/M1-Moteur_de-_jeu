#ifndef PROTOTYPE_CUBOID_H
#define PROTOTYPE_CUBOID_H

#include "glm/detail/func_geometric.hpp"
#include "glm/detail/type_vec.hpp"
#include "src/GlobalScene.h"
#include "src/Mesh.h"
#include "src/SceneObject.h"
#include <array>

#include <functional>
#include <glm/ext.hpp>
#include <optional>

//   F-----G
//  /|    /|
// E-|---H |
// | D---|-C
// |/    |/
// A-----B
class Cuboid {
    std::array<glm::vec3, 8> corners;
    glm::vec3 center;

    std::optional<SceneObject> sceneObject;

  public:
    NodeId id;

    Cuboid() {}

    Cuboid(glm::vec3 color, std::array<glm::vec3, 8> corners)
        : corners(corners) {

        std::vector<glm::vec3> normals;

        for (glm::vec3 corner : corners) {
            this->center += corner;
        }
        this->center /= 8;

        for (glm::vec3 corner : corners) {
            normals.push_back(glm::normalize(corner - this->center));
        }

        Mesh mesh =
            Mesh(std::vector(this->corners.begin(), this->corners.end()),
                 {0, 1, 3, 1, 2, 3, 7, 6, 1, 6, 2, 1, 6, 5, 2, 5, 3, 2,
                  4, 7, 0, 7, 1, 0, 5, 6, 4, 6, 7, 4, 5, 4, 3, 4, 0, 3},
                 normals);

        this->sceneObject =
            SceneObject("shaders/PBR_vs.glsl", "shaders/PBR_fs.glsl", mesh);
        this->sceneObject.value().setAlbedo(color);
        this->sceneObject.value().setMetallic(0.2);
        this->sceneObject.value().setRoughness(0.8);
    }

    Cuboid(std::array<glm::vec3, 8> corners) : Cuboid({1, 1, 1}, corners) {}

    Cuboid(glm::vec3 c1, glm::vec3 c2, glm::vec3 c3, glm::vec3 c4, glm::vec3 c5,
           glm::vec3 c6, glm::vec3 c7, glm::vec3 c8)
        : Cuboid(std::array{c1, c2, c3, c4, c5, c6, c7, c8}) {}

    Cuboid(glm::vec3 color, glm::vec3 c1, glm::vec3 c2, glm::vec3 c3,
           glm::vec3 c4, glm::vec3 c5, glm::vec3 c6, glm::vec3 c7, glm::vec3 c8)
        : Cuboid(color, std::array{c1, c2, c3, c4, c5, c6, c7, c8}) {}

    std::optional<std::reference_wrapper<SceneObject>> getSceneObject() {
        if (this->sceneObject.has_value()) {
            return this->sceneObject.value();
        } else {
            return std::nullopt;
        }
    }

    std::optional<glm::vec3> intersectsSphere(glm::vec3 position,
                                              float size) const {
        static const std::array<std::array<int, 3>, 6> faces(
            {std::array{0, 1, 3}, std::array{0, 1, 4}, std::array{0, 3, 4},
             std::array{6, 7, 5}, std::array{6, 7, 2}, std::array{6, 5, 2}});

        float minPenetration = std::numeric_limits<float>::max();
        glm::vec3 mtvAxis;

        for (std::array<int, 3> face : faces) {
            glm::vec3 normal = glm::normalize(
                glm::cross(this->corners[face[1]] - this->corners[face[0]],
                           this->corners[face[2]] - this->corners[face[0]]));

            if (glm::dot(normal, center - corners[face[0]]) > 0) {
                normal = -normal;
            }

            glm::vec3 v = position - this->corners[face[0]];
            float d = glm::dot(v, normal) - size;

            if (d > 0) {
                return std::nullopt;
            }

            if (std::abs(d) < minPenetration) {
                minPenetration = std::abs(d);
                mtvAxis = normal;
            }
        }

        return mtvAxis * minPenetration;
    }

    void toggle(GlobalScene &scene) { scene.toggleNode(this->id); }
};

#endif
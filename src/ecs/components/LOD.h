#ifndef LOD_
#define LOD_

#include "src/GlobalScene.h"
#include "src/Mesh.h"
#include <glm/ext.hpp>
#include <unordered_map>
#include <vector>

using uint = unsigned int;

enum class LODLevel { HIGH, MEDIUM, LOW, NONE };

struct LOD {
    NodeId id;

    std::vector<glm::vec3> original_vertices;
    std::vector<uint> original_indices;
    std::vector<glm::vec3> original_normals;
    std::vector<glm::vec2> original_uvs;

    std::vector<glm::vec3> current_vertices;
    std::vector<uint> current_indices;
    std::vector<glm::vec3> current_normals;
    std::vector<glm::vec2> current_uvs;

    LODLevel current_level = LODLevel::NONE;

    LOD() {}

    LOD(const Mesh &mesh, NodeId id) : id(id) {
        original_vertices = mesh.getVertices();
        original_indices = mesh.getIndices();
        original_normals = mesh.getNormals();
        original_uvs = mesh.getUvs();

        current_vertices = original_vertices;
        current_indices = original_indices;
        current_normals = original_normals;
        current_uvs = original_uvs;
    }

    static LODLevel levelFromScreenDiameterPx(float diameterPx,
                                              float screenHeightPx) {
        float highThreshold = 0.35f * screenHeightPx;
        float mediumThreshold = 0.15f * screenHeightPx;

        if (diameterPx >= highThreshold)
            return LODLevel::HIGH;
        if (diameterPx >= mediumThreshold)
            return LODLevel::MEDIUM;
        return LODLevel::LOW;
    }

    using grid_c = std::tuple<int, int, int>;

    void applyLOD(float resolution) {

        auto cp_indices = original_indices;

        std::map<grid_c, std::vector<size_t>> new_vertices_map;

        for (size_t i = 0; i < original_vertices.size(); i++) {
            glm::vec3 current = original_vertices[i];
            grid_c current_key(floor(current[0] / resolution),
                               floor(current[1] / resolution),
                               floor(current[2] / resolution));

            new_vertices_map[current_key].push_back(i);
        }

        std::unordered_map<uint, size_t> old_to_new;
        std::vector<glm::vec3> new_vertices;
        std::vector<glm::vec3> new_normals;
        std::vector<glm::vec2> new_uvs;

        bool hasNormals = !original_normals.empty();
        bool hasUVs = !original_uvs.empty();

        size_t i = 0;
        for (auto const &[key, val] : new_vertices_map) {

            glm::vec3 avg;
            glm::vec3 normal;
            glm::vec2 uv;

            for (size_t old : val) {
                old_to_new[old] = i;
                avg += original_vertices[old];

                if (hasNormals) {
                    normal += original_normals[old];
                }
                if (hasUVs) {
                    uv += original_uvs[old];
                }
            }

            avg /= val.size();
            new_vertices.push_back(avg);

            if (hasNormals) {
                normal /= val.size();
                new_normals.push_back(normal);
            }

            if (hasUVs) {
                uv /= val.size();
                new_uvs.push_back(uv);
            }

            i++;
        }

        for (uint &indice : cp_indices) {
            indice = old_to_new[indice];
        }

        this->current_vertices = new_vertices;
        this->current_indices = cp_indices;
        this->current_normals = new_normals;
        this->current_uvs = new_uvs;
    }
};

#endif
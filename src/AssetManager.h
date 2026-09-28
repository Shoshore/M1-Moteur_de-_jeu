#ifndef ASSET_MANAGER
#define ASSET_MANAGER

#include <cstddef>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/ext.hpp>

#include "src/Mesh.h"
#include "src/Texture.h"

class AssetManager {

    std::vector<Texture> textures;
    std::vector<std::shared_ptr<Mesh>> meshes;

    std::unordered_map<std::string, std::size_t> texture_map;
    std::unordered_map<std::string, std::size_t> mesh_map;

  private:
    AssetManager() {}

    static AssetManager &getAssetManager() {
        static AssetManager instance;
        return instance;
    }

    static std::optional<Mesh> buildMeshFromOFF(const char *offFile) {
        std::ifstream myfile;
        myfile.open(offFile);

        if (!myfile.is_open()) {
            std::cout << offFile << " cannot be opened" << std::endl;
            return std::nullopt;
        }

        std::string magic_s;
        myfile >> magic_s;

        if (magic_s != "OFF") {
            std::cout << magic_s << " != OFF : Not a true OFF file"
                      << std::endl;
            myfile.close();
            return std::nullopt;
        }

        int n_vertices, n_faces, dummy_int;
        myfile >> n_vertices >> n_faces >> dummy_int;

        std::string first_line;
        std::getline(myfile, first_line);
        std::getline(myfile, first_line);

        int float_count = 0;
        std::istringstream iss(first_line);
        float tmp;
        while (iss >> tmp)
            float_count++;

        bool has_normals = (float_count == 6);

        myfile.seekg(0);
        myfile >> magic_s >> n_vertices >> n_faces >> dummy_int;

        std::vector<glm::vec3> o_vertices;
        std::vector<glm::vec3> o_normals;

        for (int v = 0; v < n_vertices; ++v) {
            float x, y, z;
            myfile >> x >> y >> z;
            o_vertices.push_back(glm::vec3(x, y, z));

            if (has_normals) {
                myfile >> x >> y >> z;
                o_normals.push_back(glm::vec3(x, y, z));
            }
        }

        std::vector<uint> o_indices;
        for (int f = 0; f < n_faces; ++f) {
            int n_verts_in_face;
            myfile >> n_verts_in_face;
            std::vector<uint> face_verts(n_verts_in_face);
            for (int i = 0; i < n_verts_in_face; ++i)
                myfile >> face_verts[i];
            for (int i = 1; i < n_verts_in_face - 1; ++i) {
                o_indices.push_back(face_verts[0]);
                o_indices.push_back(face_verts[i]);
                o_indices.push_back(face_verts[i + 1]);
            }
        }

        myfile.close();

        if (has_normals) {
            return Mesh(o_vertices, o_indices, o_normals);
        } else {
            return Mesh(o_vertices, o_indices);
        }
    }

    static std::optional<Mesh> buildMeshFromOBJ(const char *objFile) {
        printf("Loading OBJ file %s...\n", objFile);

        struct VertexKey {
            unsigned int v, vt, vn;
            bool operator<(const VertexKey &other) const {
                return std::tie(v, vt, vn) <
                       std::tie(other.v, other.vt, other.vn);
            }
        };

        std::vector<glm::vec3> temp_vertices;
        std::vector<glm::vec2> temp_uvs;
        std::vector<glm::vec3> temp_normals;

        std::vector<VertexKey> face_keys;

        FILE *file = fopen(objFile, "r");
        if (file == NULL) {
            printf("Impossible to open the file!\n");
            return std::nullopt;
        }

        while (true) {
            char lineHeader[128];
            int res = fscanf(file, "%s", lineHeader);
            if (res == EOF)
                break;

            if (strcmp(lineHeader, "v") == 0) {
                glm::vec3 vertex;
                fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
                temp_vertices.push_back(vertex);
            } else if (strcmp(lineHeader, "vt") == 0) {
                glm::vec2 uv;
                fscanf(file, "%f %f\n", &uv.x, &uv.y);
                uv.y = -uv.y;
                temp_uvs.push_back(uv);
            } else if (strcmp(lineHeader, "vn") == 0) {
                glm::vec3 normal;
                fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
                temp_normals.push_back(normal);
            } else if (strcmp(lineHeader, "f") == 0) {
                unsigned int v[3], vt[3], vn[3];
                int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
                                     &v[0], &vt[0], &vn[0], &v[1], &vt[1],
                                     &vn[1], &v[2], &vt[2], &vn[2]);
                if (matches != 9) {
                    printf("File can't be read by our simple parser :-(\n");
                    fclose(file);
                    return std::nullopt;
                }
                for (int i = 0; i < 3; ++i) {
                    face_keys.push_back(
                        VertexKey{v[i] - 1, vt[i] - 1, vn[i] - 1});
                }
            } else {
                char buffer[1000];
                fgets(buffer, 1000, file);
            }
        }

        fclose(file);

        if (!temp_vertices.empty()) {
            glm::vec3 centroid(0.0f);
            for (const auto &v : temp_vertices)
                centroid += v;
            centroid /= static_cast<float>(temp_vertices.size());
            for (auto &v : temp_vertices)
                v -= centroid;
        }

        std::vector<glm::vec3> unique_vertices;
        std::vector<glm::vec3> unique_normals;
        std::vector<glm::vec2> unique_uvs;
        std::map<VertexKey, unsigned int> vertex_map;
        std::vector<unsigned int> indices;

        for (const auto &key : face_keys) {
            auto it = vertex_map.find(key);
            if (it == vertex_map.end()) {
                unique_vertices.push_back(temp_vertices[key.v]);
                unique_uvs.push_back(key.vt < temp_uvs.size()
                                         ? temp_uvs[key.vt]
                                         : glm::vec2(0.0f));
                unique_normals.push_back(key.vn < temp_normals.size()
                                             ? temp_normals[key.vn]
                                             : glm::vec3(0.0f, 1.0f, 0.0f));
                unsigned int idx = unique_vertices.size() - 1;
                vertex_map[key] = idx;
                indices.push_back(idx);
            } else {
                indices.push_back(it->second);
            }
        }

        std::vector<uint> uint_indices(indices.begin(), indices.end());

        return Mesh(unique_vertices, uint_indices, unique_normals, unique_uvs);
    }

  public:
    static std::optional<std::shared_ptr<Mesh>> loadMesh(const char *filepath) {
        AssetManager &manager = AssetManager::getAssetManager();

        std::string name(filepath);
        if (manager.mesh_map.count(name) == 0) {
            manager.mesh_map[name] = manager.meshes.size();

            std::optional<Mesh> mesh = AssetManager::buildMeshFromOBJ(filepath);

            if (mesh.has_value()) {
                manager.meshes.push_back(std::make_shared<Mesh>(mesh.value()));
            } else {
                std::optional<Mesh> mesh =
                    AssetManager::buildMeshFromOFF(filepath);

                if (mesh.has_value()) {
                    manager.meshes.push_back(
                        std::make_shared<Mesh>(mesh.value()));
                } else {
                    return std::nullopt;
                }
            }
        }

        return manager.meshes[manager.mesh_map[name]];
    }

    static Texture &loadTexture(const char *filepath) {
        AssetManager &manager = AssetManager::getAssetManager();

        std::string name(filepath);

        if (manager.texture_map.count(name) == 0) {
            manager.texture_map[name] = manager.textures.size();

            Texture texture(filepath);

            manager.textures.push_back(texture);
        }

        return manager.textures[manager.texture_map[name]];
    }
};

#endif // ASSET_MANAGER
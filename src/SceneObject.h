#ifndef SCENE_OBJECT
#define SCENE_OBJECT

#include <string>
#include <utility>
#include <vector>

#include <GL/glew.h>
#include <glm/ext.hpp>

#include "Camera.h"
#include "Light.hpp"
#include "Mesh.h"
#include "Observer.h"
#include "Texture.h"
#include "Transform.h"

using uint = unsigned int;

class SceneObject : public Subject<glm::vec3> {
    GLuint programId;

    Mesh mesh;

    glm::vec3 albedo = {0, 0, 0};
    float metallic = 0.f;
    float roughness = 0.f;
    float ao = 1.f;

    bool useTexture = false;
    bool useAlbedoMap = false;
    bool useNormalMap = false;
    bool useMetallicMap = false;
    bool useRoughnessMap = false;
    bool useAoMap = false;
    std::vector<std::pair<std::string, Texture>> textures;

    Transform transform;

  public:
    SceneObject(const char *vertexShaderPath, const char *fragmentShaderPath,
                const Mesh &mesh);

    GLuint getId() const;
    Mesh &getMesh();

    const glm::vec3 &getAlbedo() const;
    float getMetallic() const;
    float getRoughness() const;
    float getAo() const;

    void setAlbedo(const glm::vec3 &value);
    void setMetallic(float value);
    void setRoughness(float value);
    void setAo(float value);

    bool addTexture(const Texture &texture, const char *varName);
    bool addAlbedoMap(const Texture &texture);
    bool addNormalMap(const Texture &texture);
    bool addMetallicMap(const Texture &texture);
    bool addRoughnessMap(const Texture &texture);
    bool addAoMap(const Texture &texture);

    void draw(const Camera &camera, const std::vector<Light> &lights) const;
    void cleanUp();

    Transform getTransform() const;
    void setTransform(const Transform &transform);

    void updateMeshData(const std::vector<glm::vec3> &vertices,
                        const std::vector<uint> &indices,
                        const std::vector<glm::vec3> &normals,
                        const std::vector<glm::vec2> &uvs);
};

#endif // SCENE_OBJECT
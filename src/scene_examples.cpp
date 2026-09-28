#ifndef SCENE_EXAMPLES
#define SCENE_EXAMPLES

#include "GlobalScene.h"
#include "src/AssetManager.h"
#include "src/SceneObject.h"
#include "src/Texture.h"
#include "src/Transform.h"
#include <cstddef>

GlobalScene getPBRbenchmarkScene(size_t nbcarre) {
    GlobalScene scene_result;
    std::optional<Mesh *> sphereMesh =
        AssetManager::loadMesh("assets/meshes/sphere.obj");
    if (sphereMesh.has_value()) {
        for (size_t i = 0; i < nbcarre; i++) {
            for (size_t j = 0; j < nbcarre; j++) {

                SceneObject sphere("shaders/PBR_vs.glsl", "shaders/PBR_fs.glsl",
                                   *sphereMesh.value());

                sphere.setAlbedo({1., 0., 0.});
                sphere.setMetallic((float)j / (nbcarre - 1));
                sphere.setRoughness((float)i / (nbcarre - 1) + 0.01);

                NodeId id = scene_result.addMesh(sphere);
                scene_result.setTransform(
                    id, translate(i * 0.22, j * 0.22, 0).scale(0.1));
            }
        }
    }
    float intensity = 100.0f;

    scene_result.addLightToScene(
        Light(glm::vec3(-5.0f, 0.0f, 0.0f), glm::vec3(intensity)));

    scene_result.addLightToScene(
        Light(glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(intensity)));

    scene_result.addLightToScene(
        Light(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(intensity)));

    scene_result.addLightToScene(
        Light(glm::vec3(0.0f, 0.0f, -5.0f), glm::vec3(intensity)));

    return scene_result;
}

GlobalScene getTexturedSphereScene() {
    GlobalScene scene_result;

    std::optional<Mesh *> sphereMesh =
        AssetManager::loadMesh("assets/meshes/sphere.obj");
    if (sphereMesh.has_value()) {
        Texture rustedAlbedoMap =
            AssetManager::loadTexture("assets/textures/rustediron2_albedo.png");
        Texture rustedNormalMap =
            AssetManager::loadTexture("assets/textures/rustediron2_normal.png");
        Texture rustedMetallicMap = AssetManager::loadTexture(
            "assets/textures/rustediron2_metallic.png");
        Texture rustedRoughnessMap = AssetManager::loadTexture(
            "assets/textures/rustediron2_roughness.png");

        SceneObject sphere("shaders/PBR_vs.glsl", "shaders/PBR_fs.glsl",
                           *sphereMesh.value());

        sphere.addAlbedoMap(rustedAlbedoMap);
        sphere.addNormalMap(rustedNormalMap);
        sphere.addMetallicMap(rustedMetallicMap);
        sphere.addRoughnessMap(rustedRoughnessMap);

        scene_result.addMesh(sphere);

        Texture woodAlbedoMap("assets/textures/fancy-carved-wood_albedo.png");
        Texture woodNormalMap("assets/textures/fancy-carved-wood_normal.png");
        Texture woodMetallicMap(
            "assets/textures/fancy-carved-wood_metallic.png");
        Texture woodRoughnessMap(
            "assets/textures/fancy-carved-wood_roughness.png");
        Texture woodAoMap("assets/textures/fancy-carved-wood_ao.png");

        SceneObject sphere2("shaders/PBR_vs.glsl", "shaders/PBR_fs.glsl",
                            *sphereMesh.value());

        sphere2.addAlbedoMap(woodAlbedoMap);
        sphere2.addNormalMap(woodNormalMap);
        sphere2.addMetallicMap(woodMetallicMap);
        sphere2.addRoughnessMap(woodRoughnessMap);
        sphere2.addAoMap(woodAoMap);

        NodeId id = scene_result.addMesh(sphere2);
        scene_result.transform(id, translate(2, 0, 0));

        float intensity = 100.0f;

        scene_result.addLightToScene(
            Light(glm::vec3(-5.0f, 0.0f, 0.0f), glm::vec3(intensity)));

        scene_result.addLightToScene(
            Light(glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(intensity)));

        scene_result.addLightToScene(
            Light(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(intensity)));

        scene_result.addLightToScene(
            Light(glm::vec3(0.0f, 0.0f, -5.0f), glm::vec3(intensity)));
    }

    return scene_result;
}

#endif
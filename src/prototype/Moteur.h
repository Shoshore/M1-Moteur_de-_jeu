#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <optional>

#include "glm/detail/func_geometric.hpp"
#include "glm/detail/type_vec.hpp"
#include "src/AssetManager.h"
#include "src/Controls.h"
#include "src/GameEngine.h"
#include "src/Hud.h"
#include "src/VelocityIndicatorHud.h"
#include "src/ecs/ECSManager.h"
#include "src/ecs/components/Attracted.h"
#include "src/ecs/components/Collectible.h"
#include "src/ecs/components/Inventory.h"
#include "src/ecs/components/Positionable.h"
#include "src/ecs/components/VerletBody.h"
#include "src/ecs/systems/CollectibleSystem.h"
#include "src/ecs/systems/ContinuousLOD.h"
#include "src/ecs/systems/Gravity.h"
#include "src/ecs/systems/TransformPosition.h"
#include "src/ecs/systems/Verlet.h"
#include "src/ecs/utils.h"
#include "src/prototype/StellarSystem.h"

class Moteur : public GameEngine {

    EntityId characterEntityId;
    NodeId characterNodeId;
    NodeId exteriorNodeId;

    std::optional<std::shared_ptr<StellarSystem>> stellarSystem;
    std::optional<std::shared_ptr<Verlet>> verletSystem;

    bool movementFloor = false;

    Hud hud;
    int totalCollectibles = 0;
    VelocityIndicatorHud velHud;

    Hud pressXHud;

    void init() override {
        this->initSystems();
        this->initScene();

        this->getSystemUpdater()->addSystem(std::make_shared<CollectibleSystem>(
            this->getScene(), this->characterEntityId));

        this->initInputs();
        this->getCamera().setTargetDistance(10);
        this->hud.init();
        this->pressXHud.init("shaders/hud_vs.glsl", "shaders/hud_fs.glsl",
                             "assets/textures/press_x.png");
        this->velHud.init();
    }

    void initSystems() {
        this->verletSystem = std::make_shared<Verlet>();
        this->getSystemUpdater()->addSystem(this->verletSystem.value());

        this->getSystemUpdater()->addSystem(
            std::make_shared<TransformPosition>(this->getScene()));

        this->getSystemUpdater()->addSystem(std::make_shared<Gravity>());

        this->getSystemUpdater()->addSystem(std::make_shared<ContinuousLOD>(
            this->getScene(), this->getCamera()));

        this->stellarSystem =
            std::make_shared<StellarSystem>(this->getScene(), this->getPVS());
    }

    void initScene() {
        this->getScene().addLightToScene(
            Light(glm::vec3(0, 0, 0), glm::vec3(1000000)));

        std::shared_ptr<Mesh> ballMesh =
            AssetManager::loadMesh("assets/meshes/big_sphere.obj").value();

        SceneObject character = SceneObject("shaders/PBR_vs.glsl",
                                            "shaders/PBR_fs.glsl", *ballMesh);

        character.addAlbedoMap(
            AssetManager::loadTexture("assets/textures/character.jpg"));
        character.setMetallic(0.5);
        character.setRoughness(0.4);

        this->characterEntityId = ECSManager::generateEntityId();
        ECSManager::setComponentToEntity(
            Noded(this->getScene().addMesh(character)), characterEntityId);
        glm::vec3 pos = {1000, 400, 0};
        ECSManager::setComponentToEntity(Positionable(pos), characterEntityId);
        ECSManager::setComponentToEntity(VerletBody(pos, {0, 0, 0}, 1, 0),
                                         characterEntityId);
        ECSManager::setComponentToEntity(Inventory{}, characterEntityId);
        this->stellarSystem.value()->setSystemAttraction(
            this->characterEntityId);

        // Cuboid cuboid(
        //     {0.2, 0.8, 0.2},
        //     std::array{glm::vec3{1000, 390, 0}, glm::vec3{1005, 390, 0},
        //                glm::vec3{1005, 390, 5}, glm::vec3{1000, 390, 5},
        //                glm::vec3{1000, 395, 0}, glm::vec3{1000, 395, 5},
        //                glm::vec3{1005, 395, 5}, glm::vec3{1005, 395, 0}});

        // this->getScene().addMesh(cuboid.getSceneObject().value());

        // this->verletSystem.value()->addCuboid(cuboid);

        ////////////////////////////////////////Collectible//////////////////////////////////

        // this->totalCollectibles = 1;

        // auto starMeshOpt = AssetManager::loadMesh("assets/meshes/star.obj");
        // if (!starMeshOpt.has_value()) {
        //     return;
        // }

        // SceneObject starObject("shaders/PBR_vs.glsl", "shaders/PBR_fs.glsl",
        //                        *starMeshOpt.value());

        // starObject.setAlbedo(glm::vec3(1.0f, 0.9f, 0.2f));
        // starObject.setMetallic(0.0f);
        // starObject.setRoughness(0.6f);

        // NodeId collectibleNodeId = this->getScene().addMesh(starObject);

        // EntityId collectibleEntityId = ECSManager::generateEntityId();

        // glm::vec3 collectiblePosition = glm::vec3(205.0f, 200.0f, 200.0f);

        // float collectibleRadius = 1.0f;

        // ECSManager::setComponentToEntity(Noded(collectibleNodeId),
        //                                  collectibleEntityId);
        // ECSManager::setComponentToEntity(Positionable(collectiblePosition),
        //                                  collectibleEntityId);

        // ECSManager::setComponentToEntity(VerletBody(collectiblePosition,
        //                                             glm::vec3(0.0f),
        //                                             collectibleRadius, true),
        //                                  collectibleEntityId);
        // ECSManager::getComponentOfEntity<VerletBody>(collectibleEntityId)
        //     .value()
        //     .get()
        //     .isTrigger = true;
        // ECSManager::setComponentToEntity(Collectible(1),
        // collectibleEntityId);

        // Le total est compté par initEarthInterior() selon les spawns réels.
        this->totalCollectibles = 0;

        if (this->stellarSystem.has_value() &&
            this->stellarSystem.value() != nullptr &&
            this->verletSystem.has_value()) {
            this->stellarSystem.value()->initEarthInterior(
                *this->verletSystem.value(), this->totalCollectibles);
        }

        this->exteriorNodeId = this->getPVS().addScene();
        this->getPVS().init(this->exteriorNodeId);
    }

    void initInputs() {
        Controls &controls = this->getControls();

        controls.addKeyPressedCallback(
            GLFW_KEY_ESCAPE,
            new KeyCallback([this](float deltaTime) { this->stopRunning(); }));

        controls.addMouseDeltaCallback(
            new MouseMoveCallback([this](float dx, float dy) {
                this->getCamera().rotateWithMouse(dx, dy);
            }));

        controls.addKeyDownCallback(
            GLFW_KEY_W, new KeyCallback([this](float deltaTime) {
                VerletBody &body = ECSManager::getComponentOfEntity<VerletBody>(
                                       this->characterEntityId)
                                       .value();

                glm::vec3 front = this->getCamera().getFront();
                if (this->movementFloor) {
                    front = front - glm::dot(front, glm::vec3{0, 1, 0}) *
                                        glm::vec3{0, 1, 0};
                    front = glm::normalize(front);
                }
                body.acceleration += front * 10;
            }));

        controls.addKeyDownCallback(
            GLFW_KEY_A, new KeyCallback([this](float deltaTime) {
                VerletBody &body = ECSManager::getComponentOfEntity<VerletBody>(
                                       this->characterEntityId)
                                       .value();

                glm::vec3 front = this->getCamera().getFront();
                if (this->movementFloor) {
                    front = front - glm::dot(front, glm::vec3{0, 1, 0}) *
                                        glm::vec3{0, 1, 0};
                    front = glm::normalize(front);
                    front =
                        glm::normalize(glm::cross(front, glm::vec3{0, 1, 0}));
                }
                body.acceleration -= front * 10;
            }));

        controls.addKeyDownCallback(
            GLFW_KEY_S, new KeyCallback([this](float deltaTime) {
                VerletBody &body = ECSManager::getComponentOfEntity<VerletBody>(
                                       this->characterEntityId)
                                       .value();

                glm::vec3 front = this->getCamera().getFront();
                if (this->movementFloor) {
                    front = front - glm::dot(front, glm::vec3{0, 1, 0}) *
                                        glm::vec3{0, 1, 0};
                    front = glm::normalize(front);
                }
                body.acceleration -= front * 10;
            }));

        controls.addKeyDownCallback(
            GLFW_KEY_D, new KeyCallback([this](float deltaTime) {
                VerletBody &body = ECSManager::getComponentOfEntity<VerletBody>(
                                       this->characterEntityId)
                                       .value();

                glm::vec3 front = this->getCamera().getFront();
                if (this->movementFloor) {
                    front = front - glm::dot(front, glm::vec3{0, 1, 0}) *
                                        glm::vec3{0, 1, 0};
                    front = glm::normalize(front);
                    front =
                        glm::normalize(glm::cross(front, glm::vec3{0, 1, 0}));
                }
                body.acceleration += front * 10;
            }));

        controls.addKeyPressedCallback(
            GLFW_KEY_SPACE, new KeyCallback([this](float deltaTime) {
                glm::vec3 &pos = ECSManager::getComponentOfEntity<Positionable>(
                                     this->characterEntityId)
                                     .value()
                                     .get()
                                     .pos;
                VerletBody &body = ECSManager::getComponentOfEntity<VerletBody>(
                                       this->characterEntityId)
                                       .value();

                body.acceleration +=
                    this->getCamera().getUp() * 500 * body.size;
            }));

        controls.addKeyDownCallback(
            GLFW_KEY_Q, new KeyCallback([this](float deltaTime) {
                this->getCamera().tilt(deltaTime, false);
            }));

        controls.addKeyDownCallback(GLFW_KEY_E,
                                    new KeyCallback([this](float deltaTime) {
                                        this->getCamera().tilt(deltaTime, true);
                                    }));

        controls.addKeyDownCallback(GLFW_KEY_KP_ADD,
                                    new KeyCallback([this](float deltaTime) {
                                        this->getCamera().getCloser(deltaTime);
                                    }));

        controls.addKeyDownCallback(GLFW_KEY_KP_SUBTRACT,
                                    new KeyCallback([this](float deltaTime) {
                                        this->getCamera().getFurther(deltaTime);
                                    }));

        controls.addKeyPressedCallback(
            GLFW_KEY_T, new KeyCallback([this](float deltaTime) {
                glm::vec3 &pos = ECSManager::getComponentOfEntity<Positionable>(
                                     this->characterEntityId)
                                     .value()
                                     .get()
                                     .pos;
                VerletBody &body = ECSManager::getComponentOfEntity<VerletBody>(
                                       this->characterEntityId)
                                       .value();

                pos = {1000, 400, 0};
                body.last_position = pos;
                body.acceleration = {};
            }));

        controls.addKeyPressedCallback(
            GLFW_KEY_P, new KeyCallback([this](float deltaTime) {
                this->stellarSystem.value()->toggleWalls();
            }));

        controls.addKeyPressedCallback(GLFW_KEY_O,
                                       new KeyCallback([this](float deltaTime) {
                                           this->stellarSystem.value()->next();
                                       }));

        controls.addKeyPressedCallback(
            GLFW_KEY_X, new KeyCallback([this](float deltaTime) {
                if (!this->stellarSystem.has_value() ||
                    this->stellarSystem.value() == nullptr)
                    return;

                auto posOpt = ECSManager::getComponentOfEntity<Positionable>(
                    this->characterEntityId);
                auto bodyOpt = ECSManager::getComponentOfEntity<VerletBody>(
                    this->characterEntityId);
                auto attOpt = ECSManager::getComponentOfEntity<Attracted>(
                    this->characterEntityId);

                if (!posOpt.has_value() || !bodyOpt.has_value() ||
                    !attOpt.has_value())
                    return;

                Positionable &p = posOpt.value().get();
                VerletBody &b = bodyOpt.value();
                Attracted &att = attOpt.value();

                const glm::vec3 playerPos = p.pos;

                glm::vec3 pipePos(0.0f);
                const float triggerDist = 6.0f;
                if (!this->stellarSystem.value()->getNearestPipePos(
                        playerPos, triggerDist, pipePos))
                    return;

                Positionable planet =
                    ECSManager::getComponentOfEntity<Positionable>(
                        this->stellarSystem.value()->getPlanetId())
                        .value();

                glm::vec3 newPos = planet.pos + vec3(20, -0, 20);
                p.pos = newPos;
                b.last_position = newPos;
                b.acceleration = {};

                static bool isInside = false;
                isInside = !isInside;

                this->movementFloor = !this->movementFloor;

                att.clear();
                if (isInside) {
                    att.addDirectionAttraction(glm::vec3(0, -1, 0), 20.0f);
                    this->stellarSystem.value()->enter();
                } else {
                    this->getPVS().enterScene(this->exteriorNodeId);
                    this->stellarSystem.value()->setSystemAttraction(
                        this->characterEntityId);
                }
            }));
    }

    void processInput(float deltaTime) override {}

    void update(float deltaTime) override {
        if (this->stellarSystem.has_value()) {
            this->stellarSystem.value()->update(
                deltaTime, ECSManager::getComponentOfEntity<Positionable>(
                               this->characterEntityId)
                               .value()
                               .get()
                               .pos);
        }
    }

    void render(float deltaTime) override {
        std::optional<Positionable> pos =
            ECSManager::getComponentOfEntity<Positionable>(
                this->characterEntityId);

        if (pos.has_value()) {
            this->getCamera().setTarget(pos.value().pos);
        }

        hud.beginFrame((int)this->getCamera().getScreenWidth(),
                       (int)this->getCamera().getScreenHeight());

        int marginX = 40;
        int marginY = 40;
        int iconSize = 64;
        int spacing = 6;
        int maxWidth = (int)this->getCamera().getScreenWidth() - 2 * marginX;

        int collectedCount = 0;
        auto inventoryOpt = ECSManager::getComponentOfEntity<Inventory>(
            this->characterEntityId);

        if (inventoryOpt.has_value()) {
            collectedCount = inventoryOpt.value().get().collected;
        }

        hud.draw(collectedCount, totalCollectibles, marginX, marginY, iconSize,
                 spacing, maxWidth);

        hud.endFrame();

        bool nearPipe = false;

        auto posOpt = ECSManager::getComponentOfEntity<Positionable>(
            this->characterEntityId);

        if (posOpt.has_value() && this->stellarSystem.has_value() &&
            this->stellarSystem.value() != nullptr) {

            const glm::vec3 playerPos = posOpt.value().get().pos;
            nearPipe = this->stellarSystem.value()->isNearPipe(playerPos, 6.0f);
        }

        if (nearPipe) {
            const int screenW = (int)this->getCamera().getScreenWidth();
            const int screenH = (int)this->getCamera().getScreenHeight();

            const int sizePx = 96;
            const int marginPx = 20;

            const int x = screenW - marginPx - sizePx;
            const int y = screenH - marginPx - sizePx;

            pressXHud.beginFrame(screenW, screenH);
            pressXHud.draw(1, 1, x, y, sizePx, 0, sizePx);
            pressXHud.endFrame();
        }
        velHud.render(this->getCamera(), this->characterEntityId, deltaTime);
    }

    void cleanUp() override {
        hud.cleanup();
        pressXHud.cleanup();
    }

  public:
    Moteur(GLFWwindow *window, uint width, uint height)
        : GameEngine(window, width, height) {}
};
#include <cmath>
#include <memory>
#include <optional>

#include "glm/detail/func_geometric.hpp"
#include "glm/detail/type_vec.hpp"
#include <glm/ext.hpp>

#include "src/AssetManager.h"
#include "src/GlobalScene.h"
#include "src/PVS.h"
#include "src/Transform.h"
#include "src/ecs/ECSManager.h"
#include "src/ecs/components/Attracted.h"
#include "src/ecs/components/Noded.h"
#include "src/ecs/components/Positionable.h"
#include "src/ecs/components/VerletBody.h"
#include "src/ecs/utils.h"
#include <cstdlib>

#include "src/ecs/components/Collectible.h"
#include "src/ecs/systems/Verlet.h"
#include "src/prototype/Cuboid.h"

class StellarSystem {
    PVS &pvs;
    GlobalScene &scene;
    NodeId stellarSystem;

    NodeId starParent;
    NodeId star;
    NodeId starPipe;
    EntityId starEntityId;

    NodeId planetParent;
    NodeId planet;
    NodeId planetPipe;
    EntityId planetEntityId;

    NodeId moonParent;
    NodeId moon;
    NodeId moonPipe;
    EntityId moonEntityId;

    const float starMass = 500000;
    const float starSize = 150;

    const float planetMass = 100000;
    const float planetSize = 60;

    const float moonMass = 50000;
    const float moonSize = 25;

    const float speed = 1;

    const float starRotationRatio = 0.2;

    const float planetRevolutionRatio = 1;
    const float moonRevolutionRatio = 12.37;

    const float planetRotationRatio = 365;
    const float moonRotationRatio = 12.37;

    float starRotationAngle = 0;

    float planetRevolutionAngle = 0;
    float planetRotationAngle = 0;

    float moonRevolutionAngle = 0;
    float moonRotationAngle = 0;

    std::array<NodeId, 4> roomId;
    NodeId wallsId;
    bool hiddenWalls;

    void registerCelestialObject(const char *texturePath, NodeId &parent,
                                 NodeId &nodeParent, NodeId &node, NodeId &pipe,
                                 EntityId &entity, float size) {

        std::shared_ptr<Mesh> ballMesh =
            AssetManager::loadMesh("assets/meshes/big_sphere.obj").value();

        SceneObject celestialObject = SceneObject(
            "shaders/PBR_vs.glsl", "shaders/PBR_fs.glsl", *ballMesh);

        celestialObject.addAlbedoMap(AssetManager::loadTexture(texturePath));
        celestialObject.setMetallic(0.5);
        celestialObject.setRoughness(0.4);

        nodeParent = scene.addBasicNodeAsChild(parent).value();
        node = scene.addMeshAsChild(nodeParent, celestialObject).value();
        entity = ECSManager::generateEntityId();
        ECSManager::setComponentToEntity(Noded(node), entity);

        glm::vec3 pos = {0, 0, 0};

        ECSManager::setComponentToEntity(Positionable(pos, false), entity);
        ECSManager::setComponentToEntity(VerletBody(pos, {}, size, true),
                                         entity);

        std::shared_ptr<Mesh> pipeMesh =
            AssetManager::loadMesh("assets/meshes/pipe.obj").value();
        SceneObject pipeObject = SceneObject("shaders/PBR_vs.glsl",
                                             "shaders/PBR_fs.glsl", *pipeMesh);

        pipeObject.setAlbedo({0, 0.8, 0});
        pipeObject.setMetallic(0.3);
        pipeObject.setRoughness(0.8);

        pipe = scene.addMeshAsChild(node, pipeObject).value();
        EntityId pipeId = ECSManager::generateEntityId();
        ECSManager::setComponentToEntity(Noded(pipe), pipeId);
    }

    struct EarthInteriorContext {
        glm::vec3 earthCenter;
        float floorY;
        float halfSize;
        NodeId interiorRoot;
        NodeId spawnRoot;
        NodeId ballPitRoot;
        NodeId pbrRoot;
        NodeId collectRoot;
    };

    std::array<glm::vec3, 8> cornersFromAabb(const glm::vec3 &mn,
                                             const glm::vec3 &mx) {
        return std::array<glm::vec3, 8>{
            glm::vec3(mn.x, mn.y, mn.z), glm::vec3(mx.x, mn.y, mn.z),
            glm::vec3(mx.x, mn.y, mx.z), glm::vec3(mn.x, mn.y, mx.z),
            glm::vec3(mn.x, mx.y, mn.z), glm::vec3(mn.x, mx.y, mx.z),
            glm::vec3(mx.x, mx.y, mx.z), glm::vec3(mx.x, mx.y, mn.z)};
    }

    void addStaticColliderCuboid(Verlet &verletSystem, NodeId parent,
                                 const glm::vec3 &color, const glm::vec3 &mn,
                                 const glm::vec3 &mx) {
        Cuboid c(color, cornersFromAabb(mn, mx));
        auto soOpt = c.getSceneObject();
        if (soOpt.has_value()) {
            c.id = scene.addMeshAsChild(parent, soOpt.value()).value();
        }
        verletSystem.addCuboid(c);
    }

    EarthInteriorContext buildEarthInteriorBase(Verlet &verletSystem) {
        EarthInteriorContext ctx{};

        this->update(0.0f, glm::vec3(0.0f));
        auto earthT = scene.getTransform(this->planet);
        if (!earthT.has_value())
            return ctx;

        ctx.earthCenter = earthT.value().getPosition();
        ctx.floorY = ctx.earthCenter.y - 20.0f;
        ctx.halfSize = 58;

        ctx.interiorRoot = pvs.addScene();
        ctx.spawnRoot = pvs.addScene();
        this->roomId[0] = ctx.spawnRoot;
        ctx.pbrRoot = pvs.addScene();
        this->roomId[1] = ctx.pbrRoot;
        ctx.ballPitRoot = pvs.addScene();
        this->roomId[2] = ctx.ballPitRoot;
        ctx.collectRoot = pvs.addScene();
        this->roomId[3] = ctx.collectRoot;

        pvs.linkScenes(ctx.spawnRoot, ctx.pbrRoot);
        pvs.linkScenes(ctx.pbrRoot, ctx.ballPitRoot);
        pvs.linkScenes(ctx.ballPitRoot, ctx.collectRoot);
        pvs.linkScenes(ctx.collectRoot, ctx.spawnRoot);

        pvs.linkScenes(ctx.spawnRoot, ctx.interiorRoot);
        pvs.linkScenes(ctx.pbrRoot, ctx.interiorRoot);
        pvs.linkScenes(ctx.ballPitRoot, ctx.interiorRoot);
        pvs.linkScenes(ctx.collectRoot, ctx.interiorRoot);

        const float floorThickness = 2.0f;
        addStaticColliderCuboid(
            verletSystem, ctx.interiorRoot, glm::vec3(0.1f, 1.0f, 0.1f),
            ctx.earthCenter + glm::vec3(-ctx.halfSize,
                                        ctx.floorY - floorThickness,
                                        -ctx.halfSize),
            ctx.earthCenter +
                glm::vec3(+ctx.halfSize, ctx.floorY, +ctx.halfSize));

        const float wallHeight = 10.0f;
        const float wallThick = 2.0f;
        const float wallGapFloor = 0.0f;
        const float wallGapEdge = 8.0f;

        const float wallBottom = ctx.floorY + wallGapFloor;
        const float wallTop = wallBottom + wallHeight;

        const float hs = ctx.halfSize;
        const glm::vec3 &C = ctx.earthCenter;

        this->wallsId =
            this->scene.addBasicNodeAsChild(ctx.interiorRoot).value();

        addStaticColliderCuboid(
            verletSystem, this->wallsId, glm::vec3(0.5f, 0.5f, 0.5f),
            C + glm::vec3(-wallThick * 0.5f, wallBottom, -hs + wallGapEdge),
            C + glm::vec3(+wallThick * 0.5f, wallTop, hs - wallGapEdge));

        addStaticColliderCuboid(
            verletSystem, this->wallsId, glm::vec3(0.5f, 0.5f, 0.5f),
            C + glm::vec3(-hs + wallGapEdge, wallBottom, -wallThick * 0.5f),
            C + glm::vec3(hs - wallGapEdge, wallTop, +wallThick * 0.5f));

        return ctx;
    }

    void buildBallPitRoom(Verlet &verletSystem,
                          const EarthInteriorContext &ctx) {
        const float cornerOffset = ctx.halfSize * 0.55f;
        const glm::vec3 pitCenter =
            ctx.earthCenter + glm::vec3(-cornerOffset, 0.0f, -cornerOffset);
        const float pitHalfX = 9.0f;
        const float pitHalfZ = 14.0f;
        const float wallT = 2.0f;
        const float wallH = 2.0f;

        const float pitMinX = pitCenter.x - pitHalfX;
        const float pitMaxX = pitCenter.x + pitHalfX;
        const float pitMinZ = pitCenter.z - pitHalfZ;
        const float pitMaxZ = pitCenter.z + pitHalfZ;

        addStaticColliderCuboid(
            ctx.ballPitRoot ? verletSystem : verletSystem, ctx.ballPitRoot,
            glm::vec3(0.15f), glm::vec3(pitMinX - wallT, ctx.floorY, pitMinZ),
            glm::vec3(pitMinX, ctx.floorY + wallH, pitMaxZ));

        addStaticColliderCuboid(
            verletSystem, ctx.ballPitRoot, glm::vec3(0.15f),
            glm::vec3(pitMaxX, ctx.floorY, pitMinZ),
            glm::vec3(pitMaxX + wallT, ctx.floorY + wallH, pitMaxZ));

        addStaticColliderCuboid(
            verletSystem, ctx.ballPitRoot, glm::vec3(0.15f),
            glm::vec3(pitMinX - wallT, ctx.floorY, pitMinZ - wallT),
            glm::vec3(pitMaxX + wallT, ctx.floorY + wallH, pitMinZ));

        addStaticColliderCuboid(
            verletSystem, ctx.ballPitRoot, glm::vec3(0.15f),
            glm::vec3(pitMinX - wallT, ctx.floorY, pitMaxZ),
            glm::vec3(pitMaxX + wallT, ctx.floorY + wallH, pitMaxZ + wallT));

        addStaticColliderCuboid(
            verletSystem, ctx.ballPitRoot, glm::vec3(0.35f, 0.1f, 0.1f),
            pitCenter + glm::vec3(-3.0f, ctx.floorY, -3.0f),
            pitCenter + glm::vec3(-1.0f, ctx.floorY + 6.0f, -1.0f));

        addStaticColliderCuboid(
            verletSystem, ctx.ballPitRoot, glm::vec3(0.1f, 0.35f, 0.1f),
            pitCenter + glm::vec3(2.0f, ctx.floorY, 4.0f),
            pitCenter + glm::vec3(4.0f, ctx.floorY + 8.0f, 6.0f));

        auto sphereMeshOpt = AssetManager::loadMesh("assets/meshes/sphere.obj");
        if (!sphereMeshOpt.has_value())
            return;

        std::srand(0);

        const int ballCount = 25;
        const float ballRadius = 1.2f;
        const float gravityForce = 30.0f;

        auto makeGravity = [&](float force) {
            Attracted tmp;
            tmp.addDirectionAttraction(glm::vec3(0, -1, 0), force);
            return tmp;
        };

        for (int i = 0; i < ballCount; ++i) {
            SceneObject ball("shaders/PBR_vs.glsl", "shaders/PBR_fs.glsl",
                             *sphereMeshOpt.value());

            float r = 0.2f + (std::rand() / (float)RAND_MAX) * 0.8f;
            float g = 0.2f + (std::rand() / (float)RAND_MAX) * 0.8f;
            float b = 0.2f + (std::rand() / (float)RAND_MAX) * 0.8f;
            ball.setAlbedo(glm::vec3(r, g, b));
            ball.setMetallic(0.0f);
            ball.setRoughness(0.6f);

            NodeId nodeId = scene.addMeshAsChild(ctx.ballPitRoot, ball).value();
            EntityId e = ECSManager::generateEntityId();

            float rx = (std::rand() / (float)RAND_MAX);
            float rz = (std::rand() / (float)RAND_MAX);
            float ry = (std::rand() / (float)RAND_MAX);

            glm::vec3 p;
            p.x = (pitMinX + ballRadius) +
                  rx * ((pitMaxX - ballRadius) - (pitMinX + ballRadius));
            p.z = (pitMinZ + ballRadius) +
                  rz * ((pitMaxZ - ballRadius) - (pitMinZ + ballRadius));
            p.y = (ctx.floorY + 4.0f) + ry * 8.0f;

            ECSManager::setComponentToEntity(Noded(nodeId), e);
            ECSManager::setComponentToEntity(Positionable(p), e);
            ECSManager::setComponentToEntity(
                VerletBody(p, glm::vec3(0.0f), ballRadius, false), e);
            ECSManager::setComponentToEntity(makeGravity(gravityForce), e);
        }
    }

    void buildPbrRoom(const EarthInteriorContext &ctx) {
        const glm::vec3 pbrCenter =
            ctx.earthCenter +
            glm::vec3(-ctx.halfSize * 0.5f, 0.0f, +ctx.halfSize * 0.5f);

        // Lumière au centre
        scene.addLightToScene(
            Light(glm::vec3(970, -10, 30), glm::vec3(5000.0f)));

        auto addPbrStatic = [&](const char *meshPath, const glm::vec3 &pos,
                                const char *albedo, const char *normal,
                                const char *metallic, const char *roughness,
                                const char *ao, const glm::vec3 &rotDeg) {
            auto meshOpt = AssetManager::loadMesh(meshPath);
            if (!meshOpt.has_value())
                return;

            SceneObject obj("shaders/PBR_vs.glsl", "shaders/PBR_fs.glsl",
                            *meshOpt.value());

            if (albedo)
                obj.addAlbedoMap(AssetManager::loadTexture(albedo));
            if (normal)
                obj.addNormalMap(AssetManager::loadTexture(normal));
            if (metallic)
                obj.addMetallicMap(AssetManager::loadTexture(metallic));
            if (roughness)
                obj.addRoughnessMap(AssetManager::loadTexture(roughness));
            if (ao)
                obj.addAoMap(AssetManager::loadTexture(ao));

            obj.setMetallic(0.0f);
            obj.setRoughness(0.6f);

            NodeId id = scene.addMeshAsChild(ctx.pbrRoot, obj).value();
            scene.setTransform(id, translate(pos)
                                       .rotationX(rotDeg.x)
                                       .rotationY(rotDeg.y)
                                       .rotationZ(rotDeg.z)
                                       .scale(4.0f));
        };

        const float r = 12.0f;
        const float y = ctx.floorY + 3.0f;

        const glm::vec3 p0(pbrCenter.x + r, y, pbrCenter.z);
        const glm::vec3 p1(pbrCenter.x, y, pbrCenter.z + r);
        const glm::vec3 p2(pbrCenter.x - r, y, pbrCenter.z);
        const glm::vec3 p3(pbrCenter.x, y, pbrCenter.z - r);

        addPbrStatic(
            "assets/meshes/cube.obj", p0,
            "assets/textures/MetalPlates/MetalPlates_Color.png",
            "assets/textures/MetalPlates/MetalPlates_NormalGL.png",
            "assets/textures/MetalPlates/MetalPlates_Metalness.png",
            "assets/textures/MetalPlates/MetalPlates006_1K-PNG_Roughness.png",
            nullptr, glm::vec3(0.0f, 0.0f, 0.0f));

        addPbrStatic(
            "assets/meshes/big_sphere.obj", p1,
            "assets/textures/ornate-celtic-gold-bl/"
            "ornate-celtic-gold-albedo.png",
            "assets/textures/ornate-celtic-gold-bl/"
            "ornate-celtic-gold-normal-ogl.png",
            nullptr,
            "assets/textures/ornate-celtic-gold-bl/"
            "ornate-celtic-gold-roughness.png",
            "assets/textures/ornate-celtic-gold-bl/ornate-celtic-gold-ao.png",
            glm::vec3(90.0f, 0.0f, 0.0f));

        addPbrStatic("assets/meshes/cube.obj", p2,
                     "assets/textures/PaintedMetal/PaintedMetal_Color.png",
                     "assets/textures/PaintedMetal/PaintedMetal_NormalGL.png",
                     "assets/textures/PaintedMetal/PaintedMetal_Metalness.png",
                     "assets/textures/PaintedMetal/PaintedMetal_Roughness.png",
                     nullptr, glm::vec3(0.0f, 0.0f, 0.0f));

        addPbrStatic("assets/meshes/big_sphere.obj", p3,
                     "assets/textures/ChristmasTreeOrnament/"
                     "ChristmasTreeOrnament_Color.png",
                     "assets/textures/ChristmasTreeOrnament/"
                     "ChristmasTreeOrnament_NormalGL.png",
                     "assets/textures/ChristmasTreeOrnament/"
                     "ChristmasTreeOrnament_Metalness.png",
                     "assets/textures/ChristmasTreeOrnament/"
                     "ChristmasTreeOrnament_Roughness.png",
                     nullptr, glm::vec3(90.0f, 0.0f, 0.0f));
    }

    void buildCollectiblesRoom(const EarthInteriorContext &ctx,
                               int &totalCollectibles) {
        // Bottom-right corner  (+X, -Z)
        const float cornerOffset = ctx.halfSize * 0.55f;
        const glm::vec3 cCenter =
            ctx.earthCenter + glm::vec3(+cornerOffset, 0.0f, -cornerOffset);

        auto starMeshOpt = AssetManager::loadMesh("assets/meshes/star.obj");
        if (!starMeshOpt.has_value())
            return;

        const float collectibleRadius = 1.0f;

        auto spawnCollectible = [&](const glm::vec3 &p) {
            SceneObject starObj("shaders/PBR_vs.glsl", "shaders/PBR_fs.glsl",
                                *starMeshOpt.value());
            starObj.setAlbedo(glm::vec3(1.0f, 0.9f, 0.2f));
            starObj.setMetallic(0.0f);
            starObj.setRoughness(0.6f);

            NodeId nodeId =
                scene.addMeshAsChild(ctx.collectRoot, starObj).value();
            EntityId e = ECSManager::generateEntityId();

            ECSManager::setComponentToEntity(Noded(nodeId), e);
            ECSManager::setComponentToEntity(Positionable(p), e);
            ECSManager::setComponentToEntity(
                VerletBody(p, glm::vec3(0.0f), collectibleRadius, true), e);

            ECSManager::getComponentOfEntity<VerletBody>(e)
                .value()
                .get()
                .isTrigger = true;
            ECSManager::setComponentToEntity(Collectible(1), e);

            totalCollectibles += 1;
        };

        spawnCollectible(cCenter + glm::vec3(0.0f, ctx.floorY + 4.0f, 0.0f));
        spawnCollectible(cCenter + glm::vec3(4.0f, ctx.floorY + 4.0f, 0.0f));
        spawnCollectible(cCenter + glm::vec3(-4.0f, ctx.floorY + 4.0f, 0.0f));
        spawnCollectible(cCenter + glm::vec3(0.0f, ctx.floorY + 4.0f, 4.0f));
        spawnCollectible(cCenter + glm::vec3(0.0f, ctx.floorY + 4.0f, -4.0f));
    }

  public:
    StellarSystem(GlobalScene &scene, PVS &pvs) : scene(scene), pvs(pvs) {
        this->stellarSystem = scene.addBasicNode();

        this->registerCelestialObject(
            "assets/textures/sun.jpg", this->stellarSystem, this->starParent,
            this->star, this->starPipe, this->starEntityId, starSize);

        this->registerCelestialObject(
            "assets/textures/earth.jpg", this->starParent, this->planetParent,
            this->planet, this->planetPipe, this->planetEntityId, planetSize);

        this->registerCelestialObject(
            "assets/textures/moon.jpg", this->planetParent, this->moonParent,
            this->moon, this->moonPipe, this->moonEntityId, moonSize);
    }

    void update(float deltaTime, const glm::vec3 pos) {
        float deltaSpeed = deltaTime * speed;

        glm::vec3 starPos =
            ECSManager::getComponentOfEntity<Positionable>(this->starEntityId)
                .value()
                .get()
                .pos;
        glm::vec3 planetPos =
            ECSManager::getComponentOfEntity<Positionable>(this->planetEntityId)
                .value()
                .get()
                .pos;
        glm::vec3 moonPos =
            ECSManager::getComponentOfEntity<Positionable>(this->moonEntityId)
                .value()
                .get()
                .pos;

        if (glm::distance(starPos, pos) > this->starSize * 1.5) {
            // this->starRotationAngle += starRotationRatio * deltaSpeed;
        }

        this->scene.setTransform(
            this->star, rotationY(this->starRotationAngle).scale(starSize));
        this->scene.setTransform(
            this->starPipe,
            translate(1, 0, 0).rotationZ(90).scale(1.f / starSize));

        if (glm::distance(planetPos, pos) > this->planetSize * 1.5) {
            // this->planetRevolutionAngle += planetRevolutionRatio *
            // deltaSpeed; this->planetRotationAngle += planetRotationRatio *
            // deltaSpeed;
        }

        this->scene.setTransform(
            this->planetParent,
            rotationY(this->planetRevolutionAngle).translate(1000, 0, 0));
        this->scene.setTransform(this->planet,
                                 rotationX(23)
                                     .rotationX(90)
                                     .rotationZ(this->planetRotationAngle)
                                     .scale(planetSize));

        this->scene.setTransform(
            this->planetPipe,
            translate(0.f, 0.f, -1.f).rotationX(90.f).scale(4.f / planetSize));

        if (glm::distance(moonPos, pos) > this->moonSize * 1.5) {
            this->moonRevolutionAngle += moonRevolutionRatio * deltaSpeed;
            this->moonRotationAngle += moonRotationRatio * deltaSpeed;
        }

        this->scene.setTransform(
            this->moonParent,
            rotationY(this->moonRevolutionAngle).translate(200, 0, 0));
        this->scene.setTransform(this->moon, scale(moonSize));
        this->scene.setTransform(
            this->moonPipe,
            translate(1, 0, 0).rotationZ(90).scale(1.f / moonSize));

        glm::vec3 posPlanet =
            ECSManager::getComponentOfEntity<Positionable>(this->planetEntityId)
                .value()
                .get()
                .pos;
        glm::vec3 posMoon =
            ECSManager::getComponentOfEntity<Positionable>(this->moonEntityId)
                .value()
                .get()
                .pos;
    }

    bool isNearPipe(const glm::vec3 &playerPos, float triggerDist) {
        const float triggerDist2 = triggerDist * triggerDist;

        auto nearPipe = [&](NodeId id) {
            auto opt = this->scene.getTransform(id);
            if (!opt.has_value()) {
                return false;
            }

            const glm::vec3 pipePos = opt.value().getPosition();
            const glm::vec3 d = pipePos - playerPos;
            const float dist2 = glm::dot(d, d);
            return dist2 <= triggerDist2;
        };

        return nearPipe(this->starPipe) || nearPipe(this->planetPipe) ||
               nearPipe(this->moonPipe);
    }

    bool getNearestPipePos(const glm::vec3 &playerPos, float triggerDist,
                           glm::vec3 &outPipePos) {
        if (triggerDist <= 0.0f)
            return false;

        float bestDist2 = triggerDist * triggerDist;
        bool found = false;

        auto consider = [&](NodeId pipeNode) {
            auto opt = scene.getTransform(pipeNode);
            if (!opt.has_value())
                return;

            const glm::vec3 pipePos = opt.value().getPosition();
            const glm::vec3 d = pipePos - playerPos;
            const float dist2 = glm::dot(d, d);

            if (dist2 <= bestDist2) {
                bestDist2 = dist2;
                outPipePos = pipePos;
                found = true;
            }
        };

        consider(starPipe);
        consider(planetPipe);
        consider(moonPipe);

        return found;
    }

    void setSystemAttraction(EntityId id) {
        if (!ECSManager::getComponentOfEntity<Attracted>(id).has_value()) {
            ECSManager::setComponentToEntity(Attracted(), id);
        }

        Attracted &attracted =
            ECSManager::getComponentOfEntity<Attracted>(id).value();
        attracted.addBodyAttraction(this->starEntityId, AttractionMode::INWARD,
                                    starMass);
        attracted.addBodyAttraction(this->planetEntityId,
                                    AttractionMode::INWARD, planetMass / 10);
        attracted.addBodyAttraction(this->moonEntityId, AttractionMode::INWARD,
                                    moonMass);
    }

    void initEarthInterior(Verlet &verletSystem, int &totalCollectibles) {
        EarthInteriorContext ctx = buildEarthInteriorBase(verletSystem);
        if (ctx.interiorRoot == 0)
            return;

        buildBallPitRoom(verletSystem, ctx);
        buildPbrRoom(ctx);
        buildCollectiblesRoom(ctx, totalCollectibles);
    }

    EntityId getStarId() { return this->starEntityId; }

    EntityId getPlanetId() { return this->planetEntityId; }

    EntityId getMoonId() { return this->moonEntityId; }

    void enter() { this->pvs.enterScene(this->roomId[0]); }

    void next() {
        static size_t current = 0;

        current++;
        current = current % this->roomId.size();
        this->pvs.enterScene(this->roomId[current]);

        if (this->hiddenWalls) {
            this->toggleWalls();
        }
    }

    void toggleWalls() { this->scene.toggleNode(this->wallsId); }
};
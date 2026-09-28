#ifndef GAME_ENGINE
#define GAME_ENGINE

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Camera.h"
#include "Controls.h"
#include "GlobalScene.h"
#include "PVS.h"
#include "src/ecs/systems/SystemUpdater.h"

using namespace std;
using namespace glm;

using ushort = unsigned short;
using uint = unsigned int;

class GameEngine {
    GLFWwindow *window;

    float deltaTime = 0;
    float lastFrame = 0;

    Camera camera;
    Controls controls;
    GlobalScene scene;
    PVS pvs;
    std::shared_ptr<SystemUpdater> systemUpdater;

    GLuint vertexArrayId;

    bool stop = false;

    void initInternal();
    void processInputInternal();
    void updateInternal();
    void renderInternal();
    void cleanUpInternal();

  public:
    GameEngine(GLFWwindow *window, uint width, uint height);

    int run();

    void stopRunning();

    GLFWwindow *getWindow();
    Camera &getCamera();
    Controls &getControls();
    GlobalScene &getScene();
    PVS &getPVS();
    std::shared_ptr<SystemUpdater> getSystemUpdater();

    void setScene(const GlobalScene &other);

    virtual void init() {};
    virtual void processInput(float deltaTime) {};
    virtual void update(float deltaTime) {};
    virtual void render(float deltaTime) {};
    virtual void cleanUp() {};
};

#endif // GAME_ENGINE
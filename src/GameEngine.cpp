#include "GameEngine.h"

GameEngine::GameEngine(GLFWwindow *window, uint width, uint height)
    : camera(width, height), controls(), pvs(scene) {
    this->window = window;
}

void GameEngine::initInternal() {
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    this->systemUpdater = std::make_shared<SystemUpdater>();
    SystemId id = ECSManager::registerSystem(this->systemUpdater);
    this->systemUpdater->registerComponents(id);

    this->init();
}

void GameEngine::processInputInternal() {
    controls.processInput(this->window, this->camera, this->deltaTime);

    this->processInput(this->deltaTime);
}

void GameEngine::updateInternal() {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    camera.setScreenWidth(width);
    camera.setScreenHeight(height);

    this->systemUpdater->update(deltaTime);

    this->update(this->deltaTime);
}

void GameEngine::renderInternal() {
    this->camera.update(this->deltaTime);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    this->scene.draw(this->camera);

    this->render(this->deltaTime);
}

void GameEngine::cleanUpInternal() {
    this->scene.cleanUp();
    glDeleteVertexArrays(1, &this->vertexArrayId);
    glfwSetWindowShouldClose(window, true);
    glfwTerminate();

    this->cleanUp();
}

int GameEngine::run() {
    this->initInternal();

    while (!this->stop) {
        float currentFrame = glfwGetTime();
        this->deltaTime = currentFrame - this->lastFrame;
        this->lastFrame = currentFrame;
        this->processInputInternal();
        this->updateInternal();
        this->renderInternal();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    this->cleanUpInternal();
    return 0;
}

void GameEngine::stopRunning() { this->stop = true; }

GLFWwindow *GameEngine::getWindow() { return this->window; }
Camera &GameEngine::getCamera() { return this->camera; }
Controls &GameEngine::getControls() { return this->controls; }
GlobalScene &GameEngine::getScene() { return this->scene; }
PVS &GameEngine::getPVS() { return this->pvs; }
std::shared_ptr<SystemUpdater> GameEngine::getSystemUpdater() {
    return this->systemUpdater;
}

void GameEngine::setScene(const GlobalScene &other) { this->scene = other; }
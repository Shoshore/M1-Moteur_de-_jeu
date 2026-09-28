#include "Controls.h"

KeyCallback::KeyCallback(function<void(float)> callback) : callback(callback) {}

MouseMoveCallback::MouseMoveCallback(function<void(float, float)> callback)
    : callback(callback) {}

MouseKeyCallback::MouseKeyCallback(function<void(float, float, float)> callback)
    : callback(callback) {}

void Controls::processMouse(GLFWwindow *window, Camera &camera) {
    double currentX, currentY;
    glfwGetCursorPos(window, &currentX, &currentY);

    float centerX = camera.getScreenWidth() / 2.;
    float centerY = camera.getScreenHeight() / 2.;

    float deltaX = currentX - centerX;
    float deltaY = currentY - centerY;
    static int cpt = 0;

    if (cpt < 5) {
        cpt++;
    } else {
        for (MouseMoveCallback *mouseMoveCallback : this->mouseDeltaCallback) {
            mouseMoveCallback->callback(deltaX, deltaY);
        }
    }

    glfwSetCursorPos(window, centerX, centerY);
}

void Controls::callEachKey(vector<KeyCallback *> callbacks, float value) {
    for (KeyCallback *keyCallback : callbacks) {
        keyCallback->callback(value);
    }
}

void Controls::callEachMouseKey(vector<MouseKeyCallback *> callbacks,
                                float posX, float posY, float value) {
    for (MouseKeyCallback *keyCallback : callbacks) {
        keyCallback->callback(posX, posY, value);
    }
}

void Controls::processKeys(GLFWwindow *window, float deltaTime) {
    for (auto &pair : this->keysToWatch) {
        uint key = pair.first;
        bool wasPressed = pair.second;

        if (glfwGetKey(window, key) == GLFW_PRESS) {
            if (wasPressed) {
                this->callEachKey(this->keyDownMapCallback[key], deltaTime);
            } else {
                this->callEachKey(this->keyPressedMapCallback[key], deltaTime);
                this->callEachKey(this->keyDownMapCallback[key], deltaTime);
                this->keysToWatch[key] = true;
            }
        } else {
            if (wasPressed) {
                this->callEachKey(this->keyReleasedMapCallback[key], deltaTime);
                this->keysToWatch[key] = false;
            }
        }
    }
}

void Controls::processMouseKeys(GLFWwindow *window, Camera &camera,
                                float deltaTime) {
    double currentX, currentY;
    glfwGetCursorPos(window, &currentX, &currentY);

    for (auto &pair : this->mouseKeysToWatch) {
        uint key = pair.first;
        bool wasPressed = pair.second;

        if (glfwGetMouseButton(window, key) == GLFW_PRESS) {
            if (wasPressed) {
                this->callEachMouseKey(this->mouseKeyDownMapCallback[key],
                                       currentX, currentY, deltaTime);
            } else {
                this->callEachMouseKey(this->mouseKeyPressedMapCallback[key],
                                       currentX, currentY, deltaTime);
                this->callEachMouseKey(this->mouseKeyDownMapCallback[key],
                                       currentX, currentY, deltaTime);
                this->mouseKeysToWatch[key] = true;
            }
        } else {
            if (wasPressed) {
                this->callEachMouseKey(this->mouseKeyReleasedMapCallback[key],
                                       currentX, currentY, deltaTime);
                this->mouseKeysToWatch[key] = false;
            }
        }
    }
}

void Controls::addKeyPressedCallback(KeyId key, KeyCallback *callback) {
    this->keysToWatch[key] = false;
    this->keyPressedMapCallback[key].push_back(callback);
}

void Controls::addKeyReleasedCallback(KeyId key, KeyCallback *callback) {
    this->keysToWatch[key] = false;
    this->keyReleasedMapCallback[key].push_back(callback);
}

void Controls::addKeyDownCallback(KeyId key, KeyCallback *callback) {
    this->keysToWatch[key] = false;
    this->keyDownMapCallback[key].push_back(callback);
}

void Controls::addMouseDeltaCallback(MouseMoveCallback *callback) {
    this->mouseDeltaCallback.push_back(callback);
}

void Controls::addMouseKeyPressedCallback(KeyId key,
                                          MouseKeyCallback *callback) {
    this->mouseKeysToWatch[key] = false;
    this->mouseKeyPressedMapCallback[key].push_back(callback);
}

void Controls::addMouseKeyReleasedCallback(KeyId key,
                                           MouseKeyCallback *callback) {
    this->mouseKeysToWatch[key] = false;
    this->mouseKeyReleasedMapCallback[key].push_back(callback);
}

void Controls::addMouseKeyDownCallback(KeyId key, MouseKeyCallback *callback) {
    this->mouseKeysToWatch[key] = false;
    this->mouseKeyDownMapCallback[key].push_back(callback);
}

void Controls::processInput(GLFWwindow *window, Camera &camera,
                            float deltaTime) {
    this->processKeys(window, deltaTime);
    this->processMouse(window, camera);
    this->processMouseKeys(window, camera, deltaTime);
}
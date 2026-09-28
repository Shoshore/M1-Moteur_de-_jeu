#include "Camera.h"

#include <algorithm>

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include <glm/ext.hpp>

static float clipAngle180(float _angle) {
    while (_angle >= 180.f || _angle < -180.f) {
        _angle += (_angle > 180.f) ? -360.f : 360.f;
    }
    return _angle;
}

Camera::Camera(uint screenWidth, uint screenHeight)
    : screenWidth(screenWidth), screenHeight(screenHeight) {}

void Camera::update(const vec3 &pos) { this->setTarget(pos); }

uint Camera::getScreenWidth() const { return this->screenWidth; }
void Camera::setScreenWidth(uint newWidth) { this->screenWidth = newWidth; }

uint Camera::getScreenHeight() const { return this->screenHeight; }
void Camera::setScreenHeight(uint newHeight) { this->screenHeight = newHeight; }

float Camera::getFov() const { return this->fov; }
void Camera::setFov(float newFov) { this->fov = newFov; }

float Camera::getZNear() const { return this->zNear; }
void Camera::setZNear(float newZNear) { this->zNear = newZNear; }

float Camera::getZFar() const { return this->zFar; }
void Camera::setZFar(float newZFar) { this->zFar = newZFar; }

vec3 Camera::getPosition() const { return this->position; }
void Camera::setPosition(vec3 newPosition) { this->position = newPosition; }

vec3 Camera::getTarget() const { return this->target; }
void Camera::setTarget(vec3 newTarget) { this->target = newTarget; }

float Camera::getTargetDistance() const { return this->targetDistance; }
void Camera::setTargetDistance(float newTargetDistance) {
    this->targetDistance = newTargetDistance;
}

float Camera::getTranslationSpeed() const { return this->translationSpeed; }
void Camera::setTranslationSpeed(float newTranslationSpeed) {
    this->translationSpeed = newTranslationSpeed;
}

vec3 Camera::projectVectorOnPlan(vec3 toProject, vec3 normal) {
    normal = normalize(normal);
    return cross(normal, cross(toProject, normal));
}

vec3 Camera::getFront() const { return this->rotation * VEC_FRONT; }
vec3 Camera::getRight() const { return this->rotation * VEC_RIGHT; }
vec3 Camera::getUp() const { return this->rotation * VEC_UP; }

void Camera::update(float deltaTime) {

    if (this->targetDistance < this->zNear) {
        this->targetDistance = this->zNear;
    }

    if (this->targetDistance > this->zFar) {
        this->targetDistance = this->zFar;
    }

    this->position = this->target - this->getFront() * this->targetDistance;

    this->projectionMatrix = perspective(
        radians(this->fov), ((float)this->screenWidth) / this->screenHeight,
        this->zNear, this->zFar);

    this->viewMatrix = lookAt(this->position, this->target, this->getUp());
}

mat4 Camera::getView() const { return this->viewMatrix; }

mat4 Camera::getProjection() const { return this->projectionMatrix; }

void Camera::rotateWithMouse(float deltaX, float deltaY) {
    const float sensitivity = 0.001f;
    const static float ROTATE_MOUSE_FACTOR = 50.f;

    deltaX = std::clamp(deltaX, -ROTATE_MOUSE_FACTOR, ROTATE_MOUSE_FACTOR);
    deltaY = std::clamp(deltaY, -ROTATE_MOUSE_FACTOR, ROTATE_MOUSE_FACTOR);

    if (deltaX != 0 || deltaY != 0) {
        glm::vec3 axis{0, 0, 0};

        axis += this->getUp() * -deltaX;
        axis += this->getRight() * deltaY;

        axis = glm::normalize(axis);

        this->rotation = glm::normalize(
            glm::angleAxis(glm::length(glm::vec2{deltaX, deltaY}) * sensitivity,
                           axis) *
            this->rotation);
    }
}

void Camera::tilt(float deltaTime, bool clockwise) {
    const float rotationSpeed = 1;

    if (deltaTime != 0) {
        glm::vec3 axis = glm::normalize(this->getFront());

        this->rotation = glm::normalize(
            glm::angleAxis(rotationSpeed * deltaTime * (clockwise ? -1 : 1),
                           axis) *
            this->rotation);
    }
}

void Camera::forward(float deltaTime) {
    vec3 direction = this->rotation * vec3(0.0f, 0.0f, 1.0f);
    this->targetDistance -= deltaTime * this->translationSpeed;
}

void Camera::backward(float deltaTime) {
    vec3 direction = this->rotation * vec3(0.0f, 0.0f, -1.0f);
    this->targetDistance += deltaTime * this->translationSpeed;
}

void Camera::up(float deltaTime) {
    this->position +=
        normalize(vec3(.0f, 1.0f, 0.0f)) * this->translationSpeed * deltaTime;
}

void Camera::down(float deltaTime) {
    this->position +=
        normalize(vec3(.0f, -1.0f, 0.0f)) * this->translationSpeed * deltaTime;
}

void Camera::getCloser(float deltaTime) {
    this->targetDistance -= deltaTime * 1;
}

void Camera::getFurther(float deltaTime) {
    this->targetDistance += deltaTime * 1;
}
#ifndef CAMERA
#define CAMERA

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Observer.h"

using namespace std;
using namespace glm;

using uint = unsigned int;

const static vec3 VEC_UP(0.f, 1.f, 0.f);
const static vec3 VEC_FRONT(0.f, 0.f, 1.f);
const static vec3 VEC_RIGHT(1.f, 0.f, 0.f);

static float clipAngle180(float _angle);

class Camera : public Observer<vec3> {
    uint screenWidth;
    uint screenHeight;

    float fov = 45;
    float zNear = .1;
    float zFar = 1000;

    vec3 position = vec3(0, 0, 0);
    quat rotation = quat();

    mat4 viewMatrix = mat4();
    mat4 projectionMatrix = mat4();

    vec3 target = vec3(0, 0, 0);
    float targetDistance = 3.;

    float translationSpeed = 2.5;

    vec3 projectVectorOnPlan(vec3 toProject, vec3 normal);

  public:
    Camera(uint screenWidth, uint screenHeight);

    void update(const vec3 &pos) override;

    uint getScreenWidth() const;
    void setScreenWidth(uint newWidth);

    uint getScreenHeight() const;
    void setScreenHeight(uint newHeight);

    float getFov() const;
    void setFov(float newFov);

    float getZNear() const;
    void setZNear(float newZNear);

    float getZFar() const;
    void setZFar(float newZFar);

    vec3 getPosition() const;
    void setPosition(vec3 newPosition);

    vec3 getTarget() const;
    void setTarget(vec3 newTarget);

    float getTargetDistance() const;
    void setTargetDistance(float newTargetDistance);

    float getTranslationSpeed() const;
    void setTranslationSpeed(float newTranslationSpeed);

    vec3 getFront() const;
    vec3 getRight() const;
    vec3 getUp() const;

    void update(float deltaTime);

    mat4 getView() const;
    mat4 getProjection() const;

    void rotateWithMouse(float deltaX, float deltaY);

    void tilt(float deltaTime, bool clockwise);
    void forward(float deltaTime);
    void backward(float deltaTime);
    void up(float deltaTime);
    void down(float deltaTime);

    void getCloser(float deltaTime);
    void getFurther(float deltaTime);
};

#endif // CAMERA
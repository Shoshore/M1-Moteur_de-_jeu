#ifndef CONTROLS
#define CONTROLS

#include <functional>
#include <map>

#include "GLFW/glfw3.h"

#include "Camera.h"

using namespace std;

using uint = unsigned int;
using KeyId = uint;

class KeyCallback {
  public:
    function<void(float)> callback;
    KeyCallback(function<void(float)> callback);
};

class MouseMoveCallback {
  public:
    function<void(float, float)> callback;
    MouseMoveCallback(function<void(float, float)> callback);
};

class MouseKeyCallback {
  public:
    function<void(float, float, float)> callback;
    MouseKeyCallback(function<void(float, float, float)> callback);
};

class Controls {
    map<KeyId, vector<KeyCallback *>> keyPressedMapCallback;
    map<KeyId, vector<KeyCallback *>> keyReleasedMapCallback;
    map<KeyId, vector<KeyCallback *>> keyDownMapCallback;

    map<KeyId, bool> keysToWatch;

    vector<MouseMoveCallback *> mouseDeltaCallback;
    map<KeyId, vector<MouseKeyCallback *>> mouseKeyPressedMapCallback;
    map<KeyId, vector<MouseKeyCallback *>> mouseKeyReleasedMapCallback;
    map<KeyId, vector<MouseKeyCallback *>> mouseKeyDownMapCallback;

    map<KeyId, bool> mouseKeysToWatch;

    void processMouse(GLFWwindow *window, Camera &camera);
    void callEachKey(vector<KeyCallback *> callbacks, float value);
    void callEachMouseKey(vector<MouseKeyCallback *> callbacks, float posX,
                          float posY, float value);
    void processKeys(GLFWwindow *window, float deltaTime);
    void processMouseKeys(GLFWwindow *window, Camera &camera, float deltaTime);

  public:
    void addKeyPressedCallback(KeyId key, KeyCallback *callback);
    void addKeyReleasedCallback(KeyId key, KeyCallback *callback);
    void addKeyDownCallback(KeyId key, KeyCallback *callback);

    void addMouseDeltaCallback(MouseMoveCallback *callback);
    void addMouseKeyPressedCallback(KeyId key, MouseKeyCallback *callback);
    void addMouseKeyReleasedCallback(KeyId key, MouseKeyCallback *callback);
    void addMouseKeyDownCallback(KeyId key, MouseKeyCallback *callback);

    void processInput(GLFWwindow *window, Camera &camera, float deltaTime);
};

#endif // CONTROLS
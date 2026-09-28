#ifndef PVS_
#define PVS_

#include "src/GlobalScene.h"
#include <optional>
#include <set>

class PVS {
    GlobalScene &globalScene;

    std::set<NodeId> scenes;

    std::optional<NodeId> currentScene;

    std::map<NodeId, std::set<NodeId>> graph;

    void hideConnected(NodeId scene) {
        this->globalScene.hideNode(scene);

        for (NodeId connected : graph[scene]) {
            this->globalScene.hideNode(connected);
        }
    }

    void showConnected(NodeId scene) {
        this->globalScene.showNode(scene);

        for (NodeId connected : graph[scene]) {
            this->globalScene.showNode(connected);
        }
    }

  public:
    PVS(GlobalScene &globalScene) : globalScene(globalScene) {}

    void init(NodeId startingScene) {
        for (NodeId scene : this->scenes) {
            this->globalScene.hideNode(scene);
        }

        this->enterScene(startingScene);
    }

    void enterScene(NodeId scene) {
        if (this->currentScene.has_value()) {
            this->hideConnected(this->currentScene.value());
        }

        this->currentScene = scene;

        this->showConnected(scene);
    }

    void exitScene(NodeId scene) {
        if (this->currentScene.has_value()) {
            this->hideConnected(this->currentScene.value());
        }

        this->currentScene = std::nullopt;
    }

    NodeId addScene() {
        NodeId newScene = this->globalScene.addBasicNode();

        this->scenes.insert(newScene);
        this->globalScene.hideNode(newScene);

        return newScene;
    }

    void removeScene(NodeId scene) {
        if (this->currentScene == scene) {
            this->exitScene(scene);
        }

        this->globalScene.removeNode(scene);
        this->scenes.erase(scene);
    }

    void linkScenes(NodeId first, NodeId second) {
        this->graph[first].insert(second);
        this->graph[second].insert(first);

        if (this->currentScene == first) {
            this->globalScene.showNode(second);
        }

        if (this->currentScene == second) {
            this->globalScene.showNode(first);
        }
    }

    std::optional<NodeId> getCurrentScene() { return currentScene; }
};

#endif // PVS_
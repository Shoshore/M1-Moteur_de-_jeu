#ifndef SCENE
#define SCENE

#include <map>
#include <optional>
#include <set>
#include <stack>
#include <vector>

#include "Light.hpp"
#include "SceneObject.h"
#include "Transform.h"

class GlobalScene;

typedef size_t NodeId;

class GlobalScene {
    struct SceneNode {
        NodeId node;
        Transform localTransform;
        Transform cumulativeTransform;
        bool shouldUpdate;

        SceneNode() : shouldUpdate(false) {}
        SceneNode(NodeId node) : node(node), shouldUpdate(false) {}
        SceneNode(NodeId node, bool shouldUpdate)
            : node(node), shouldUpdate(shouldUpdate) {}

        inline void transformLocal(const Transform &transform) {
            this->localTransform.transform(transform);
        }

        inline void setLocalTransform(const Transform &transform) {
            this->localTransform = transform;
        }

        inline void transformCumulative(const Transform &transform) {
            this->cumulativeTransform.transform(transform);
        }

        inline void setCumulativeTransform(const Transform &transform) {
            this->cumulativeTransform = transform;
        }
    };

    class SceneTree {
        std::map<NodeId, std::set<NodeId>> tree;
        std::map<NodeId, NodeId> parents;
        std::map<NodeId, SceneNode> nodes;

        void removeFromParents(NodeId node) {
            if (this->parents.count(node)) {
                NodeId parent = this->parents[node];
                this->parents.erase(node);
                this->tree[parent].erase(node);
            }
        }

        void addChildNodeUnsafe(NodeId parent, NodeId node) {
            this->tree[parent].insert(node);
            this->parents[node] = parent;
            this->nodes[node] =
                SceneNode(node, this->nodes[parent].shouldUpdate);
        }

        void markSubTree(NodeId node) {
            std::stack<NodeId> toProcess;
            toProcess.push(node);

            while (!toProcess.empty()) {
                NodeId current = toProcess.top();
                toProcess.pop();

                this->nodes[current].shouldUpdate = true;

                std::set<NodeId> children = this->tree[current];
                for (auto it = children.begin(); it != children.end(); ++it) {
                    toProcess.push(*it);
                }
            }
        }

      public:
        SceneTree() {}

        bool hasNode(NodeId node) const { return this->tree.count(node); }

        bool addNode(NodeId node) {
            if (!this->hasNode(node)) {
                this->tree[node];
                this->nodes[node] = SceneNode(node);
                return true;
            }

            return false;
        }

        bool addChildNode(NodeId parent, NodeId node) {
            if (this->hasNode(parent) && this->addNode(node)) {
                this->addChildNodeUnsafe(parent, node);
                return true;
            }

            return false;
        }

        bool changeParent(NodeId parent, NodeId node) {
            if (this->hasNode(parent), this->hasNode(node)) {
                this->removeFromParents(node);
                this->addChildNodeUnsafe(parent, node);
                return true;
            }

            return false;
        }

        std::vector<NodeId> getAllChildren(NodeId node) {
            std::vector<NodeId> result;

            if (this->hasNode(node)) {
                std::stack<NodeId> toProcess;

                toProcess.push(node);

                while (!toProcess.empty()) {
                    NodeId current = toProcess.top();
                    toProcess.pop();

                    result.push_back(current);

                    if (this->hasNode(current)) {
                        std::set<NodeId> children = this->tree[current];
                        for (auto it = children.begin(); it != children.end();
                             ++it) {
                            toProcess.push(*it);
                        }
                    }
                }
            }

            return result;
        }

        std::vector<NodeId> remove(NodeId node) {
            std::vector<NodeId> toRemove = this->getAllChildren(node);

            for (const NodeId &id : toRemove) {
                this->removeFromParents(id);
                this->tree.erase(id);
                this->nodes.erase(node);
            }

            return toRemove;
        }

        bool transformLocal(NodeId node, const Transform &transform) {
            if (this->hasNode(node)) {
                this->nodes[node].transformLocal(transform);

                this->markSubTree(node);

                return true;
            }

            return false;
        }

        bool setLocalTransform(NodeId node, const Transform &transform) {
            if (this->hasNode(node)) {
                this->nodes[node].setLocalTransform(transform);

                this->markSubTree(node);

                return true;
            }

            return false;
        }

        bool shouldUpdate(NodeId node) {
            return this->hasNode(node) && this->nodes[node].shouldUpdate;
        }

        std::optional<Transform> getCumulativeTransform(NodeId node) {
            if (this->hasNode(node)) {
                SceneNode &sceneNode = this->nodes[node];
                if (!sceneNode.shouldUpdate) {
                    return sceneNode.cumulativeTransform;
                }

                std::stack<Transform> transforms;
                std::stack<NodeId> nodes;

                SceneNode *current = &sceneNode;
                transforms.push(current->localTransform);
                nodes.push(current->node);

                while (current->shouldUpdate &&
                       this->parents.count(current->node)) {
                    current = &this->nodes[this->parents[current->node]];
                    transforms.push(current->localTransform);
                    nodes.push(current->node);
                }

                if (this->parents.count(current->node)) {
                    current = &this->nodes[this->parents[current->node]];
                    transforms.push(current->cumulativeTransform);
                    nodes.push(current->node);
                }

                Transform cumulative = transforms.top();
                transforms.pop();
                nodes.pop();

                while (!transforms.empty()) {
                    cumulative.transform(transforms.top());
                    transforms.pop();

                    this->nodes[nodes.top()].setCumulativeTransform(cumulative);
                    this->nodes[nodes.top()].shouldUpdate = false;
                    nodes.pop();
                }

                return sceneNode.cumulativeTransform;
            }

            return std::nullopt;
        }
    };

    template <typename Content> struct NodeList {
        std::map<NodeId, size_t> idNodeMap;
        std::vector<Content> nodes;
        std::vector<NodeId> ids;
        std::vector<bool> flags;
        std::vector<bool> hidden;

        void addNode(NodeId id, Content node) {
            this->nodes.push_back(node);
            this->ids.push_back(id);
            this->flags.push_back(true);
            this->hidden.push_back(false);
            this->idNodeMap[id] = nodes.size() - 1;
        }

        std::optional<Content *> getNode(NodeId id) {
            if (this->idNodeMap.count(id) && this->flags[this->idNodeMap[id]]) {
                return &this->nodes[this->idNodeMap[id]];
            }
            return std::nullopt;
        }

        bool removeNode(NodeId id) {
            if (this->idNodeMap.count(id) && this->flags[this->idNodeMap[id]]) {
                this->flags[this->idNodeMap[id]] = false;
                return true;
            }
            return false;
        }

        void toggle(NodeId id) {
            this->hidden[this->idNodeMap[id]] =
                !this->hidden[this->idNodeMap[id]];
        }

        void show(NodeId id) { this->hidden[this->idNodeMap[id]] = false; }

        void hide(NodeId id) { this->hidden[this->idNodeMap[id]] = true; }

        bool hasId(NodeId id) { return this->idNodeMap.count(id); }
    };

    struct BasicNode {};

    NodeId idCpt = 1;

    SceneTree tree;
    BasicNode root;
    static const NodeId ROOT_ID = 0;

    NodeList<SceneObject> meshList;
    NodeList<BasicNode> basicNodeList;

    NodeId newId() {
        this->idCpt++;
        return this->idCpt - 1;
    }
    std::vector<Light> lights;

  public:
    GlobalScene() {
        this->basicNodeList.addNode(ROOT_ID, BasicNode());
        this->tree.addNode(ROOT_ID);
    }

    void cleanUp() {}

    void transform(const Transform &transform) {
        this->transform(ROOT_ID, transform);
    }

    void setTransform(const Transform &transform) {
        this->setTransform(ROOT_ID, transform);
    }

    void transform(NodeId id, const Transform &transform) {
        this->tree.transformLocal(id, transform);
    }

    void setTransform(NodeId id, const Transform &transform) {
        this->tree.setLocalTransform(id, transform);
    }

    std::optional<Transform> getTransform(NodeId id) {
        return this->tree.getCumulativeTransform(id);
    }

    void draw(const Camera &camera) {
        for (size_t i = 0; i < this->meshList.flags.size(); i++) {
            if (this->meshList.flags[i] && !this->meshList.hidden[i]) {

                if (this->tree.shouldUpdate(this->meshList.ids[i])) {
                    this->meshList.nodes[i].setTransform(
                        this->tree.getCumulativeTransform(this->meshList.ids[i])
                            .value());
                }
                this->meshList.nodes[i].draw(camera, this->lights);
            }
        }
    }

    NodeId addMesh(const SceneObject &mesh) {
        NodeId newId = this->newId();
        this->meshList.addNode(newId, mesh);

        this->tree.addChildNode(ROOT_ID, newId);

        return newId;
    }

    std::optional<NodeId> addMeshAsChild(NodeId parent,
                                         const SceneObject &mesh) {
        if (this->meshList.hasId(parent) || this->basicNodeList.hasId(parent)) {
            NodeId newId = this->addMesh(mesh);

            this->changeParent(parent, newId);

            return newId;
        }

        return std::nullopt;
    }

    std::optional<SceneObject *> getMesh(NodeId id) {
        return this->meshList.getNode(id);
    }

    NodeId addBasicNode() {
        NodeId newId = this->newId();
        BasicNode basicNode;
        this->basicNodeList.addNode(newId, basicNode);

        this->tree.addChildNode(ROOT_ID, newId);

        return newId;
    }

    std::optional<NodeId> addBasicNodeAsChild(NodeId parent) {
        if (this->meshList.hasId(parent) || this->basicNodeList.hasId(parent)) {
            NodeId newId = this->addBasicNode();

            this->changeParent(parent, newId);

            return newId;
        }

        return std::nullopt;
    }

    bool setSceneAsParent(NodeId id) {
        return this->tree.changeParent(ROOT_ID, id);
    }

    bool changeParent(NodeId newParent, NodeId child) {
        return this->tree.changeParent(newParent, child);
    }

    bool removeNode(NodeId id) {
        std::vector<NodeId> removed = this->tree.remove(id);

        for (const NodeId &id : removed) {
            if (this->meshList.hasId(id)) {
                this->meshList.removeNode(id);
            } else if (this->basicNodeList.hasId(id)) {
                this->basicNodeList.removeNode(id);
            }
        }

        return removed.size();
    }

    void toggleNode(NodeId id) {
        for (NodeId child : this->tree.getAllChildren(id)) {
            this->meshList.toggle(child);
        }
    }

    void showNode(NodeId id) {
        for (NodeId child : this->tree.getAllChildren(id)) {
            this->meshList.show(child);
        }
    }

    void hideNode(NodeId id) {
        for (NodeId child : this->tree.getAllChildren(id)) {
            this->meshList.hide(child);
        }
    }

    void addLightToScene(const Light &light) { this->lights.push_back(light); }
};

#endif // SCENE
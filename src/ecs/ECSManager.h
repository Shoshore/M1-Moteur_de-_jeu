#ifndef ECS_MANAGER
#define ECS_MANAGER

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "System.h"
#include "src/ecs/Signature.h"
#include "utils.h"

class ECSManager {
    struct ComponentListBase {
        virtual ~ComponentListBase() = default;
        virtual void remove(EntityId entity) = 0;
    };

    template <typename T> struct ComponentList : ComponentListBase {
        std::vector<T> components;

        std::unordered_map<EntityId, size_t> indices;
        std::queue<size_t> empty_indices;

        template <typename U> void set(U &&component, EntityId entity) {
            if (this->empty_indices.empty()) {
                indices[entity] = components.size();
                components.push_back(std::forward<U>(component));
            } else {
                size_t index = this->empty_indices.front();
                components[index] = std::forward<U>(component);
                this->indices[entity] = index;
                this->empty_indices.pop();
            }
        }

        void remove(EntityId entity) override {
            auto it = this->indices.find(entity);
            if (it == this->indices.end()) {
                return;
            }
            this->components[it->second] = T{};
            this->empty_indices.push(it->second);
            this->indices.erase(entity);
        }

        std::optional<std::reference_wrapper<T>> get(EntityId entity) {
            auto it = this->indices.find(entity);
            if (it == this->indices.end()) {
                return std::nullopt;
            }
            return std::ref(this->components[it->second]);
        }
    };

    template <typename T> struct SignedManager {
        unsigned long counter = 0;
        std::unordered_set<T> unused;
        std::unordered_map<T, ECS::Signature> signatures;

        T generateId() {
            T element;
            if (unused.empty()) {
                element = T{++this->counter};
            } else {
                element = *this->unused.begin();
                this->unused.erase(this->unused.begin());
            }
            this->signatures[element] = ECS::Signature();
            return element;
        }

        void remove(T element) {
            this->signatures.erase(element);
            if (element.value < this->counter) {
                this->unused.insert(element);
            }
        }

        const ECS::Signature &getSignature(T element) const {
            return signatures.at(element);
        }

        const std::set<T>
        getAllCompatible(const ECS::Signature &signature) const {
            std::set<T> result;

            for (auto const &[element, other] : this->signatures) {
                if ((other & signature) == signature) {
                    result.insert(element);
                }
            }

            return result;
        }

        void registerComponent(T element, size_t index) {
            this->signatures.at(element).set(index);
        }

        void unregisterComponent(T element, size_t index) {
            this->signatures.at(element).reset(index);
        }
    };

    SignedManager<EntityId> entityManager;
    SignedManager<SystemId> systemManager;

    std::map<std::type_index, std::unique_ptr<ComponentListBase>> componentMap;

    std::unordered_map<std::type_index, size_t> componentIndices;

    std::unordered_map<SystemId, std::shared_ptr<System>> systems;

    ECSManager(const ECSManager &) = delete;
    ECSManager &operator=(const ECSManager &) = delete;

    template <typename T> void registerType(size_t index) {
        componentMap[std::type_index(typeid(T))] =
            std::make_unique<ComponentList<T>>();
        componentIndices[std::type_index(typeid(T))] = index;
    }

    template <typename Tuple, size_t... Is>
    void registerAll(std::index_sequence<Is...>) {
        (registerType<std::tuple_element_t<Is, Tuple>>(Is), ...);
    }

    template <typename T> ComponentList<T> &getList() {
        return static_cast<ComponentList<T> &>(
            *componentMap.at(std::type_index(typeid(T))));
    }

    void entityComponentsChanged(EntityId entity);

    ECSManager();

    inline static ECSManager &getManager() {
        static ECSManager instance;
        return instance;
    }

  public:
    static EntityId generateEntityId();
    static void removeEntity(EntityId entity);

    template <typename T>
    static void setComponentToEntity(T &&component, EntityId entity) {
        ECSManager &ecs = ECSManager::getManager();
        static_assert(is_in_tuple<T, ComponentTypes>::value,
                      "Component type T is not registered in ComponentTypes. "
                      "Did you forget to add it to the tuple in Component.h?");
        using C = std::remove_reference_t<T>;
        ecs.getList<C>().set(std::forward<T>(component), entity);
        ecs.entityManager.registerComponent(entity,
                                            ecs.componentIndices.at(typeid(C)));
        ecs.entityComponentsChanged(entity);
    }

    template <typename T>
    static void removeComponentFromEntity(EntityId entity) {
        ECSManager &ecs = ECSManager::getManager();
        static_assert(is_in_tuple<T, ComponentTypes>::value,
                      "Component type T is not registered in ComponentTypes. "
                      "Did you forget to add it to the tuple in Component.h?");
        ecs.getList<T>().remove(entity);
        ecs.entityManager.unregisterComponent(
            entity, ecs.componentIndices.at(typeid(T)));
        ecs.entityComponentsChanged(entity);
    }

    template <typename T>
    static std::optional<std::reference_wrapper<T>>
    getComponentOfEntity(EntityId entity) {
        ECSManager &ecs = ECSManager::getManager();
        static_assert(is_in_tuple<T, ComponentTypes>::value,
                      "Component type T is not registered in ComponentTypes. "
                      "Did you forget to add it to the tuple in Component.h?");
        return ecs.getList<T>().get(entity);
    }

    static SystemId registerSystem(std::shared_ptr<System> system);

    template <typename T>
    static void registerComponentToSystem(SystemId system) {
        ECSManager &ecs = ECSManager::getManager();
        ecs.systemManager.registerComponent(system,
                                            ecs.componentIndices.at(typeid(T)));
        ECS::Signature signature = ecs.systemManager.getSignature(system);
        std::set<EntityId> compatible =
            ecs.entityManager.getAllCompatible(signature);
        ecs.systems[system]->setEntities(compatible);
    }

    static void removeSystem(SystemId system);
};

#endif // ECS_MANAGER
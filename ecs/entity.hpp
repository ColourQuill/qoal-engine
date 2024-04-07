#pragma once

#include "components/components.hpp"

#include <vector>
#include <memory>
#include <type_traits>
#include <stdexcept>
#include <iostream>

namespace ecs {
    class Entity {
        private:

        public:
        std::vector<std::shared_ptr<Component>> components;

        Entity() {}

        template <class T, typename... Args, typename = std::enable_if_t<std::is_base_of<Component, T>::value>>
        T& AddComponent(Args&&... args) {
            std::shared_ptr<T> new_component = std::make_shared<T>(std::forward<Args>(args)...);
            for (auto const &kv : components) {
                if (typeid(*kv) == typeid(T)) {
                    std::cout << "Cannot add a second copy of the same component to entity." << std::endl;
                    throw std::runtime_error("Cannot add a second copy of the same component to entity.");
                }
            }
            components.push_back(std::move(new_component));
            return *new_component;
        }

        template <class T, typename = std::enable_if_t<std::is_base_of<Component, T>::value>>
        T& GetComponent() {
            for (int i = 0; i < components.size(); i++) {
                auto& kv = components.at(i);
                if (typeid(*kv) == typeid(T)) {
                    return *std::dynamic_pointer_cast<T>(kv);
                }
            }
            throw std::runtime_error("Could not find component in entity.");
        }
        
        template <class T, typename = std::enable_if_t<std::is_base_of<Component, T>::value>>
        bool HasComponent() {
            for (int i = 0; i < components.size(); i++) {
                auto& kv = components.at(i);
                if (typeid(*kv) == typeid(T)) {
                    return true;
                }
            }
            return false;
        }
    };
}
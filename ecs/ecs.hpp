#pragma once

#include "entity.hpp"

#include <memory>
#include <vector>
#include <unordered_map>
#include <iostream>

namespace ecs {
    class EntityManager {
        public:
        std::vector<std::shared_ptr<Entity>> entities;

        Entity& AddEntity(std::shared_ptr<Entity> entity = std::make_shared<Entity>()) {
            entities.push_back(entity);
            return *entities.back();
        }
        void RemoveEntity(Entity& entity) {
            int i = 0;
            for (auto& e : entities) {
                if (&(*e) == &entity) {
                    entities.erase(entities.begin() + i);
                }
                i++;
            }
        }
        void RemoveEntity(size_t id) {
            if (entities.size() < id) {
                throw std::runtime_error("Entity does not exist at specified id.");
            }
            entities.erase(entities.begin() + id);
        }

        private:
    };
}
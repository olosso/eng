#pragma once

#include <glm/gtc/matrix_transform.hpp>

#include "teng_model.hpp"
#include <memory>

namespace teng {

    struct RigidBodyComponent {
        glm::vec3 velocity{0.f};
        float mass{1.0f};
    };

    struct TransformComponent {
        glm::vec3 translation{};
        glm::vec3 scale{1.f, 1.f, 1.f};
        glm::vec3 rotation{};

        // A transformation matrix transform * rotate.y * rotate.x * rotate.z * scale.
        // Uses Tait-Bryan angles
        glm::mat4 mat4();
        glm::mat3 normalMatrix();
    };

    class GameObject {

        public:
            using id_t = unsigned int;

            static GameObject CreateGameObject() {
                static id_t id = 0;
                return GameObject{id++};
            };

            static GameObject CreateGameObject(float mass) {
                static id_t id = 0;
                return GameObject{id++, mass};
            };

            // Delete copy constructor.
            // We don't want GameObjects to unnecessarily profilate.
            GameObject(const GameObject&) = delete;
            GameObject &operator=(const GameObject&) = delete;

            // Move constructor
            // By default assignment or use of as an argument implies move.
            GameObject(GameObject&&) = default;
            GameObject &operator=(GameObject&&) = default;

            // Getters
            float getMass() { return p_RigidBody.mass; };
            id_t getId() const {
                return id;
            };

            // Data model
            std::shared_ptr<Model> model{};
            glm::vec3 color{};
            TransformComponent p_Transform;
            RigidBodyComponent p_RigidBody;

        private:

            // Private constructor so that we can control the id's of game objects.
            GameObject(id_t objId) : id{objId} {};

            GameObject(id_t objId, float mass) : id{objId} {
                p_RigidBody.mass = mass;
            };

            id_t id;
    };
}

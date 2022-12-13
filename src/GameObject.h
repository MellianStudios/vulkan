#ifndef MELLIANCLIENT_GAMEOBJECT_H
#define MELLIANCLIENT_GAMEOBJECT_H

#include <memory>
#include "Model.h"

struct Transform2dComponent
{
    glm::vec2 translation{};
    glm::vec2 scale{1.f, 1.f};
    float rotation;

    glm::mat2 mat2()
    {
        const float s = glm::sin(rotation);
        const float c = glm::cos(rotation);

        glm::mat2 rotation_matrix{{c,  s},
                                  {-s, c}};

        glm::mat2 scale_matrix{
            {scale.x, .0f},
            {.0f,     scale.y}
        };

        return rotation_matrix * scale_matrix;
    }
};

class GameObject
{
public:
    using id_t = unsigned int;
    std::shared_ptr<Model> model{};
    glm::vec3 color{};
    Transform2dComponent transform_2d;

    GameObject(const GameObject &) = delete;

    GameObject &operator=(const GameObject &) = delete;

    GameObject(GameObject &&) = default;

    GameObject &operator=(GameObject &&) = default;

    static GameObject createGameObject()
    {
        static id_t current_id = 0;

        return GameObject{current_id++};
    }

    id_t getId() const
    {
        return id;
    }

private:
    id_t id;

    GameObject(id_t object_id) : id{object_id}
    {

    }
};

#endif //MELLIANCLIENT_GAMEOBJECT_H

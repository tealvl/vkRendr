#include <glm/glm.hpp>

namespace rendr{
struct Camera
{   
    glm::vec3 up {0.0f, 0.0f, 1.0f};
    glm::vec3 pos{1.0f, 1.0f, 1.0f};
    glm::vec3 look_at_pos{0.0f, 0.0f, 0.0f};

    float fov = 45.0f;
    float aspect = 16.0f / 9.0f;
    float near_plane = 0.1f;
    float far_plane = 10.0f;
};
}
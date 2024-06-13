#pragma once

#include <glm/glm.hpp>

class Camera
{
public:
    Camera();

    const glm::mat4 &GetViewMatrix() const;
    const glm::mat4 &GetProjectionMatrix() const;

    void Update(float dt);

    void SetCameraPos(const glm::vec3 &newPos);
    void SetCameraPosX(float x);

    void LookAt(const glm::vec3 &center);
    void LookAtX(float x);

private:
    glm::mat4 _ViewMatrix = glm::mat4(1.0f);
    glm::mat4 _ProjMatrix = glm::mat4(1.0f);

    glm::vec3 _Pos = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 _Center = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 _Front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 _WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);

    const float _Speed = 5.0f;
    const float _MouseSensitivity = 0.08f;
    const float _Fov = 45.0f;

    float _Yaw = -90.0f, _Pitch = 0.0f;
};

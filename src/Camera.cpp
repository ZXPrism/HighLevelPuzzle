#include "Camera.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Application.h"
#include "Config.h"

Camera::Camera()
{
#if 0
    gApp.RegisterOnCursorPosFunc([&](double xPos, double yPos) {
        static double xpos = 0.0, ypos = 0.0;

        if (glfwGetMouseButton(gApp.GetWindowHandle(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        { // mainly from LearnOpenGL :)
            double dx = xPos - xpos, dy = ypos - yPos;

            dx *= _MouseSensitivity;
            dy *= _MouseSensitivity;

            _Yaw += dx;
            _Pitch += dy;

            if (_Pitch > 89.0f)
                _Pitch = 89.0f;
            if (_Pitch < -89.0f)
                _Pitch = -89.0f;

            glm::vec3 front;
            front.x = cos(glm::radians(_Yaw)) * cos(glm::radians(_Pitch));
            front.y = sin(glm::radians(_Pitch));
            front.z = sin(glm::radians(_Yaw)) * cos(glm::radians(_Pitch));
            _Front = glm::normalize(front);

            xpos = xPos;
            ypos = yPos;
        }

        xpos = xPos;
        ypos = yPos;
    });
#endif
    gApp.RegisterOnWindowSizeFunc(
        [&](int width, int height) { _ProjMatrix = glm::perspective(_Fov, 1.0f * width / height, 0.1f, 100.0f); });

    _ProjMatrix = glm::perspective(_Fov, 1.0f * cWindowWidth / cWindowHeight, 0.1f, 100.0f);
}

const glm::mat4 &Camera::GetViewMatrix() const
{
    return _ViewMatrix;
}

const glm::mat4 &Camera::GetProjectionMatrix() const
{
    return _ProjMatrix;
}

void Camera::SetCameraPos(const glm::vec3 &newPos)
{
    _Pos = newPos;
}

void Camera::LookAt(const glm::vec3 &center)
{
    _Front = glm::normalize(center - _Pos);
}

void Camera::Update(float dt)
{
    auto window = gApp.GetWindowHandle();
    float speed = _Speed * dt;

    auto right = glm::normalize(glm::cross(_Front, _WorldUp));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        _Pos += speed * _Front;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        _Pos -= speed * _Front;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        _Pos -= right * speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        _Pos += right * speed;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        _Pos += glm::normalize(glm::cross(right, _Front)) * speed;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        _Pos -= glm::normalize(glm::cross(right, _Front)) * speed;

    _ViewMatrix = glm::lookAt(_Pos, _Pos + _Front, _WorldUp);
}

//
// OpenGL framework and demo boilerplate
// (c) 2026 by Sergei Kuratiov. MIT License
//

#include "framework.h"
#include "Camera.h"

//
//  Globals
//
Camera* Camera::m_pInstance = nullptr;

// Constructor
Camera::Camera() {
    updateVectors();
}

void Camera::onKeyDown(int key) {
    switch (key) {
        case 'W': m_moveDir.z = -1.f; break;
        case 'S': m_moveDir.z = 1.f; break;
        case 'A': m_moveDir.x = -1.f; break;
        case 'D': m_moveDir.x = 1.f; break;
        case VK_SPACE: m_moveDir.y = 1.f; break;
        case VK_SHIFT: m_moveDir.y = -1.f; break;
    }
}

void Camera::onKeyUp(int key) {
    switch (key) {
        case 'W':
        case 'S': m_moveDir.z = 0.f; break;

        case 'A':
        case 'D': m_moveDir.x = 0.f; break;

        case VK_SPACE:
        case VK_SHIFT: m_moveDir.y = 0.f; break;
    }
}

void Camera::onMouseMove(float dx, float dy, float frameTime) {
    float alpha = 1.f - expf(-m_smoothFactorMouse * frameTime);
    m_mouseDX = glm::mix(m_mouseDX, dx, alpha);
    m_mouseDY = glm::mix(m_mouseDY, dy, alpha);

    m_yaw += m_mouseDX * m_sensitivity;
    m_pitch += m_mouseDY * m_sensitivity;

    m_pitch = glm::clamp(m_pitch, -89.f, 89.f);

    updateVectors();
}

void Camera::setRoll(float r) {
    m_roll = r;
    updateVectors();
}

void Camera::addRoll(float dr) {
    m_roll += dr;
    updateVectors();
}

void Camera::updateOnControls(float frameTime) {
    glm::vec3 targetVelocity = m_moveDir * m_speed;

    float alpha = 1.f - expf(-m_smoothFactorMove * frameTime);
    m_velocity = glm::mix(m_velocity, targetVelocity, alpha);

    m_position += m_front * m_velocity.z * frameTime;
    m_position += m_right * m_velocity.x * frameTime;
    m_position += m_worldUp * m_velocity.y * frameTime;
}

void Camera::updateVectors() {
    // front from yaw/pitch
    glm::vec3 f;
    f.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    f.y = sin(glm::radians(m_pitch));
    f.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));

    m_front = glm::normalize(f);

    // base right/up
    glm::vec3 r = glm::normalize(glm::cross(m_front, m_worldUp));
    glm::vec3 u = glm::normalize(glm::cross(r, m_front));

    // apply roll (rotate around front axis)
    if (m_roll != 0.f) {
        glm::mat4 rollMat = glm::rotate(
            glm::mat4(1.f),
            glm::radians(m_roll),
            m_front
        );

        r = glm::normalize(glm::vec3(rollMat * glm::vec4(r, 0.f)));
        u = glm::normalize(glm::vec3(rollMat * glm::vec4(u, 0.f)));
    }

    m_right = r;
    m_up = u;
}

void Camera::getViewMatrixUpdated(glm::mat4& view) const {
    view = glm::mat4(1.f);

    // Right
    view[0][0] = m_right.x;
    view[1][0] = m_right.y;
    view[2][0] = m_right.z;

    // Up
    view[0][1] = m_up.x;
    view[1][1] = m_up.y;
    view[2][1] = m_up.z;

    // Forward (OpenGL = -Z)
    view[0][2] = -m_front.x;
    view[1][2] = -m_front.y;
    view[2][2] = -m_front.z;

    // Translation
    view[3][0] = -glm::dot(m_right, m_position);
    view[3][1] = -glm::dot(m_up, m_position);
    view[3][2] = glm::dot(m_front, m_position);
    view[3][3] = 1.f;
}

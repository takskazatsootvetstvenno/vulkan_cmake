#include "Camera.h"

#include "Logger.h"

namespace sge {
Camera::Camera() {}

void Camera::setOrtographicProjection(float left, float right, float top, float bottom, float near, float far) {
    m_projectionMatrix = glm::mat4{1.0f};
    m_projectionMatrix[0][0] = 2.f / (right - left);
    m_projectionMatrix[1][1] = 2.f / (bottom - top);
    m_projectionMatrix[2][2] = 1.f / (far - near);
    m_projectionMatrix[3][0] = -(right + left) / (right - left);
    m_projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
    m_projectionMatrix[3][2] = -near / (far - near);
}

void Camera::setPerspectiveProjection(float fovy, float aspect, float near, float far) {
    m_projectionMatrix = glm::perspective(fovy, aspect, near, far);
}

void Camera::changeZoom(float add_value) { m_zoom += add_value; }

void Camera::loadDefaultCircleCamera() {
    m_projectionMatrix = glm::mat4{1.f};
    m_viewMatrix = glm::mat4{1.f};
    m_circle_pos = 0.f;
    m_circle_radius = 1.f;
    m_circle_height = 0.f;
    m_zoom = 45.f;
    setViewCircleCamera(20.f, 5.f);
}

const glm::vec3 Camera::getCameraPos() const noexcept { return m_cameraPos; }

const glm::mat4& Camera::getProjection() const noexcept { return m_projectionMatrix; }

void Camera::setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up) {
    m_cameraPos = position;
    m_viewMatrix = glm::lookAt(position, target, up);
}

void Camera::setViewCircleCamera(const float radius, const float height) {
    m_circle_radius = radius;
    m_circle_height = height;
    setViewTarget(glm::vec3(m_circle_radius * cos(m_circle_pos), m_circle_height, m_circle_radius * sin(m_circle_pos)),
                  glm::vec3(0.f, 0.f, 0.f));
}

void Camera::changeCirclePos(const float add_value) {
    m_circle_pos += add_value;
    setViewTarget(glm::vec3(m_circle_radius * cos(m_circle_pos), m_circle_height, m_circle_radius * sin(m_circle_pos)),
                  glm::vec3(0.f, 0.f, 0.f));
}

void Camera::changeCircleHeight(const float add_value) {
    m_circle_height += add_value;
    setViewTarget(glm::vec3(m_circle_radius * cos(m_circle_pos), m_circle_height, m_circle_radius * sin(m_circle_pos)),
                  glm::vec3(0.f, 0.f, 0.f));
}

const glm::mat4& Camera::getView() const noexcept { return m_viewMatrix; }
const float Camera::getZoom() const noexcept { return m_zoom; }
}  // namespace sge

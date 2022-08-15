#include "Camera.h"
#include "Logger.h"
namespace sge
{
	Camera::Camera()
	{

	}

	void Camera::setOrtographicProjection(float left, float right, float top, float bottom, float near, float far)
	{
		m_projectionMatrix = glm::mat4{ 1.0f };
		m_projectionMatrix[0][0] = 2.f / (right - left);
		m_projectionMatrix[1][1] = 2.f / (bottom - top);
		m_projectionMatrix[2][2] = 1.f / (far - near);
		m_projectionMatrix[3][0] = -(right + left) / (right - left);
		m_projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
		m_projectionMatrix[3][2] = -near / (far - near);
	}

	void Camera::setPerspectiveProjection(float fovy, float aspect, float near, float far)
	{
		//assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
		const float tanHalfFovy = tan(fovy / 2.f);
		m_projectionMatrix = glm::mat4{ 0.0f };
		m_projectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
		m_projectionMatrix[1][1] = 1.f / (tanHalfFovy);
		m_projectionMatrix[2][2] = far / (far - near);
		m_projectionMatrix[2][3] = 1.f;
		m_projectionMatrix[3][2] = -(far * near) / (far - near);
	}

	void Camera::changeZoom(float add_value)
	{
		m_zoom += add_value;
	}

	void Camera::loadDefaultCircleCamera()
	{
		m_projectionMatrix = glm::mat4{ 1.f };
		m_viewMatrix = glm::mat4{ 1.f };
		m_circle_pos = 0.f;
		m_circle_radius = 1.f;
		m_circle_height = 0.f;
		m_zoom = 45.f;
		setViewCircleCamera(20.f, 5.f);
	}

	const glm::vec3 Camera::getCameraPos() const noexcept
	{
		return m_cameraPos;
	}

	const glm::mat4& Camera::getProjection() const
	{
		return m_projectionMatrix;
	}
	void Camera::setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up) {
		m_cameraPos = position;
		const glm::vec3 w{ glm::normalize(direction) };
		const glm::vec3 u{ glm::normalize(glm::cross(w, up)) };
		const glm::vec3 v{ glm::cross(w, u) };

		m_viewMatrix = glm::mat4{ 1.f };
		m_viewMatrix[0][0] = u.x;
		m_viewMatrix[1][0] = u.y;
		m_viewMatrix[2][0] = u.z;
		m_viewMatrix[0][1] = v.x;
		m_viewMatrix[1][1] = v.y;
		m_viewMatrix[2][1] = v.z;
		m_viewMatrix[0][2] = w.x;
		m_viewMatrix[1][2] = w.y;
		m_viewMatrix[2][2] = w.z;
		m_viewMatrix[3][0] = -glm::dot(u, position);
		m_viewMatrix[3][1] = -glm::dot(v, position);
		m_viewMatrix[3][2] = -glm::dot(w, position);
	}

	void Camera::setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up) {
		setViewDirection(position, target - position, up);
	}

	void Camera::setViewYXZ(glm::vec3 position, glm::vec3 rotation) {
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
		const glm::vec3 u{ (c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1) };
		const glm::vec3 v{ (c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3) };
		const glm::vec3 w{ (c2 * s1), (-s2), (c1 * c2) };
		m_viewMatrix = glm::mat4{ 1.f };
		m_viewMatrix[0][0] = u.x;
		m_viewMatrix[1][0] = u.y;
		m_viewMatrix[2][0] = u.z;
		m_viewMatrix[0][1] = v.x;
		m_viewMatrix[1][1] = v.y;
		m_viewMatrix[2][1] = v.z;
		m_viewMatrix[0][2] = w.x;
		m_viewMatrix[1][2] = w.y;
		m_viewMatrix[2][2] = w.z;
		m_viewMatrix[3][0] = -glm::dot(u, position);
		m_viewMatrix[3][1] = -glm::dot(v, position);
		m_viewMatrix[3][2] = -glm::dot(w, position);
	}

	void Camera::setViewCircleCamera(const float radius, const float height)
	{
		m_circle_radius = radius;
		m_circle_height = height;
		setViewTarget(
			glm::vec3(m_circle_radius * cos(m_circle_pos), m_circle_height, m_circle_radius * sin(m_circle_pos)),
			glm::vec3(0.f, 0.f, 0.f));
	}

	void Camera::changeCirclePos(const float add_value)
	{
		
		m_circle_pos += add_value;
		setViewTarget(
			glm::vec3(m_circle_radius * cos(m_circle_pos), m_circle_height, m_circle_radius * sin(m_circle_pos)),
			glm::vec3(0.f, 0.f, 0.f));
	}

	void Camera::changeCircleHeight(const float add_value)
	{
		m_circle_height += add_value;
		setViewTarget(
			glm::vec3(m_circle_radius * cos(m_circle_pos), m_circle_height, m_circle_radius * sin(m_circle_pos)),
			glm::vec3(0.f, 0.f, 0.f));
	}

	const glm::mat4& Camera::getView() const
	{
		return m_viewMatrix;
	}
	const float Camera::getZoom() const
	{
		return m_zoom;
	}
}
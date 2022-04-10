#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace sge {
	class Camera
	{
	public:
		Camera();
		void setOrtographicProjection(float left, float right, float top, float bottom, float near, float far);
		void setPerspectiveProjection(float fovy, float aspect, float near, float far);
		void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3(0.f, -1.f, 0.f));
		void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3(0.f, -1.f, 0.f));
		void setViewYXZ(glm::vec3 position, glm::vec3 rotation);
		void setViewCircleCamera(const float radius, const float height);
		void changeCirclePos(float add_value);
		void changeCircleHeight(float add_value);
		void changeZoom(float add_value);
		void loadDefaultCircleCamera();
		const glm::mat4& getProjection() const;
		const glm::mat4& getView() const;
		const float getZoom() const;
	private:
		glm::mat4 m_projectionMatrix{ 1.f };
		glm::mat4 m_viewMatrix{ 1.f };
		float m_circle_pos = 0.f;
		float m_circle_radius = 1.f;
		float m_circle_height = 0.f;
		float m_zoom = 45.f;
	};
}
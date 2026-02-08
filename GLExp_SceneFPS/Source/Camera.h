//
// OpenGL framework and demo boilerplate
// (c) 2026 by Sergei Kuratiov. MIT License
//

#pragma once

class Camera {
public:
	static Camera* getInstance() {
		return (!m_pInstance) ?
			m_pInstance = new Camera() : m_pInstance;
	}

	Camera(const Camera&) = delete;
	virtual ~Camera() = default;
	
    void onKeyDown(int key);
    void onKeyUp(int key);

    void setRoll(float r);
    void addRoll(float dr);

    void onMouseMove(float dx, float dy, float frameTime);
    void updateOnControls(float frameTime);
      
    void getViewMatrixUpdated(glm::mat4& view) const;

protected:
	Camera();

private:
	static Camera* m_pInstance;

    // Position / rotation
    glm::vec3 m_position{ 0.0f, 0.0f, 0.0f };

    float m_yaw = -90.f;
    float m_pitch = 0.f;
    float m_roll = 0.f;

    // Direction vectors
    glm::vec3 m_front{ 0.f, 0.f, -1.f };
    glm::vec3 m_right{ 1.f, 0.f, 0.f };
    glm::vec3 m_up{ 0.f, 1.f, 0.f };
    glm::vec3 m_worldUp{ 0.f, 1.f, 0.f };

    // Movement
    glm::vec3 m_moveDir{ 0.f };

    float m_speed = 5.f;
    float m_sensitivity = 0.1f;

	// Smoothing
    float m_mouseDX = 0.f;
    float m_mouseDY = 0.f;

    glm::vec3 m_velocity{ 0.f };
    float m_smoothFactorMouse = 20.f;   
    float m_smoothFactorMove = 10.f;  

    void updateVectors();
};


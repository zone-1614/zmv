#pragma once
#include <cmath>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class CameraMovement {
    Left, Right, Up, Down,
    Forward, Backward
};

class Camera {
public:
    glm::vec3 camera_position;
    glm::vec3 camera_forward;
    glm::vec3 camera_right;
    glm::vec3 camera_up;

    float fov;
    float movement_speed;
    float look_around_speed;
    float phi;
    float theta;

    Camera() : 
        camera_position{0.0f},
        camera_forward{0.0f, 0.0f, -1.0f},
        camera_right{1.0f, 0.0f, 0.0f},
        camera_up{0.0f, 1.0f, 0.0f},
        fov(60.0f),
        movement_speed(1.5f),
        look_around_speed(1.0f),
        phi(270.0f),
        theta(90.0f) { }

    glm::mat4 compute_view_matrix() const {
        return glm::lookAt(camera_position, camera_position + camera_forward, camera_up);
    }

    glm::mat4 compute_projection_matrix(int width, int height) const {
        return glm::perspective(glm::radians(fov), static_cast<float>(width)/height, 0.1f, 10000.0f);
    }

    void reset() {
        camera_position = glm::vec3(0.0f);
        camera_forward = glm::vec3(0.0f, 0.0f, -1.0f);
        camera_right = glm::vec3(1.0f, 0.0f, 0.0f);
        camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
        phi = 270.0f;
        theta = 90.0f;
    }

    void move(const CameraMovement &direction, float deltaTime) {
        const float distance = movement_speed * deltaTime;
        switch (direction) {
            case CameraMovement::Left:
                camera_position -= distance * camera_right;
                break;
            case CameraMovement::Right:
                camera_position += distance * camera_right;
                break;
            case CameraMovement::Up:
                camera_position += distance * glm::vec3(0.0f, 1.0f, 0.0f);
                break;
            case CameraMovement::Down:
                camera_position -= distance * glm::vec3(0.0f, 1.0f, 0.0f);
                break;
            case CameraMovement::Forward:
                camera_position += distance * camera_forward;
                break;
            case CameraMovement::Backward: 
                camera_position -= distance * camera_forward;
                break;
        }
    }

    void look_around(float dPhi, float dTheta) {
        phi += look_around_speed * dPhi;
        if (phi < 0.0f) phi = 360.0f;
        if (phi > 360.0f) phi = 0.0f;
        
        theta += look_around_speed * dTheta;
        if (theta < 1.0f) theta = 1.0f;
        if (theta > 180.0f) theta = 180.0f;

        const float phi_radians = glm::radians(phi);
        const float theta_radians = glm::radians(theta);
        camera_forward = glm::vec3(
            std::cos(phi_radians) * std::sin(theta_radians),
            std::cos(theta_radians), 
            std::sin(phi_radians) * std::sin(theta_radians)
        );
        camera_right = glm::normalize(glm::cross(camera_forward, glm::vec3(0.0f, 1.0f, 0.0f)));
        camera_up = glm::normalize(glm::cross(camera_right, camera_forward));
    }

};
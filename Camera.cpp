#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace gps {

    // Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp)
        : cameraPosition(cameraPosition), yaw(-90.0f), pitch(0.0f) {
        cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUp));
        cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));
    }

    // Get the view matrix
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraPosition + cameraFrontDirection, cameraUpDirection);
    }
    glm::vec3 Camera::getFrontDirection() {
        return cameraFrontDirection;
    }

    // Update camera movement
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        if (direction == MOVE_FORWARD)
            cameraPosition += speed * cameraFrontDirection;
        if (direction == MOVE_BACKWARD)
            cameraPosition -= speed * cameraFrontDirection;
        if (direction == MOVE_LEFT)
            cameraPosition -= cameraRightDirection * speed;
        if (direction == MOVE_RIGHT)
            cameraPosition += cameraRightDirection * speed;
    }

    // Update camera rotation
    void Camera::rotate(float pitchDelta, float yawDelta) {
        yaw += yawDelta;
        pitch += pitchDelta;

        // Constrain pitch
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        updateCameraVectors();
    }
    void Camera::setTarget(glm::vec3 newTarget) {
        cameraFrontDirection = glm::normalize(newTarget - cameraPosition);
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
        cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));
    }
    

    // Update camera vectors
    void Camera::updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFrontDirection = glm::normalize(front);

        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
        cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));
    }

    // Get the current position of the camera
    glm::vec3 Camera::getPosition() {
        return cameraPosition;
    }

    // Set a new position for the camera
    void Camera::setPosition(glm::vec3 newPosition) {
        cameraPosition = newPosition;
    }
}

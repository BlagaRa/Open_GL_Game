#ifndef Camera_hpp
#define Camera_hpp

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

namespace gps {

    enum MOVE_DIRECTION { MOVE_FORWARD, MOVE_BACKWARD, MOVE_RIGHT, MOVE_LEFT };

    class Camera {

    public:
        // Camera constructor
        Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp);

        // Get the view matrix
        glm::mat4 getViewMatrix();

        // Update camera movement
        void move(MOVE_DIRECTION direction, float speed);

        // Update camera rotation
        void rotate(float pitch, float yaw);

        // Get the current position of the camera
        glm::vec3 getPosition();

        // Set a new position for the camera
        void setPosition(glm::vec3 newPosition);

        // Set a new target for the camera
        void setTarget(glm::vec3 newTarget);
        glm::vec3 getFrontDirection();


    private:
        glm::vec3 cameraPosition;
        glm::vec3 cameraFrontDirection;
        glm::vec3 cameraRightDirection;
        glm::vec3 cameraUpDirection;

        float yaw;    // Rotation around Y-axis
        float pitch;  // Rotation around X-axis

        // Helper to recalculate camera vectors
        void updateCameraVectors();
    };
}

#endif /* Camera_hpp */

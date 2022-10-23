#ifndef CAMERA_H
#define CAMERA_H

#include"glm.hpp"
#include"opengl.hpp"
#include <vector>
#include"component.hpp"
#include"transform.hpp"
// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera;

static Camera* mainCamera;
// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera:public Component{
public:
    // camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // euler Angles
    float Yaw;
    float Pitch;
    // camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
    Transform* trans;
    float Fov=45;
    float w=SCR_WIDTH;
    float h=SCR_HEIGHT;
    float near = 0.1f;
    float far = 100.f;
    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        name = "camera";
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }
    // constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    void* operator new(std::size_t size) throw(std::bad_alloc) {
        auto addr = std::malloc(size);
        if (mainCamera == NULL)
            mainCamera = (Camera*)addr;
        return addr;
    }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {

        return glm::lookAt(Vector3f(0,0,0), Front, Up);
    }

    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        if (trans == NULL) {
            trans=object->GetComponent<Transform>();
            if (trans == NULL) {
                trans=object->AddComponent<Transform>();
            }
                
        }
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            trans->Translate(Front * velocity);
        if (direction == BACKWARD)
            trans->Translate(-Front * velocity);
        if (direction == LEFT)
            trans->Translate(-Right * velocity);
        if (direction == RIGHT)
            trans->Translate(Right * velocity);
    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }

    glm::mat4 GetModel() {
        if (trans == NULL) {
            trans=object->GetComponent<Transform>();
            if (trans == NULL) {
                trans=object->AddComponent<Transform>();
            }
                
        }
        glm::mat4 mat(1.f);
        mat = glm::translate(mat, -trans->GetPosition());
        return mat;
    }

    glm::mat4 GetView() {
        /*return glm::lookAt(trans->GetPosition(), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));*/
        return GetViewMatrix();
    }

    glm::mat4 GetProjection() {
        return glm::perspective(glm::radians(Fov), float(w) / h, near, far);
    }

    glm::mat4 GetMVP() {
        if (object == NULL)
            throw std::runtime_error("CAMERA ATTACH NO OBJECT!\n");
        glm::mat4 mat(1.f);
        mat = GetProjection()* GetView()* GetModel();
        return mat;
    }

    void SetFront(float x, float y, float z) {
        Front = { x,y,z };
        Front = glm::normalize(Front);
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }

    void SetFront(glm::vec3 f) {
        Front = f;
        Front = glm::normalize(Front);
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }

private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up = glm::normalize(glm::cross(Right, Front));
    }

    
    
    
};
#endif
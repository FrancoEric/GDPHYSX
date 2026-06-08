#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;
using namespace glm;


class Camera
{
protected:
    mat4 proj;
    vec3 camPos;
    vec3 worldUp;
    vec3 camCenter;
    mat4 view;
    float yaw = -90.0f;
    float pitch = 0.0f;
    vec3 front = vec3(0.f, 0.f, -1.f);

    // window size (needed for projection)
    float windowWidth;
    float windowHeight;

public:

    Camera(float w, float h)
    {
        windowWidth = w;
        windowHeight = h;
        
        camPos = vec3(0.f, 0.f, 10.f); //starting pos
        worldUp = vec3(0.f, 1.0f, 0.f); //constant
        camCenter = vec3(0.f, 3.f, 0.f); //starting pos where the cam is looking at
    }
    

    virtual ~Camera() {}
    // forces child classes to define their own projection
    virtual void updateProjection() = 0;

    
    // GETTERS

    vec3 getCamPos()
    {
        return camPos;
    }

    vec3 getCamFront()
    {
        return front;
    }

    mat4 getView()
    {
        return view;
	}

    mat4 getProjection()
    {
        return proj;
	}

    // MOVEMENT FUNCTIONS (shared by both camera types)
    void moveForward(float speed)
    {
        camPos += front * speed;
        camCenter = camPos + front;
    }

    void moveRight(float speed)
    {
        vec3 right = normalize(cross(front, worldUp));
        camPos += right * speed;
        camCenter = camPos + front;
    }

    void moveUp(float speed)
    {
        camPos += worldUp * speed;
        camCenter = camPos + front;
    }


    virtual void rotateCam(float xoffset, float yoffset)
    {
        float sensitivity = 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;
        pitch += yoffset;

        // prevent flip
        if (pitch > 89.0f)  pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;

        // recalculate forward direction
        vec3 direction;
        direction.x = cos(radians(yaw)) * cos(radians(pitch));
        direction.y = sin(radians(pitch));
        direction.z = sin(radians(yaw)) * cos(radians(pitch));

        front = normalize(direction);

        // update camera center position and direction
        camCenter = camPos + front;
    }


    void Update(GLuint& shaderProgram)
    {
        view = lookAt(camPos, camCenter, worldUp);

        // call correct projection based on camera type
        updateProjection();

        unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, value_ptr(proj));

        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, value_ptr(view));
    }
};


class PerspectiveCamera : public Camera
{
private:
    float fov = 60.f;
    float nearPlane = 0.1f;
    float farPlane = 1000.f;

    bool firstPerson = false;
    vec3 fpOffset = vec3(0.f, 5.0f, 0.f); // eye height offset, change as needed
    
    float radius = 15.f; // distance from object
    vec3 target = vec3(0.f); // main object position

public:
    PerspectiveCamera(float w, float h)
        : Camera(w, h)
    {
    }

    // set the object we orbit around
    void setTarget(vec3 t)
    {
        target = t + vec3(0.0f, 200.f, 0.0f);
    }

    bool isFirstPerson()
    {
        return firstPerson;
    }
    
    void setFirstPerson(bool fp)
    {
        firstPerson = fp;
    }
    
    // Override rotation to orbit instead of free move
    void rotateCam(float xoffset, float yoffset) override
    {
        float sensitivity = 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;
        pitch += yoffset;

        // limits camera from flipping
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;

        if (!firstPerson)
        {
            // orbit only in third person
            camPos.x = target.x + radius * cos(radians(yaw)) * cos(radians(pitch));
            camPos.y = target.y + radius * sin(radians(pitch));
            camPos.z = target.z + radius * sin(radians(yaw)) * cos(radians(pitch));
            camCenter = target;
        }
        // followTarget handles camPos in first person
    }

    float getYaw()
    {
        return yaw;
    }
    
    void rotateYaw(float amount)
    {
        yaw += amount;
        
        // recalculate front vector so moveForward uses the new direction
        vec3 direction;
        direction.x = cos(radians(yaw)) * cos(radians(pitch));
        direction.y = sin(radians(pitch));
        direction.z = sin(radians(yaw)) * cos(radians(pitch));
        front = normalize(direction);
        camCenter = camPos + front;
    }
    
    void followTarget(vec3 t)
    {
        target = t;

        if (firstPerson)
        {
            // place cam at tank position eye height
            camPos = target + fpOffset;
            // look in the direction yaw/pitch points
            vec3 direction;
            direction.x = cos(radians(yaw)) * cos(radians(pitch));
            direction.y = sin(radians(pitch));
            direction.z = sin(radians(yaw)) * cos(radians(pitch));
            front = normalize(direction);
            camCenter = camPos + front;
        }
        else
        {
            // orbitting camera
            camPos.x = target.x + radius * cos(radians(yaw)) * cos(radians(pitch));
            camPos.y = target.y + radius * sin(radians(pitch));
            camPos.z = target.z + radius * sin(radians(yaw)) * cos(radians(pitch));
            camCenter = target;
        }
    }
    
    void updateProjection() override
    {
        proj = perspective(
            radians(fov),
            windowWidth / windowHeight,
            nearPlane,
            farPlane
        );
    }
    void adjustFov(float amount)
    {
        fov += amount;
        if (fov < 10.f) fov = 10.f; // max zoom in
        if (fov > 120.f) fov = 120.f; // max zoom out
    }
};


class OrthographicCamera : public Camera
{
private:
    float left = -30.f;
    float rightBound = 30.f;
    float bottom = -30.f;
    float top = 30.f;

    float nearPlane = 0.1f;
    float farPlane = 1000.f;

public:
    OrthographicCamera(float w, float h) : Camera(w, h)
    {
        // position camera above the scene
        camPos = vec3(0.f, 40.f, 0.f);

        // look straight down
        camCenter = vec3(0.f, 0.f, 0.f);

        worldUp = vec3(0.f, 0.f, -1.f);
    }

    void updateProjection() override
    {
        proj = ortho(
            left,
            rightBound,
            bottom,
            top,
            nearPlane,
            farPlane
        );
    }
    
    void pan(float x, float z)
    {
        camPos.x += x;
        camPos.z += z;
        camCenter.x += x;
        camCenter.z += z;
    }
    
};

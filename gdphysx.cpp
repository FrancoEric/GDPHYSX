#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <iostream>
#include <string>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "object.h"
#include "physicsWorld.h"
#include "forceGenerator.h"
#include "forceRegistry.h"
#include "gravityForceGenerator.h"
#include "dragForceGenerator.h"
#include "anchoredSpring.h"
#include "particleSpring.h"
#include "bungeeSpring.h"
#include "particleContact.h"
#include "contactResolver.h"
#include "particleLink.h"
#include "rod.h"
#include "chain.h"
#include "anchoredChain.h"
#include "lineDrawable.h"
#include "camera.h"



//#include "phase1FireworkSpawner.h"
//#include "PC02wheel.h"

#include "chrono"
using namespace std::chrono_literals;

using namespace std;
using namespace glm;

float windowWidth = 800;
float windowHeight = 800;

bool spacePressed = false;
bool enterPressed = false;

bool useOrtho = true;

float snakeSpeed = 100.f;
float snakeHeading = 0.f; // radians
float turnSpeed = 3.f; // radians per second


void Key_Callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_SPACE && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        spacePressed = true;
    }
    else
	{
		spacePressed = false;
	}

    if (key == GLFW_KEY_ENTER && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        enterPressed = true;
    }
    else
    {
		enterPressed = false;
    }

    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
    {
        useOrtho = true;
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS)
    {
        useOrtho = false;
    }
}

GLuint loadShaders()
{
    fstream vertsrc("shaders/shader.vert");
    stringstream vertbuff;

    vertbuff << vertsrc.rdbuf();

    string vertS = vertbuff.str();
    const char* v = vertS.c_str();

    fstream fragsrc("shaders/shader.frag");
    stringstream fragbuff;

    fragbuff << fragsrc.rdbuf();

    string fragS = fragbuff.str();
    const char* f = fragS.c_str();

    //cout << "==========================" << endl;
    //cout << fragS << endl;
    //cout << "==========================" << endl;

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &v, NULL);
    glCompileShader(vertexShader);

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &f, NULL);
    glCompileShader(fragShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragShader);

    glLinkProgram(shaderProgram);

    return shaderProgram;
}

struct MeshData
{
    int startVertex;
    int vertexCount;
    GLuint texture;
    vec3 color;
	bool useTexture;
};

MeshData loadModel(const string& path, GLuint texture, bool useTexture, vec3 color, vector<float>& fullVertexData)
{
    tinyobj::attrib_t attributes;
    vector<tinyobj::shape_t> shapes;
    vector<tinyobj::material_t> materials;
    string warning, error;

    bool success = tinyobj::LoadObj(&attributes, &shapes, &materials, &warning, &error, path.c_str());
    if (!success)
        cout << "OBJ load failed: " << path << error << std::endl;

    MeshData mesh;
    mesh.color = color;
    mesh.startVertex = fullVertexData.size() / 8;
    mesh.texture = texture;
	mesh.useTexture = useTexture;

    for (int j = 0; j < shapes.size(); j++)
        for (int i = 0; i < shapes[j].mesh.indices.size(); i++)
        {
            tinyobj::index_t vData = shapes[j].mesh.indices[i];

            if (vData.texcoord_index < 0)
            {
                std::cout << "No UVs!" << std::endl;
            }

            fullVertexData.push_back(attributes.vertices[vData.vertex_index * 3]);
            fullVertexData.push_back(attributes.vertices[vData.vertex_index * 3 + 1]);
            fullVertexData.push_back(attributes.vertices[vData.vertex_index * 3 + 2]);

            fullVertexData.push_back(attributes.normals[vData.normal_index * 3]);
            fullVertexData.push_back(attributes.normals[vData.normal_index * 3 + 1]);
            fullVertexData.push_back(attributes.normals[vData.normal_index * 3 + 2]);

            fullVertexData.push_back(attributes.texcoords[vData.texcoord_index * 2]);
            fullVertexData.push_back(attributes.texcoords[vData.texcoord_index * 2 + 1]);
        }

    mesh.vertexCount = (fullVertexData.size() / 8) - mesh.startVertex;

    return mesh;
}

float askUserFloat(string text)
{
    float value;
    cout << endl << text;
    cin >> value;
	return value;
}

GLuint loadTexture(const string& path, GLenum wrapMode = GL_CLAMP_TO_EDGE)
{
    int imgWidth, imgHeight, colorChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &imgWidth, &imgHeight, &colorChannels, 0);
    if (!data)
    {
        cout << "Failed to load texture: " << path << std::endl;
        return 0;
    }

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    GLenum format = GL_RGB;
    if (colorChannels == 1)
        format = GL_RED;
    else if (colorChannels == 3)
        format = GL_RGB;
    else if (colorChannels == 4)
        format = GL_RGBA;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, format, imgWidth, imgHeight, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    return tex;
}

int main()
{
    if (!glfwInit())
        return -1;

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "PC02 Franco", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    glfwSetKeyCallback(window, Key_Callback);

    Camera* activeCamera;
    PerspectiveCamera* perspCam;
    OrthographicCamera* orthoCam;

    //16ms per frame 
    constexpr std::chrono::nanoseconds timestep(16ms);

	// Load model data ------------------------------------------------
    vector<GLfloat> fullVertexData;
    vector<GLuint> textures;
    vector<MeshData> meshes;

	textures.push_back(loadTexture("3D/green-grass.jpg"));

    //static colors, 19
    vec3 white = vec3(1.f, 1.f, 1.f);
    vec3 black = vec3(0.f, 0.f, 0.f);
    vec3 red = vec3(1.f, 0.f, 0.f);
    vec3 green = vec3(0.f, 1.f, 0.f);
    vec3 blue = vec3(0.f, 0.f, 1.f);
    vec3 yellow = vec3(1.f, 1.f, 0.f);
    vec3 cyan = vec3(0.f, 1.f, 1.f);
    vec3 magenta = vec3(1.f, 0.f, 1.f);
    vec3 orange = vec3(1.f, 0.5f, 0.f);
    vec3 purple = vec3(0.5f, 0.f, 1.f);
    vec3 pink = vec3(1.f, 0.4f, 0.7f);
    vec3 gray = vec3(0.5f, 0.5f, 0.5f);
    vec3 darkGray = vec3(0.2f, 0.2f, 0.2f);
    vec3 lightGray = vec3(0.8f, 0.8f, 0.8f);
    vec3 brown = vec3(0.6f, 0.3f, 0.1f);
    vec3 lime = vec3(0.6f, 1.f, 0.f);
    vec3 skyBlue = vec3(0.4f, 0.8f, 1.f);
    vec3 navy = vec3(0.0f, 0.0f, 0.5f);
    vec3 teal = vec3(0.f, 0.5f, 0.5f);

	//path, texture, useTexture, color, fullVertexData array
	//meshes.push_back(loadModel("3D/sphere.obj", textures[0], false, white, fullVertexData));
 //   meshes.push_back(loadModel("3D/sphere.obj", black, fullVertexData));
    meshes.push_back(loadModel("3D/sphere.obj", textures[0], false, red, fullVertexData));
    meshes.push_back(loadModel("3D/sphere.obj", textures[0], false, green, fullVertexData));
 //   meshes.push_back(loadModel("3D/sphere.obj", textures[0], false, blue, fullVertexData));
	//meshes.push_back(loadModel("3D/sphere.obj", textures[0], false, yellow, fullVertexData));
	//meshes.push_back(loadModel("3D/sphere.obj", textures[0], false, cyan, fullVertexData));
	//meshes.push_back(loadModel("3D/sphere.obj", magenta, fullVertexData));
	//meshes.push_back(loadModel("3D/sphere.obj", orange, fullVertexData));
	//meshes.push_back(loadModel("3D/sphere.obj", purple, fullVertexData));
	//meshes.push_back(loadModel("3D/sphere.obj", pink, fullVertexData));
	//meshes.push_back(loadModel("3D/sphere.obj", gray, fullVertexData));
	//meshes.push_back(loadModel("3D/sphere.obj", darkGray, fullVertexData));
	//meshes.push_back(loadModel("3D/sphere.obj", lightGray, fullVertexData));
	//meshes.push_back(loadModel("3D/sphere.obj", brown, fullVertexData));
	//meshes.push_back(loadModel("3D/sphere.obj", lime, fullVertexData));
	//meshes.push_back(loadModel("3D/sphere.obj", skyBlue, fullVertexData));
	//meshes.push_back(loadModel("3D/sphere.obj", navy, fullVertexData));
 //   meshes.push_back(loadModel("3D/sphere.obj", teal, fullVertexData));
	// end of model loading -------------------------------------------

    GLuint VBO, VAO, lineVBO, lineVAO;
    glGenVertexArrays(1, &VAO);
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &lineVBO);

    GLuint shaderProgram = loadShaders();

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * fullVertexData.size(), fullVertexData.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * 2, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    vec3 startPos = vec3(0), endPos = vec3(0);
    vec3 lineVertices[2] =
    {
        startPos,
        endPos
    };

    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(lineVertices), lineVertices);

	// Load objects ------------------------------------------------
	PhysicsWorld* world = new PhysicsWorld();
	world->setGravity(vec3(0, 0, 0));

	vector<lineDrawable*> lines;

	float appleScale = 10;
    float snakeHeadScale = 20;
	float snakeTailScale = 10;
    float mass = 10;
    float restitution = 0.9;
	float rodLength = 100;

    vector<Object*> bodySegments;
    vector<Rod*> bodyRods;
    float segmentSpacing = snakeHeadScale + snakeTailScale + 5.f;
    float eatDistance;

	float arenaHalfWidth = 400;
    vec3 arenaCorners[4] = {
        vec3(-arenaHalfWidth, -arenaHalfWidth, 0), //bot left
		vec3(arenaHalfWidth, -arenaHalfWidth, 0), //bot right
        vec3(arenaHalfWidth, arenaHalfWidth, 0), //top right
        vec3(-arenaHalfWidth, arenaHalfWidth, 0) //top left 
	};

	//position, scale, mass, restitution, mesh index
    Object* snakeHead = new Object(vec3(0, 0, 0), vec3(snakeHeadScale), mass * 10.f, restitution, 1);
	world->addParticle(snakeHead);
    Object* apple1 = new Object(vec3(100, 100, 0), vec3(appleScale), mass, restitution, 0);
    world->addParticle(apple1);
    Object* apple2 = new Object(vec3(-100, -100, 0), vec3(appleScale), mass, restitution, 0);
    world->addParticle(apple2);
    Object* apple3 = new Object(vec3(-100, 100, 0), vec3(appleScale), mass, restitution, 0);
    world->addParticle(apple3);
    Object* apple4 = new Object(vec3(100, -100, 0), vec3(appleScale), mass, restitution, 0);
    world->addParticle(apple4);
	// end of object loading ---------------------------------------

    eatDistance = snakeHeadScale + appleScale; // sum of radii, rough approximation
    vector<Object*> apples = { apple1, apple2, apple3, apple4 };

    perspCam = new PerspectiveCamera(windowWidth, windowHeight);
    orthoCam = new OrthographicCamera(windowWidth, windowHeight);

    vec3 orbitTarget = vec3(0); // orbit around the chain/ball cluster
    perspCam->setTarget(orbitTarget);
    orthoCam->followTarget(orbitTarget); // sets initial camPos based on target

    activeCamera = orthoCam; // start in ortho, matches your old view

    float viewVal = 400;
    mat4 proj = ortho(
        -viewVal, viewVal,
        -viewVal, viewVal,
        0.1f, 1000.f
    );
 //   mat4 proj = perspective(
 //       radians(45.f), //fov
 //       windowWidth / windowHeight,
 //       0.1f, 1000.f
	//);
    vec3 camPos = vec3(0.f, 0.f, -500.f);
    vec3 worldUp = vec3(0.f, 1.0f, 0.f);
    vec3 camCenter = vec3(0.f, 0.f, 0.f);
    mat4 view = lookAt(camPos, camCenter, worldUp);

    bool useManualCam = true;

	using clock = std::chrono::high_resolution_clock;
	auto currentTime = clock::now();
	auto prevTime = currentTime;
    std::chrono::nanoseconds currentNS(0);

    unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");
    unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");
    unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
    unsigned int objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor");
    GLuint tex0Adress = glGetUniformLocation(shaderProgram, "tex0");
    unsigned int useTextureLoc = glGetUniformLocation(shaderProgram, "useTexture");

    while (!glfwWindowShouldClose(window))
    {

        //  frame delta time (for steering, camera, anything per-frame)
        currentTime = clock::now();
        auto dur = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - prevTime);
        prevTime = currentTime;
        currentNS += dur;
        float deltaTime = dur.count() / (float)(1E09); // real seconds since last frame

        // snake steering 
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) 
            snakeHeading += turnSpeed * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) 
            snakeHeading -= turnSpeed * deltaTime;

        vec3 dir = vec3(cos(snakeHeading), sin(snakeHeading), 0.f);
        snakeHead->position += dir * snakeSpeed * deltaTime;

        // apple eating check 
        for (int i = 0; i < (int)apples.size();)
        {
            Object* apple = apples[i];
            float dist = glm::distance(snakeHead->position, apple->position);
            if (dist < eatDistance)
            {
                apple->isDestroyed = true;

                Object* lastSeg = bodySegments.empty() ? snakeHead : bodySegments.back();
                vec3 spawnDir = normalize(-dir);
                vec3 spawnPos = lastSeg->position + spawnDir * segmentSpacing;

                Object* newSeg = new Object(spawnPos, vec3(snakeTailScale), mass, restitution, 1);
                world->addParticle(newSeg);

                Rod* newRod = new Rod();
                newRod->particles[0] = lastSeg;
                newRod->particles[1] = newSeg;
                newRod->length = segmentSpacing;
                world->links.push_back(newRod);

                bodySegments.push_back(newSeg);
                bodyRods.push_back(newRod);

                apples.erase(apples.begin() + i); // remove now 
                continue; // don't increment i, vector shifted
            }
            i++;
        }

        if (!useManualCam)
        {
            // switch active camera based on toggle
            if (useOrtho)
                activeCamera = orthoCam;
            else
                activeCamera = perspCam;

            // WASD orbit rotation
            float orbitSpeed = 100.0f * (16.0f / 1000.0f);
            float xoffset = 0.f, yoffset = 0.f;

            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) xoffset -= orbitSpeed;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) xoffset += orbitSpeed;
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) yoffset += orbitSpeed;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) yoffset -= orbitSpeed;

            if (xoffset != 0.f || yoffset != 0.f)
                activeCamera->rotateCam(xoffset, yoffset);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);

        if (!useManualCam)
            activeCamera->Update(shaderProgram);
       
        if (useManualCam)
        {
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, value_ptr(proj));
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, value_ptr(view));
        }
       

        //physics calls
        if(currentNS >= timestep)
        {
			//cout << "physics call" << endl;
            constexpr float timestepSec = timestep.count() / (float)(1E09);
			currentNS -= timestep;

			world->update(timestepSec);
		}

		//rendering calls
        for (Object* obj : world->particles)
        {
            mat4 transform = obj->GetTransform();

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, meshes[obj->meshIndex].texture);
            glUniform1i(useTextureLoc, meshes[obj->meshIndex].useTexture);
            glUniform1i(tex0Adress, 0);

            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, value_ptr(transform));
            glUniform3fv(objectColorLoc, 1, value_ptr(meshes[obj->getMeshIndex()].color));
            glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, meshes[obj->getMeshIndex()].startVertex, meshes[obj->getMeshIndex()].vertexCount);
        }

        //rendering lines 
        for (lineDrawable* line : lines)
        {
            glDisable(GL_DEPTH_TEST);
            line->getLineData();
			lineVertices[0] = line->pos1;
            lineVertices[1] = line->pos2;

            glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(lineVertices), lineVertices);

            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, value_ptr(mat4(1.0f)));
            glUniform3fv(objectColorLoc, 1, value_ptr(line->color));

            glBindVertexArray(lineVAO);
			glDrawArrays(GL_LINES, 0, 2);
            glEnable(GL_DEPTH_TEST);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();

	return 0;
}

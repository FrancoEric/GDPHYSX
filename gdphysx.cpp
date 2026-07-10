#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

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

//#include "phase1FireworkSpawner.h"

#include "chrono"
using namespace std::chrono_literals;

using namespace std;
using namespace glm;

float windowWidth = 800;
float windowHeight = 800;

bool spacePressed = false;

void Key_Callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        spacePressed = true;
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
    vec3 color;
};

MeshData loadModel(const string& path, vec3 color, vector<float>& fullVertexData)
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
    mesh.startVertex = fullVertexData.size() / 6;

    for (int j = 0; j < shapes.size(); j++)
        for (int i = 0; i < shapes[j].mesh.indices.size(); i++)
        {
            tinyobj::index_t vData = shapes[j].mesh.indices[i];

            fullVertexData.push_back(attributes.vertices[vData.vertex_index * 3]);
            fullVertexData.push_back(attributes.vertices[vData.vertex_index * 3 + 1]);
            fullVertexData.push_back(attributes.vertices[vData.vertex_index * 3 + 2]);

            fullVertexData.push_back(attributes.normals[vData.normal_index * 3]);
            fullVertexData.push_back(attributes.normals[vData.normal_index * 3 + 1]);
            fullVertexData.push_back(attributes.normals[vData.normal_index * 3 + 2]);
        }

    mesh.vertexCount = (fullVertexData.size() / 6) - mesh.startVertex;

    return mesh;
}

float askUserFloat(string text)
{
    float value;
    cout << endl << text;
    cin >> value;
	return value;
}

int main()
{
    if (!glfwInit())
        return -1;

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Assignment4 Eric Franco", NULL, NULL);
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


    //16ms per frame 
    constexpr std::chrono::nanoseconds timestep(16ms);

	// Load model data ------------------------------------------------
    vector<GLfloat> fullVertexData;
    vector<MeshData> meshes;

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

	meshes.push_back(loadModel("3D/sphere.obj", white, fullVertexData));
 //   meshes.push_back(loadModel("3D/sphere.obj", black, fullVertexData));
 //   meshes.push_back(loadModel("3D/sphere.obj", red, fullVertexData));
 //   meshes.push_back(loadModel("3D/sphere.obj", green, fullVertexData));
 //   meshes.push_back(loadModel("3D/sphere.obj", blue, fullVertexData));
	//meshes.push_back(loadModel("3D/sphere.obj", yellow, fullVertexData));
	//meshes.push_back(loadModel("3D/sphere.obj", cyan, fullVertexData));
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

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

    //phase 2 stuff 
    float chainLength = askUserFloat("Chain length: ");
    float particleGap = askUserFloat("Particle gap: ");
    float particleRadius = askUserFloat("Particle radius: ");
    float gravity = askUserFloat("Gravity: ");
    cout << endl << "Apply force: ";
    float forceX = askUserFloat("x: ");
    float forceY = askUserFloat("y: ");
    float forceZ = askUserFloat("z: ");
	bool appliedForce = false;

	// Load objects ------------------------------------------------
	PhysicsWorld* world = new PhysicsWorld();
	world->setGravity(vec3(0, gravity, 0));

	vector<lineDrawable*> lines;
    //glLineWidth(100.0f);

    float mass = 50;
    float restitution = 0.9;
    float chainTop = 300;
    float ballTop = 250;
    float z = 0;

	//position, scale, mass, restitution, mesh index

    // phase 2 stuff 
    for(int i = -2; i <= 2; i++)
    {
        Object* obj = new Object(vec3(i * particleGap, ballTop, z), vec3(particleRadius), mass, restitution, 0);
        world->addParticle(obj);
        AnchoredChain* chain = new AnchoredChain(vec3(i * particleGap, chainTop, z), chainLength, obj, blue);
		world->addForceGeneratorToIndex(world->particles.size() - 1, chain);
		lines.push_back(chain);
	}
	// end of object loading ---------------------------------------

    float viewVal = 400;
    mat4 proj = ortho(
        -viewVal, viewVal,
        -viewVal, viewVal,
        -100.f, 1000.f
    );
    vec3 camPos = vec3(0.f, 0.f, 50.f);
    //camPos *= -1;
    vec3 worldUp = vec3(0.f, 1.0f, 0.f);
    vec3 camCenter = vec3(0.f, 0.f, 0.f);
    mat4 view = lookAt(camPos, camCenter, worldUp);

	using clock = std::chrono::high_resolution_clock;
	auto currentTime = clock::now();
	auto prevTime = currentTime;
    std::chrono::nanoseconds currentNS(0);

    unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");
    unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");
    unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
    unsigned int objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor");

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glUniformMatrix4fv(projLoc, 1, GL_FALSE, value_ptr(proj));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, value_ptr(view));

		currentTime = clock::now();
		auto dur = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - prevTime);
		prevTime = currentTime;
		currentNS += dur;

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

        //phase 2 stuff
        if(spacePressed && !appliedForce)
        {
			world->applyForceToIndex(0, vec3(forceX, forceY, forceZ));
            appliedForce = true;
			//cout << "Force applied to first particle" << endl;
		}

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
	return 0;
}

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

#include "chrono"
using namespace std::chrono_literals;

using namespace std;
using namespace glm;

float windowWidth = 800;
float windowHeight = 800;

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

int main()
{
    if (!glfwInit())
        return -1;

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Hello World", NULL, NULL);
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


    //16ms per frame 
    constexpr std::chrono::nanoseconds timestep(16ms);

	// Load model data ------------------------------------------------
    vector<GLfloat> fullVertexData;
    vector<MeshData> meshes;

    //static colors
	vec3 white = vec3(1.f, 1.f, 1.f);
	vec3 red = vec3(1.f, 0.f, 0.f);

	meshes.push_back(loadModel("3D/sphere.obj", white, fullVertexData));
	// end of model loading -------------------------------------------

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    GLuint shaderProgram = loadShaders();

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * fullVertexData.size(), fullVertexData.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

	// Load objects ------------------------------------------------
	vector<Object> objects;
	//position, scale, mesh index
	objects.push_back(Object(vec3(0, 0, 0), vec3(1, 1, 1), 0));
	// end of object loading ---------------------------------------

	objects[0].velocity = vec3(1, 0, 0);
	//objects[0].acceleration = vec3(-1, 0, 0);

    mat4 proj = ortho(
        -2.f, 2.f,
        -2.f, 2.f,
        -100.f, 100.f
    );
    vec3 camPos = vec3(0.f, 0.f, 10.f);
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

    //hw 1 stuff
    float border = 1;
    Object* orb = &objects[0];

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
            constexpr float timestepSec = timestep.count() / (float)(1E09);
			currentNS -= timestep;

            //hw 1 stuff
            if(orb->position.x > border)
                orb->velocity.x = -1;
            else if(orb->position.x < -border)
				orb->velocity.x = 1;

            for (Object& obj : objects)
				obj.updateParticle(timestepSec);
		}

		//rendering calls
        for (Object& obj : objects)
        {
            mat4 transform = obj.GetTransform();
            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, value_ptr(transform));
            glUniform3fv(objectColorLoc, 1, value_ptr(meshes[obj.getMeshIndex()].color));

            glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, meshes[obj.getMeshIndex()].startVertex, meshes[obj.getMeshIndex()].vertexCount);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
	return 0;
}

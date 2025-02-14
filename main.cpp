//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#if defined (__APPLE__)
    #define GLFW_INCLUDE_GLCOREARB
    #define GL_SILENCE_DEPRECATION
#else
    #define GLEW_STATIC
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"

#include <iostream>

float distanta = 5.0f;

bool isFirstPerson = false;       // Indică dacă suntem în first-person
bool isTransitioning = false;     // Indică dacă se efectuează tranziția
float transitionProgress = 0.0f;  // Progresul tranziției (0.0 - third-person, 1.0 - first-person)
glm::vec3 thirdPersonPosition;    // Poziția camerei în third-person
glm::vec3 firstPersonPosition;    // Poziția camerei în first-person
float transitionSpeed = 2.0f;     // Viteza tranziției

struct BoundingBox {
	glm::vec3 min; // Coordonatele minime (xmin, ymin, zmin)
	glm::vec3 max; // Coordonatele maxime (xmax, ymax, zmax)
};

gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

int glWindowWidth = 1400;
int glWindowHeight = 800;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;
glm::vec3 lightTarget(0.0f, 0.0f, 0.0f); // Target the map's center


const unsigned int SHADOW_WIDTH = 4096;
const unsigned int SHADOW_HEIGHT = 4096;
GLuint depthMapTexture;


glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

gps::Camera myCamera(
				glm::vec3(0.0f, 3.0f, 5.5f), 
				glm::vec3(0.0f, 0.0f, 0.0f),
				glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 0.07f;

bool pressedKeys[1024];
float angleY = 0.0f;
float initial_formula1 = 15.0f;
bool formula1_front = true;
float formula1_speed = 0.02;
GLfloat lightAngle;

GLuint shadowMapFBO;
gps::Model3D nanosuit;
gps::Model3D ground;
gps::Model3D lightCube;
gps::Model3D screenQuad;
gps::Model3D statuiaLibertatii;
gps::Model3D steve;
gps::Model3D star;
gps::Model3D pamant;
gps::Model3D copaci;
gps::Model3D container;
gps::Model3D arma;
gps::Model3D bullet;
gps::Model3D formula1;

gps::Shader depthMapShader;
gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;



bool showDepthMap;

float starYPosition = 1.0f; // Poziția pe axa Y a stelei
float starSpeed = 0.02f;    // Viteza de mișcare
int starDirection = -1;

float lastX = 1024.0f / 2.0f; // Center X
float lastY = 768.0f / 2.0f;  // Center Y
bool firstMouse = true;

float sensitivity = 0.15f;

bool cinematicMode = false; 
float lastCinematicFrameTime = 0.0f;

float inainte_inapoi = 0;
float stanga_dreapta = 0;
bool firstperson = false;
float target_distance = 5.0f;
float cameraTransitionSpeed = 2.0f;


glm::vec3 bulletPosition(0.0f, -0.006f, 0.12f); // Initial position relative to the gun
float bulletSpeed = 10.0f;
float bulletspeed = 0.2f;
float bullet_pos = 1.3f;
bool pleaca_glontul = false;
bool steveExists = true;

float zi_noapte = 1.0f;
float starunghi = 0.0f;
bool rainisOn = false;



GLenum glCheckError_(const char *file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO	
}



void cinematicView(float elapsedTime) {
	static float angle = 0.0f; // Keeps track of the rotation angle
	float radius = 50.0f;
	float height = 10.0f;

	glm::vec3 center(0.0f, 0.0f, 0.0f);
	float camX = center.x + radius * cos(glm::radians(angle));
	float camZ = center.z + radius * sin(glm::radians(angle));
	glm::vec3 position(camX, height, camZ);

	myCamera.setPosition(position);
	myCamera.setTarget(center);

	// Rotate the camera smoothly
	angle += 20.0f * elapsedTime; // Increase speed based on time
	if (angle > 360.0f)
		angle -= 360.0f;

	view = myCamera.getViewMatrix();
	myCustomShader.useShaderProgram();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}


std::map<std::string, BoundingBox> boundingBoxes;
void initBoundingBoxes() {
	boundingBoxes["pamant"] = { glm::vec3(-100.0f, -3.0f, -100.0f) , glm::vec3(100.0f, -0.6f, 100.0f) }; // Exemplu pământ
	boundingBoxes["steve"] = { glm::vec3(-6.0f, -1.7f, -7.0f) , glm::vec3(-4.0f, 1.0f, -5.0f)  };    // Exemplu Steve
	boundingBoxes["star"] = { glm::vec3(-3.5f, -1.0f, -6.5f) , glm::vec3(-2.5f, 1.0f, -5.5f)  };    // Exemplu stea

	// Containere mai mari
	boundingBoxes["container1"] = { glm::vec3(-12.5f, -1.5f , -12.5f ) , glm::vec3(-5.0f , 8.0f , 1.0f )  }; // Container 1
	boundingBoxes["container2"] = { glm::vec3(-12.5f, 2.5f, -12.5f) , glm::vec3(-5.0f, 8.0f, 1.0f)  };  // Container 2 suspendat

	// Statuia Libertății cu bounding box mai mare
	boundingBoxes["statue"] = { glm::vec3(4.0f, -1.5f, -7.0f) , glm::vec3(6.0f, 2.0f, -5.0f)  }; // Box ajustat pentru Statuia Libertății
}

bool checkCollision(glm::vec3 cameraPosition, BoundingBox box) {
	return (
		cameraPosition.x >= box.min.x && cameraPosition.x <= box.max.x &&
		cameraPosition.y >= box.min.y && cameraPosition.y <= box.max.y &&
		cameraPosition.z >= box.min.z && cameraPosition.z <= box.max.z
		);
}



bool checkCollisionWithBullet(glm::vec3 bulletPosition, BoundingBox box, float margin = 1.0f) {
	return (
		bulletPosition.x >= (box.min.x - margin) && bulletPosition.x <= (box.max.x + margin) &&
		bulletPosition.y >= (box.min.y - margin) && bulletPosition.y <= (box.max.y + margin) &&
		bulletPosition.z >= (box.min.z - margin) && bulletPosition.z <= (box.max.z + margin)
		);
}


void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;

	if (key == GLFW_KEY_0 && action == GLFW_PRESS && !isFirstPerson) {
		// Inițiază tranziția către first-person
		isFirstPerson = true;
		isTransitioning = true;
		thirdPersonPosition = myCamera.getPosition();
		firstPersonPosition = thirdPersonPosition + myCamera.getFrontDirection() * 1.0f; // Ajustează distanța
	}

	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		rainisOn = !rainisOn; // Toggle rain
	}

	if (key == GLFW_KEY_9 && action == GLFW_PRESS && isFirstPerson) {
		// Inițiază tranziția către third-person
		isFirstPerson = false;
		isTransitioning = true;
	}

	if (key >= 0 && key < 1024) {
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}



void mouseCallback(GLFWwindow* window, double xpos, double ypos) {

	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // Inverted since Y-coordinates range from bottom to top
	lastX = xpos;
	lastY = ypos;

	xoffset *= sensitivity;
	yoffset *= sensitivity;

	myCamera.rotate(yoffset, xoffset);

	// Update view matrix
	view = myCamera.getViewMatrix();
	myCustomShader.useShaderProgram();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}

void updateCameraTransition(float deltaTime) {
	if (!isTransitioning) return;

	if (isFirstPerson) {
		// Tranziție către first-person
		transitionProgress += transitionSpeed * deltaTime;
		if (transitionProgress >= 1.0f) {
			transitionProgress = 1.0f;
			isTransitioning = false; // Termină tranziția
		}
	}
	else {
		// Tranziție către third-person
		transitionProgress -= transitionSpeed * deltaTime;
		if (transitionProgress <= 0.0f) {
			transitionProgress = 0.0f;
			isTransitioning = false; // Termină tranziția
		}
	}
}


void processMovement() {

	if (pressedKeys[GLFW_KEY_3]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (pressedKeys[GLFW_KEY_2]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	if (pressedKeys[GLFW_KEY_1]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}
	if (pressedKeys[GLFW_KEY_J])
	{
		lightAngle += 0.3;
	}
	if (pressedKeys[GLFW_KEY_L])
	{
		lightAngle -= 0.3;
	}
	if (pressedKeys[GLFW_KEY_SPACE])
	{
		pleaca_glontul = true;
	}
	else {
		pleaca_glontul = false;
		bullet_pos = 1.3f;
	}
	if (pressedKeys[GLFW_KEY_0]) {
		target_distance = 0.0f;
	}
	if (pressedKeys[GLFW_KEY_9]) {
		target_distance = 5.0f;
	}
	float deltaDistance = target_distance - distanta;
	if (fabs(deltaDistance) > 0.01f) { // Continuă doar dacă diferența e semnificativă
		distanta += deltaDistance * cameraTransitionSpeed * 0.016f;
	}

	// Obține poziția și direcția actuală a camerei
	glm::vec3 oldCameraPosition = myCamera.getPosition();


	if (pressedKeys[GLFW_KEY_P]) {
		// Toggle cinematic mode
		cinematicMode = true;

		// Reset cinematic time if starting
		if (cinematicMode) {
			
			lastCinematicFrameTime = glfwGetTime();
			

		}
	}
	if (pressedKeys[GLFW_KEY_O]) {
		cinematicMode = false;
	}
	// Skip other movement processing if in cinematic mode
	if (cinematicMode) {
		return;
	}

	if (pressedKeys[GLFW_KEY_Q]) {
		angleY -= 1.0f;
	}

	if (pressedKeys[GLFW_KEY_E]) {
		angleY += 1.0f;
	}

	if (pressedKeys[GLFW_KEY_C]) {
		zi_noapte = 1.0f;

	}

	if (pressedKeys[GLFW_KEY_V]) {
		zi_noapte =0.2f;

	}


	if (pressedKeys[GLFW_KEY_W] || pressedKeys[GLFW_KEY_UP]) {
		inainte_inapoi -= 0.07;
	}
	if (pressedKeys[GLFW_KEY_S] || pressedKeys[GLFW_KEY_DOWN]) {
		inainte_inapoi += 0.07;

	}
	if (pressedKeys[GLFW_KEY_A] || pressedKeys[GLFW_KEY_LEFT]) {
		stanga_dreapta -= 0.07;

	}
	if (pressedKeys[GLFW_KEY_D] || pressedKeys[GLFW_KEY_RIGHT]) {
		stanga_dreapta += 0.07;
	}

	if (pressedKeys[GLFW_KEY_W] || pressedKeys[GLFW_KEY_UP]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_S] || pressedKeys[GLFW_KEY_DOWN]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_A] || pressedKeys[GLFW_KEY_LEFT]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_D] || pressedKeys[GLFW_KEY_RIGHT]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}

		glm::vec3 newCameraPosition = myCamera.getPosition();
		bool hasCollision = false;
		for (const auto& pair : boundingBoxes) {
			if (checkCollision(newCameraPosition, pair.second)) {
				hasCollision = true;
				break;
			}
		}

		// Revert position if collision detected
		if (hasCollision) {
			myCamera.setPosition(oldCameraPosition);
		}
	

		
	

	// Update the view matrix
	view = myCamera.getViewMatrix();
	myCustomShader.useShaderProgram();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}


bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}
	

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    
    //window scaling for HiDPI displays
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    //for sRBG framebuffer
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

    //for antialising
    glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}
	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

#if not defined (__APPLE__)
    // start GLEW extension handler
    glewExperimental = GL_TRUE;
    glewInit();
#endif

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	return true;
}



void initOpenGLState()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initObjects() {
	nanosuit.LoadModel("objects/nanosuit/nanosuit.obj");
	ground.LoadModel("objects/ground/ground.obj");
	lightCube.LoadModel("objects/cube/cube.obj");
	screenQuad.LoadModel("objects/quad/quad.obj");
	statuiaLibertatii.LoadModel("objects/LibertyStatue/LibertStatue.obj");
	steve.LoadModel("objects/steve2/Minecraft_Simple_Rig.obj");
	star.LoadModel("objects/enderStar/end_star.obj");
	pamant.LoadModel("objects/pamant/10450_Rectangular_Grass_Patch_v1_iterations-2.obj");
	copaci.LoadModel("objects/copaci/trees9.obj");
	container.LoadModel("objects/container/Container.obj");
	arma.LoadModel("objects/arma/arma.obj");
	bullet.LoadModel("objects/bullet/bullet.obj");
	formula1.LoadModel("objects/formula 1/Formula 1 mesh.obj");

}

void initShaders() {
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	lightShader.useShaderProgram();
	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();
	depthMapShader.loadShader("shaders/depthMap.vert", "shaders/depthMap.frag");
	depthMapShader.useShaderProgram();
}

void initUniforms() {
	myCustomShader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	
	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 10.0f, 30.0f);
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");	
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	//set light color
	lightColor = glm::vec3(zi_noapte, zi_noapte, zi_noapte); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void initFBO() {
	//TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
	
	//generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO);
	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture,
		0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix() {

	const GLfloat near_plane = 0.1f, far_plane = 50.0f;
	glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
	glm::mat4 lightView = glm::lookAt(lightDirTr, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 lightProjection = glm::ortho(-90.0f, 90.0f, -10.0f, 10.0f, near_plane, far_plane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
	return lightSpaceTrMatrix;
}



void updateBulletState(float deltaTime) {
    if (pleaca_glontul) {

        glm::vec3 bulletDirection = glm::normalize(myCamera.getFrontDirection()); // Direcția glonțului
        bulletPosition += bulletDirection * bulletSpeed * deltaTime; // Actualizează poziția glonțului

        // Debugging: Afișează poziția actualizată a glonțului

        // Verifică coliziunea cu Steve
        if (steveExists && checkCollisionWithBullet(bulletPosition, boundingBoxes["steve"])) {
            steveExists = false; // Elimină pe Steve
        }

        // Oprește glonțul dacă iese dintr-un anumit interval
        
    }
}



void drawObjects(gps::Shader shader, bool depthPass) {
	shader.useShaderProgram();

	// Obține direcția camerei și calculează unghiul de rotație
	// Obține direcția și poziția camerei
	glm::vec3 cameraPosition = myCamera.getPosition();
	glm::vec3 cameraDirection = glm::normalize(myCamera.getFrontDirection());
	
	// Calculăm poziția nanosuit, la o distanță constantă în fața camerei
	glm::vec3 nanosuitPosition = cameraPosition + cameraDirection*distanta; // 2.0f este distanța față de cameră
	
	// Calculăm unghiul de rotație față de axa Z
	float angle = atan2(cameraDirection.x, cameraDirection.z);

	// Transformarea pentru nanosuit
	model = glm::mat4(1.0f);
	model = glm::translate(model, nanosuitPosition); // Poziționăm nanosuit în fața camerei
	model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotește nanosuit după direcția camerei
	model = glm::translate(model, glm::vec3(0.0f, -0.6f, 0.0f));
	model = glm::scale(model, glm::vec3(1.6f)); // Ajustăm dimensiunea
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	nanosuit.Draw(shader);
	float yaw = glm::atan(cameraDirection.z, cameraDirection.x);
	float pitch = glm::asin(cameraDirection.y);

	model = glm::mat4(1.0f);
	model = glm::translate(model, nanosuitPosition);
	//model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, -yaw + glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, -pitch, glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::translate(model, glm::vec3(-0.6f, -0.5f, 1.3f));
	model = glm::scale(model, glm::vec3(0.005f));
	

	


	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	arma.Draw(shader);


	model = glm::mat4(1.0f);
	model = glm::translate(model, nanosuitPosition);
	//model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, -yaw + glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, -pitch, glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::translate(model, glm::vec3(0.0f, -0.006f, 0.12f));
	model = glm::scale(model, glm::vec3(0.0008f));


	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	lightCube.Draw(shader);


	if (pleaca_glontul) {
		model = glm::mat4(1.0f);
		model = glm::translate(model, nanosuitPosition);
		//model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, -yaw + glm::radians(93.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, -pitch, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::translate(model, glm::vec3(-0.6f, -0.5f, bullet_pos));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.105f));
		bullet_pos += bulletspeed;

		if (checkCollisionWithBullet(bulletPosition, boundingBoxes["steve"])) {
			steveExists = false; // Steve is hit and no longer exists
		}

		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		// do not send the normal matrix if we are rendering in the depth map
		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}

		bullet.Draw(shader);
	}
	if (!pleaca_glontul) {
		model = glm::mat4(1.0f);
		model = glm::translate(model, nanosuitPosition);
		//model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, -yaw + glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, -pitch, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::translate(model, glm::vec3(-0.6f, -0.5f, bullet_pos));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.105f));




		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		// do not send the normal matrix if we are rendering in the depth map
		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}

		bullet.Draw(shader);
	}
	
	

	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.5f, 0.0f));
	model = glm::scale(model, glm::vec3(0.2f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	pamant.Draw(shader);
	if (steveExists) {
		model = glm::translate(glm::mat4(1.0f), glm::vec3(-5.0f, -1.7f, -6.0f));
		model = glm::scale(model, glm::vec3(1.0f));

		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		// do not send the normal matrix if we are rendering in the depth map
		if (!depthPass) {
			normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}

		steve.Draw(shader);
	}

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, starYPosition, -6.0f));
	model = glm::rotate(model, glm::radians(starunghi), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.3f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	star.Draw(shader);
	// Actualizăm poziția stelei
	starYPosition += starDirection * starSpeed;
	starunghi += 2.0f;

	// Verificăm limitele de mișcare (sus și jos)
	if (starYPosition > 1.0f) {
		starDirection = -1; // Începe să coboare
	}
	else if (starYPosition < -1.0f) {
		starDirection = 1;  // Începe să urce
	}

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, starYPosition, -6.0f));
	model = glm::rotate(model, glm::radians(starunghi), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.3f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	star.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, -1.0f, -6.0f));
	model = glm::scale(model, glm::vec3(5.0f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	statuiaLibertatii.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-9.0f, -1.0f, -6.0f));
	model = glm::scale(model, glm::vec3(0.02f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	container.Draw(shader);
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-9.0f, +3.0f, -6.0f));

	model = glm::scale(model, glm::vec3(0.02f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	container.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, -1.0f, +20.0f));
	model = glm::scale(model, glm::vec3(0.3f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	copaci.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, -1.0f, -20.0f));
	model = glm::scale(model, glm::vec3(0.3f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	copaci.Draw(shader);

	
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-20.0f, -1.0f, +3.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.3f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	copaci.Draw(shader);
	model = glm::translate(glm::mat4(1.0f), glm::vec3(+20.0f, -1.0f, +3.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.3f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	copaci.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.0f, -0.65f, initial_formula1));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.016f));
	
	
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	formula1.Draw(shader);
	if (formula1_front == true) {
		if (initial_formula1 <= -15.0f) {
			formula1_front = !formula1_front;
			formula1_speed = 0.05f;
		}
		initial_formula1 -= formula1_speed;
		if (initial_formula1 >= -4.0f)
		{
			formula1_speed += 0.001f;
		}
		else {
			formula1_speed -= 0.002f;
		}
	}
	else {
		if (initial_formula1 >= 15.0f) {
			formula1_front = !formula1_front;
			formula1_speed = 0.02f;
		}
		initial_formula1 += formula1_speed;
	}
}



void renderScene() {

	// depth maps creation pass
	//TODO - Send the light-space transformation matrix to the depth map creation shader and
	//		 render the scene in the depth map

	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	drawObjects(depthMapShader, true);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// render depth map on screen - toggled with the M key

	if (showDepthMap) {
		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT);

		screenQuadShader.useShaderProgram();

		//bind the depth map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);
	}
	else {

		// final scene rendering pass (with shadows)

		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myCustomShader.useShaderProgram();
		
		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
				
		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

		myCustomShader.useShaderProgram();

		// Pass the rain toggle to the shader
		GLuint rainisOnLoc = glGetUniformLocation(myCustomShader.shaderProgram, "rainisOn");
		glUniform1i(rainisOnLoc, rainisOn ? 1 : 0);

		// Pass time for rain animation
		float currentTime = glfwGetTime();
		GLuint timeLoc = glGetUniformLocation(myCustomShader.shaderProgram, "time");
		glUniform1f(timeLoc, currentTime);


		//bind the shadow map
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
			1,
			GL_FALSE,
			glm::value_ptr(computeLightSpaceTrMatrix()));
		lightColor = glm::vec3(zi_noapte, zi_noapte, zi_noapte); //white light
		lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
		glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

		drawObjects(myCustomShader, false);


		//draw a white cube around the light

		lightShader.useShaderProgram();

		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

		model = lightRotation;
		model = glm::translate(model, 1.0f * lightDir);
		model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		lightCube.Draw(lightShader);
	}
	// Draw the crosshair

	mySkyBox.Draw(skyboxShader, view, projection);
}

void cleanup() {
	glDeleteTextures(1,& depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

void initSkybox() {
	std::vector<const GLchar*> faces;
	faces.push_back("skybox/right.tga");
	faces.push_back("skybox/left.tga");
	faces.push_back("skybox/top.tga");
	faces.push_back("skybox/bottom.tga");
	faces.push_back("skybox/back.tga");
	faces.push_back("skybox/front.tga");

	mySkyBox.Load(faces);
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
		glm::value_ptr(view));


}

int main(int argc, const char * argv[]) {

	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}

	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initBoundingBoxes();
	glCheckError();
	initFBO();
	glCheckError();
	initSkybox();


	while (!glfwWindowShouldClose(glWindow)) {
		float currentFrameTime = glfwGetTime();
		float deltaTime = currentFrameTime - lastCinematicFrameTime;
		lastCinematicFrameTime = currentFrameTime;


		if (cinematicMode) {
			cinematicView(deltaTime);
			processMovement();
			
		}
		else {
			
			updateCameraTransition(deltaTime); // Actualizează tranziția camerei
			processMovement();   
			updateBulletState(deltaTime);



		}

		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}


	cleanup();

	return 0;
}
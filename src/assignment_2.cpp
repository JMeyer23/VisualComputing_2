#include <cstdlib>
#include <iostream>

#include "mygl/shader.h"
#include "mygl/model.h"
#include "mygl/camera.h"

#include "boat.h"
#include "water.h"


struct DayLight {
    Vector3D directLight;
    Vector3D ambientLight;
    Vector3D position;
};
struct SpotLight {
    Vector3D directLight;
    Vector3D position;
    Vector3D direction;
    float cutoffAngle = M_PI; //default to 180 degrees
};

// Saving these two is necessary for switching between day and night
DayLight LIGHT_DAY{{1, 1, 1},
                   {0.6, 0.6, 0.6},
                   {100, 300, 0}};
DayLight LIGHT_NIGHT{{0.25, 0.25, 0.3},
                     {0.1,  0.1,  0.2},
                     {-100, 300,  0}};

// These are the basic positions to which rotation and translation will be applied
// Spotlight array: 0=HeadLightLeft, 1=HeadLightRight, 2=PositionLightLeft, 3=PositionLightRight
const Vector3D SPOT_LIGHT_POSITIONS[4] = {{-1, 2, -0.2}, {1, 2, -0.2}, {-1, 2, -2}, {1, 2, -2}};
const Vector3D SPOT_LIGHT_DIRECTIONS[4] = {{-1, 0, 20},{1, 0, 20},{-20, 2, -2},{20, 2, -2}};


Vector3D BACKGROUND_COLOR = {80.0 / 255, 160.0 / 255, 240.0 / 255};


struct {
    Camera camera;
    bool cameraFollowBoat;
    float zoomSpeedMultiplier;

    Boat boat;
    Model water;

    ShaderProgram shaderBoat;
    ShaderProgram shaderWater;

    WaterSim waterSim;

    DayLight lightDayNight;
    SpotLight spotLights[4];

} sScene;

struct {
    bool mouseButtonPressed = false;
    Vector2D mousePressStart;
    bool keyPressed[Boat::eControl::CONTROL_COUNT] = {false, false, false, false};
} sInput;

void updateLights() {
    for (int i=0;i<4;i++){
        sScene.spotLights[i].position=sScene.boat.transformation * Vector4D(SPOT_LIGHT_POSITIONS[i]);
        sScene.spotLights[i].direction=sScene.boat.transformation * Vector4D(SPOT_LIGHT_DIRECTIONS[i]);
    }
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    /* input for light control */
    if (key == GLFW_KEY_8 && action == GLFW_PRESS) {
        sScene.lightDayNight = LIGHT_DAY;
    }
    if (key == GLFW_KEY_9 && action == GLFW_PRESS) {
        sScene.lightDayNight = LIGHT_NIGHT;
    }
    /* input for camera control */
    if (key == GLFW_KEY_0 && action == GLFW_PRESS) {
        sScene.cameraFollowBoat = false;
        sScene.camera.lookAt = {0.0f, 0.0f, 0.0f};
        cameraUpdateOrbit(sScene.camera, {0.0f, 0.0f}, 0.0f);
    }
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        sScene.cameraFollowBoat = false;
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        sScene.cameraFollowBoat = true;
    }

    /* input for boat control */
    if (key == GLFW_KEY_W) {
        sInput.keyPressed[Boat::eControl::THROTTLE_UP] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
    if (key == GLFW_KEY_S) {
        sInput.keyPressed[Boat::eControl::THROTTLE_DOWN] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }

    if (key == GLFW_KEY_A) {
        sInput.keyPressed[Boat::eControl::RUDDER_LEFT] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
    if (key == GLFW_KEY_D) {
        sInput.keyPressed[Boat::eControl::RUDDER_RIGHT] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }

    /* close window on escape */
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    /* make screenshot and save in work directory */
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        screenshotToPNG("screenshot.png");
    }
}

void mousePosCallback(GLFWwindow *window, double x, double y) {
    if (sInput.mouseButtonPressed) {
        Vector2D diff = sInput.mousePressStart - Vector2D(x, y);
        cameraUpdateOrbit(sScene.camera, diff, 0.0f);
        sInput.mousePressStart = Vector2D(x, y);
    }
}

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        sInput.mouseButtonPressed = (action == GLFW_PRESS);

        double x, y;
        glfwGetCursorPos(window, &x, &y);
        sInput.mousePressStart = Vector2D(x, y);
    }
}

void mouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
    cameraUpdateOrbit(sScene.camera, {0, 0}, sScene.zoomSpeedMultiplier * yoffset);
}

void windowResizeCallback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
    sScene.camera.width = width;
    sScene.camera.height = height;
}

void sceneInit(float width, float height) {
    sScene.camera = cameraCreate(width, height, to_radians(45.0), 0.01, 500.0, {10.0, 10.0, 10.0}, {0.0, 0.0, 0.0});
    sScene.cameraFollowBoat = true;
    sScene.zoomSpeedMultiplier = 0.05f;

    sScene.boat = boatLoad("../assets/boat/boat.obj");
    sScene.water = modelLoad("../assets/water/water.obj").front();

    sScene.shaderBoat = shaderLoad("shader/default.vert", "shader/color.frag");
    sScene.shaderWater = shaderLoad("shader/default.vert", "shader/color.frag");

    // Light
    sScene.lightDayNight = LIGHT_DAY;
    sScene.spotLights[0] = {{1, 1, 1}, SPOT_LIGHT_POSITIONS[0], SPOT_LIGHT_DIRECTIONS[0], 1.3};
    sScene.spotLights[1] = {{1, 1, 1}, SPOT_LIGHT_POSITIONS[1], SPOT_LIGHT_DIRECTIONS[1], 1.3};
    sScene.spotLights[2] = {{1, 0, 0}, SPOT_LIGHT_POSITIONS[2], SPOT_LIGHT_DIRECTIONS[2]};
    sScene.spotLights[3] = {{0, 1, 0}, SPOT_LIGHT_POSITIONS[3], SPOT_LIGHT_DIRECTIONS[3]};
}


void sceneUpdate(float dt) {
    sScene.waterSim.accumTime += dt;

    boatMove(sScene.boat, sScene.waterSim, sInput.keyPressed, dt);

    updateLights();

    if (!sScene.cameraFollowBoat)
        cameraFollow(sScene.camera, sScene.boat.position);
}

void render() {
    /* setup camera and model matrices */
    Matrix4D proj = cameraProjection(sScene.camera);
    Matrix4D view = cameraView(sScene.camera);
    glUseProgram(sScene.shaderBoat.id);
    shaderUniform(sScene.shaderBoat, "uProj", proj);
    shaderUniform(sScene.shaderBoat, "uView", view);
    shaderUniform(sScene.shaderBoat, "uModel", sScene.boat.transformation);

    for (unsigned int i = 0; i < sScene.boat.partModel.size(); i++) {
        auto &model = sScene.boat.partModel[i];
        glBindVertexArray(model.mesh.vao);

        shaderUniform(sScene.shaderBoat, "uModel", sScene.boat.transformation);

        for (auto &material: model.material) {
            /* set material properties */
            shaderUniform(sScene.shaderBoat, "uMaterial.ambient", material.ambient);
            shaderUniform(sScene.shaderBoat, "uMaterial.diffuse", material.diffuse);
            shaderUniform(sScene.shaderBoat, "uMaterial.specular", material.specular);
            shaderUniform(sScene.shaderBoat, "uMaterial.shininess", material.shininess);
            shaderUniform(sScene.shaderBoat, "uCamera.position", sScene.camera.position);

            shaderUniform(sScene.shaderBoat, "uLightDayNight.directLight", sScene.lightDayNight.directLight);
            shaderUniform(sScene.shaderBoat, "uLightDayNight.ambientLight", sScene.lightDayNight.ambientLight);
            shaderUniform(sScene.shaderBoat, "uLightDayNight.position", sScene.lightDayNight.position);

            for(int u=0;u<4;u++){
                shaderUniform(sScene.shaderBoat, "uSpotLights["+std::to_string(u)+"].directLight", sScene.spotLights[u].directLight);
                shaderUniform(sScene.shaderBoat, "uSpotLights["+std::to_string(u)+"].position", sScene.spotLights[u].position);
                shaderUniform(sScene.shaderBoat, "uSpotLights["+std::to_string(u)+"].direction", sScene.spotLights[u].direction);
                shaderUniform(sScene.shaderBoat, "uSpotLights["+std::to_string(u)+"].cutoffAngle", sScene.spotLights[u].cutoffAngle);
            }



            glDrawElements(GL_TRIANGLES, material.indexCount, GL_UNSIGNED_INT,
                           (const void *) (material.indexOffset * sizeof(unsigned int)));
        }
    }

    /* render water */
    {
        glUseProgram(sScene.shaderWater.id);

        /* setup camera and model matrices */
        shaderUniform(sScene.shaderWater, "uProj", proj);
        shaderUniform(sScene.shaderWater, "uView", view);
        shaderUniform(sScene.shaderWater, "uModel", Matrix4D::identity());

        /* set material properties */
        shaderUniform(sScene.shaderWater, "uMaterial.ambient", sScene.water.material.front().ambient);
        shaderUniform(sScene.shaderWater, "uMaterial.diffuse", sScene.water.material.front().diffuse);
        shaderUniform(sScene.shaderWater, "uMaterial.specular", sScene.water.material.front().specular);
        shaderUniform(sScene.shaderWater, "uMaterial.shininess", sScene.water.material.front().shininess);
        shaderUniform(sScene.shaderWater, "uCamera.position", sScene.camera.position);

        shaderUniform(sScene.shaderWater, "uLightDayNight.ambientLight", sScene.lightDayNight.ambientLight);
        shaderUniform(sScene.shaderWater, "uLightDayNight.directLight", sScene.lightDayNight.directLight);
        shaderUniform(sScene.shaderWater, "uLightDayNight.position", sScene.lightDayNight.position);

        for(int u=0;u<4;u++){
            shaderUniform(sScene.shaderWater, "uSpotLights["+std::to_string(u)+"].directLight", sScene.spotLights[u].directLight);
            shaderUniform(sScene.shaderWater, "uSpotLights["+std::to_string(u)+"].position", sScene.spotLights[u].position);
            shaderUniform(sScene.shaderWater, "uSpotLights["+std::to_string(u)+"].direction", sScene.spotLights[u].direction);
            shaderUniform(sScene.shaderWater, "uSpotLights["+std::to_string(u)+"].cutoffAngle", sScene.spotLights[u].cutoffAngle);
        }

        glBindVertexArray(sScene.water.mesh.vao);
        glDrawElements(GL_TRIANGLES, sScene.water.material.front().indexCount, GL_UNSIGNED_INT,
                       (const void *) (sScene.water.material.front().indexOffset * sizeof(unsigned int)));
    }

    /* cleanup opengl state */
    glBindVertexArray(0);
    glUseProgram(0);
}


void sceneDraw() {
    glClearColor(BACKGROUND_COLOR.x * sScene.lightDayNight.ambientLight.x,
                 BACKGROUND_COLOR.y * sScene.lightDayNight.ambientLight.y,
                 BACKGROUND_COLOR.z * sScene.lightDayNight.ambientLight.z, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /*------------ render scene -------------*/
    render();

    /* cleanup opengl state */
    glBindVertexArray(0);
    glUseProgram(0);
}

int main(int argc, char **argv) {
    /*---------- init window ------------*/
    int width = 1280;
    int height = 720;
    GLFWwindow *window = windowCreate("Assignment 2 - Shader Programming", width, height);
    if (!window) { return EXIT_FAILURE; }

    /* set window callbacks */
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mousePosCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, mouseScrollCallback);
    glfwSetFramebufferSizeCallback(window, windowResizeCallback);

    /*---------- init opengl stuff ------------*/
    glEnable(GL_DEPTH_TEST);

    /* setup scene */
    sceneInit(width, height);

    /*-------------- main loop ----------------*/
    double timeStamp = glfwGetTime();
    double timeStampNew = 0.0;
    while (!glfwWindowShouldClose(window)) {
        /* poll and process input and window events */
        glfwPollEvents();

        /* update scene */
        timeStampNew = glfwGetTime();
        sceneUpdate(timeStampNew - timeStamp);
        timeStamp = timeStampNew;

        /* draw all objects in the scene */
        sceneDraw();

        /* swap front and back buffer */
        glfwSwapBuffers(window);
    }


    /*-------- cleanup --------*/
    boatDelete(sScene.boat);
    modelDelete(sScene.water);
    shaderDelete(sScene.shaderBoat);
    shaderDelete(sScene.shaderWater);
    windowDelete(window);

    return EXIT_SUCCESS;
}

#include <cstdlib>
#include <iostream>

#include "mygl/shader.h"
#include "mygl/model.h"
#include "mygl/camera.h"

#include "boat.h"
#include "water.h"



struct Light {
    Vector3D directLight;
    Vector3D ambientLight;
    Vector3D position;
};

// Constants:
// These two are necessary for switching between day and night
Light LIGHT_DAY{{0.9, 0.9, 0.9},
                {0.7, 0.7, 0.7},
                {100, 300, 0}};
Light LIGHT_NIGHT{{0.25, 0.25, 0.3},
                  {0.1,  0.1,  0.2},
                  {-100, 300,  0}};

// These are the basic positions to which rotation and translation will be applied
Vector3D HEAD_LIGHT_LEFT_POSITION = {-1, 2, -0.2};
Vector3D HEAD_LIGHT_RIGHT_POSITION = {1, 2, -0.2};
Vector3D POSITION_LIGHT_LEFT_POSITION = {-1, 2, -2};
Vector3D POSITION_LIGHT_RIGHT_POSITION = {1, 2, -2};

// The following are only used in scene init, and would not need to be constants, but this way all light
// configuration is at one place
Light HEAD_LIGHT_LEFT = {{1,1,1},{0,0,0},HEAD_LIGHT_LEFT_POSITION};
Light HEAD_LIGHT_RIGHT = {{1,1,1},{0,0,0},HEAD_LIGHT_RIGHT_POSITION};
Light POSITION_LIGHT_LEFT = {{1,0,0},{0,0,0},POSITION_LIGHT_LEFT_POSITION};
Light POSITION_LIGHT_RIGHT = {{0,1,0},{0,0,0},POSITION_LIGHT_RIGHT_POSITION};

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

    Light lightDayNight;
    Light headLightLeft;
    Light headLightRight;
    Light positionLightLeft;
    Light positionLightRight;

} sScene;

struct {
    bool mouseButtonPressed = false;
    Vector2D mousePressStart;
    bool keyPressed[Boat::eControl::CONTROL_COUNT] = {false, false, false, false};
} sInput;


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
    sScene.headLightLeft = HEAD_LIGHT_LEFT;
    sScene.headLightRight = HEAD_LIGHT_RIGHT;
    sScene.positionLightLeft = POSITION_LIGHT_LEFT;
    sScene.positionLightRight = POSITION_LIGHT_RIGHT;
}

void sceneUpdate(float dt) {
    sScene.waterSim.accumTime += dt;

    boatMove(sScene.boat, sScene.waterSim, sInput.keyPressed, dt);

    sScene.headLightLeft.position = sScene.boat.transformation * Vector4D(HEAD_LIGHT_LEFT_POSITION);
    sScene.headLightRight.position = sScene.boat.transformation * Vector4D(HEAD_LIGHT_RIGHT_POSITION);
    sScene.positionLightLeft.position = sScene.boat.transformation * Vector4D(POSITION_LIGHT_LEFT_POSITION);
    sScene.positionLightRight.position = sScene.boat.transformation * Vector4D(POSITION_LIGHT_RIGHT_POSITION);

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

            shaderUniform(sScene.shaderBoat, "uSpotLights[0].directLight", sScene.headLightLeft.directLight);
            shaderUniform(sScene.shaderBoat, "uSpotLights[0].position", sScene.headLightLeft.position);
            shaderUniform(sScene.shaderBoat, "uSpotLights[1].directLight", sScene.headLightRight.directLight);
            shaderUniform(sScene.shaderBoat, "uSpotLights[1].position", sScene.headLightRight.position);
            shaderUniform(sScene.shaderBoat, "uSpotLights[2].directLight", sScene.positionLightLeft.directLight);
            shaderUniform(sScene.shaderBoat, "uSpotLights[2].position", sScene.positionLightLeft.position);
            shaderUniform(sScene.shaderBoat, "uSpotLights[3].directLight", sScene.positionLightRight.directLight);
            shaderUniform(sScene.shaderBoat, "uSpotLights[3].position", sScene.positionLightRight.position);


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

        shaderUniform(sScene.shaderWater, "uSpotLights[0].directLight", sScene.headLightLeft.directLight);
        shaderUniform(sScene.shaderWater, "uSpotLights[0].position", sScene.headLightLeft.position);
        shaderUniform(sScene.shaderWater, "uSpotLights[1].directLight", sScene.headLightRight.directLight);
        shaderUniform(sScene.shaderWater, "uSpotLights[1].position", sScene.headLightRight.position);
        shaderUniform(sScene.shaderWater, "uSpotLights[2].directLight", sScene.positionLightLeft.directLight);
        shaderUniform(sScene.shaderWater, "uSpotLights[2].position", sScene.positionLightLeft.position);
        shaderUniform(sScene.shaderWater, "uSpotLights[3].directLight", sScene.positionLightRight.directLight);
        shaderUniform(sScene.shaderWater, "uSpotLights[3].position", sScene.positionLightRight.position);

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

#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

out vec3 tNormal;
out vec3 tFragPos;
out vec2 tUV;

// Add parameters for the water waves
const int numWaves = 3;
float amplitudes[numWaves] = float[](0.1, 0.05, 0.2);
float wavelengths[numWaves] = float[](1.0, 0.5, 2.0);
float phases[numWaves] = float[](0.0, 1.0, 2.0);
vec2 directions[numWaves] = vec2[](vec2(1.0, 0.0), vec2(0.5, 0.5), vec2(-1.0, 0.0));

// Time variable
uniform float uTime;

void main(void)
{
    // Initialize vertex position and normal
    vec3 vertexPosition = aPosition;
    vec3 vertexNormal = aNormal;

    // Compute the displacement of the water surface over time
    for (int i = 0; i < numWaves; ++i)
    {
        float waveHeight = amplitudes[i] * sin(wavelengths[i] * dot(vertexPosition.xz, directions[i]) + phases[i] + uTime);
        vertexPosition.y += waveHeight;
    }

    // Calculate final position in clip space
    gl_Position = uProj * uView * uModel * vec4(vertexPosition, 1.0);

    // Calculate the surface normal using partial derivatives
    float dx = 0.001;
    float dy = 0.001;
    float waveHeightX = 0.0;
    float waveHeightY = 0.0;

    for (int i = 0; i < numWaves; ++i)
    {
        waveHeightX += amplitudes[i] * sin(wavelengths[i] * dot((vertexPosition.xz + vec2(dx, 0.0)), directions[i]) + phases[i] + uTime);
        waveHeightY += amplitudes[i] * sin(wavelengths[i] * dot((vertexPosition.xz + vec2(0.0, dy)), directions[i]) + phases[i] + uTime);
    }

    vec3 normalX = vec3(1.0, waveHeightX, 0.0);
    vec3 normalY = vec3(0.0, waveHeightY, 1.0);

    tNormal = normalize(cross(normalX, normalY));
    tFragPos = vec3(uModel * vec4(vertexPosition, 1.0));
    tUV = aUV;
}

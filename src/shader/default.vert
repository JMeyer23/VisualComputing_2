#version 330

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;
uniform float uTime;  // Time variable

uniform vec4 wave1Params;
uniform vec4 wave2Params;
uniform vec4 wave3Params;

in vec3 aPosition;  // Original vertex position

out vec3 tFragPos;  // Output for fragment shader
out vec3 tNormal;   // Output for fragment shader

// Function to calculate the height of the water surface at a given point
float calculateWaterHeight(vec3 position, float time, vec4 waveParams)
{
    float amplitude = waveParams[0];
    float phase = waveParams[1];
    float wavelength = waveParams[2];

    float waveHeight = amplitude * sin(wavelength * dot(position, vec3(1.0, 0.0, 1.0)) + time + phase);

    return waveHeight;
}

// Function to compute the rotation matrix based on triangle corners
mat4 calculateRotationMatrix(vec3 position, float time, vec4 waveParams)
{
    float amplitude = waveParams[0];
    float phase = waveParams[1];
    float wavelength = waveParams[2];

    // Compute the gradient of the wave function
    float epsilon = 0.1;
    float dx = calculateWaterHeight(position + vec3(epsilon, 0.0, 0.0), time, waveParams) - calculateWaterHeight(position, time, waveParams);
    float dy = calculateWaterHeight(position + vec3(0.0, 0.0, epsilon), time, waveParams) - calculateWaterHeight(position, time, waveParams);

    // Compute the normal vector
    vec3 normal = normalize(vec3(-dx, 0.0, -dy));

    // Compute the tangent vector
    vec3 tangent = normalize(vec3(1.0, 0.0, 1.0));

    // Compute the bitangent vector
    vec3 bitangent = cross(normal, tangent);

    // Compute the rotation matrix
    mat4 rotationMatrix = mat4(
    vec4(tangent, 0.0),
    vec4(normal, 0.0),
    vec4(bitangent, 0.0),
    vec4(0.0, 0.0, 0.0, 1.0)
    );

    return rotationMatrix;
}

void main()
{
    // Compute the height of the water surface at the current point
    float height1 = calculateWaterHeight(aPosition, uTime, wave1Params);
    float height2 = calculateWaterHeight(aPosition, uTime, wave2Params);
    float height3 = calculateWaterHeight(aPosition, uTime, wave3Params);

    // Sum the heights of all waves
    float totalHeight = height1 + height2 + height3;

    // Compute the rotation matrix
    mat4 rotationMatrix = calculateRotationMatrix(aPosition, uTime, wave1Params);

    // Compute the new position of the vertex with displacement
    vec3 displacedPosition = aPosition + vec3(0.0, totalHeight, 0.0);

    // Compute the new normal vector
    vec3 newNormal = normalize((rotationMatrix * vec4(0.0, 1.0, 0.0, 0.0)).xyz);

    // Compute the new position and normal vector
    tFragPos = vec3(uModel * vec4(displacedPosition, 1.0));
    tNormal = vec3(uModel * vec4(newNormal, 0.0));

    // Compute the final position of the vertex
    gl_Position = uProj * uView * uModel * vec4(displacedPosition, 1.0);
}

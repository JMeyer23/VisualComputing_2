#version 330


uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;
uniform float uTime;// Time variable

uniform vec4 wave1Params;
uniform vec4 wave2Params;
uniform vec4 wave3Params;

in vec3 aPosition;// Original vertex position

out vec3 FragPos;// Output for fragment shader -----
out vec3 Normal;// Output for fragment shader -----

// Function to calculate the height of the water surface at a given point
float calculateWaterHeight(vec3 position, float time, vec4 waveParams)
{
    float amplitude = waveParams[0];
    float phase = waveParams[1];
    float wavelength = waveParams[2];

    float waveHeight = amplitude * sin(wavelength* dot(position, vec3(1.0, 0.0, 1.0)) + time + phase);

    return waveHeight;

}


void main()
{
    vec3 displacedPosition = aPosition;

    // Calculate the height of the water surface at the current vertex position
    float waterHeight1 = calculateWaterHeight(aPosition, uTime, wave1Params);
    float waterHeight2 = calculateWaterHeight(aPosition, uTime, wave2Params);
    float waterHeight3 = calculateWaterHeight(aPosition, uTime, wave3Params);


    // Sum the heights of all waves
    float waterHeight = waterHeight1 + waterHeight2 + waterHeight3;

    // Displace the vertex position in the y-axis based on the water height
    displacedPosition.y += waterHeight;

    // Compute the surface normal
    float epsilon = 0.1;// Small value for numerical stability
    float dx = calculateWaterHeight(aPosition + vec3(epsilon, 0.0, 0.0), uTime, wave1Params) - waterHeight;
    float dy = calculateWaterHeight(aPosition + vec3(0.0, 0.0, epsilon), uTime, wave1Params) - waterHeight;
    vec3 normal = normalize(vec3(-dx, 0.0, -dy));

    FragPos = displacedPosition;
    Normal = normal;

    gl_Position = uProj * uView * uModel * vec4(displacedPosition, 1.0);
}

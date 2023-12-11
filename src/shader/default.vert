// variant 2

#version 330 core

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;
uniform float uTime;// Time variable

in vec3 aPosition;// Original vertex position

out vec3 FragPos;// Output for fragment shader -----
out vec3 Normal;// Output for fragment shader -----

// Function to calculate the height of the water surface at a given point
float calculateWaterHeight(vec3 position, float time)
{
    float sumHeight = 0.0;

    // Wave 1
    float amplitude1 = 0.6f;
    float angularFrequency1 = 0.5f;
    float phase1 = 0.25f;
    vec2 direction1 = normalize(vec2(1.0f, 1.0f));
    sumHeight += amplitude1 * sin(angularFrequency1 * dot(position.xz, direction1) + time * phase1);

    // Wave 2
    float amplitude2 = 0.7f;
    float angularFrequency2 = 0.25f;
    float phase2 = 0.1f;
    vec2 direction2 = normalize(vec2(1.0f, -1.0f));
    sumHeight += amplitude2 * sin(angularFrequency2 * dot(position.xz, direction2) + time * phase2);

    // Wave 3
    float amplitude3 = 0.1f;
    float angularFrequency3 = 0.9f;
    float phase3 = 0.9f;
    vec2 direction3 = normalize(vec2(-1.0f, 0.0f));
    sumHeight += amplitude3 * sin(angularFrequency3 * dot(position.xz, direction3) + time * phase3);

    return sumHeight;
}

void main()
{
    vec3 displacedPosition = aPosition;

    // Calculate the height of the water surface at the current vertex position
    float waterHeight = calculateWaterHeight(aPosition, uTime);

    // Displace the vertex position in the y-axis based on the water height
    displacedPosition.y += waterHeight;


    // Calculate the normal of the water surface at the current vertex position
    float heightX = calculateWaterHeight(aPosition + vec3(0.01, 0, 0), uTime);
    float heightY = calculateWaterHeight(aPosition + vec3(0, 0.01, 0), uTime);
    vec3 normal = normalize(vec3(-1, 0, -heightX) * vec3(0, -1, -heightY));

    // Pass the vertex position and normal to the Fragment Shader
    FragPos = displacedPosition;
    Normal = normal;

    // Transform the vertex position to clip coordinates
    gl_Position = uProj * uView * uModel * vec4(displacedPosition, 1.0);

}

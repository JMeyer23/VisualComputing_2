#version 330 core

struct Material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light{
    vec3 directLight;
    vec3 ambientLight;
    vec3 position;
    bool ambient; //TODO: actually use those?
    bool diffuse;
    bool specular;
};

struct Surface{
    vec3 position;
    vec3 normal;
};

struct Camera{
    vec3 position;
};

in vec3 tNormal;
in vec3 tFragPos;

out vec4 FragColor;

uniform Material uMaterial;
uniform Surface uSurface;
uniform Camera uCamera;

// Light sources
uniform Light uLightDayNight;
uniform Light uSpotLights[4];


void main(void)
{
    vec3 surfaceNormal = normalize(tNormal);


    // Sun/Moon
    vec3 ambientLight = uMaterial.diffuse * uLightDayNight.ambientLight;//TODO: use uMaterial.ambient?

    vec3 diffuseLightDayNight =
        uMaterial.diffuse
        * uLightDayNight.directLight
        * max(dot(surfaceNormal, normalize(uLightDayNight.position)),0.0);

    vec3 specularLightDayNight =
        uMaterial.specular
        * uLightDayNight.directLight
        * pow(max(dot(surfaceNormal, normalize(uCamera.position+uLightDayNight.position)),0.0),uMaterial.shininess);

    vec3 light = ambientLight + diffuseLightDayNight + specularLightDayNight;


    // Point Lights
    for (int i=0; i<4; i++){
        vec3 diffuseLight =
        uMaterial.diffuse
        * uSpotLights[i].directLight
        * max(dot(surfaceNormal, normalize(uSpotLights[i].position-tFragPos)),0.0);
        vec3 specularLight =
        uMaterial.specular
        * uSpotLights[i].directLight
        * pow(max(dot(surfaceNormal, normalize(uCamera.position+uSpotLights[0].position-tFragPos)),0.0),uMaterial.shininess);

        float distance = length(tFragPos - uSpotLights[i].position);
        float attenuation = 1.0 / (1.0 + 0.5 * distance + 0.3 * distance * distance); //TODO: adjust

        light += (diffuseLight+specularLight)*attenuation;
    }

    //TODO: correct water reflection?



    FragColor = vec4(light, 1.0);
}

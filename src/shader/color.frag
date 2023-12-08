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
    vec3 direction;
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
uniform vec3 uAmbientLightColor;
uniform Surface uSurface;
uniform Light uLightDayNight;

void main(void)
{
    vec3 ambientLight = uMaterial.diffuse * uLightDayNight.ambientLight;//TODO: use uMaterial.ambient?
    vec3 diffuseLight = uMaterial.diffuse * uLightDayNight.directLight
    * dot(normalize(tNormal), normalize(uLightDayNight.direction));
    vec3 specularLight = uMaterial.specular; //TODO: specular light
    FragColor = vec4(ambientLight+diffuseLight, 1.0);
}

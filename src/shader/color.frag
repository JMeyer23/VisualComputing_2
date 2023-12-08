#version 330 core

struct Material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light{
    vec3 color;
    vec3 direction;
//vec3 ambient color; -> set globally?
    bool ambient;
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

out vec4 FragColor;

uniform Material uMaterial;
uniform vec3 uAmbientLightColor;

void main(void)
{
    vec3 ambientLight = uMaterial.diffuse*uAmbientLightColor;
    vec3 diffuseLight = uMaterial.diffuse;
    vec3 specularLight = uMaterial.specular;
    FragColor = vec4(ambientLight, 1.0);
}
